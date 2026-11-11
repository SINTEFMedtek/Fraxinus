/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxRouteGuidanceRep.h"

#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkConeSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include "cxView.h"
#include "cxTool.h"
#include "cxTrackingService.h"
#include "cxSpaceProvider.h"
#include "cxCoordinateSystemHelpers.h"

namespace cx
{

RouteGuidanceRepPtr RouteGuidanceRep::New(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider, const QString& uid)
{
	return wrap_new(new RouteGuidanceRep(tracking, spaceProvider), uid);
}

RouteGuidanceRep::RouteGuidanceRep(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider) :
	mTracking(tracking),
	mSpaceProvider(spaceProvider)
{
	mMapper = vtkPolyDataMapperPtr::New();

	mActor = vtkActorPtr::New();
	mActor->SetMapper(mMapper);
	mActor->GetProperty()->SetColor(0.15, 0.85, 0.35);
	mActor->SetVisibility(false);

	if (mTracking)
		connect(mTracking.get(), &TrackingService::activeToolChanged, this, &RouteGuidanceRep::activeToolChangedSlot);
	this->activeToolChangedSlot();
}

RouteGuidanceRep::~RouteGuidanceRep()
{
}

void RouteGuidanceRep::setRoute(const std::vector<Vector3D>& positions, const std::vector<int>& generationNumbers, const std::vector<double>& radius)
{
	mGuidance.setRoute(positions, generationNumbers, radius);
	mShownBranchingIndex = -1;
	this->setModified();
}

void RouteGuidanceRep::addRepActorsToViewRenderer(ViewPtr view)
{
	view->getRenderer()->AddActor(mActor);
}

void RouteGuidanceRep::removeRepActorsFromViewRenderer(ViewPtr view)
{
	view->getRenderer()->RemoveActor(mActor);
}

void RouteGuidanceRep::activeToolChangedSlot()
{
	if (mTool)
		disconnect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &RouteGuidanceRep::setModified);
	mTool = mTracking ? mTracking->getActiveTool() : ToolPtr();
	if (mTool)
		connect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &RouteGuidanceRep::setModified);
	this->setModified();
}

void RouteGuidanceRep::onModifiedStartRender()
{
	this->updateArrow();
}

void RouteGuidanceRep::updateArrow()
{
	RouteGuidanceArrow arrow;
	if (mTool && mSpaceProvider)
	{
		Vector3D toolTip_r = mSpaceProvider->getActiveToolTipPoint(CoordinateSystem::reference(), true);
		arrow = mGuidance.getArrow(toolTip_r);
	}

	mActor->SetVisibility(arrow.visible);
	if (arrow.visible && arrow.branchingIndex != mShownBranchingIndex)
		this->updateArrowGeometry(arrow);
	mShownBranchingIndex = arrow.visible ? arrow.branchingIndex : -1;
}

void RouteGuidanceRep::updateArrowGeometry(const RouteGuidanceArrow& arrow)
{
	const double shaftRadius = 0.06 * arrow.size;
	const double headRadius = 0.16 * arrow.size;
	const double headLength = 0.4 * arrow.size;
	const int curveSegments = 20;

	// Arrow head along the last part of the branch to take.
	Vector3D headDirection = (arrow.tip - arrow.corner).normalized();
	double headHeight = std::min(headLength, (arrow.tip - arrow.corner).norm());
	Vector3D shaftEnd = arrow.tip - headHeight * headDirection;

	// Shaft: quadratic Bezier from the tail in the parent branch, bent by the branching point.
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
	lines->InsertNextCell(curveSegments + 1);
	for (int i = 0; i <= curveSegments; ++i)
	{
		double t = double(i) / curveSegments;
		Vector3D p = (1-t)*(1-t)*arrow.tail + 2*(1-t)*t*arrow.corner + t*t*shaftEnd;
		lines->InsertCellPoint(points->InsertNextPoint(p.data()));
	}
	vtkSmartPointer<vtkPolyData> curve = vtkSmartPointer<vtkPolyData>::New();
	curve->SetPoints(points);
	curve->SetLines(lines);

	vtkSmartPointer<vtkTubeFilter> shaft = vtkSmartPointer<vtkTubeFilter>::New();
	shaft->SetInputData(curve);
	shaft->SetRadius(shaftRadius);
	shaft->SetNumberOfSides(16);
	shaft->CappingOff(); // Rounded tail below; the other end is inside the arrow head

	// Rounded tail: a flat cap faces the camera and the scope light head-on, and is too bright up close.
	vtkSmartPointer<vtkSphereSource> tailCap = vtkSmartPointer<vtkSphereSource>::New();
	tailCap->SetCenter(arrow.tail.data());
	tailCap->SetRadius(shaftRadius);
	tailCap->SetThetaResolution(16);
	tailCap->SetPhiResolution(16);

	vtkSmartPointer<vtkConeSource> head = vtkSmartPointer<vtkConeSource>::New();
	head->SetResolution(24);
	head->SetHeight(headHeight);
	head->SetRadius(headRadius);
	head->SetDirection(headDirection.data());
	Vector3D headCenter = shaftEnd + 0.5 * headHeight * headDirection;
	head->SetCenter(headCenter.data());

	vtkSmartPointer<vtkAppendPolyData> append = vtkSmartPointer<vtkAppendPolyData>::New();
	append->AddInputConnection(shaft->GetOutputPort());
	append->AddInputConnection(head->GetOutputPort());
	append->AddInputConnection(tailCap->GetOutputPort());

	// The cone has no normals, so the appended arrow has none and would be shaded flat.
	// Smooth normals, but keep the sharp edge at the base of the arrow head.
	vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->SetInputConnection(append->GetOutputPort());
	normals->SetFeatureAngle(60);
	normals->SplittingOn();
	normals->Update();
	mMapper->SetInputData(normals->GetOutput());
}

} // namespace cx

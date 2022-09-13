/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#include "cxFraxinusVideoGeneratorWidget.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxLogger.h"
#include "cxRouteToTarget.h"
#include "cxPinpointWidget.h"
#include "cxPointMetric.h"
#include "cxVBcameraPath.h"
#include "cxRouteToTargetFilterService.h"
#include "cxDataSelectWidget.h"
#include "cxBronchoscopyRegistration.h"
#include "cxBranchList.h"
#include "cxMetricManager.h"
#include "cxUtilHelpers.h"

namespace cx {

FraxinusVideoGeneratorWidget::FraxinusVideoGeneratorWidget(VisServicesPtr services, QWidget* parent):
	VBWidget(services, parent),
	mServices(services),
	mMetricManager(new MetricManager(services->view(), services->patient(), services->tracking(), services->spaceProvider(), services->file())),
	mTargetUid("simulation_video_target"),
	mFlyThrough3DViewGroupNumber(2),
	mZoomSetting(1010)
{
	this->setObjectName(this->getWidgetName());
	this->setWindowTitle("Virtual Bronchoscopy Video Generator");
	this->setRecordVideoOption(true);

	mCenterline = StringPropertySelectMesh::New(services->patient());
	mCenterline->setValueName("Centerline tree: ");

	QGroupBox* simulateBox = new QGroupBox(tr("Simulate routes"));
	QVBoxLayout* simulateVLayout = new QVBoxLayout();
	simulateVLayout->addWidget(new DataSelectWidget(mServices->view(), mServices->patient(), this, mCenterline));
	mStartButton = new QPushButton("Simulate and record all possible routes", this);
	simulateVLayout->addWidget(mStartButton);
	simulateBox->setLayout(simulateVLayout);
	mVerticalLayout->insertWidget(mVerticalLayout->count()-1, simulateBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.

	connect(mStartButton, &QPushButton::clicked, this, &FraxinusVideoGeneratorWidget::startSimulationClickedSlot);

}

FraxinusVideoGeneratorWidget::~FraxinusVideoGeneratorWidget()
{

}

void FraxinusVideoGeneratorWidget::startSimulationClickedSlot()
{
	CoordinateSystem ref = CoordinateSystem::reference();
	Vector3D p_origin(0,0,0);
	mMetricManager->addPoint(p_origin, ref, mTargetUid, QColor(250, 0, 0, 255));

	MeshPtr centerlineMesh = mCenterline->getMesh();
	if(!centerlineMesh)
		return;

	vtkPolyDataPtr centerline_r = centerlineMesh->getTransformedPolyDataCopy(centerlineMesh->get_rMd());
	Eigen::MatrixXd centerlinePointsMatrix = makeTransformedMatrix(centerline_r);

	BranchListPtr branchList = BranchListPtr(new BranchList());
	branchList->findBranchesInCenterline(centerlinePointsMatrix);
	mEndPositions.clear();
	std::vector<BranchPtr> branches = branchList->getBranches();
	for (int i = 0; i<branches.size(); i++)
	{
		if(branches[i]->getChildBranches().empty()) //Finding the end point of branches without children
		{
			Eigen::MatrixXd positions = branches[i]->getPositions();
			mEndPositions.push_back(positions.rightCols(1));
		}
	}

	connect(this, &VBWidget::cameraAtEndPosition, this, &FraxinusVideoGeneratorWidget::navigateNextRoute);
	emit cameraAtEndPosition();
}

void FraxinusVideoGeneratorWidget::navigateNextRoute()
{
	if(mEndPositions.empty())
	{
		disconnect(this, &VBWidget::cameraAtEndPosition, this, &FraxinusVideoGeneratorWidget::navigateNextRoute);
		return;
	}

	mPlaybackSlider->setValue(mPlaybackSlider->minimum());

	Eigen::Vector3d endPositionInBranch = mEndPositions[0];
	mEndPositions.erase(mEndPositions.begin());
	DataPtr data = mServices->patient()->getData(mTargetUid);
	PointMetricPtr targetPoint = boost::dynamic_pointer_cast<PointMetric>(data);
	targetPoint->setCoordinate(endPositionInBranch);
	this->makeRoute(targetPoint, mCenterline->getMesh());

	mServices->view()->zoomCamera3D(mFlyThrough3DViewGroupNumber, mZoomSetting);

	sleep_ms(1000); // Need some time to make video recording ready for next acquisition ?
	this->playButtonClickedSlot();
}


void FraxinusVideoGeneratorWidget::makeRoute(PointMetricPtr targetPoint, MeshPtr centerline)
{
	if(!centerline || !targetPoint)
		return;

	RouteToTargetFilterPtr routeToTargetFilter = RouteToTargetFilterPtr(new RouteToTargetFilter(mServices, 0));
	std::vector<SelectDataStringPropertyBasePtr> input = routeToTargetFilter->getInputTypes();
	routeToTargetFilter->getOutputTypes();
	routeToTargetFilter->getOptions();

	routeToTargetFilter->setSmoothing(false);

	input[0]->setValue(centerline->getUid());
	input[1]->setValue(targetPoint->getUid());

	if(routeToTargetFilter->execute())
	{
		routeToTargetFilter->postProcess();
		this->setRoutePositions(routeToTargetFilter->getRoutePositions());
		this->setCameraRotationAlongRoute(routeToTargetFilter->getCameraRotation());
		this->setBranchingIndexAlongRoute(routeToTargetFilter->getBranchingIndex());
		emit cameraPathChanged(MeshPtr()); //Generating spline of new path
	}
}

QString FraxinusVideoGeneratorWidget::getWidgetName()
{
	return "fraxinus_video_generator_widget";
}

} //namespace cx

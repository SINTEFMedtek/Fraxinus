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

#include "cxFraxinusWorkflowStatesNavigation.h"
#include <QApplication>
#include <QMainWindow>
#include "cxStateService.h"
#include "cxSettings.h"
#include "cxTrackingService.h"
#include "cxTool.h"
#include "cxVideoService.h"
#include "cxPatientModelService.h"
#include "cxLogger.h"
#include "cxVisServices.h"
#include "cxClippers.h"
#include "cxInteractiveClipper.h"
#include "cxActiveData.h"
#include "cxImage.h"
#include "cxMesh.h"
#include "cxSelectDataStringPropertyBase.h"
#include "cxPointMetric.h"
#include "cxDistanceMetric.h"
#include "cxRouteToTargetFilterService.h"
#include "cxViewGroupData.h"
#include "cxProfile.h"
#include "cxDataLocations.h"
#include "cxTransferFunctions3DPresets.h"
#include "cxApplication.h"
#include "cxSyncedValue.h"
#include "cxCameraControl.h"
#include "cxFraxinusNavigationWidget.h"
#include "cxFraxinusRegistrationWidget.h"
#include "cxVBCameraZoomSetting3D.h"

namespace cx
{

TrackingWorkflowState::TrackingWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "TrackingUid", "Tracking", services, true),
    m3DViewGroupNumber(0)
{}

TrackingWorkflowState::~TrackingWorkflowState()
{}

QIcon TrackingWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/patient.svg");
}

void TrackingWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();
}

bool TrackingWorkflowState::canEnter() const
{
	return true;
}

void TrackingWorkflowState::addDataToView()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

    //Assuming 3D
    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(m3DViewGroupNumber);

    MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
    if(airwaysTubes)
        viewGroup0_3D->addData(airwaysTubes->getUid());

    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        ViewPtr view_3D = services->view()->get3DView(m3DViewGroupNumber);
        camera_control->setView(view_3D);
        camera_control->setAnteriorView();
        view_3D->setZoomFactor(0.5);
    }
    this->setDefaultCameraStyle();
}

// --------------------------------------------------------
// --------------------------------------------------------

RegistrationWorkflowState::RegistrationWorkflowState(QState* parent, VisServicesPtr services) :
    FraxinusWorkflowState(parent, "RegistrationUid", "Registration", services, false),
    m3DViewGroupNumber(0)
{
}

RegistrationWorkflowState::~RegistrationWorkflowState()
{}

void RegistrationWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();

    FraxinusRegistrationWidget* fraxinusRegistrationWidget = this->getFraxinusRegistrationWidget();
    if(fraxinusRegistrationWidget)
    {
        MeshPtr tubeCenterline = this->getTubeCenterline();
        if (tubeCenterline)
            fraxinusRegistrationWidget->setDefaultCenterlineMesh(tubeCenterline);
    }
}

void RegistrationWorkflowState::onExit(QEvent * event)
{
	ImagePtr ctImage = this->getCTImage();
	if(ctImage)
	{
		ctImage->setInitialWindowLevel(-1, -1);
	}
	WorkflowState::onExit(event);
}

QIcon RegistrationWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/import.svg");
}

FraxinusRegistrationWidget* RegistrationWorkflowState::getFraxinusRegistrationWidget()
{
    QMainWindow* mainWindow = this->getMainWindow();

    QString widgetName(FraxinusRegistrationWidget::getWidgetName());
    return mainWindow->findChild<FraxinusRegistrationWidget*>(widgetName);
}

bool RegistrationWorkflowState::canEnter() const
{
    return mServices->patient()->isPatientValid(); //TO DO: Check if tracking is enabled
}

void RegistrationWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	
    MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
    MeshPtr airwaysTubesCenterline = this->getTubeCenterline();

	//Assuming 3D ACS
    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(m3DViewGroupNumber);

    if(airwaysTubes)
    {
        QColor color = airwaysTubes->getColor();
        double opacity = 0.3;
        color.setAlphaF(opacity);
        airwaysTubes->setColor(color);
        viewGroup0_3D->addData(airwaysTubes->getUid());
    }

    if(airwaysTubesCenterline)
        viewGroup0_3D->addData(airwaysTubesCenterline->getUid());
	
	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    viewGroup1_2D->getGroup2DZoom()->set(0.2);
    viewGroup1_2D->getGlobal2DZoom()->set(0.2);

    ImagePtr ctImage = this->getCTImage();
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());

    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        ViewPtr view_3D = services->view()->get3DView(m3DViewGroupNumber);
        camera_control->setView(view_3D);
        camera_control->setAnteriorView();
        view_3D->setZoomFactor(1.5);
    }
    this->setDefaultCameraStyle();
}

// --------------------------------------------------------
// --------------------------------------------------------

NavigationWorkflowState::NavigationWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "NavigationUid", "Navigation", services, false),
    mFlyThrough3DViewGroupNumber(2),
    mSurfaceModel3DViewGroupNumber(0)
{
}

NavigationWorkflowState::~NavigationWorkflowState()
{}

QIcon NavigationWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/processing.svg");
}

FraxinusNavigationWidget* NavigationWorkflowState::getFraxinusNavigationWidget()
{
    QMainWindow* mainWindow = this->getMainWindow();

    QString widgetName(FraxinusNavigationWidget::getWidgetName());
    return mainWindow->findChild<FraxinusNavigationWidget*>(widgetName);
}

void NavigationWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
    this->setupFraxinusNavigationWidget(mFlyThrough3DViewGroupNumber, mSurfaceModel3DViewGroupNumber);
    this->addDataToView();

    FraxinusNavigationWidget* fraxinusNavigationWidget = this->getFraxinusNavigationWidget();
    if(fraxinusNavigationWidget)
    {
        fraxinusNavigationWidget->setCenterline(this->getTubeCenterline());
        fraxinusNavigationWidget->updateDataOnEntry();
        StructuresSelectionWidget* structureSelectionWidget = fraxinusNavigationWidget->getStructuresSelectionWidget();
        if(structureSelectionWidget)
            structureSelectionWidget->onEntry();
    }

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        ViewPtr viewSurface_3D = services->view()->get3DView(mSurfaceModel3DViewGroupNumber);
        camera_control->setView(viewSurface_3D);
        camera_control->setAnteriorView();
        viewSurface_3D->setZoomFactor(1.5);
    }

    QTimer::singleShot(0, this, SLOT(setAnyplaneCameraStyle()));
}

void NavigationWorkflowState::setupFraxinusNavigationWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber)
{
    std::vector<unsigned int> viewGroupNumbers;
    viewGroupNumbers.push_back(flyThrough3DViewGroupNumber);
    viewGroupNumbers.push_back(surfaceModel3DViewGroupNumber);
    FraxinusNavigationWidget* fraxinusNavigationWidget = this->getFraxinusNavigationWidget();
    if (fraxinusNavigationWidget)
    {
        this->setupViewOptionsForStructuresSelection(fraxinusNavigationWidget->getStructuresSelectionWidget(), viewGroupNumbers);

        ImagePtr ctImage_copied = this->getCTImageCopied();
        std::vector<DataPtr> volumeViewObjects;
        volumeViewObjects.push_back(ctImage_copied);

        std::vector<DataPtr> tubeViewObjects;
        MeshPtr tubes = mFraxinusSegmentations->getAirwaysTubes();
        tubeViewObjects.push_back(tubes);

        for(DataPtr object : tubeViewObjects)
            fraxinusNavigationWidget->addObjectToTubeView(object);
        for(DataPtr object : volumeViewObjects)
            fraxinusNavigationWidget->addObjectToVolumeView(object);

        fraxinusNavigationWidget->setViewGroupNumber(flyThrough3DViewGroupNumber);
    }

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    if(services)
        services->view()->zoomCamera3D(flyThrough3DViewGroupNumber, VB3DCameraZoomSetting::getZoomFactor());
}


bool NavigationWorkflowState::canEnter() const
{
	if(this->getCTImage())
		return true;
	else
		return false;
}

void NavigationWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

    ImagePtr ctImage = this->getCTImage();
    MeshPtr routeToTarget = this->getRouteToTarget();
    MeshPtr extendedRouteToTarget = this->getExtendedRouteToTarget();
    MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
    MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
    PointMetricPtr targetPoint = this->getTargetPoint();
    MeshPtr nodules = mFraxinusSegmentations->getNodules();
    //DistanceMetricPtr distanceToTargetMetric = this->getDistanceToTargetMetric();


    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(mSurfaceModel3DViewGroupNumber);
    this->setTransferfunction3D("Default", ctImage);
//    if(ctImage)
//        viewGroup0_3D->addData(ctImage->getUid());
    if(airways)
    {
        QColor c = airways->getColor();
        c.setAlphaF(0.6);
        airways->setColor(c);
        viewGroup0_3D->addData(airways->getUid());
    }
    if(targetPoint)
        viewGroup0_3D->addData(targetPoint->getUid());
    if(extendedRouteToTarget)
        viewGroup0_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
        viewGroup0_3D->addData(routeToTarget->getUid());
    //if(distanceToTargetMetric)
    //	viewGroup0_3D->addData(distanceToTargetMetric->getUid());

    ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    viewGroup1_2D->getGroup2DZoom()->set(0.4);
    viewGroup1_2D->getGlobal2DZoom()->set(0.4);

    if(ctImage)
        viewGroup1_2D->addData(ctImage->getUid());
    if(targetPoint)
        viewGroup1_2D->addData(targetPoint->getUid());

    ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(mFlyThrough3DViewGroupNumber);
    //this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage_copied);
    if(targetPoint)
        viewGroup2_3D->addData(targetPoint->getUid());
    if(airwaysTubes)
        viewGroup2_3D->addData(airwaysTubes->getUid());
    if(extendedRouteToTarget)
        viewGroup2_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
        viewGroup2_3D->addData(routeToTarget->getUid());
    if(nodules)
        viewGroup1_2D->addData(nodules->getUid());
}

void NavigationWorkflowState::onExit(QEvent * event)
{
	WorkflowState::onExit(event);
}

} //namespace cx


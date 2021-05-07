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
#include "cxFraxinusNavigationWidget.h"

namespace cx
{

TrackingWorkflowState::TrackingWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "TrackingUid", "Tracking", services, true)
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
	
	ImagePtr ctImage = this->getCTImage();
	
	//Assuming 3D
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(ctImage)
	{
		this->setTransferfunction3D("Default", ctImage);
		viewGroup0_3D->addData(ctImage->getUid());
	}
}

// --------------------------------------------------------
// --------------------------------------------------------

RegistrationWorkflowState::RegistrationWorkflowState(QState* parent, VisServicesPtr services) :
    FraxinusWorkflowState(parent, "RegistrationUid", "Registration", services, false)
{
}

RegistrationWorkflowState::~RegistrationWorkflowState()
{}

void RegistrationWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();
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

bool RegistrationWorkflowState::canEnter() const
{
    return mServices->patient()->isPatientValid(); //TO DO: Check if tracking is enabled
}

void RegistrationWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	
	ImagePtr ctImage = this->getCTImage();
	
	//Assuming 3D ACS
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(ctImage)
	{
		ctImage->setInitialWindowLevel(-1, -1);
		this->setTransferfunction3D("Default", ctImage);
		this->setTransferfunction2D("2D CT Lung", ctImage);
		viewGroup0_3D->addData(ctImage->getUid());
	}
	
	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	viewGroup1_2D->getGroup2DZoom()->set(0.1);
	viewGroup1_2D->getGlobal2DZoom()->set(0.1);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());
}

// --------------------------------------------------------
// --------------------------------------------------------

NavigationWorkflowState::NavigationWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "NavigationUid", "Navigation", services, false),
    m3DViewGroupNumber(0)
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
    this->addDataToView();

    this->setupFraxinusNavigationWidget(m3DViewGroupNumber);
    FraxinusNavigationWidget* fraxinusNavigationWidget = this->getFraxinusNavigationWidget();
    if(fraxinusNavigationWidget)
    {
        fraxinusNavigationWidget->setCenterline(this->getTubeCenterline());
        StructuresSelectionWidget* structureSelectionWidget = fraxinusNavigationWidget->getStructuresSelectionWidget();
        if(structureSelectionWidget)
            structureSelectionWidget->onEntry();
    }
}

void NavigationWorkflowState::setupFraxinusNavigationWidget(int viewGroupNumber)
{
    std::vector<unsigned int> viewGroupNumbers;
    viewGroupNumbers.push_back(viewGroupNumber);
    FraxinusNavigationWidget* fraxinusNavigationWidget = this->getFraxinusNavigationWidget();
    if (fraxinusNavigationWidget)
        this->setupViewOptionsForStructuresSelection(fraxinusNavigationWidget->getStructuresSelectionWidget(), viewGroupNumbers);
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
	MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
	ImagePtr ctImage = this->getCTImage();
	
	
	//Assuming 3D
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(airways)
		viewGroup0_3D->addData(airways->getUid());
	else if(ctImage)
	{
		ctImage->setInitialWindowLevel(-1, -1);
		this->setTransferfunction3D("Default", ctImage);
		this->setTransferfunction2D("2D CT Lung", ctImage);
		viewGroup0_3D->addData(ctImage->getUid());
	}
	
	//Assuming ACS
	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	viewGroup1_2D->getGroup2DZoom()->set(0.2);
	viewGroup1_2D->getGlobal2DZoom()->set(0.2);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());
}

void NavigationWorkflowState::onExit(QEvent * event)
{
	mFraxinusSegmentations->close();
	
	WorkflowState::onExit(event);
}

} //namespace cx


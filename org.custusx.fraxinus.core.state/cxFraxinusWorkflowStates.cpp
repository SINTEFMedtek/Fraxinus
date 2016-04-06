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

#include "cxFraxinusWorkflowStates.h"
#include "cxStateService.h"
#include "cxSettings.h"
#include "cxTrackingService.h"
#include "cxTool.h"
#include "cxVideoService.h"
#include "cxPatientModelService.h"
#include "cxLogger.h"
#include "cxVisServices.h"

namespace cx
{

ImportWorkflowState::ImportWorkflowState(QState* parent, CoreServicesPtr services) :
	WorkflowState(parent, "ImportUid", "Import", services)
{}

ImportWorkflowState::~ImportWorkflowState()
{}

QIcon ImportWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/import.png");
}

bool ImportWorkflowState::canEnter() const
{
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

ProcessWorkflowState::ProcessWorkflowState(QState* parent, CoreServicesPtr services) :
	WorkflowState(parent, "ProcessUid", "Process", services)
{
	connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

ProcessWorkflowState::~ProcessWorkflowState()
{}

QIcon ProcessWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/process.png");
}

void ProcessWorkflowState::onEntry(QEvent * event)
{
	this->autoStartHardware();
}

bool ProcessWorkflowState::canEnter() const
{
    //return mBackend->getPatientService()->isPatientValid();
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

PinpointWorkflowState::PinpointWorkflowState(QState* parent, CoreServicesPtr services) :
	WorkflowState(parent, "PinpointUid", "Pinpoint", services)
{
	connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}
PinpointWorkflowState::~PinpointWorkflowState()
{}
QIcon PinpointWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/pinpoint.png");
}

bool PinpointWorkflowState::canEnter() const
{
// We need to perform patient orientation prior to
// running and us acq. Thus we need access to the reg mode.
    //return mBackend->getPatientService()->isPatientValid();
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

RouteToTargetWorkflowState::RouteToTargetWorkflowState(QState* parent, CoreServicesPtr services) :
	WorkflowState(parent, "RouteToTargetUid", "Route To Target", services)
{
	connect(mServices->patient().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(canEnterSlot()));
}

RouteToTargetWorkflowState::~RouteToTargetWorkflowState()
{}

QIcon RouteToTargetWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/routetotarget.png");
}

bool RouteToTargetWorkflowState::canEnter() const
{
    //return !mBackend->getPatientService()->getData().empty();
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyFlyThroughWorkflowState::VirtualBronchoscopyFlyThroughWorkflowState(QState* parent, CoreServicesPtr services) :
	WorkflowState(parent, "VirtualBronchoscopyFlyThroughUid", "Virtual Bronchoscopy Fly Through", services)
{
	connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

VirtualBronchoscopyFlyThroughWorkflowState::~VirtualBronchoscopyFlyThroughWorkflowState()
{}

QIcon VirtualBronchoscopyFlyThroughWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/virtualbronchoscopy.png");
}

void VirtualBronchoscopyFlyThroughWorkflowState::onEntry(QEvent * event)
{
    //this->autoStartHardware();
}

bool VirtualBronchoscopyFlyThroughWorkflowState::canEnter() const
{
    //return mBackend->getPatientService()->isPatientValid();
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyCutPlanesWorkflowState::VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services) :
	WorkflowState(parent, "VirtualBronchoscopyCutPlanesUid", "Virtual Bronchoscopy Cut Planes", services)
{
	connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

VirtualBronchoscopyCutPlanesWorkflowState::~VirtualBronchoscopyCutPlanesWorkflowState()
{}

QIcon VirtualBronchoscopyCutPlanesWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/virtualbronchoscopy.png");
}

void VirtualBronchoscopyCutPlanesWorkflowState::onEntry(QEvent * event)
{
	//this->autoStartHardware();

}

bool VirtualBronchoscopyCutPlanesWorkflowState::canEnter() const
{
	//return mBackend->getPatientService()->isPatientValid();
	return true;
}
} //namespace cx


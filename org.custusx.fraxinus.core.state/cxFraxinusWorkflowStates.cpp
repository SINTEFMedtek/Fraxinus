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
#include "cxStateServiceBackend.h"
#include "cxPatientModelService.h"
#include "cxLogger.h"

namespace cx
{

// --------------------------------------------------------
// --------------------------------------------------------

ProcessWorkflowState::ProcessWorkflowState(QState* parent, StateServiceBackendPtr backend) :
                WorkflowState(parent, "ProcessnUid", "Process", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

void ProcessWorkflowState::onEntry(QEvent * event)
{
	this->autoStartHardware();
}

bool ProcessWorkflowState::canEnter() const
{
	return mBackend->getPatientService()->isPatientValid();
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

PinpointWorkflowState::PinpointWorkflowState(QState* parent, StateServiceBackendPtr backend) :
                WorkflowState(parent, "PinpointUid", "Pinpoint", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}
;

bool PinpointWorkflowState::canEnter() const
{
// We need to perform patient orientation prior to
// running and us acq. Thus we need access to the reg mode.
	return mBackend->getPatientService()->isPatientValid();
}
;

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

RouteToTargetWorkflowState::RouteToTargetWorkflowState(QState* parent, StateServiceBackendPtr backend) :
                WorkflowState(parent, "RouteToTargetUid", "Route To Target", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(dataAddedOrRemoved()), this, SLOT(canEnterSlot()));
}

bool RouteToTargetWorkflowState::canEnter() const
{
	return !mBackend->getPatientService()->getData().empty();
}

// --------------------------------------------------------
// --------------------------------------------------------

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyWorkflowState::VirtualBronchoscopyWorkflowState(QState* parent, StateServiceBackendPtr backend) :
                WorkflowState(parent, "VirtualBronchoscopyUid", "Virtual Bronchoscopy", backend)
{
	connect(mBackend->getPatientService().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

void VirtualBronchoscopyWorkflowState::onEntry(QEvent * event)
{
	this->autoStartHardware();
}

bool VirtualBronchoscopyWorkflowState::canEnter() const
{
	return mBackend->getPatientService()->isPatientValid();
}


}


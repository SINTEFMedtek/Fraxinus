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

#include "cxFraxinusWorkflowStateMachine.h"
#include "cxFraxinusWorkflowStates.h"
#include "cxVisServices.h"
#include "cxPatientModelService.h"
#include "cxActiveData.h"

namespace cx
{

CustusXWorkflowStateMachine::CustusXWorkflowStateMachine(VisServicesPtr services) :
	WorkflowStateMachine(services)
{
	WorkflowState* importWorkflowState = this->newState(new ImportWorkflowState(mParentState, services));
	WorkflowState* processWorkflowState = this->newState(new ProcessWorkflowState(mParentState, services));
	WorkflowState* pinpointWorkflowState = this->newState(new PinpointWorkflowState(mParentState, services));
	WorkflowState* routeToTargetWorkflowState = this->newState(new RouteToTargetWorkflowState(mParentState, services));
	WorkflowState* virtualBronchoscopyFlyThroughWorkflowState = this->newState(new VirtualBronchoscopyFlyThroughWorkflowState(mParentState, services));
	WorkflowState* virtualBronchoscopyCutPlanesWorkflowState = this->newState(new VirtualBronchoscopyCutPlanesWorkflowState(mParentState, services));

	this->newState(processWorkflowState);
	this->newState(pinpointWorkflowState);
	this->newState(routeToTargetWorkflowState);
	this->newState(virtualBronchoscopyFlyThroughWorkflowState);
	this->newState(virtualBronchoscopyCutPlanesWorkflowState);

	//set initial state on all levels
	this->setInitialState(mParentState);
    mParentState->setInitialState(importWorkflowState);

	//Create transitions
	ActiveDataPtr activeData = mServices->patient()->getActiveData();
	importWorkflowState->addTransition(activeData.get(), SIGNAL(activeImageChanged(const QString&)), processWorkflowState);
	processWorkflowState->addTransition(processWorkflowState, SIGNAL(airwaysSegmented()), pinpointWorkflowState);
}

CustusXWorkflowStateMachine::~CustusXWorkflowStateMachine()
{}

} //namespace cx

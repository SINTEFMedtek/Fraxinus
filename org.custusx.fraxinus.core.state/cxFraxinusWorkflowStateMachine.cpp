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
#include "cxLogger.h"
#include "cxImage.h"

namespace cx
{

FraxinusWorkflowStateMachine::FraxinusWorkflowStateMachine(VisServicesPtr services) :
	WorkflowStateMachine(services)
{
	mPatientWorkflowState = dynamic_cast<FraxinusWorkflowState*>(this->newState(new PatientWorkflowState(mParentState, services)));
    mImportWorkflowState = this->newState(new ImportWorkflowState(mParentState, services));
    mProcessWorkflowState = this->newState(new ProcessWorkflowState(mParentState, services));
    mPinpointWorkflowState = this->newState(new PinpointWorkflowState(mParentState, services));
    mVirtualBronchoscopyFlyThroughWorkflowState = this->newState(new VirtualBronchoscopyFlyThroughWorkflowState(mParentState, services));
    mVirtualBronchoscopyCutPlanesWorkflowState = this->newState(new VirtualBronchoscopyCutPlanesWorkflowState(mParentState, services));

    //logic for enabling workflowsteps
    connect(mServices->patient().get(), &PatientModelService::patientChanged, mImportWorkflowState, &ImportWorkflowState::canEnterSlot);
    connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, mProcessWorkflowState, &ProcessWorkflowState::canEnterSlot);
    connect(mProcessWorkflowState, SIGNAL(airwaysSegmented()), mPinpointWorkflowState, SLOT(canEnterSlot()));
    connect(mPinpointWorkflowState, SIGNAL(routeToTargetCreated()), mVirtualBronchoscopyFlyThroughWorkflowState, SLOT(canEnterSlot()));
    connect(mPinpointWorkflowState, SIGNAL(routeToTargetCreated()), mVirtualBronchoscopyCutPlanesWorkflowState, SLOT(canEnterSlot()));

	//set initial state on all levels
    this->setInitialState(mParentState);
    mParentState->setInitialState(mPatientWorkflowState);

	//Create transitions
	mPatientWorkflowState->addTransition(mServices->patient().get(), SIGNAL(patientChanged()), mImportWorkflowState);
	mImportWorkflowState->addTransition(this, SIGNAL(dataAdded()), mProcessWorkflowState);
	mProcessWorkflowState->addTransition(mProcessWorkflowState, SIGNAL(airwaysSegmented()), mPinpointWorkflowState);
	mPinpointWorkflowState->addTransition(mPinpointWorkflowState, SIGNAL(routeToTargetCreated()), mVirtualBronchoscopyFlyThroughWorkflowState);

    connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &FraxinusWorkflowStateMachine::dataAddedOrRemovedSlot);
}

FraxinusWorkflowStateMachine::~FraxinusWorkflowStateMachine()
{}

void FraxinusWorkflowStateMachine::dataAddedOrRemovedSlot()
{
	if(mPatientWorkflowState->getCTImage())
		emit dataAdded();
}

} //namespace cx

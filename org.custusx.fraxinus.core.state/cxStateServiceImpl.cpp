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

#include "cxStateServiceImpl.h"

#include <iostream>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include "cxDefinitions.h"
#include "cxEnumConverter.h"
#include "cxXmlOptionItem.h"

#include "cxSettings.h"
#include "cxWorkflowStateMachine.h"
#include "cxFraxinusWorkflowStateMachine.h"
#include "cxDataLocations.h"
#include "cxConfig.h"
#include "cxVLCRecorder.h"

#include "cxTrackingServiceProxy.h"
#include "cxPatientModelServiceProxy.h"
#include "cxSpaceProviderImpl.h"
#include "cxVideoServiceProxy.h"
#include "cxApplicationsParser.h"
#include "cxProfile.h"
#include "cxLogger.h"
#include "cxVisServices.h"

namespace cx
{

StateServiceImpl::StateServiceImpl(ctkPluginContext* context)
{
	mServices = VisServices::create(context);
	this->initialize();
}

StateServiceImpl::~StateServiceImpl()
{
}

bool StateServiceImpl::isNull()
{
	return false;
}

void StateServiceImpl::initialize()
{
	this->changeDefaultSettings();
	this->fillDefaultSettings();

	ProfileManager::initialize();

	mWorkflowStateMachine.reset(new FraxinusWorkflowStateMachine(mServices));
	mWorkflowStateMachine->start();

	connect(mWorkflowStateMachine.get(), &WorkflowStateMachine::activeStateChanged, this, &StateServiceImpl::workflowStateChanged);
	connect(mWorkflowStateMachine.get(), &WorkflowStateMachine::activeStateAboutToChange, this, &StateServiceImpl::workflowStateAboutToChange);

	connect(ProfileManager::getInstance(), &ProfileManager::activeProfileChanged, this, &StateServiceImpl::applicationStateChanged);
}

QString StateServiceImpl::getApplicationStateName() const
{
	return ProfileManager::getInstance()->activeProfile()->getUid();
}

QStringList StateServiceImpl::getAllApplicationStateNames() const
{
	return ProfileManager::getInstance()->getProfiles();
}

QString StateServiceImpl::getVersionName()
{
	return QString("%1").arg(CustusX_VERSION_STRING);
}

QActionGroup* StateServiceImpl::getWorkflowActions()
{
	return mWorkflowStateMachine->getActionGroup();
}

WorkflowStateMachinePtr StateServiceImpl::getWorkflow()
{
	return mWorkflowStateMachine;
}

void StateServiceImpl::setWorkFlowState(QString uid)
{
	mWorkflowStateMachine->setActiveState(uid);
}

/**Enter all default Settings here.
 *
 */
void StateServiceImpl::changeDefaultSettings()
{
	this->fillDefault("Automation/autoStartTracking", false);
	this->fillDefault("Automation/autoStartStreaming", false);
	this->fillDefault("Automation/autoReconstruct", false);
	this->fillDefault("Automation/autoSelectActiveTool", true);
	this->fillDefault("Automation/autoSave", true);
	this->fillDefault("Automation/autoLoadRecentPatient", false);
	this->fillDefault("Automation/autoLoadRecentPatientWithinHours", 8);
	this->fillDefault("Automation/autoShowNewData", true);
	this->fillDefault("Automation/autoShowNewDataInViewGroup0", true);
	this->fillDefault("Automation/autoShowNewDataInViewGroup1", true);
	this->fillDefault("Automation/autoShowNewDataInViewGroup3", false);
	this->fillDefault("Automation/autoShowNewDataInViewGroup2", false);
	this->fillDefault("Automation/autoShowNewDataInViewGroup4", false);
	this->fillDefault("Automation/autoResetCameraToSuperiorViewWhenAutoShowingNewData", true);
	this->fillDefault("Automation/autoCenterToImageCenterViewWhenAutoShowingNewData", true);

	this->fillDefault("backgroundColor", QColor("black"));
	this->fillDefault("TrackingPositionFilter/enabled", false);
	this->fillDefault("Gui/showMenuBar", false);
	this->fillDefault("View/clinicalView", "Radiological");
	this->fillDefault("View/shadingOn", false);
	this->fillDefault("View/showMetricNamesInCorner", true);
    this->fillDefault("View3D/showManualTool", true);
	this->fillDefault("View3D/sphereRadius", 1.3);
}


Desktop StateServiceImpl::getActiveDesktop()
{
	ApplicationsParser parser;
	return parser.getDesktop(mWorkflowStateMachine->getActiveUidState());
}

void StateServiceImpl::saveDesktop(Desktop desktop)
{
	ApplicationsParser parser;
	parser.setDesktop(mWorkflowStateMachine->getActiveUidState(),
					  desktop);
}

void StateServiceImpl::resetDesktop()
{
	ApplicationsParser parser;
	parser.resetDesktop(mWorkflowStateMachine->getActiveUidState());
}

} //namespace cx

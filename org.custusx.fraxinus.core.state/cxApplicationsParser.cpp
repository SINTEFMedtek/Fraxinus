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

#include "cxApplicationsParser.h"

#include <iostream>
#include <QApplication>
#include <QByteArray>
#include <QDir>
#include "cxEnumConverter.h"
#include "cxXmlOptionItem.h"

#include "cxSettings.h"
#include "cxWorkflowStateMachine.h"
#include "cxDataLocations.h"
#include "cxConfig.h"

#include "cxTrackingServiceProxy.h"
#include "cxPatientModelServiceProxy.h"
#include "cxSpaceProviderImpl.h"
#include "cxVideoServiceProxy.h"
#include "cxProfile.h"

#include "cxFraxinusVBWidget.h"
#include "cxProcedurePlanningWidget.h"
#include "cxFraxinusTrackingWidget.h"
#include "cxFraxinusRegistrationWidget.h"
#include "cxFraxinusNavigationWidget.h"
#include "cxFraxinusSimulatorWidget.h"
#include "cxLogger.h"


namespace cx
{

ApplicationsParser::ApplicationsParser()
{
	Desktop desktop;

	QStringList standardToolbars;
	standardToolbars << "Workflow" << "Help";

	//-----------------------------------------------------
	// NEW/LOAD PATIENT
	desktop = Desktop("LAYOUT_3D", QByteArray::fromBase64(""));
	QStringList toolbars;
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("new_load_patient_widget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("help_widget", Qt::RightDockWidgetArea, false);
	mWorkflowDefaultDesktops["FraxinusPatientUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// IMPORT
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("import_widget", Qt::LeftDockWidgetArea, true);
	//desktop.addPreset("dicom_widget", Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusImportUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// PROCESS
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	//desktop.addPreset("Airway Segmentation Filter Widget", Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusProcessUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// PINPOINT
	desktop = Desktop("LAYOUT_ACS3D", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("pinpoint_widget", Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusPinpointUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// VIRTUAL BRONCHOSCOPY FLY-THROUGH
	desktop = Desktop("LAYOUT_VB_FLY_THROUGH", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusVBWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["VirtualBronchoscopyFlyThroughUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// VIRTUAL BRONCHOSCOPY CUT-PLANES
	desktop = Desktop("LAYOUT_VB_CUT_PLANES", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusVBWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["VirtualBronchoscopyCutPlanesUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// VIRTUAL BRONCHOSCOPY ANYPLANE
	desktop = Desktop("LAYOUT_VB_3D_ANY", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusVBWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["VirtualBronchoscopyAnyplaneUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// VIRTUAL BRONCHOSCOPY PROCEDURE PLANNING
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(ProcedurePlanningWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["ProcedurePlanningUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// TRACKING
	desktop = Desktop("LAYOUT_3D", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusTrackingWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusTrackingUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// REGISTRATION
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusRegistrationWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusRegistrationUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// NAVIGATION
	desktop = Desktop("LAYOUT_VB_3D_ANY", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusNavigationWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusNavigationUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	// SIMULATOR
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars;
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset(FraxinusSimulatorWidget::getWidgetName(), Qt::LeftDockWidgetArea, true);
	mWorkflowDefaultDesktops["FraxinusSimulatorUid"] = desktop;
	//-----------------------------------------------------
}

void ApplicationsParser::addToolbarsToDesktop(Desktop& desktop, QStringList toolbars)
{
	for (int i=0; i<toolbars.size(); ++i)
	{
		desktop.addPreset(toolbars[i]+"ToolBar", Qt::TopToolBarArea);
	}
}

void ApplicationsParser::addDefaultDesktops(QString workflowStateUid, QString layoutUid, QString mainwindowstate)
{
	mWorkflowDefaultDesktops[workflowStateUid] = Desktop(layoutUid,
														 QByteArray::fromBase64(mainwindowstate.toLatin1()));
}

Desktop ApplicationsParser::getDefaultDesktop(QString workflowName)
{
	//TODO use applicationName!!!
	if (!mWorkflowDefaultDesktops.count(workflowName))
	{
		CX_LOG_WARNING() << "Cannot find workflow: " << workflowName;
		return mWorkflowDefaultDesktops["DEFAULT"];
	}
	return mWorkflowDefaultDesktops[workflowName];
}

Desktop ApplicationsParser::getDesktop(QString workflowName)
{
	return this->getDefaultDesktop(workflowName);
}

void ApplicationsParser::setDesktop(QString workflowName, Desktop desktop)
{
	XmlOptionFile file = this->getSettings();
	QDomElement desktopElement = file.descend(workflowName).descend("custom").getElement();
//	QDomElement desktopElement = mXmlFile.descend(applicationName).descend("workflows").descend(workflowName).descend("custom").getElement();
	desktopElement.setAttribute("mainwindowstate", QString(desktop.mMainWindowState.toBase64()));
	desktopElement.setAttribute("layoutuid", desktop.mLayoutUid);
	desktopElement.setAttribute("secondarylayoutuid", desktop.mSecondaryLayoutUid);
	file.save();
}

XmlOptionFile ApplicationsParser::getSettings()
{
	XmlOptionFile retval = ProfileManager::getInstance()->activeProfile()->getXmlSettings();
	retval = retval.descend("workflows");
	return retval;
}

void ApplicationsParser::resetDesktop(QString workflowName)
{
	XmlOptionFile file = this->getSettings();

	QDomElement workflowElement = file.descend(workflowName).getElement();
	workflowElement.removeChild(workflowElement.namedItem("custom"));
	file.save();
}


} //namespace cx

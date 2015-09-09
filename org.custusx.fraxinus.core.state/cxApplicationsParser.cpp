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
#include "cxDataLocations.h"
#include "cxWorkflowStateMachine.h"
#include "cxDataLocations.h"
#include "cxConfig.h"
#include "cxStateServiceBackend.h"

#include "cxTrackingServiceProxy.h"
#include "cxPatientModelServiceProxy.h"
#include "cxSpaceProviderImpl.h"
#include "cxVideoServiceProxy.h"
#include "cxProfile.h"

namespace cx
{

ApplicationsParser::ApplicationsParser()
{
//	mXmlFile = profile()->getXmlSettings().descend("applications");

	QString fullState =
			"AAAA/wAAAAD9AAAAAgAAAAAAAAGEAAADlvwCAAAACvsAAAAiAEMAbwBuAHQAZQB4AHQARABvAGMAawBXAGkAZABnAGUAdAEAAAMgAAAAWgAAAAAAAAAA+wAAADIASQBtAGEAZwBlAFAAcgBvAHAAZQByAHQAaQBlAHMARABvAGMAawBXAGkAZABnAGUAdAEAAAM0AAAAuwAAAAAAAAAA/AAAAEEAAAI7AAACAwEAAB36AAAAAAEAAAAV+wAAAEAAVgBvAGwAdQBtAGUAUAByAG8AcABlAHIAdABpAGUAcwBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AQAAAAD/////AAABGQD////7AAAAPABNAGUAcwBoAFAAcgBvAHAAZQByAHQAaQBlAHMAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAEAAAAA/////wAAAP4A////+wAAADoAUABvAGkAbgB0AFMAYQBtAHAAbABpAG4AZwBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAD/////AAAAAAAAAAD7AAAAOgBDAGEAbQBlAHIAYQBDAG8AbgB0AHIAbwBsAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAAAAAAAAAPsAAAAuAEkARwBUAEwAaQBuAGsAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAUEA////+wAAADgAVQBTAEEAYwBxAHUAcwBpAHQAaQBvAG4AVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAANQA////+wAAAEIAVAByAGEAYwBrAGUAZABDAGUAbgB0AGUAcgBsAGkAbgBlAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAADDAP////sAAAA0AE4AYQB2AGkAZwBhAHQAaQBvAG4AVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAMEA////+wAAADIARgByAGEAbQBlAFQAcgBlAGUAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAGYA////+wAAADwAVABvAG8AbABQAHIAbwBwAGUAcgB0AGkAZQBzAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQBAAAAAP////8AAAGEAP////sAAABGAFIAZQBnAGkAcwB0AHIAYQB0AGkAbwBuAEgAaQBzAHQAbwByAHkAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAQIA////+wAAAEQAQwBhAGwAaQBiAHIAYQB0AGkAbwBuAE0AZQB0AGgAbwBkAHMAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAZMA////+wAAAEgAVgBpAHMAdQBhAGwAaQB6AGEAdABpAG8AbgBNAGUAdABoAG8AZABzAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAAFMAP////sAAABGAFMAZQBnAG0AZQBuAHQAYQB0AGkAbwBuAE0AZQB0AGgAbwBkAHMAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAPEA////+wAAAEYAUgBlAGcAaQBzAHQAcgBhAHQAaQBvAG4ATQBlAHQAaABvAGQAcwBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAD/////AAABYgD////7AAAAPgBJAG0AYQBnAGUAUAByAG8AcABlAHIAdABpAGUAcwBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AQAAAAD/////AAAA8AD////7AAAAFABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAAAAAGBAAABSgD////7AAAALABNAGUAdAByAGkAYwBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAD/////AAAAZgD////7AAAALABFAHIAYQBzAGUAcgBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAD/////AAAA2wD////7AAAAMABUAHIAYQBjAGsAUABhAGQAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAHQA////+wAAADYAVwBpAHIAZQBQAGgAYQBuAHQAbwBtAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAADyAP////wAAAKCAAABVQAAAL4BAAAd+gAAAAEBAAAAAvsAAAAuAEMAbwBuAHMAbwBsAGUAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAEAAAAA/////wAAAFQA////+wAAAC4AQgByAG8AdwBzAGUAcgBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AQAAAAAAAAFvAAAAiAD////8AAADKAAAAMsAAAAAAP////oAAAAAAQAAAAH7AAAAQABUAHIAYQBuAHMAZgBlAHIARgB1AG4AYwB0AGkAbwBuAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQBAAAAAP////8AAAAAAAAAAPsAAAA+AFMAaABpAGYAdABDAG8AcgByAGUAYwB0AGkAbwBuAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAAAAAAAAAPsAAABCAEkAbQBhAGcAZQBSAGUAZwBpAHMAdAByAGEAdABpAG8AbgBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAACwAAAS0AAAAAAAAAAD7AAAARgBQAGEAdABpAGUAbgB0AFIAZQBnAGkAcwB0AHIAYQB0AGkAbwBuAFcAaQBkAGcAZQB0AEQAbwBjAGsAVwBpAGQAZwBlAHQAAAAAAP////8AAAAAAAAAAPwAAAJdAAABQAAAAAAA////+gAAAAABAAAAAfsAAAAUAEQAbwBjAGsAVwBpAGQAZwBlAHQBAAAAAP////8AAAAAAAAAAPsAAAA2AFQAbwBvAGwATQBhAG4AYQBnAGUAcgBXAGkAZABnAGUAdABEAG8AYwBrAFcAaQBkAGcAZQB0AAAAAAD/////AAAAiAD///8AAAADAAAAAAAAAAD8AQAAAAH7AAAAMABQAGwAYQB5AGIAYQBjAGsAVwBpAGQAZwBlAHQARABvAGMAawBXAGkAZABnAGUAdAAAAAAA/////wAAAVMA////AAAF9gAAA5YAAAAEAAAABAAAAAgAAAAI/AAAAAQAAAAAAAAAAAAAAAAAAAAAAAAAAQAAAAAAAAACAAAACQAAAB4AVwBvAHIAawBmAGwAbwB3AFQAbwBvAGwAQgBhAHIBAAAAAP////8AAAAAAAAAAAAAABYARABhAHQAYQBUAG8AbwBsAEIAYQByAQAAAOX/////AAAAAAAAAAAAAAAiAE4AYQB2AGkAZwBhAHQAaQBvAG4AVABvAG8AbABCAGEAcgEAAAGC/////wAAAAAAAAAAAAAAJgBDAGEAbQBlAHIAYQAzAEQAVgBpAGUAdwBUAG8AbwBsAEIAYQByAQAAAfv/////AAAAAAAAAAAAAAAWAFQAbwBvAGwAVABvAG8AbABCAGEAcgEAAAME/////wAAAAAAAAAAAAAAIgBTAGMAcgBlAGUAbgBzAGgAbwB0AFQAbwBvAGwAQgBhAHIBAAADWf////8AAAAAAAAAAAAAABwARABlAHMAawB0AG8AcABUAG8AbwBsAEIAYQByAQAAA4r/////AAAAAAAAAAAAAAAsAEkAbgB0AGUAcgBhAGMAdABvAHIAUwB0AHkAbABlAFQAbwBvAGwAQgBhAHIAAAAD3/////8AAAAAAAAAAAAAABYASABlAGwAcABUAG8AbwBsAEIAYQByAQAAA98AAAF4AAAAAAAAAAA=";

//	this->addDefaultDesktops(
//				"PatientDataUid",
//				"LAYOUT_3D_ACS",
//				"");

	Desktop desktop;

	QStringList standardToolbars;
	standardToolbars << "Workflow" << "Navigation" << "Tools" << "Screenshot" << "Desktop";


	//-----------------------------------------------------
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	QStringList toolbars;
	toolbars << standardToolbars << "Data" << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("MeshInfoWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("VolumePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("SlicePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("DicomWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("ConsoleWidget", Qt::LeftDockWidgetArea, false);
	desktop.addPreset("HelpWidget", Qt::RightDockWidgetArea, false);
    mWorkflowDefaultDesktops["ImportUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars << "RegistrationHistory" << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("ConsoleWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("org_custusx_registration_gui_widget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("RegistrationHistoryWidget", Qt::LeftDockWidgetArea, false);
    mWorkflowDefaultDesktops["ProcessUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars << "Camera3DViews" << "Sampler" << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("MeshInfoWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("VolumePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("SlicePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("ConsoleWidget", Qt::LeftDockWidgetArea, false);
    mWorkflowDefaultDesktops["PinpointUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars << "InteractorStyle" << "ToolOffset" << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
    mWorkflowDefaultDesktops["RouteToTargetUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
	desktop = Desktop("LAYOUT_US_Acquisition", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("IGTLinkWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("USReconstruction", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("org_custusx_acquisition_widgets_acquisition", Qt::LeftDockWidgetArea, true);
    mWorkflowDefaultDesktops["VirtualBronchoscopyUid"] = desktop;
	//-----------------------------------------------------

	//-----------------------------------------------------
    /*
	desktop = Desktop("LAYOUT_3D_ACS", QByteArray::fromBase64(""));
	toolbars.clear();
	toolbars << standardToolbars << "Sampler" << "Help";
	this->addToolbarsToDesktop(desktop, toolbars);
	desktop.addPreset("MeshInfoWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("VolumePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("SlicePropertiesWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("MetricWidget", Qt::LeftDockWidgetArea, true);
	desktop.addPreset("ConsoleWidget", Qt::LeftDockWidgetArea, false);
	desktop.addPreset("PlaybackWidget", Qt::BottomDockWidgetArea, false);
	mWorkflowDefaultDesktops["PostOpControllUid"] = desktop;
    */
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
		return mWorkflowDefaultDesktops["DEFAULT"];
	return mWorkflowDefaultDesktops[workflowName];
}

Desktop ApplicationsParser::getDesktop(QString workflowName)
{
	Desktop retval;
	XmlOptionFile file = this->getSettings();
	QDomElement workflowElement = file.descend(workflowName).getElement();
	QDomElement desktopElement;
	if (workflowElement.namedItem("custom").isNull())
	{
		return this->getDefaultDesktop(workflowName);
	}
	else
	{
		desktopElement = workflowElement.namedItem("custom").toElement();
	}
	retval.mMainWindowState = QByteArray::fromBase64(desktopElement.attribute("mainwindowstate").toLatin1());
	retval.mLayoutUid = desktopElement.attribute("layoutuid");
	retval.mSecondaryLayoutUid = desktopElement.attribute("secondarylayoutuid");

	return retval;
}

void ApplicationsParser::setDesktop(QString workflowName, Desktop desktop)
{
	XmlOptionFile file = this->getSettings();
	QDomElement desktopElement = file.descend(workflowName).descend("custom").getElement();
//	QDomElement desktopElement =
//			mXmlFile.descend(applicationName).descend("workflows").descend(workflowName).descend("custom").getElement();
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

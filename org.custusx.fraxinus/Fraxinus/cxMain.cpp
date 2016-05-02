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

#include <iostream>
//#include "cxFraxinusMainWindow.h"
#include "cxMainWindow.h"
#include "cxMainWindowApplicationComponent.h"
#include "cxLogicManager.h"
#include "cxApplication.h"
#include "cxViewService.h"
#include "cxLayoutData.h"
#include "cxView.h"
#include "cxDataLocations.h"
#include "cxProfile.h"


namespace cx
{
void addAdditionalDefaultLayouts()
{

	ViewServicePtr viewService = logicManager()->getViewService();
	{
		LayoutData layout = LayoutData::create("LAYOUT_VB_FLY_THROUGH", "VB FLY-THROUGH", 3, 5);
		layout.setView(2, View::VIEW_3D, LayoutRegion(0, 0, 3, 3));
		layout.setView(0, View::VIEW_3D, LayoutRegion(0, 3, 1, 2));
		layout.setView(1, ptAXIAL, LayoutRegion(1, 3, 1, 2));
		layout.setView(1, ptCORONAL, LayoutRegion(2, 3, 1, 2));
		viewService->addDefaultLayout(layout);
	}
	{
		LayoutData layout = LayoutData::create("LAYOUT_VB_CUT_PLANES", "VB CUT-PLANES", 3, 5);
		layout.setView(0, View::VIEW_3D, LayoutRegion(0, 0, 3, 3));
		layout.setView(2, View::VIEW_3D, LayoutRegion(0, 3, 1, 2));
		layout.setView(1, ptAXIAL, LayoutRegion(1, 3, 1, 2));
		layout.setView(1, ptCORONAL, LayoutRegion(2, 3, 1, 2));
		viewService->addDefaultLayout(layout);
	}
}
} //cx

int main(int argc, char *argv[])
{

#if !defined(WIN32)
  //for some reason this does not work with dynamic linking on windows
  //instead we solve the problem by adding a handmade header for the cxResources.qrc file
  Q_INIT_RESOURCE(cxResources);
#endif

  cx::Application app(argc, argv);
  app.setOrganizationName("Fraxinus");
  app.setOrganizationDomain("www.sintef.no");
  app.setApplicationName(CX_SYSTEM_BASE_NAME);
  app.setWindowIcon(QIcon(":/icons/icons/Fraxinus2.png"));
  app.setAttribute(Qt::AA_DontShowIconsInMenus, false);

  cx::DataLocations::setWebsiteURL("http://www.custusx.org/fraxinus");
  cx::ApplicationComponentPtr mainwindow(new cx::MainWindowApplicationComponent<cx::MainWindow>());

  cx::ProfileManager::getInstance("Bronchoscopy");
  cx::LogicManager::initialize(mainwindow);

  cx::addAdditionalDefaultLayouts();

  int retVal = app.exec();

  cx::LogicManager::shutdown(); // shutdown all global resources, _after_ gui is deleted.

  return retVal;
}

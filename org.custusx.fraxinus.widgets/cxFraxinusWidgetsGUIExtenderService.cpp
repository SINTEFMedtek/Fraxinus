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

#include "cxFraxinusWidgetsGUIExtenderService.h"
#include "ctkPluginContext.h"
#include "cxFiltersWidget.h"
#include "cxRegServices.h"
#include "cxPinpointWidget.h"
#include "cxNewLoadPatientWidget.h"
#include "cxFraxinusVBWidget.h"
#include "cxProcedurePlanningWidget.h"

namespace cx
{

FraxinusWidgetsGUIExtenderService::FraxinusWidgetsGUIExtenderService(ctkPluginContext *context) :
  mContext(context)
{
}

std::vector<GUIExtenderService::CategorizedWidget> FraxinusWidgetsGUIExtenderService::createWidgets() const
{
	RegServicesPtr services = RegServices::create(mContext);

	std::vector<CategorizedWidget> retval;
	retval.push_back(GUIExtenderService::CategorizedWidget(new FiltersWidget(services, NULL, QStringList("Route to target"), "fraxinus_routetotargetwidget"),"Fraxinus"));
	//retval.push_back(GUIExtenderService::CategorizedWidget(new FiltersWidget(services, NULL, QStringList("Airway Segmentation Filter"), "fraxinus_airwaysegmentationwidget"),"Fraxinus"));
	retval.push_back(GUIExtenderService::CategorizedWidget(new PinpointWidget(services, NULL),"Fraxinus"));
	retval.push_back(GUIExtenderService::CategorizedWidget(new NewLoadPatientWidget(NULL, services->patient()),"Fraxinus"));
	retval.push_back(GUIExtenderService::CategorizedWidget(new FraxinusVBWidget(services, NULL),"Fraxinus"));
	retval.push_back(GUIExtenderService::CategorizedWidget(new ProcedurePlanningWidget(services, NULL),"Fraxinus"));

	return retval;
}


} /* namespace cx */

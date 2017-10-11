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

#include "cxFraxinusMainWindowApplicationComponent.h"
#include "cxLayoutData.h"
#include "cxViewService.h"

namespace cx
{
void FraxinusMainWindowApplicationComponent::addAdditionalDefaultLayouts()
{
	std::cout << "addAdditionalDefaultLayouts" << std::endl;
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
	{
		// ACS 3D
		LayoutData layout = LayoutData::create("LAYOUT_ACAS", "ACAS", 2, 2);
		layout.setView(1, ptAXIAL, LayoutRegion(0, 0));
		layout.setView(1, ptCORONAL, LayoutRegion(0, 1));
		layout.setView(1, ptSAGITTAL, LayoutRegion(1, 1));
		layout.setView(0, ptAXIAL, LayoutRegion(1, 0));
		viewService->addDefaultLayout(layout);
	}
}

}//cx

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

#ifndef CXFRAXINUSWIDGETSPLUGINACTIVATOR_H_
#define CXFRAXINUSWIDGETSPLUGINACTIVATOR_H_

#include <ctkPluginActivator.h>
#include "boost/shared_ptr.hpp"

namespace cx
{

typedef boost::shared_ptr<class FraxinusWidgetsGUIExtenderService> FraxinusWidgetsGUIExtenderServicePtr;
typedef boost::shared_ptr<class RegisteredService> RegisteredServicePtr;

/**
 * Activator for the fraxinus widgets plugin
 *
 * \ingroup org_custusx_fraxinus_widgets
 *
 * \date 2015-09-22
 * \author Janne Beate Bakeng
 */
class FraxinusWidgetsPluginActivator :  public QObject, public ctkPluginActivator
{
	Q_OBJECT
	Q_INTERFACES(ctkPluginActivator)
	Q_PLUGIN_METADATA(IID "org_custusx_fraxinus_widgets")

public:

	FraxinusWidgetsPluginActivator();
	~FraxinusWidgetsPluginActivator();

	void start(ctkPluginContext* context);
	void stop(ctkPluginContext* context);

private:
	RegisteredServicePtr mRegistration;
};

} // namespace cx

#endif /* CXFRAXINUSWIDGETSPLUGINACTIVATOR_H_ */

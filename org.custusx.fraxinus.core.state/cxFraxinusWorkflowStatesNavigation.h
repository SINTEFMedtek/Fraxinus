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

#ifndef CXCUSTUSXWORKFLOWSTATESNAVIGATION_H_
#define CXCUSTUSXWORKFLOWSTATESNAVIGATION_H_

#include "org_custusx_fraxinus_core_state_Export.h"

#include <iostream>
#include <QState>
#include <QStateMachine>
#include <QString>
#include "cxFraxinusWorkflowStates.h"
#include "cxTypeConversions.h"
#include "cxRequestEnterStateTransition.h"
#include "cxWorkflowState.h"
#include "boost/shared_ptr.hpp"
#include "cxViewService.h"

class QMainWindow;

namespace cx
{
typedef boost::shared_ptr<class StateServiceBackend> StateServiceBackendPtr;
typedef boost::shared_ptr<class TransferFunctions3DPresets> TransferFunctions3DPresetsPtr;
class FraxinusTrackingWidget;
class FraxinusRegistrationWidget;
class FraxinusNavigationWidget;


class org_custusx_fraxinus_core_state_EXPORT TrackingWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
    TrackingWorkflowState(QState* parent, CoreServicesPtr services);
    virtual ~TrackingWorkflowState();
    virtual QIcon getIcon() const;
    virtual bool canEnter() const;
    virtual void onEntry(QEvent* event);

private:
    virtual void addDataToView();
};

class org_custusx_fraxinus_core_state_EXPORT RegistrationWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
    RegistrationWorkflowState(QState* parent, VisServicesPtr services);
    virtual ~RegistrationWorkflowState();
    virtual QIcon getIcon() const;
    FraxinusRegistrationWidget* getFraxinusRegistrationWidget();
	virtual bool canEnter() const;
	virtual void onEntry(QEvent *event);
    virtual void onExit(QEvent * event);

private:
    virtual void addDataToView();
    int m3DViewGroupNumber;
};

class org_custusx_fraxinus_core_state_EXPORT NavigationWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
    NavigationWorkflowState(QState* parent, CoreServicesPtr services);
    virtual ~NavigationWorkflowState();
    virtual QIcon getIcon() const;
    FraxinusNavigationWidget* getFraxinusNavigationWidget();
	virtual void onEntry(QEvent* event);
    void setupFraxinusNavigationWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber);
    void onExit(QEvent *event);
	virtual bool canEnter() const;

private:
    virtual void addDataToView();
    int mFlyThrough3DViewGroupNumber;
    int mSurfaceModel3DViewGroupNumber;
};


/**
 * @}
 */
}
#endif /* CXCUSTUSXWORKFLOWSTATESNAVIGAION_H_ */

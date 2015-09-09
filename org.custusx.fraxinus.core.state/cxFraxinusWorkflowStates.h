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

#ifndef CXCUSTUSXWORKFLOWSTATES_H_
#define CXCUSTUSXWORKFLOWSTATES_H_

#include "org_custusx_fraxinus_core_state_Export.h"

#include <iostream>
#include <QState>
#include <QStateMachine>
#include <QString>
#include <QAction>
#include "cxTypeConversions.h"
#include "cxRequestEnterStateTransition.h"
#include "cxWorkflowState.h"
#include "boost/shared_ptr.hpp"


namespace cx
{
typedef boost::shared_ptr<class StateServiceBackend> StateServiceBackendPtr;

class org_custusx_fraxinus_core_state_EXPORT ImportWorkflowState: public WorkflowState
{
Q_OBJECT

public:
    ImportWorkflowState(QState* parent, StateServiceBackendPtr backend) :
                    WorkflowState(parent, "ImportUid", "Import", backend)
	{}
    virtual ~ImportWorkflowState() {}
    virtual QIcon getIcon() const { return QIcon("://icons/preset_save.png"); }

	virtual bool canEnter() const { return true; }
};

class org_custusx_fraxinus_core_state_EXPORT ProcessWorkflowState: public WorkflowState
{
Q_OBJECT

public:
    ProcessWorkflowState(QState* parent, StateServiceBackendPtr backend);
    virtual ~ProcessWorkflowState() {}
	virtual QIcon getIcon() const
	{
        return QIcon("://icons/preset_save.png");
	}
	virtual void onEntry(QEvent* event);
	virtual bool canEnter() const;
};

class org_custusx_fraxinus_core_state_EXPORT PinpointWorkflowState: public WorkflowState
{
Q_OBJECT

public:
    PinpointWorkflowState(QState* parent, StateServiceBackendPtr backend);

    virtual ~PinpointWorkflowState() {}
	virtual QIcon getIcon() const
	{
        return QIcon("://icons/preset_save.png");
	}

	virtual bool canEnter() const;
};

class org_custusx_fraxinus_core_state_EXPORT RouteToTargetWorkflowState: public WorkflowState
{
Q_OBJECT

public:
    RouteToTargetWorkflowState(QState* parent, StateServiceBackendPtr backend);

    virtual ~RouteToTargetWorkflowState()
	{}
	virtual QIcon getIcon() const
	{
        return QIcon("://icons/preset_save.png");
	}

	virtual bool canEnter() const;
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyWorkflowState: public WorkflowState
{
Q_OBJECT

public:
    VirtualBronchoscopyWorkflowState(QState* parent, StateServiceBackendPtr backend);
    virtual ~VirtualBronchoscopyWorkflowState()
	{}
	virtual QIcon getIcon() const
	{
        return QIcon("://icons/preset_save.png");
	}

	virtual void onEntry(QEvent* event);
	virtual bool canEnter() const;
};

/**
 * @}
 */
}
#endif /* CXCUSTUSXWORKFLOWSTATES_H_ */

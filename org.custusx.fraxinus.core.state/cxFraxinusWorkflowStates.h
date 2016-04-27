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
#include "cxViewService.h"

class QMainWindow;

namespace cx
{
typedef boost::shared_ptr<class StateServiceBackend> StateServiceBackendPtr;
class VBWidget;

class org_custusx_fraxinus_core_state_EXPORT FraxinusWorkflowState : public WorkflowState
{
	Q_OBJECT
	ImagePtr getActiveImage();
public:
	FraxinusWorkflowState(QState* parent, QString uid, QString name, CoreServicesPtr services);
	virtual void setCameraStyleInGroup(CAMERA_STYLE_TYPE style, int groupIdx);
	virtual void onEntry(QEvent* event);
protected:
	void onEntryDefault();
	void useClipper(bool on);
	MeshPtr getCenterline();
	MeshPtr getRouteTotarget();
	QMainWindow *getMainWindow();
	VBWidget *getVBWidget();
	MeshPtr getAirwaysContour();
	ImagePtr getCTImage();
protected slots:
	virtual void setDefaultCameraStyle();
	virtual void setVBCameraStyle();
};

class org_custusx_fraxinus_core_state_EXPORT PatientWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
    PatientWorkflowState(QState* parent, CoreServicesPtr services);
    virtual ~PatientWorkflowState();
    virtual QIcon getIcon() const;
    virtual void onEntry(QEvent* event);
    virtual bool canEnter() const;
};

class org_custusx_fraxinus_core_state_EXPORT ImportWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
    ImportWorkflowState(QState* parent, VisServicesPtr services);
    virtual ~ImportWorkflowState();
    virtual QIcon getIcon() const;
	virtual bool canEnter() const;
	virtual void onEntry(QEvent *event);
};

class org_custusx_fraxinus_core_state_EXPORT ProcessWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

	void performAirwaysSegmentation(ImagePtr image);
public:
    ProcessWorkflowState(QState* parent, CoreServicesPtr services);
    virtual ~ProcessWorkflowState();
    virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	virtual bool canEnter() const;

signals:
	void airwaysSegmented();
private slots:
	void imageSelected();
};

class org_custusx_fraxinus_core_state_EXPORT PinpointWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

	PointMetricPtr getTargetPoint();
	void createRouteToTarget();
public:
    PinpointWorkflowState(QState* parent, CoreServicesPtr services);
    virtual ~PinpointWorkflowState();
    virtual QIcon getIcon() const;
	virtual bool canEnter() const;
signals:
	void routeToTargetCreated();
private slots:
	void dataAddedOrRemovedSlot();
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyFlyThroughWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

	void showAirwaysAndRouteToTarget();
public:
	VirtualBronchoscopyFlyThroughWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~VirtualBronchoscopyFlyThroughWorkflowState();
    virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	virtual bool canEnter() const;
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyCutPlanesWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services);
	virtual ~VirtualBronchoscopyCutPlanesWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	virtual bool canEnter() const;
};

/**
 * @}
 */
}
#endif /* CXCUSTUSXWORKFLOWSTATES_H_ */

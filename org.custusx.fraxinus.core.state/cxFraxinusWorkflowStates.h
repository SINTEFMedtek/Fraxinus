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
#include <QDialog>
#include <QCheckBox>
#include "cxTypeConversions.h"
#include "cxRequestEnterStateTransition.h"
#include "cxWorkflowState.h"
#include "boost/shared_ptr.hpp"
#include "cxViewService.h"
#include "cxFraxinusSegmentations.h"

class QMainWindow;

namespace cx
{
typedef boost::shared_ptr<class StateServiceBackend> StateServiceBackendPtr;
typedef boost::shared_ptr<class TransferFunctions3DPresets> TransferFunctions3DPresetsPtr;
typedef boost::shared_ptr<class FraxinusSegmentations> FraxinusSegmentationsPtr;
class FraxinusVBWidget;
class PinpointWidget;
class StructuresSelectionWidget;
class ProcedurePlanningWidget;


class org_custusx_fraxinus_core_state_EXPORT FraxinusWorkflowState : public WorkflowState
{
	Q_OBJECT
public:
	FraxinusWorkflowState(QState* parent, QString uid, QString name, CoreServicesPtr services, bool enableAction = true);
	virtual void setCameraStyleInGroup(CAMERA_STYLE_TYPE style, int groupIdx);
	virtual void onEntry(QEvent* event);

	ImagePtr getCTImage() const;

protected:
	MeshPtr getTubeCenterline() const;
	MeshPtr getRouteToTarget() const;
	MeshPtr getExtendedRouteToTarget() const;
	QMainWindow *getMainWindow();
	FraxinusVBWidget *getVBWidget();
	ProcedurePlanningWidget *getProcedurePlanningWidget();
	PinpointWidget *getPinpointWidget();
	ImagePtr getCTImageCopied() const;
	ImagePtr createCopiedImage(ImagePtr originalImage) const;
	PointMetricPtr getTargetPoint() const;
	PointMetricPtr getEndoscopePoint() const;
	DistanceMetricPtr getDistanceToTargetMetric() const;
	void createRouteToTarget();
	std::vector< Eigen::Vector3d > mRouteToTargetPositions;
	std::vector< double > mRouteToTargetCameraRotations;
	FraxinusSegmentationsPtr mFraxinusSegmentations;


	void setTransferfunction3D(QString transferfunction, ImagePtr image);
	void setTransferfunction2D(QString transferfunction, ImagePtr image);
	void setPointPickerIn3Dview(bool active);
	void setRTTInVBWidget();
	void setupViewOptionsInVBWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber);
	void setupViewOptionsForStructuresSelection(StructuresSelectionWidget *widget, std::vector<unsigned int> viewGroupNumbers);
	void setupVBWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber);
	void cleanupVBWidget();
	void setupProcedurePlanningWidget(int viewGroupNumber);

	InteractiveClipperPtr enableInvertedClipper(QString clipper_name, bool on);
	void removeAllDataFromClipper(InteractiveClipperPtr clipper);

	virtual void addDataToView() = 0;

protected slots:
	virtual void setDefaultCameraStyle();
	virtual void setVBFlythroughCameraStyle();
	virtual void setVBCutplanesCameraStyle();
	virtual void setAnyplaneCameraStyle();

signals:
	void routeToTargetCreated();

private:
	void onEntryDefault(QEvent *event);
	ImagePtr getActiveImage();
	TransferFunctions3DPresetsPtr getTransferfunctionPresets();
};

class org_custusx_fraxinus_core_state_EXPORT PatientWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	PatientWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~PatientWorkflowState();
	virtual QIcon getIcon() const;
	virtual bool canEnter() const;
	virtual void onEntry(QEvent* event);

private:
	virtual void addDataToView();
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
	virtual void onExit(QEvent * event);

private:
	virtual void addDataToView();
};

class org_custusx_fraxinus_core_state_EXPORT ProcessWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	ProcessWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~ProcessWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;
signals:
  void segmentationFinished();
private slots:
	void segmentationFinishedSlot();
private:
	virtual void addDataToView();
};

class org_custusx_fraxinus_core_state_EXPORT PinpointWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	PinpointWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~PinpointWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent *event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;
private slots:
	void dataAddedOrRemovedSlot();
	void createRoute();
	void pointChanged();
private:
	void addDataToView();
	void deleteOldRouteToTarget();

	bool mPointChanged;
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyFlyThroughWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	VirtualBronchoscopyFlyThroughWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~VirtualBronchoscopyFlyThroughWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;
private:
	void addDataToView();
	int mFlyThrough3DViewGroupNumber;
	int mSurfaceModel3DViewGroupNumber;
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyCutPlanesWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services);
	virtual ~VirtualBronchoscopyCutPlanesWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;

private:
	void addDataToView();
	int mFlyThrough3DViewGroupNumber;
	int mSurfaceModel3DViewGroupNumber;
};

class org_custusx_fraxinus_core_state_EXPORT VirtualBronchoscopyAnyplaneWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	VirtualBronchoscopyAnyplaneWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~VirtualBronchoscopyAnyplaneWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;
private:
	void addDataToView();
	int mFlyThrough3DViewGroupNumber;
	int mSurfaceModel3DViewGroupNumber;
};

class org_custusx_fraxinus_core_state_EXPORT ProcedurePlanningWorkflowState: public FraxinusWorkflowState
{
Q_OBJECT

public:
	ProcedurePlanningWorkflowState(QState* parent, CoreServicesPtr services);
	virtual ~ProcedurePlanningWorkflowState();
	virtual QIcon getIcon() const;
	virtual void onEntry(QEvent* event);
	void onExit(QEvent *event);
	virtual bool canEnter() const;

private:
	void addDataToView();
	int m3DViewGroupNumber;
	int m2DViewGroupNumber;
};

/**
 * @}
 */
}
#endif /* CXCUSTUSXWORKFLOWSTATES_H_ */

/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef FRAXINUSEBUSSIMULATORIMPLWIDGET_H
#define FRAXINUSEBUSSIMULATORIMPLWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include "cxFraxinusEBUSSimulatorWidget.h"
#include "cxFraxinusTrackingWidget.h"
#include "cxFraxinusVBWidget.h"
#include "cxBaseWidget.h"
#include "cxForwardDeclarations.h"
#include "cxXmlOptionItem.h"
#include "cxStreamerService.h"
#include "cxRegisteredService.h"

class QPushButton;
class QMainWindow;
class ctkPluginContext;

namespace cx {

class org_custusx_fraxinus_widgets_EXPORT FraxinusEBUSSimulatorImplWidget : public FraxinusEBUSSimulatorWidget
{
	Q_OBJECT

public:
	FraxinusEBUSSimulatorImplWidget(RegServicesPtr services, ctkPluginContext* context,
	                            QWidget *parent = 0);
	virtual ~FraxinusEBUSSimulatorImplWidget();

	void setParentWidget(QWidget *parent);
	void setCTImage(ImagePtr CTImage);
	void setVBWidget(FraxinusVBWidget *fraxinusVBWidget);

public slots:
	void stopEBUSSimulatorOnWorkflowExitSlot();

private slots:
	void startStopClickedSlot();

private:
	void startEBUSSimulator();
	void stopEBUSSimulator();
	void startUltrasoundStreaming();
	void stopUltrasoundStreaming();
	SimulatedStreamerService* getSimulatorStreamerService();

	FraxinusTrackingWidget* getTrackingWidget() const;
	RegServicesPtr mServices;
	ctkPluginContext* mPluginContext;
	FraxinusVBWidget* mFraxinusVBWidget;
	ImagePtr mCTImage;
	QWidget* mParentWidget;
	XmlOptionFile mOptions;
	QPushButton* mStartStopButton;
	bool mEBUSSimulatorEnabled;
	double mOriginalAnyplaneViewOffsetValue = 0.25;
	SimulatedStreamerService* mVolumeSliceService = nullptr;
	RegisteredServicePtr mVolumeSliceRegistration;

};
} //namespace cx


#endif //FRAXINUSEBUSSIMULATORIMPLWIDGET_H

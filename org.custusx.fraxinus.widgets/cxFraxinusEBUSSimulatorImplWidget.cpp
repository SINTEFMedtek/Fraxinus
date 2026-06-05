/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxFraxinusEBUSSimulatorImplWidget.h"
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMainWindow>
#include "cxRegServices.h"
#include "cxLogger.h"
#include "cxProfile.h"
#include "cxSettings.h"
#include "cxFraxinusVBWidget.h"
#include "cxViewService.h"
#include "cxImage.h"
#include "cxVideoService.h"
#include <QApplication>
#include <QMutex>
#include <QMutexLocker>
#include "cxRegisteredService.h"
#include "cxReporter.h"
#include "cxTrackingService.h"
#include "cxPatientModelService.h"
#include "cxVolumeSliceStreamer.h"
#include "cxSelectDataStringProperty.h"

namespace cx {

// ---------------------------------------------------------------------------
// VolumeSliceStreamerService — file-local, only used by FraxinusEBUSSimulatorImplWidget

class VolumeSliceStreamerService : public SimulatedStreamerService
{
public:
	VolumeSliceStreamerService(PatientModelServicePtr patientModelService, TrackingServicePtr trackingService) :
		mPatientModelService(patientModelService), mTrackingService(trackingService) {}

	QString getName() override { return "Volume Slicer"; }
	QString getType() const override { return "volume_slice_streamer"; }
	void stop() override {}

	std::vector<PropertyPtr> getSettings(QDomElement root) override
	{
		return { getInputImageOption(root) };
	}

	StreamerPtr createStreamer(QDomElement root) override
	{
		QMutexLocker lock(&mStreamerMutex);
		mStreamer.reset(new VolumeSliceStreamer(mPatientModelService));
		ToolPtr tool = mTrackingService->getFirstProbe();
		if(!tool)
			reporter()->sendWarning("VolumeSliceStreamerService: No probe tool found");
		ImagePtr imageCopy = createImageCopy(mImageUidToStream);
		if(!mStreamer->initialize(imageCopy, tool))
			reporter()->sendWarning("VolumeSliceStreamerService: Streamer not initialized");
		return mStreamer;
	}

	void setImageToStream(QString imageUid) override
	{
		if(mSelectImageDataAdapter)
		{
			mSelectImageDataAdapter->blockSignals(true);
			mSelectImageDataAdapter->setValue(imageUid);
			mSelectImageDataAdapter->blockSignals(false);
		}
		mImageUidToStream = imageUid;
		settings()->setValue("VolumeSlice/volume", imageUid);
		if(mStreamer)
			mStreamer->setSourceImage(createImageCopy(imageUid));
	}

private:
	ImagePtr createImageCopy(QString imageUid)
	{
		ImagePtr image = mPatientModelService->getData<Image>(imageUid);
		if(!image)
		{
			reporter()->sendWarning("VolumeSliceStreamerService: No image with uid: " + imageUid);
			return ImagePtr();
		}
		return image->copy();
	}

	StringPropertySelectImagePtr getInputImageOption(QDomElement root)
	{
		if(!mSelectImageDataAdapter)
		{
			mSelectImageDataAdapter = StringPropertySelectImage::New(mPatientModelService);
			QString selected = settings()->value("VolumeSlice/volume", "").toString();
			connect(mSelectImageDataAdapter.get(), &StringPropertySelectImage::dataChanged,
		        [this](QString uid){ this->setImageToStream(uid); });
			mSelectImageDataAdapter->setValue(selected);
		}
		return mSelectImageDataAdapter;
	}

	QString mImageUidToStream;
	StringPropertySelectImagePtr mSelectImageDataAdapter;
	VolumeSliceStreamerPtr mStreamer;
	QMutex mStreamerMutex;
	PatientModelServicePtr mPatientModelService;
	TrackingServicePtr mTrackingService;
};

// ---------------------------------------------------------------------------

FraxinusEBUSSimulatorImplWidget::FraxinusEBUSSimulatorImplWidget(RegServicesPtr services, ctkPluginContext *context,
																																 QWidget* parent):
	FraxinusEBUSSimulatorWidget(parent, context, this->getWidgetName(), "EBUS-Simulator"),
	mEBUSSimulatorEnabled(false),
	mServices(services),
	mPluginContext(context),
	mParentWidget(parent)
{
	mOptions = profile()->getXmlSettings().descend("fraxinusebussimulatorwidget");

	VolumeSliceStreamerService* service = new VolumeSliceStreamerService(services->patient(), services->tracking());
	mVolumeSliceService = service;
	mVolumeSliceRegistration = RegisteredServicePtr(new RegisteredService(context, service, StreamerService_iid));

	mStartStopButton = new QPushButton("Start", this);

	QVBoxLayout* EBUSSimulatorLayout = new QVBoxLayout();
	EBUSSimulatorLayout->addWidget(mStartStopButton);

	this->setLayout(EBUSSimulatorLayout);

	connect(mStartStopButton, &QPushButton::clicked, this, &FraxinusEBUSSimulatorImplWidget::startStopClickedSlot);
}

FraxinusEBUSSimulatorImplWidget::~FraxinusEBUSSimulatorImplWidget()
{
}

FraxinusTrackingWidget* FraxinusEBUSSimulatorImplWidget::getTrackingWidget() const
{
	QWidgetList widgets = qApp->topLevelWidgets();
	for(QWidget* w : widgets)
		if(w->objectName() == "main_window")
			return w->findChild<FraxinusTrackingWidget*>(FraxinusTrackingWidget::getWidgetName());
	return nullptr;
}

void FraxinusEBUSSimulatorImplWidget::setParentWidget(QWidget* parent)
{
	mParentWidget = parent;
}

void FraxinusEBUSSimulatorImplWidget::startStopClickedSlot()
{
	mEBUSSimulatorEnabled = !mEBUSSimulatorEnabled;
	if(mEBUSSimulatorEnabled)
	{
		mStartStopButton->setText("Stop");
		this->startEBUSSimulator();
		emit EBUSSimulatorStarted();
	}
	else
	{
		mStartStopButton->setText("Start");
		this->stopEBUSSimulator();
		emit EBUSSimulatorStopped();
	}
}
void FraxinusEBUSSimulatorImplWidget::stopEBUSSimulatorOnWorkflowExitSlot()
{
	if(mEBUSSimulatorEnabled)
		startStopClickedSlot();
}

void FraxinusEBUSSimulatorImplWidget::startEBUSSimulator()
{
	FraxinusTrackingWidget* trackingWidget = getTrackingWidget();
	if(trackingWidget)
	{
		connect(trackingWidget, &FraxinusTrackingWidget::trackingReady, this, &FraxinusEBUSSimulatorImplWidget::startUltrasoundStreaming);
		trackingWidget->startUltrasoundSimulation();
	}
	else
	{
		startUltrasoundStreaming();
	}

	mServices->view()->setActiveLayout("LAYOUT_RT_3D_ANY");
}

void FraxinusEBUSSimulatorImplWidget::stopEBUSSimulator()
{
	stopUltrasoundStreaming();

	FraxinusTrackingWidget* trackingWidget = getTrackingWidget();
	if(trackingWidget)
		trackingWidget->stopUltrasoundSimulation();

	mServices->view()->setActiveLayout("LAYOUT_VB_3D_ANY");
}

void FraxinusEBUSSimulatorImplWidget::startUltrasoundStreaming()
{
	FraxinusTrackingWidget* trackingWidget = getTrackingWidget();
	if(trackingWidget)
		disconnect(trackingWidget, &FraxinusTrackingWidget::trackingReady, this, &FraxinusEBUSSimulatorImplWidget::startUltrasoundStreaming);
	if(!mCTImage)
	{
		CX_LOG_WARNING() << "FraxinusEBUSSimulatorImplWidget::startUSSimulator: No CT image set.";
		return;
	}
	SimulatedStreamerService* simulatorStreamerService = getSimulatorStreamerService();
	if(simulatorStreamerService)
	{
		CX_LOG_INFO() << "Setting US simulator input to: " << mCTImage->getUid();
		simulatorStreamerService->setImageToStream(mCTImage->getUid());
	}
	else
		CX_LOG_WARNING() << "Cannot find SimulatedImageStreamerService";

	mServices->video()->setConnectionMethod("volume_slice_streamer");
	mServices->video()->openConnection();

	if(mFraxinusVBWidget)
	{
		mFraxinusVBWidget->setNavigateAlongAirwayWall(true);
		mFraxinusVBWidget->disableAutomaticRotation();
	}

	mOriginalAnyplaneViewOffsetValue = settings()->value("Navigation/anyplaneViewOffset").toDouble();
	settings()->setValue("Navigation/anyplaneViewOffset", 0.5);
}

void FraxinusEBUSSimulatorImplWidget::stopUltrasoundStreaming()
{
	SimulatedStreamerService* simulatorStreamerService = getSimulatorStreamerService();
	if(simulatorStreamerService)
		simulatorStreamerService->stop();

	mServices->video()->closeConnection();

	if(mFraxinusVBWidget)
		mFraxinusVBWidget->setNavigateAlongAirwayWall(false);

	settings()->setValue("Navigation/anyplaneViewOffset", mOriginalAnyplaneViewOffsetValue);
}

SimulatedStreamerService* FraxinusEBUSSimulatorImplWidget::getSimulatorStreamerService()
{
	return mVolumeSliceService;
}

void FraxinusEBUSSimulatorImplWidget::setCTImage(ImagePtr	CTImage)
{
	mCTImage = CTImage;
}

void FraxinusEBUSSimulatorImplWidget::setVBWidget(FraxinusVBWidget *fraxinusVBWidget)
{
	mFraxinusVBWidget = fraxinusVBWidget;
}

} //namespace cx

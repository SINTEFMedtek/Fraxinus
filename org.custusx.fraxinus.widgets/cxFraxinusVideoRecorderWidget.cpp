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

#include "cxFraxinusVideoRecorderWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMainWindow>
#include <qfileinfo.h>
#include "cxVisServices.h"
#include "cxApplication.h"
#include "cxFraxinusTrackingWidget.h"
#include "cxVideoService.h"
#include "cxLogger.h"
#include "cxPatientModelService.h"
#include "cxViewService.h"
#include "cxTrackingService.h"
#include "cxStyles.h"


namespace cx {

FraxinusVideoRecorderWidget::FraxinusVideoRecorderWidget(VisServicesPtr services, AcquisitionServicePtr acquisitionService, QWidget* parent):
	BaseWidget(parent, this->getWidgetName(), "Record video"),
	mServices(services),
	mAcquisitionService(acquisitionService),
	mContext(AcquisitionService::tUS)
{
	this->setObjectName(this->getWidgetName());
	this->setWindowTitle("Virtual Bronchoscopy Video Recorder");

	QGroupBox* recordBox = new QGroupBox(tr("Record bronchoscope video"));
	QVBoxLayout* recordVLayout = new QVBoxLayout();
	mStartStopButton = new QPushButton("Start video recording", this);
	mStartStopButtonBackgroundColor.setColor(QPalette::Button, Styles::getRed());
	mStartStopButton->setPalette(mStartStopButtonBackgroundColor);
	recordVLayout->addWidget(mStartStopButton);
	recordBox->setLayout(recordVLayout);
	recordVLayout->insertWidget(recordVLayout->count()-1, recordBox);

	connect(mStartStopButton, &QPushButton::clicked, this, &FraxinusVideoRecorderWidget::startStopClickedSlot);

	this->setLayout(recordVLayout);
}

FraxinusVideoRecorderWidget::~FraxinusVideoRecorderWidget()
{

}

void FraxinusVideoRecorderWidget::startStopClickedSlot()
{
	if(!mIsRecording)
	{
		connect(mAcquisitionService.get(), &AcquisitionService::stateChanged, this, &FraxinusVideoRecorderWidget::recordStateChangedSlot);
		mStartStopButton->setText("Stop video recording");
		mStartStopButtonBackgroundColor.setColor(QPalette::Button, Styles::getYellow());
		mStartStopButton->setPalette(mStartStopButtonBackgroundColor);
		startRecording();
	}
	else
	{
		mStartStopButton->setText("Start video recording");
		mStartStopButtonBackgroundColor.setColor(QPalette::Button, Styles::getRed());
		mStartStopButton->setPalette(mStartStopButtonBackgroundColor);
		stopRecording();
	}
}

void FraxinusVideoRecorderWidget::recordStateChangedSlot()
{
	AcquisitionService::STATE state = mAcquisitionService->getState();

	mStartStopButton->blockSignals(true);

	if(state == AcquisitionService::sRUNNING)
	{
		mStartStopButtonBackgroundColor.setColor(QPalette::Button, Styles::getGreen());
		mStartStopButton->setPalette(mStartStopButtonBackgroundColor);
		disconnect(mAcquisitionService.get(), &AcquisitionService::stateChanged, this, &FraxinusVideoRecorderWidget::recordStateChangedSlot);
	}

	mStartStopButton->blockSignals(false);
}

void FraxinusVideoRecorderWidget::startRecording()
{
	QString patientFolder = mServices->patient()->getActivePatientFolder();
	if (patientFolder.isEmpty() || QFileInfo(patientFolder).fileName() == "NoPatient")
		createNewPatient(); //create new patient if no patient

	mServices->view()->setActiveLayout("LAYOUT_RT_1X1");
	mFraxinusTrackingWidget = this->getTrackingWidget();
	if(!mFraxinusTrackingWidget)
		return;
	mTrackingService = mFraxinusTrackingWidget->getTrackingService();
	startTracking();
	startStreaming();
	checkIfReadyToRecordVideo();
	mIsRecording = true;
}

void FraxinusVideoRecorderWidget::stopRecording()
{
	stopRecordingVideo();
	//stopStreaming(); //Do not stop streaming, due to problems restarting it
	stopTracking();
	mIsRecording = false;
}

void FraxinusVideoRecorderWidget::createNewPatient()
{
	QString actionName = "CreatePatientWithoutDialog";
	triggerMainWindowActionWithObjectName(actionName);
}

void FraxinusVideoRecorderWidget::startTracking()
{
	if(mFraxinusTrackingWidget)
		mFraxinusTrackingWidget->startTracking();
}

void FraxinusVideoRecorderWidget::stopTracking()
{
	mFraxinusTrackingWidget = this->getTrackingWidget();
	if(mFraxinusTrackingWidget)
		mFraxinusTrackingWidget->stopTracking();
}

void FraxinusVideoRecorderWidget::startStreaming()
{
	mServices->video()->setConnectionMethod("open_cv_streamer");

	if(!mServices->video()->isConnected())
		mServices->video()->openConnection();
}

void FraxinusVideoRecorderWidget::stopStreaming()
{
	if(mServices->video()->isConnected())
		mServices->video()->closeConnection();
}

void FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo()
{
	VideoServicePtr videoService = mServices->video();
	disconnect(mAcquisitionService.get(), &AcquisitionService::usReadinessChanged, this, &FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo);
	disconnect(videoService.get(), &VideoService::connected, this, &FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo);

	bool tracking = mTrackingService->getState()==Tool::tsTRACKING;
	bool streaming = videoService->isConnected();
	bool acquisition = mAcquisitionService->isReady(mContext);
	if(!acquisition || !tracking || !streaming)
	{
		connect(mAcquisitionService.get(), &AcquisitionService::usReadinessChanged, this, &FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo);
		connect(videoService.get(), &VideoService::connected, this, &FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo);
		return;
	}

	mTimer.start();
	checkIfToolIsReadyAndStartRecordingSlot();

}

void FraxinusVideoRecorderWidget::checkIfToolIsReadyAndStartRecordingSlot()
{
	mTool = ToolPtr();
	mTool = mTrackingService->getFirstProbe();
	if(mTool)
		connect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &FraxinusVideoRecorderWidget::startRecordingVideo);
	else
	{
		if(mTimer.elapsed() < 20000) //Trying in 20 seconds
			QTimer::singleShot(500, this, SLOT(checkIfToolIsReadyAndStartRecordingSlot()));
		else
			CX_LOG_WARNING() << "In FraxinusVideoRecorderWidget::checkIfReadyToRecordVideo: Cannot find tool - not able to record video.";
	}
}


void FraxinusVideoRecorderWidget::startRecordingVideo()
{
	disconnect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &FraxinusVideoRecorderWidget::startRecordingVideo);
	QString category = QString("BronchoscopyVideo");
	RecordSessionPtr session = mAcquisitionService->getSession("");
	mAcquisitionService->startRecord(mContext, category, session);
}

void FraxinusVideoRecorderWidget::stopRecordingVideo()
{
	mAcquisitionService->stopRecord();
}

QMainWindow* FraxinusVideoRecorderWidget::getMainWindow()
{
	QWidgetList widgets = qApp->topLevelWidgets();
	for (QWidgetList::iterator i = widgets.begin(); i != widgets.end(); ++i)
		if ((*i)->objectName() == "main_window")
			return (QMainWindow*) (*i);
	return NULL;
}

FraxinusTrackingWidget* FraxinusVideoRecorderWidget::getTrackingWidget()
{
	QMainWindow* mainWindow = this->getMainWindow();
	QString widgetName(FraxinusTrackingWidget::getWidgetName());
	return mainWindow->findChild<FraxinusTrackingWidget*>(widgetName);
}


QString FraxinusVideoRecorderWidget::getWidgetName()
{
	return "fraxinus_video_recorder_widget";
}

} //namespace cx

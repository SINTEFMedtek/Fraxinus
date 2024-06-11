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
#include "cxAcquisitionService.h"
#include "cxLogger.h"
#include "cxPatientModelService.h"
#include "cxViewService.h"


namespace cx {

FraxinusVideoRecorderWidget::FraxinusVideoRecorderWidget(VisServicesPtr services, AcquisitionServicePtr acquisitionService, QWidget* parent):
	BaseWidget(parent, this->getWidgetName(), "Record video"),
	mServices(services),
	mAcquisitionService(acquisitionService)
{
	this->setObjectName(this->getWidgetName());
	this->setWindowTitle("Virtual Bronchoscopy Video Recorder");


	QGroupBox* recordBox = new QGroupBox(tr("Record bronchoscope video"));
	QVBoxLayout* recordVLayout = new QVBoxLayout();
	mStartStopButton = new QPushButton("Start Recording", this);
	recordVLayout->addWidget(mStartStopButton);
	recordBox->setLayout(recordVLayout);
	recordVLayout->insertWidget(recordVLayout->count()-1, recordBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.

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
		startRecording();
		mStartStopButton->setText("Stop Recording");
	}
	else
	{
		stopRecording();
		mStartStopButton->setText("Start Recording");
	}
}

//void FraxinusVideoRecorderWidget::recordStateChangedSlot()
//{
//	AcquisitionService::STATE state = mAcquisitionService->getState();

//	mStartStopButton->blockSignals(true);

//	switch (state)
//	{
//	case AcquisitionService::sRUNNING :
//			mStartStopButton->setChecked(true);
//		mStartStopButton->setText("Stop");
//		mStartStopButton->setIcon(QIcon(":/icons/open_icon_library/media-playback-stop.png"));
//			mStartStopButton->setEnabled(true);
//			mCancelButton->setEnabled(true);
//		break;
//	case AcquisitionService::sNOT_RUNNING :
//			mStartStopButton->setChecked(false);
//		mStartStopButton->setText("Start");
//		mStartStopButton->setIcon(QIcon(":/icons/open_icon_library/media-record-3.png"));
//			mStartStopButton->setEnabled(true);
//		mCancelButton->setEnabled(false);
//		break;
//	case AcquisitionService::sPOST_PROCESSING :
//			mStartStopButton->setChecked(false);
//		mStartStopButton->setText("Processing...");
//			mStartStopButton->setIcon(QIcon(":/icons/open_icon_library/media-record-3.png"));
//			mStartStopButton->setEnabled(false);
//			mCancelButton->setEnabled(false);
//		break;
//	}

//	mStartStopButton->blockSignals(false);
//}

void FraxinusVideoRecorderWidget::startRecording()
{
	QString patientFolder = mServices->patient()->getActivePatientFolder();
	if (patientFolder.isEmpty() || QFileInfo(patientFolder).fileName() == "NoPatient")
		createNewPatient(); //create new patient if no patient

	mServices->view()->setActiveLayout("LAYOUT_RT_1X1");
	startTracking();
	startStreaming();
	startRecordingVideo();
	mIsRecording = true;
}

void FraxinusVideoRecorderWidget::stopRecording()
{
	stopRecordingVideo();
	stopStreaming();
	stopTracking();
	mIsRecording = false;
}

void FraxinusVideoRecorderWidget::createNewPatient()
{
	QString actionName = "NewPatient";
	triggerMainWindowActionWithObjectName(actionName);
}

void FraxinusVideoRecorderWidget::startTracking()
{
	mFraxinusTrackingWidget = this->getTrackingWidget();
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

void FraxinusVideoRecorderWidget::startRecordingVideo()
{
	AcquisitionService::TYPES context(AcquisitionService::tUS);
	QString category = QString("BronchoscopyVideo");
	mAcquisitionService->startRecord(context, category);
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

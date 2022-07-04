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

#include "cxFraxinusVideoGeneratorWidget.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QMainWindow>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxLogger.h"
#include "cxRouteToTarget.h"
#include "cxPinpointWidget.h"
#include "cxPointMetric.h"
#include "cxVBcameraPath.h"

namespace cx {

FraxinusVideoGeneratorWidget::FraxinusVideoGeneratorWidget(VisServicesPtr services, QWidget* parent):
	VBWidget(services, parent),
	mServices(services)
{
	this->setObjectName(this->getWidgetName());
	this->setWindowTitle("Virtual Bronchoscopy Video Generator");
	this->setRecordVideoOption(true);

	connect(mPlaybackSlider, &QSlider::valueChanged, this, &FraxinusVideoGeneratorWidget::playbackSliderChanged);
}

FraxinusVideoGeneratorWidget::~FraxinusVideoGeneratorWidget()
{

}

void FraxinusVideoGeneratorWidget::playbackSliderChanged(int cameraPositionInPermill)
{
	//Write position and time to file
}


QString FraxinusVideoGeneratorWidget::getWidgetName()
{
	return "fraxinus_video_generator_widget";
}

} //namespace cx

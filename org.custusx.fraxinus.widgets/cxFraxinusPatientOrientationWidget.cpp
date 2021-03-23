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

#include "cxFraxinusPatientOrientationWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxCameraControl.h"
#include "cxViewService.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "cxView.h"



namespace cx {

FraxinusPatientOrientationWidget::FraxinusPatientOrientationWidget(VisServicesPtr services, QWidget* parent):
    BaseWidget(parent, this->getWidgetName(), "Patient Orientation"),
    mStandard3DViewActions(new QActionGroup(this)),
    mServices(services)
{

    QVBoxLayout* orientationsLayout = new QVBoxLayout;

    mAnteriorButton = new QPushButton(QIcon(":/icons/camera_view_A.png"), "Anterior", this);
    mPosteriorButton = new QPushButton(QIcon(":/icons/camera_view_P.png"), "Posterior", this);
    mLeftButton = new QPushButton(QIcon(":/icons/camera_view_L.png"), "Left", this);
    mRightButton = new QPushButton(QIcon(":/icons/camera_view_R.png"), "Right", this);
    mSuperiorButton = new QPushButton(QIcon(":/icons/camera_view_S.png"), "Superior", this);
    mInferiorButton = new QPushButton(QIcon(":/icons/camera_view_I.png"), "Inferior", this);

    mAnteriorButton->setStyleSheet("Text-align:left");
    mPosteriorButton->setStyleSheet("Text-align:left");
    mLeftButton->setStyleSheet("Text-align:left");
    mRightButton->setStyleSheet("Text-align:left");
    mSuperiorButton->setStyleSheet("Text-align:left");
    mInferiorButton->setStyleSheet("Text-align:left");

    orientationsLayout->addWidget(mAnteriorButton);
    orientationsLayout->addWidget(mPosteriorButton);
    orientationsLayout->addWidget(mLeftButton);
    orientationsLayout->addWidget(mRightButton);
    orientationsLayout->addWidget(mSuperiorButton);
    orientationsLayout->addWidget(mInferiorButton);

    this->setLayout(orientationsLayout);

    connect(mAnteriorButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setAnteriorViewSlot);
    connect(mPosteriorButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setPosteriorViewSlot);
    connect(mLeftButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setLeftViewSlot);
    connect(mRightButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setRightViewSlot);
    connect(mSuperiorButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setSuperiorViewSlot);
    connect(mInferiorButton, &QPushButton::clicked, this, &FraxinusPatientOrientationWidget::setInferiorViewSlot);


}


FraxinusPatientOrientationWidget::~FraxinusPatientOrientationWidget()
{

}

QString FraxinusPatientOrientationWidget::getWidgetName()
{
    return "fraxinus_patient_orientation_widget";
}

void FraxinusPatientOrientationWidget::setAnteriorViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::AnteriorDirection());
}

void FraxinusPatientOrientationWidget::setPosteriorViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::PosteriorDirection());
}

void FraxinusPatientOrientationWidget::setLeftViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::LeftDirection());
}

void FraxinusPatientOrientationWidget::setRightViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::RightDirection());
}

void FraxinusPatientOrientationWidget::setSuperiorViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::SuperiorDirection());
}

void FraxinusPatientOrientationWidget::setInferiorViewSlot()
{
    CameraControlPtr cameraControl = mServices->view()->getCameraControl();
    cameraControl->setStandard3DView(CameraControl::InferiorDirection());
}


} //namespace cx

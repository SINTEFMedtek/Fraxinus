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

#include "cxFraxinusRegistrationWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QListWidgetItem>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxApplication.h"
#include "cxRegistrationService.h"
#include "cxRegServices.h"
#include "cxTrackingService.h"
#include "cxProfile.h"
#include "cxRecordTrackingWidget.h"
#include "cxRecordSessionWidget.h"
#include "cxRecordSession.h"
#include "cxAcquisitionService.h"
#include "cxSelectDataStringProperty.h"
#include "cxRecordSessionWidget.h"
#include "cxRecordSession.h"
#include "cxHelperWidgets.h"
#include "cxStringProperty.h"
#include "cxDataSelectWidget.h"
#include "cxMesh.h"
#include "cxBronchoscopyRegistration.h"


namespace cx {

FraxinusRegistrationWidget::FraxinusRegistrationWidget(VisServicesPtr services, RegServicesPtr regServices, QWidget* parent):
    BaseWidget(parent, this->getWidgetName(), "Registration"),
    mBronchoscopyRegistration(new BronchoscopyRegistration()),
    mServices(services),
    mRegServices(regServices)
{
    mOptions = profile()->getXmlSettings().descend("fraxinusregistrationwidget");

    mSetOrientationButton = new QPushButton("Set Orientation", this);
    connect(mSetOrientationButton, &QPushButton::clicked, this, &FraxinusRegistrationWidget::setOrientationClickedSlot);
    mRegistrationButton = new QPushButton("Registration", this);
    connect(mRegistrationButton, &QPushButton::clicked, this, &FraxinusRegistrationWidget::registrationClickedSlot);
//    mRejectRegistrationButton = new QPushButton("Reject Registration", this);
//    connect(mRejectRegistrationButton, &QPushButton::clicked, this, &FraxinusRegistrationWidget::rejectRegistrationClickedSlot);

    mSelectCenterlineWidget = StringPropertySelectMesh::New(mServices->patient());
    mSelectCenterlineWidget->setValueName("CT airway centerline: ");

    mRecordTrackingWidget = new RecordTrackingWidget(mOptions.descend("recordTracker"), mRegServices->acquisition(), mServices, "bronc_path", this);
    mRecordTrackingWidget->hideMergeWithExistingSession();
    mRecordTrackingWidget->getSessionSelector()->setHelp("Select bronchoscope path for registration");
    mRecordTrackingWidget->getSessionSelector()->setDisplayName("Bronchoscope path");

    QVBoxLayout* verticalLayout = new QVBoxLayout;

    verticalLayout->addStretch();
    verticalLayout->addWidget(mSetOrientationButton);
    verticalLayout->addStretch();
    verticalLayout->addWidget(new DataSelectWidget(mServices->view(), mServices->patient(), this, mSelectCenterlineWidget));
    verticalLayout->addStretch();
    verticalLayout->addWidget(mRecordTrackingWidget);
    verticalLayout->addStretch();
    verticalLayout->addWidget(mRegistrationButton);
    verticalLayout->addStretch();
    //verticalLayout->addWidget(mRejectRegistrationButton);
    verticalLayout->addStretch();
    verticalLayout->addStretch();

    this->setLayout(verticalLayout);

}

FraxinusRegistrationWidget::~FraxinusRegistrationWidget()
{

}

QString FraxinusRegistrationWidget::getWidgetName()
{
    return "fraxinus_registration_widget";
}

void FraxinusRegistrationWidget::setDefaultCenterlineMesh(MeshPtr mesh)
{
    mSelectCenterlineWidget->setValue(mesh->getUid());
}

void FraxinusRegistrationWidget::processCenterline()
{
    //this->initializeTrackingService();

    if(!mSelectCenterlineWidget->getMesh())
    {
        reportError("No centerline");
        return;
    }
    vtkPolyDataPtr centerline = mSelectCenterlineWidget->getMesh()->getVtkPolyData();//input
    Transform3D rMd = mSelectCenterlineWidget->getMesh()->get_rMd();
    int maxNumberOfGenerations = 6;
    mBronchoscopyRegistration->processCenterline(centerline, rMd, maxNumberOfGenerations);
}


void FraxinusRegistrationWidget::setOrientationClickedSlot()
{
    if(mServices->tracking()->getState() < 3)  //Check if system is tracking
        return;

    Transform3D prMt = mServices->tracking()->getActiveTool()->get_prMt();
    Transform3D tMtm = createTransformRotateY(M_PI) * createTransformRotateZ(M_PI / 2);
    mRegServices->registration()->doFastRegistration_Orientation(tMtm, prMt);
}

void FraxinusRegistrationWidget::registrationClickedSlot()
{
    this->processCenterline();

    if(!mBronchoscopyRegistration->isCenterlineProcessed())
    {
        reportError("Centerline not processed");
        return;
    }

    Transform3D old_rMpr = mServices->patient()->get_rMpr();//input to registrationAlgorithm

    TimedTransformMap trackerRecordedData_prMt = mRecordTrackingWidget->getRecordedTrackerData_prMt();

    if(trackerRecordedData_prMt.empty())
    {
        reportError("No bronchoscope positions found");
        return;
    }

    Transform3D new_rMpr = Transform3D(mBronchoscopyRegistration->runBronchoscopyRegistration(trackerRecordedData_prMt,old_rMpr,0));
    new_rMpr = new_rMpr * old_rMpr;//output
    mRegServices->registration()->addPatientRegistration(new_rMpr, "Bronchoscopy centerline to tracking data");
}

void FraxinusRegistrationWidget::rejectRegistrationClickedSlot()
{

}


} //namespace cx

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

#include "cxFraxinusNavigationWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QListWidgetItem>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxApplication.h"
#include "cxTrackingService.h"
#include "cxTrackerConfigurationImpl.h"
#include "cxBronchoscopePositionProjection.h"
#include "cxTrackingPositionFilter.h"
#include "trackingSystemBronchoscopy/cxTrackingSystemBronchoscopyService.h"


namespace cx {

FraxinusNavigationWidget::FraxinusNavigationWidget(VisServicesPtr service, QWidget* parent):
    BaseWidget(parent, this->getWidgetName(), "Navigation"),
    mServices(service),
    mLockToCenterlineEnabled(false)
{

    mLockToCenterlineButton = new QPushButton("Connect to centerline", this);
    mLockToCenterlineButton->setFixedSize(200,30);
    mButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mLockToCenterlineButton->setPalette(mButtonBackgroundColor);
    connect(mLockToCenterlineButton, &QPushButton::clicked, this, &FraxinusNavigationWidget::lockToCenterlineSlot);

    QGroupBox* viewBox = new QGroupBox(tr("View"));
    mViewSelectionWidget = new ViewSelectionWidget(mServices, this);
    QVBoxLayout* viewLayout = new QVBoxLayout();
    viewLayout->addWidget(mViewSelectionWidget);
    viewBox->setLayout(viewLayout);

    mStructuresSelectionWidget = new StructuresSelectionWidget(mServices,this);
    QGroupBox* structuresBox = new QGroupBox(tr("Select structures"));
    QVBoxLayout* structuresLayout = new QVBoxLayout();
    structuresLayout->addWidget(mStructuresSelectionWidget);
    structuresBox->setLayout(structuresLayout);

    QVBoxLayout* verticalLayout = new QVBoxLayout;
    verticalLayout->addStretch();
    verticalLayout->addWidget(mLockToCenterlineButton);
    verticalLayout->addStretch();
    verticalLayout->addWidget(viewBox);
    verticalLayout->addStretch();
    verticalLayout->addWidget(structuresBox);
    verticalLayout->addStretch();

    this->setLayout(verticalLayout);
}

FraxinusNavigationWidget::~FraxinusNavigationWidget()
{

}

QString FraxinusNavigationWidget::getWidgetName()
{
    return "fraxinus_navigation_widget";
}

StructuresSelectionWidget* FraxinusNavigationWidget::getStructuresSelectionWidget()
{
    return mStructuresSelectionWidget;
}

void FraxinusNavigationWidget::setCenterline(MeshPtr centerline)
{
    mCenterline = centerline;
}

bool FraxinusNavigationWidget::enableLockToCenterline()
{
    if (mLockToCenterlineEnabled)
        return true;

    if (!mCenterline)
    {
        CX_LOG_WARNING() << "FraxinusNavigationWidget::enableLockToCenterline: Centerline not set.";
        return false;
    }

    BronchoscopePositionProjectionPtr projectionCenterlinePtr = BronchoscopePositionProjectionPtr(new BronchoscopePositionProjection());
    projectionCenterlinePtr->setRunFromWidget(false);
    projectionCenterlinePtr->processCenterline(mCenterline->getVtkPolyData(), mCenterline->get_rMd(), mServices->patient()->get_rMpr());
    projectionCenterlinePtr->setAdvancedCenterlineOption(true);
    projectionCenterlinePtr->setMaxDistanceToCenterline(30);
    projectionCenterlinePtr->setMaxSearchDistance(20);
    projectionCenterlinePtr->setAlpha(1.0);

    mCenterlineProjectionTrackingSystem = TrackingSystemBronchoscopyServicePtr(new TrackingSystemBronchoscopyService(mServices->tracking(), projectionCenterlinePtr));
    if ( !mCenterlineProjectionTrackingSystem->setTrackingSystem("org.custusx.core.tracking.system.igstk") )
    {
        CX_LOG_WARNING() << "FraxinusNavigationWidget::enableLockToCenterline: Tracking system org.custusx.core.tracking.system.igstk";
        return false;
    }

    mServices->tracking()->unInstallTrackingSystem(mCenterlineProjectionTrackingSystem->getBase());
    mServices->tracking()->installTrackingSystem(mCenterlineProjectionTrackingSystem);

    mLockToCenterlineEnabled = true;
    return true;
}

bool FraxinusNavigationWidget::disableLockToCenterline()
{
    if (!mLockToCenterlineEnabled)
        return true;

    if (mCenterlineProjectionTrackingSystem)
    {
        mServices->tracking()->unInstallTrackingSystem(mCenterlineProjectionTrackingSystem);
        mServices->tracking()->installTrackingSystem(mCenterlineProjectionTrackingSystem->getBase());
        mCenterlineProjectionTrackingSystem.reset();
        mLockToCenterlineEnabled = false;
        return true;
    }

    return false;
}

void FraxinusNavigationWidget::lockToCenterlineSlot()
{
    if(!mLockToCenterlineEnabled)
    {
        if(this->enableLockToCenterline())
        {
            mLockToCenterlineButton->setText("Disconnect from centerline");
            mButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
            mLockToCenterlineButton->setPalette(mButtonBackgroundColor);
        }

    }
    else
    {
        if(this->disableLockToCenterline())
        {
            mLockToCenterlineButton->setText("Connect to centerline");
            mButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
            mLockToCenterlineButton->setPalette(mButtonBackgroundColor);
        }
    }
}

void FraxinusNavigationWidget::addObjectToVolumeView(DataPtr object)
{
    if(mViewSelectionWidget)
        mViewSelectionWidget->addObjectToVolumeView(object);
}

void FraxinusNavigationWidget::addObjectToTubeView(DataPtr object)
{
    if(mViewSelectionWidget)
        mViewSelectionWidget->addObjectToTubeView(object);
}

void FraxinusNavigationWidget::setViewGroupNumber(unsigned int viewGroupNumber)
{
    if(mViewSelectionWidget)
        mViewSelectionWidget->setViewGroupNumber(viewGroupNumber);
}

void FraxinusNavigationWidget::updateDataOnEntry()
{
    if(mViewSelectionWidget)
        mViewSelectionWidget->updateDataOnEntry();
}

} //namespace cx

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

#include "cxFraxinusTrackingWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QListWidgetItem>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxFraxinusTrackingWidget.h"
#include "cxApplication.h"
#include "cxTrackingService.h"
#include "cxTrackerConfigurationImpl.h"
//#include "cxToolListWidget.h"


namespace cx {

FraxinusTrackingWidget::FraxinusTrackingWidget(TrackingServicePtr trackingService, QWidget* parent):
    BaseWidget(parent, this->getWidgetName(), "Tracking"),
    mTrackingService(trackingService)
{
    TrackerConfigurationPtr config = mTrackingService->getConfiguration();
    //mConfigFilesComboBox->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);

    QStringList applications;
    QString application("Bronchoscopy");
    applications.append(application);
    QStringList trackingSystems;
    QString trackingSystem("Aurora");
    trackingSystems.append(trackingSystem);
    this->addToolsToComboBoxes(4, config, applications, trackingSystems);

    mStartTrackingButton = new QPushButton("Start Tracking", this);
    connect(mStartTrackingButton, &QPushButton::clicked, this, &FraxinusTrackingWidget::startTrackingClickedSlot);
    mStopTrackingButton = new QPushButton("Start Tracking", this);
    connect(mStopTrackingButton, &QPushButton::clicked, this, &FraxinusTrackingWidget::stopTrackingClickedSlot);

    QVBoxLayout* verticalLayout = new QVBoxLayout;
    QGridLayout* gridLayout = new QGridLayout();

    for (int i=0; i<mToolFilesComboBoxs.size(); i++)
    {
        QString label= "Tool ";
        label.append(QString::number(i+1));
        gridLayout->addWidget(new QLabel(label), i, 0);
        gridLayout->addWidget(mToolFilesComboBoxs[i], i, 1);
    }
    verticalLayout->addSpacing(50);
    gridLayout->addWidget(mStartTrackingButton, mToolFilesComboBoxs.size(), 0);
    gridLayout->addWidget(mStopTrackingButton, mToolFilesComboBoxs.size(), 1);
    verticalLayout->addLayout(gridLayout);

    this->setLayout(verticalLayout);

    connect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::updateButtonStatusSlot);
    this->updateButtonStatusSlot();
}

FraxinusTrackingWidget::~FraxinusTrackingWidget()
{

}

QString FraxinusTrackingWidget::getWidgetName()
{
    return "fraxinus_tracking_widget";
}

void FraxinusTrackingWidget::addToolsToComboBoxes(int numberOfTools, TrackerConfigurationPtr config, QStringList applicationsFilter, QStringList trackingsystemsFilter)
{
    QStringList toolPathList = config->getToolsGivenFilter(applicationsFilter, trackingsystemsFilter);

    mToolFilesComboBoxs.clear();
    for(int i=0; i<numberOfTools; i++)
    {
        mToolFilesComboBoxs.push_back(new QComboBox());

        for(QString toolPath : toolPathList)
        {
            QString toolName = config->getTool(toolPath).mName;
            mToolFilesComboBoxs[i]->addItem(toolName);
            int index = mToolFilesComboBoxs[i]->findText(toolName);
            mToolFilesComboBoxs[i]->setItemData(index, toolPath, Qt::ToolTipRole);
        }
    }
}

void FraxinusTrackingWidget::startTrackingClickedSlot(bool checked)
{
    Q_UNUSED(checked);
    mTrackingService->setState(Tool::tsTRACKING);
}

void FraxinusTrackingWidget::stopTrackingClickedSlot(bool checked)
{
    Q_UNUSED(checked);
    mTrackingService->setState(Tool::tsINITIALIZED);
}

void FraxinusTrackingWidget::updateButtonStatusSlot()
{
    Tool::State state = mTrackingService->getState();

    mStartTrackingButton->setEnabled(state < Tool::tsTRACKING);
    mStopTrackingButton->setEnabled(state >= Tool::tsTRACKING);

}



} //namespace cx

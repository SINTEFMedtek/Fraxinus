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

#include "cxViewSelectionWidget.h"
#include <QButtonGroup>
#include <QRadioButton>
#include <QApplication>
#include <QMainWindow>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxLogger.h"
#include "cxMesh.h"

namespace cx {

ViewSelectionWidget::ViewSelectionWidget(VisServicesPtr services, QWidget* parent):
        BaseWidget(parent, this->getWidgetName(), "View"),
        mServices(services),
        mOpacityValue(1),
        mViewGroupNumber(0)
{

    // Selector for displaying volume or artificial tubes
    QButtonGroup *displaySelectorGroup = new QButtonGroup(this);
    mTubeButton = new QRadioButton(tr("Tubes"));
    mTubeButton->setChecked(true);
    mVolumeButton = new QRadioButton(tr("Volume"));
    displaySelectorGroup->addButton(mTubeButton);
    displaySelectorGroup->addButton(mVolumeButton);

    // Selector for airway tube opacity
    QButtonGroup *opacitySelectorGroup = new QButtonGroup(this);
    mOpacityOnButton = new QRadioButton(tr("Opacity on"));
    mOpacityOffButton = new QRadioButton(tr("Opacity off"));
    mOpacityOffButton->setChecked(true);
    opacitySelectorGroup->addButton(mOpacityOnButton);
    opacitySelectorGroup->addButton(mOpacityOffButton);

    //QGroupBox* viewBox = new QGroupBox(tr("View"));
    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->addWidget(mTubeButton,0,0);
    gridLayout->addWidget(mVolumeButton,1,0);
    gridLayout->addWidget(mOpacityOnButton,0,1);
    gridLayout->addWidget(mOpacityOffButton,1,1);
    //viewBox->setLayout(gridLayout);

    this->setLayout(gridLayout);

    connect(mTubeButton, &QRadioButton::clicked, this, &ViewSelectionWidget::displayTubes);
    connect(mVolumeButton, &QRadioButton::clicked, this, &ViewSelectionWidget::displayVolume);
    connect(mOpacityOnButton, &QRadioButton::clicked, this, &ViewSelectionWidget::airwayOpacityOn);
    connect(mOpacityOffButton, &QRadioButton::clicked, this, &ViewSelectionWidget::airwayOpacityOff);
}

ViewSelectionWidget::~ViewSelectionWidget()
{
}

QString ViewSelectionWidget::getWidgetName()
{
    return "fraxinus_view_selection_widget";
}

void ViewSelectionWidget::displayVolume()
{
    this->hideDataObjects(mTubeViewObjects);
    this->displayDataObjects(mVolumeViewObjects);
    mOpacityOnButton->hide();
    mOpacityOffButton->hide();
}

void ViewSelectionWidget::displayTubes()
{
    this->hideDataObjects(mVolumeViewObjects);
    this->displayDataObjects(mTubeViewObjects);
    mOpacityOnButton->show();
    mOpacityOffButton->show();
}

void ViewSelectionWidget::airwayOpacityOn()
{
    this->setAirwayOpacity(true);
}

void ViewSelectionWidget::airwayOpacityOff()
{
    this->setAirwayOpacity(false);
}

void ViewSelectionWidget::updateDataOnEntry()
{
    if(mVolumeButton->isChecked())
            this->displayVolume();
    else
    {
        this->displayTubes();
        this->setAirwayOpacity(mOpacityOnButton->isChecked());
    }
}


void ViewSelectionWidget::displayDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        mServices->view()->getGroup(mViewGroupNumber)->addData(object->getUid());
    }
}

void ViewSelectionWidget::hideDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        mServices->view()->getGroup(mViewGroupNumber)->removeData(object->getUid());
    }
}

void ViewSelectionWidget::addObjectToVolumeView(DataPtr object)
{
    mVolumeViewObjects.push_back(object);
}

void ViewSelectionWidget::addObjectToTubeView(DataPtr object)
{
    mTubeViewObjects.push_back(object);
}

double ViewSelectionWidget::getOpacity()
{
    return mOpacityValue;
}

bool ViewSelectionWidget::isVolumeButtonChecked()
{
    return mVolumeButton->isChecked();
}

bool ViewSelectionWidget::isTubeButtonChecked()
{
    return mTubeButton->isChecked();
}

void ViewSelectionWidget::setVolumeButtonChecked(bool checked)
{
    mVolumeButton->setChecked(checked);
}

void ViewSelectionWidget::setTubeButtonChecked(bool checked)
{
    mTubeButton->setChecked(checked);
}

void ViewSelectionWidget::setAirwayOpacity(bool opacity)
{
    double opacityValueOff = 1.0;
    double opacityValueOn = 0.6;
    if(opacity)
        mOpacityValue = opacityValueOn;
    else
        mOpacityValue = opacityValueOff;

    foreach(DataPtr object, mTubeViewObjects)
    {
        MeshPtr mesh = boost::dynamic_pointer_cast<Mesh>(object);
        if(mesh)
        {
            QColor color = mesh->getColor();
            color.setAlphaF(mOpacityValue);
            mesh->setColor(color);
        }
    }
}

void ViewSelectionWidget::setViewGroupNumber(unsigned int viewGroupNumber)
{
    mViewGroupNumber = viewGroupNumber;
}

} //namespace cx

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

#include "cxFraxinusVBWidget.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QKeyEvent>
#include <QRadioButton>
#include <QVBoxLayout>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"


namespace cx {

FraxinusVBWidget::FraxinusVBWidget(VisServicesPtr services, QWidget* parent):
    VBWidget(services, parent),
    mServices(services)
{
    this->setObjectName(this->getWidgetName());

    // Selector for displaying volume or artificial tubes
    QButtonGroup *displaySelectorGroup = new QButtonGroup(this);
    mVolumeButton = new QRadioButton(tr("Volume"));
    mVolumeButton->setChecked(true);
    mTubeButton = new QRadioButton(tr("Tubes"));
    displaySelectorGroup->addButton(mVolumeButton);
    displaySelectorGroup->addButton(mTubeButton);

    QGroupBox* viewBox = new QGroupBox(tr("View"));
    QVBoxLayout* viewVLayout = new QVBoxLayout;
    viewVLayout->addWidget(mVolumeButton);
    viewVLayout->addWidget(mTubeButton);
    viewBox->setLayout(viewVLayout);
    mVerticalLayout->insertWidget(mVerticalLayout->count()-1, viewBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
    mVerticalLayout->addStretch(); //And add some more stretch

    connect(mVolumeButton, &QRadioButton::clicked, this, &FraxinusVBWidget::displayVolume);
    connect(mTubeButton, &QRadioButton::clicked, this, &FraxinusVBWidget::displayTubes);
}

FraxinusVBWidget::~FraxinusVBWidget()
{

}

QString FraxinusVBWidget::getWidgetName()
{
    return "fraxinus_virtual_bronchoscopy_widget";
}

void FraxinusVBWidget::keyPressEvent(QKeyEvent* event)
{
    enum class Key7{NONE, SHOWVOLUME, SHOWTUBES};
    Key7 key7 = Key7::NONE;
    if (event->key()==Qt::Key_7)
    {
        if(mVolumeButton->isChecked())
            key7 = Key7::SHOWTUBES;
        else if(mTubeButton->isChecked())
            key7 = Key7::SHOWVOLUME;
    }

    if (event->key()==Qt::Key_V || key7 == Key7::SHOWVOLUME)
    {
        if(mControlsEnabled) {
            mVolumeButton->setChecked(true);
            this->displayVolume();
            return;
        }
    }

    if (event->key()==Qt::Key_T || key7 == Key7::SHOWTUBES)
    {
        if(mControlsEnabled) {
            mTubeButton->setChecked(true);
            this->displayTubes();
            return;
        }
    }

    VBWidget::keyPressEvent(event);
}

void FraxinusVBWidget::displayVolume()
{
    this->hideDataObjects(mTubeViewObjects);
    this->displayDataObjects(mVolumeViewObjects);
}

void FraxinusVBWidget::displayTubes()
{
    this->hideDataObjects(mVolumeViewObjects);
    this->displayDataObjects(mTubeViewObjects);
}

void FraxinusVBWidget::displayDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        mServices->view()->getGroup(mViewGroupNumber)->addData(object->getUid());
    }
}

void FraxinusVBWidget::hideDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        mServices->view()->getGroup(mViewGroupNumber)->removeData(object->getUid());
    }
}

void FraxinusVBWidget::setViewGroupNumber(unsigned int viewGroupNumber)
{
    mViewGroupNumber = viewGroupNumber;
}

void FraxinusVBWidget::addObjectToVolumeView(DataPtr object)
{
    mVolumeViewObjects.push_back(object);
}

void FraxinusVBWidget::addObjectToTubeView(DataPtr object)
{
    mTubeViewObjects.push_back(object);
}



} //namespace cx

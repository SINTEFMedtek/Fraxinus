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

#include "cxStructuresSelectionWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxLogger.h"

namespace cx {

StructuresSelectionWidget::StructuresSelectionWidget(VisServicesPtr services, QWidget* parent):
        BaseWidget(parent, this->getWidgetName(), "Select Structures"),
        mServices(services)
{

    QVBoxLayout* structuresLayout = new QVBoxLayout;

    mViewLungsButton = new QPushButton("Lungs");
    mViewLungsButtonBackgroundColor = mViewLungsButton->palette();
    mViewLungsButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewLungsButton->setPalette(mViewLungsButtonBackgroundColor);
    structuresLayout->addWidget(mViewLungsButton);
    mViewLungsButton->setEnabled(false);

    mViewLesionsButton = new QPushButton("Lesions");
    mViewLesionsButtonBackgroundColor = mViewLesionsButton->palette();
    mViewLesionsButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewLesionsButton->setPalette(mViewLesionsButtonBackgroundColor);
    structuresLayout->addWidget(mViewLesionsButton);
    mViewLesionsButton->setEnabled(false);

    mViewLymphNodesButton = new QPushButton("Lymph Nodes");
    mViewLymphNodesButtonBackgroundColor = mViewLymphNodesButton->palette();
    mViewLymphNodesButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewLymphNodesButton->setPalette(mViewLymphNodesButtonBackgroundColor);
    structuresLayout->addWidget(mViewLymphNodesButton);
    mViewLymphNodesButton->setEnabled(false);

    mViewVenaCavaButton = new QPushButton("Vena Cava");
    mViewVenaCavaButtonBackgroundColor = mViewVenaCavaButton->palette();
    mViewVenaCavaButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewVenaCavaButton->setPalette(mViewVenaCavaButtonBackgroundColor);
    structuresLayout->addWidget(mViewVenaCavaButton);
    mViewVenaCavaButton->setEnabled(false);

    mViewAzygosButton = new QPushButton("Vena Azygos");
    mViewAzygosButtonBackgroundColor = mViewAzygosButton->palette();
    mViewAzygosButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewAzygosButton->setPalette(mViewAzygosButtonBackgroundColor);
    structuresLayout->addWidget(mViewAzygosButton);
    mViewAzygosButton->setEnabled(false);

    mViewAortaButton = new QPushButton("Aorta");
    mViewAortaButtonBackgroundColor = mViewAortaButton->palette();
    mViewAortaButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewAortaButton->setPalette(mViewAortaButtonBackgroundColor);
    structuresLayout->addWidget(mViewAortaButton);
    mViewAortaButton->setEnabled(false);

    mViewSubclavianButton = new QPushButton("Subclavian Artery");
    mViewSubclavianButtonBackgroundColor = mViewSubclavianButton->palette();
    mViewSubclavianButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewSubclavianButton->setPalette(mViewSubclavianButtonBackgroundColor);
    structuresLayout->addWidget(mViewSubclavianButton);
    mViewSubclavianButton->setEnabled(false);

    mViewSmallVesselsButton = new QPushButton("Pulmonary Vessels");
    mViewSmallVesselsButtonBackgroundColor = mViewSmallVesselsButton->palette();
    mViewSmallVesselsButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewSmallVesselsButton->setPalette(mViewSmallVesselsButtonBackgroundColor);
    structuresLayout->addWidget(mViewSmallVesselsButton);
    mViewSmallVesselsButton->setEnabled(false);

    mViewHeartButton = new QPushButton("Heart");
    mViewHeartButtonBackgroundColor = mViewHeartButton->palette();
    mViewHeartButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewHeartButton->setPalette(mViewHeartButtonBackgroundColor);
    structuresLayout->addWidget(mViewHeartButton);
    mViewHeartButton->setEnabled(false);

    mViewEsophagusButton = new QPushButton("Esophagus");
    mViewEsophagusButtonBackgroundColor = mViewEsophagusButton->palette();
    mViewEsophagusButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewEsophagusButton->setPalette(mViewEsophagusButtonBackgroundColor);
    structuresLayout->addWidget(mViewEsophagusButton);
    mViewEsophagusButton->setEnabled(false);

    mViewSpineButton = new QPushButton("Spine");
    mViewSpineButtonBackgroundColor = mViewSpineButton->palette();
    mViewSpineButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
    mViewSpineButton->setPalette(mViewSpineButtonBackgroundColor);
    structuresLayout->addWidget(mViewSpineButton);
    mViewSpineButton->setEnabled(false);

    //structuresLayout->addStretch();
    this->setLayout(structuresLayout);
//    structuresBox->setLayout(structuresLayout);
//    mVerticalLayout->insertWidget(mVerticalLayout->count()-1, structuresBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
//    mVerticalLayout->addStretch(); //And add some more stretch

    connect(mViewLungsButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewLungsSlot);
    connect(mViewLesionsButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewLesionsSlot);
    connect(mViewLymphNodesButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewLymphNodesSlot);
    connect(mViewSpineButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewSpineSlot);
    connect(mViewSmallVesselsButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewSmallVesselsSlot);
    connect(mViewVenaCavaButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewVenaCavaSlot);
    connect(mViewAzygosButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewAzygosSlot);
    connect(mViewAortaButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewAortaSlot);
    connect(mViewSubclavianButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewSubclavianSlot);
    connect(mViewHeartButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewHeartSlot);
    connect(mViewEsophagusButton, &QPushButton::clicked, this, &StructuresSelectionWidget::viewEsophagusSlot);

}

StructuresSelectionWidget::~StructuresSelectionWidget()
{

}


QString StructuresSelectionWidget::getWidgetName()
{
    return "fraxinus_structures_selection_widget";
}


void StructuresSelectionWidget::displayDataObjects(std::vector<DataPtr> objects)
{
    CX_LOG_DEBUG() << "mViewGroupNumbers.size(): " << mViewGroupNumbers.size();
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        for(int i=0; i<mViewGroupNumbers.size(); i++)
        {
            CX_LOG_DEBUG() << "Adding for group number " << mViewGroupNumbers[i];
            ViewGroupDataPtr viewGroup = mServices->view()->getGroup(mViewGroupNumbers[i]);
            if(viewGroup)
                viewGroup->addData(object->getUid());
            else
                CX_LOG_WARNING() << "In StructuresSelectionWidget::displayDataObjects: Cannot find view group for data view.";
        }
    }
}

void StructuresSelectionWidget::hideDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        for(int i=0; i<mViewGroupNumbers.size(); i++)
        {
            ViewGroupDataPtr viewGroup = mServices->view()->getGroup(mViewGroupNumbers[i]);
            if(viewGroup)
                viewGroup->removeData(object->getUid());
            else
                CX_LOG_WARNING() << "In StructuresSelectionWidget::hideDataObjects: Cannot find view group for data view.";
        }
    }
}

void StructuresSelectionWidget::setViewGroupNumbers(std::vector<unsigned int> viewGroupNumbers)
{
    mViewGroupNumbers = viewGroupNumbers;
}


void StructuresSelectionWidget::addLungObject(DataPtr object)
{
    mViewLungsButton->setEnabled(true);
    mLungsObjects.push_back(object);
    mViewLungsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewLungsButton->setPalette(mViewLungsButtonBackgroundColor);
}

void StructuresSelectionWidget::addLesionObject(DataPtr object)
{
    mViewLesionsButton->setEnabled(true);
    mLesionsObjects.push_back(object);
    mViewLesionsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewLesionsButton->setPalette(mViewLesionsButtonBackgroundColor);
}

void StructuresSelectionWidget::addLymphNodeObject(DataPtr object)
{
    mViewLymphNodesButton->setEnabled(true);
    mLymphNodesObjects.push_back(object);
    mViewLymphNodesButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewLymphNodesButton->setPalette(mViewLymphNodesButtonBackgroundColor);
}

void StructuresSelectionWidget::addSpineObject(DataPtr object)
{
    mViewSpineButton->setEnabled(true);
    mSpineObjects.push_back(object);
    mViewSpineButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewSpineButton->setPalette(mViewSpineButtonBackgroundColor);
}

void StructuresSelectionWidget::addSmallVesselObject(DataPtr object)
{
    mViewSmallVesselsButton->setEnabled(true);
    mSmallVesselsObjects.push_back(object);
    mViewSmallVesselsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewSmallVesselsButton->setPalette(mViewSmallVesselsButtonBackgroundColor);
}

void StructuresSelectionWidget::addVenaCavaObject(DataPtr object)
{
    mViewVenaCavaButton->setEnabled(true);
    mVenaCavaObjects.push_back(object);
    mViewVenaCavaButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewVenaCavaButton->setPalette(mViewVenaCavaButtonBackgroundColor);
}

void StructuresSelectionWidget::addAzygosObject(DataPtr object)
{
    mViewAzygosButton->setEnabled(true);
    mAzygosObjects.push_back(object);
    mViewAzygosButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewAzygosButton->setPalette(mViewAzygosButtonBackgroundColor);
}

void StructuresSelectionWidget::addAortaObject(DataPtr object)
{
    mViewAortaButton->setEnabled(true);
    mAortaObjects.push_back(object);
    mViewAortaButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewAortaButton->setPalette(mViewAortaButtonBackgroundColor);
}

void StructuresSelectionWidget::addSubclavianObject(DataPtr object)
{
    mViewSubclavianButton->setEnabled(true);
    mSubclavianObjects.push_back(object);
    mViewSubclavianButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewSubclavianButton->setPalette(mViewSubclavianButtonBackgroundColor);
}

void StructuresSelectionWidget::addHeartObject(DataPtr object)
{
    mViewHeartButton->setEnabled(true);
    mHeartObjects.push_back(object);
    mViewHeartButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewHeartButton->setPalette(mViewHeartButtonBackgroundColor);
}

void StructuresSelectionWidget::addEsophagusObject(DataPtr object)
{
    mViewEsophagusButton->setEnabled(true);
    mEsophagusObjects.push_back(object);
    mViewEsophagusButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
    mViewEsophagusButton->setPalette(mViewEsophagusButtonBackgroundColor);
}

void StructuresSelectionWidget::viewLungsSlot()
{
    mViewLungsEnabled = !mViewLungsEnabled;

    if(mViewLungsEnabled)
    {
        this->displayDataObjects(mLungsObjects);
        mViewLungsButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewLungsButton->setPalette(mViewLungsButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mLungsObjects);
        mViewLungsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewLungsButton->setPalette(mViewLungsButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewLesionsSlot()
{
    mViewLesionsEnabled = !mViewLesionsEnabled;

    if(mViewLesionsEnabled)
    {
        this->displayDataObjects(mLesionsObjects);
        mViewLesionsButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewLesionsButton->setPalette(mViewLesionsButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mLesionsObjects);
        mViewLesionsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewLesionsButton->setPalette(mViewLesionsButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewLymphNodesSlot()
{
    mViewLymphNodesEnabled = !mViewLymphNodesEnabled;

    if(mViewLymphNodesEnabled)
    {
        this->displayDataObjects(mLymphNodesObjects);
        mViewLymphNodesButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewLymphNodesButton->setPalette(mViewLymphNodesButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mLymphNodesObjects);
        mViewLymphNodesButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewLymphNodesButton->setPalette(mViewLymphNodesButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewSpineSlot()
{
    mViewSpineEnabled = !mViewSpineEnabled;

    if(mViewSpineEnabled)
    {
        this->displayDataObjects(mSpineObjects);
        mViewSpineButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewSpineButton->setPalette(mViewSpineButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mSpineObjects);
        mViewSpineButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewSpineButton->setPalette(mViewSpineButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewSmallVesselsSlot()
{
    mViewSmallVesselsEnabled = !mViewSmallVesselsEnabled;

    if(mViewSmallVesselsEnabled)
    {
        this->displayDataObjects(mSmallVesselsObjects);
        mViewSmallVesselsButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewSmallVesselsButton->setPalette(mViewSmallVesselsButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mSmallVesselsObjects);
        mViewSmallVesselsButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewSmallVesselsButton->setPalette(mViewSmallVesselsButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewVenaCavaSlot()
{
    mViewVenaCavaEnabled = !mViewVenaCavaEnabled;

    if(mViewVenaCavaEnabled)
    {
        this->displayDataObjects(mVenaCavaObjects);
        mViewVenaCavaButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewVenaCavaButton->setPalette(mViewVenaCavaButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mVenaCavaObjects);
        mViewVenaCavaButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewVenaCavaButton->setPalette(mViewVenaCavaButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewAzygosSlot()
{
    mViewAzygosEnabled = !mViewAzygosEnabled;

    if(mViewAzygosEnabled)
    {
        this->displayDataObjects(mAzygosObjects);
        mViewAzygosButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewAzygosButton->setPalette(mViewAzygosButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mAzygosObjects);
        mViewAzygosButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewAzygosButton->setPalette(mViewAzygosButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewAortaSlot()
{
    mViewAortaEnabled = !mViewAortaEnabled;

    if(mViewAortaEnabled)
    {
        this->displayDataObjects(mAortaObjects);
        mViewAortaButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewAortaButton->setPalette(mViewAortaButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mAortaObjects);
        mViewAortaButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewAortaButton->setPalette(mViewAortaButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewSubclavianSlot()
{
    mViewSubclavianEnabled = !mViewSubclavianEnabled;

    if(mViewSubclavianEnabled)
    {
        this->displayDataObjects(mSubclavianObjects);
        mViewSubclavianButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewSubclavianButton->setPalette(mViewSubclavianButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mSubclavianObjects);
        mViewSubclavianButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewSubclavianButton->setPalette(mViewSubclavianButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewHeartSlot()
{
    mViewHeartEnabled = !mViewHeartEnabled;

    if(mViewHeartEnabled)
    {
        this->displayDataObjects(mHeartObjects);
        mViewHeartButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewHeartButton->setPalette(mViewHeartButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mHeartObjects);
        mViewHeartButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewHeartButton->setPalette(mViewHeartButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::viewEsophagusSlot()
{
    mViewEsophagusEnabled = !mViewEsophagusEnabled;

    if(mViewEsophagusEnabled)
    {
        this->displayDataObjects(mEsophagusObjects);
        mViewEsophagusButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
        mViewEsophagusButton->setPalette(mViewEsophagusButtonBackgroundColor);
    }
    else
    {
        this->hideDataObjects(mEsophagusObjects);
        mViewEsophagusButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
        mViewEsophagusButton->setPalette(mViewEsophagusButtonBackgroundColor);
    }
}

void StructuresSelectionWidget::onEntry()
{
    if(mViewLungsButton->isEnabled())
    {
        mViewLungsEnabled = !mViewLungsEnabled;
        this->viewLungsSlot();
    }
    if(mViewLesionsButton->isEnabled())
    {
        mViewLesionsEnabled = !mViewLesionsEnabled;
        this->viewLesionsSlot();
    }
    if(mViewLymphNodesButton->isEnabled())
    {
        mViewLymphNodesEnabled = !mViewLymphNodesEnabled;
        this->viewLymphNodesSlot();
    }
    if(mViewSpineButton->isEnabled())
    {
        mViewSpineEnabled = !mViewSpineEnabled;
        this->viewSpineSlot();
    }
    if(mViewSmallVesselsButton->isEnabled())
    {
        mViewSmallVesselsEnabled = !mViewSmallVesselsEnabled;
        this->viewSmallVesselsSlot();
    }
    if(mViewVenaCavaButton->isEnabled())
    {
        mViewVenaCavaEnabled = !mViewVenaCavaEnabled;
        this->viewVenaCavaSlot();
    }
    if(mViewAzygosButton->isEnabled())
    {
        mViewAzygosEnabled = !mViewAzygosEnabled;
        this->viewAzygosSlot();
    }
    if(mViewAortaButton->isEnabled())
    {
        mViewAortaEnabled = !mViewAortaEnabled;
        this->viewAortaSlot();
    }
    if(mViewSubclavianButton->isEnabled())
    {
        mViewSubclavianEnabled = !mViewSubclavianEnabled;
        this->viewSubclavianSlot();
    }
    if(mViewHeartButton->isEnabled())
    {
        mViewHeartEnabled = !mViewHeartEnabled;
        this->viewHeartSlot();
    }
    if(mViewEsophagusButton->isEnabled())
    {
        mViewEsophagusEnabled = !mViewEsophagusEnabled;
        this->viewEsophagusSlot();
    }

}



} //namespace cx

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

#include "cxProcedurePlanningWidget.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxStructuresSelectionWidget.h"
#include "cxFraxinusVBWidget.h"
#include "cxApplication.h"
#include "cxFraxinusPatientOrientationWidget.h"

namespace cx {

ProcedurePlanningWidget::ProcedurePlanningWidget(VisServicesPtr services, QWidget* parent):
    BaseWidget(parent, this->getWidgetName(), "Procedure Planning"),
    mServices(services)
{

    QPushButton *centerToImage = new QPushButton(QIcon(":/icons/center_image.png"), " Center Image", this);
    connect(centerToImage, &QPushButton::clicked, this, &ProcedurePlanningWidget::centerToImage);

    mFraxinusPatientOrientationWidget = new FraxinusPatientOrientationWidget(mServices, this);
    QGroupBox* orientationBox = new QGroupBox(tr("Set orientation"));
    QVBoxLayout* orientationLayout = new QVBoxLayout();
    orientationLayout->addWidget(mFraxinusPatientOrientationWidget);
    orientationBox->setLayout(orientationLayout);

    mStructuresSelectionWidget = new StructuresSelectionWidget(mServices,this);
    QGroupBox* structuresBox = new QGroupBox(tr("Select structures"));
    QVBoxLayout* structuresLayout = new QVBoxLayout();
    structuresLayout->addWidget(mStructuresSelectionWidget);
    structuresBox->setLayout(structuresLayout);

    QVBoxLayout* verticalLayout = new QVBoxLayout;
    verticalLayout->addStretch();
    verticalLayout->addWidget(centerToImage);
    verticalLayout->addStretch();
    verticalLayout->addWidget(orientationBox);
    verticalLayout->addStretch();
    verticalLayout->addWidget(structuresBox);
    verticalLayout->addStretch();

    this->setLayout(verticalLayout);
}

ProcedurePlanningWidget::~ProcedurePlanningWidget()
{

}


QString ProcedurePlanningWidget::getWidgetName()
{
    return "fraxinus_procedure_planning_widget";
}

StructuresSelectionWidget* ProcedurePlanningWidget::getStructuresSelectionWidget()
{
    return mStructuresSelectionWidget;
}

void ProcedurePlanningWidget::centerToImage()
{
    triggerMainWindowActionWithObjectName("CenterToImageCenter");
}


} //namespace cx

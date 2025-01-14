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

#include <QLabel>
#include <QDialog>
#include "cxTumorInformationWidget.h"
#include "cxMesh.h"
#include "cxLogger.h"
#include "cxVisServices.h"
#include "cxPatientModelService.h"
#include "vtkPolyData.h"
#include "cxTrackingService.h"


namespace cx {

TumorInformationWidget::TumorInformationWidget(VisServicesPtr services, QWidget* parent):
	BaseWidget(parent, this->getWidgetName(), "Tumor Information"),
	mTumorComboBox(new QComboBox),
	mTumorVolumeLabel(new QLabel),
	mServices(services)
{

	mDeleteTumorButton = new QPushButton("&Delete this tumor", this);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	QHBoxLayout *horizontalLayout = new QHBoxLayout();

	horizontalLayout->addWidget(mTumorVolumeLabel);
	horizontalLayout->addWidget(mDeleteTumorButton);

	this->updateTumorList();

	mainLayout->addWidget(mTumorComboBox);
	mainLayout->addLayout(horizontalLayout);

	this->setLayout(mainLayout);

	connect(mDeleteTumorButton, &QPushButton::clicked, this, &TumorInformationWidget::deleteTumorSlot);
	connect(mTumorComboBox, QOverload<int>::of(&QComboBox::highlighted), this, &TumorInformationWidget::tumorChangedSlot);
	connect(mTumorComboBox, QOverload<int>::of(&QComboBox::activated), this, &TumorInformationWidget::tumorChangedSlot);
}


TumorInformationWidget::~TumorInformationWidget()
{
	disconnect(mDeleteTumorButton, &QPushButton::clicked, this, &TumorInformationWidget::deleteTumorSlot);
	disconnect(mTumorComboBox, QOverload<int>::of(&QComboBox::highlighted), this, &TumorInformationWidget::tumorChangedSlot);
	disconnect(mTumorComboBox, QOverload<int>::of(&QComboBox::activated), this, &TumorInformationWidget::tumorChangedSlot);
}

QString TumorInformationWidget::getWidgetName()
{
	return "tumor_information_widget";
}

void TumorInformationWidget::onEntery()
{
	this->updateTumorList();
}

void TumorInformationWidget::onExit()
{
	for(int i=0; i<mTumors.size(); i++)
	{
		QColor color = mTumors[i]->getColor();
		color.setAlpha(255);
		mTumors[i]->setColor(color);
	}
}

void TumorInformationWidget::setTumorMeshes(std::vector<MeshPtr> tumors)
{
	mTumors = tumors;
}

void TumorInformationWidget::updateTumorList()
{
	mTumorComboBox->clear();
	for(int i=0; i<mTumors.size(); i++)
	{
		mTumorComboBox->addItem(mTumors[i]->getName());
	}
	tumorChangedSlot(mTumorComboBox->currentIndex());
}

void TumorInformationWidget::deleteTumorSlot()
{
	if(mDeleteDialog)
		return;

	int currentIndex = mTumorComboBox->currentIndex();
	if(currentIndex < 0)
		return;

	mDeleteDialog = new QDialog();
	mDeleteDialog->setWindowTitle(tr("Delete tumor"));

	mYesButton = new QPushButton(tr("&YES"));
	mNoButton = new QPushButton(tr("&NO"));

	QVBoxLayout *verticalLayout = new QVBoxLayout();
	QLabel *text = new QLabel("Are you sure you want to delete the tumor?");
	verticalLayout->addWidget(text);
	QHBoxLayout *horizontalLayout = new QHBoxLayout();
	horizontalLayout->addWidget(mYesButton);
	horizontalLayout->addStretch();
	horizontalLayout->addWidget(mNoButton);
	verticalLayout->addLayout(horizontalLayout);

	mDeleteDialog->setLayout(verticalLayout);
	mDeleteDialog->show();
	mDeleteDialog->activateWindow();

	connect(mYesButton, &QPushButton::clicked, this, &TumorInformationWidget::deleteTumor);
	connect(mNoButton, &QPushButton::clicked, this, &TumorInformationWidget::close);
	connect(mDeleteDialog, &QDialog::rejected, this, &TumorInformationWidget::close);
}

void TumorInformationWidget::close()
{
	if(!mDeleteDialog)
		return;

	disconnect(mYesButton, &QPushButton::clicked, this, &TumorInformationWidget::deleteTumor);
	disconnect(mNoButton, &QPushButton::clicked, this, &TumorInformationWidget::close);
	disconnect(mDeleteDialog, &QDialog::rejected, this, &TumorInformationWidget::close);
	mDeleteDialog->close();
	mDeleteDialog = nullptr;

}

void TumorInformationWidget::deleteTumor()
{
	int currentIndex = mTumorComboBox->currentIndex();
	if(currentIndex < 0)
		return;

	mTumorComboBox->removeItem(currentIndex);
	mServices->patient()->removeData(mTumors[currentIndex]->getUid());
	mTumors.erase(mTumors.begin() + currentIndex);

	this->close();

	this->tumorChangedSlot(mTumorComboBox->currentIndex());
}

void TumorInformationWidget::tumorChangedSlot(int currentIndex)
{
	if(currentIndex < 0 || currentIndex >= mTumors.size())
	{
		mTumorVolumeLabel->clear();
		return;
	}

	moveToTumor(mTumors[currentIndex]);

	for(int i=0; i<mTumors.size(); i++)
	{
		QColor color = mTumors[i]->getColor();
		if(i==currentIndex)
			color.setAlpha(255);
		else
			color.setAlpha(70);
		mTumors[i]->setColor(color);
	}

	mTumorVolumeLabel->setText(QString("Tumor volume: %1 ml").arg(mTumors[currentIndex]->getVolumeSizeMl(), 0, 'f', 2));

}

void TumorInformationWidget::moveToTumor(MeshPtr tumor)
{
	if(!tumor)
		return;

	vtkPolyDataPtr vtkPolyDataTumor =  tumor->getVtkPolyData();
	Transform3D rMd = tumor->get_rMd();
	Vector3D centerOfTumor_d(vtkPolyDataTumor->GetCenter());
	Vector3D centerOfTumor_r(centerOfTumor_d(0)+rMd(0,3), centerOfTumor_d(1)+rMd(1,3), centerOfTumor_d(2)+rMd(2,3));

	ToolPtr manualTool = mServices->tracking()->getManualTool();
	if(!manualTool)
		return;
	Transform3D rMpr = mServices->patient()->get_rMpr();
	Transform3D manualTool_prMt = manualTool->get_prMt();
	Transform3D manualTool_rMt = rMpr*manualTool_prMt;
	manualTool_rMt(0,3) = centerOfTumor_r[0];
	manualTool_rMt(1,3) = centerOfTumor_r[1];
	manualTool_rMt(2,3) = centerOfTumor_r[2];
	manualTool->set_prMt(rMpr.inverse()*manualTool_rMt);
	mServices->patient()->setCenter(centerOfTumor_r);
}


} //namespace cx

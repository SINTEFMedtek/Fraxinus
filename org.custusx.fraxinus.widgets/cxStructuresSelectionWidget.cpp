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
#include "cxEnumConversion.h"
#include "cxTransferFunctionWidget.h"
#include "cxDoubleWidgets.h"

namespace cx {

SelectableStructure::SelectableStructure(QString name)
{
  mName = name;
}
SelectableStructure::SelectableStructure()
{
  mName = "";
}

StructuresSelectionWidget::StructuresSelectionWidget(VisServicesPtr services, QWidget* parent):
	BaseWidget(parent, this->getWidgetName(), "Select Structures"),
	mStructuresLayout(new QVBoxLayout),
	mServices(services)
{
	mPETSlider.reset(new DoublePropertyImageTFSlider2DAnd3D);
	mPETSlider->setDisplayName("PET");

	for(int i = lsFIRST_STRUCTURE_BUTTON; i <= lsLAST_STRUCTURE_BUTTON; ++i)
	{
		addLungStructureButton(LUNG_STRUCTURES(i));
	}
	addLungStructureButton(lsPET_REGISTERED);

	mStructuresLayout->addLayout(this->getPETSliderLayout());
	this->setLayout(mStructuresLayout);
}

StructuresSelectionWidget::~StructuresSelectionWidget()
{
	QMapIterator<LUNG_STRUCTURES, SelectableStructure> i(mSelectableStructuresMap);
	while(i.hasNext())
	{
		i.next();
		disconnect(i.value().mConnection);
	}

	disconnect(mResetButton, &QPushButton::pressed, this, &StructuresSelectionWidget::resetButtonPressed);
}

QLayout* StructuresSelectionWidget::getPETSliderLayout()
{
	QHBoxLayout *layout = new QHBoxLayout();
	mResetButton = new QPushButton();
	mResetButton->setIcon(QIcon(":/icons/preset_reset.png"));
	connect(mResetButton, &QPushButton::pressed, this, &StructuresSelectionWidget::resetButtonPressed);

	layout->addWidget(mResetButton);
	layout->addWidget(this->getSliderWidget());
	return layout;
}

QWidget* StructuresSelectionWidget::getSliderWidget()
{
//	QWidget* sliderWidget = new SliderGroupWidget(this, mPETSlider);
	ScalarInteractionWidget* sliderWidget = new ScalarInteractionWidget(this, mPETSlider);
	//Don't show all the slider widgets
	sliderWidget->enableLabel();
	sliderWidget->enableSlider();
	sliderWidget->addToOwnLayout();
	return sliderWidget;
}

void StructuresSelectionWidget::resetButtonPressed()
{
	if(mPETImage)
		mPETImage->resetTransferFunctions();
}

void StructuresSelectionWidget::addLungStructureButton(LUNG_STRUCTURES lungStructure)
{
	QString name = enum2string(lungStructure);
	SelectableStructure structure(name);

	structure.mButton = new QPushButton(name);
	structure.mButtonBackgroundColor = structure.mButton->palette();
	structure.mButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
	structure.mButton->setPalette(structure.mButtonBackgroundColor);
	structure.mButton->setEnabled(false);
	mStructuresLayout->addWidget(structure.mButton);

	//Need to store the connection to be able to disconnect, because of the lambda function
	structure.mConnection = connect(structure.mButton, &QPushButton::clicked, this, [=]() { this->viewStructureSlot(lungStructure); });

	mSelectableStructuresMap.insert(lungStructure, structure);
}

void StructuresSelectionWidget::resetButtons()
{
	QMapIterator<LUNG_STRUCTURES, SelectableStructure> i(mSelectableStructuresMap);
	while(i.hasNext())
	{
		i.next();
		mSelectableStructuresMap[i.key()].mObjects.clear();
		mSelectableStructuresMap[i.key()].mButtonBackgroundColor.setColor(QPalette::Button, Qt::black);
		mSelectableStructuresMap[i.key()].mButton->setPalette(mSelectableStructuresMap[i.key()].mButtonBackgroundColor);
		mSelectableStructuresMap[i.key()].mButton->setEnabled(false);
	}
}

QString StructuresSelectionWidget::getWidgetName()
{
	return "fraxinus_structures_selection_widget";
}


void StructuresSelectionWidget::displayDataObjects(std::vector<DataPtr> objects)
{
	for(const DataPtr &object : objects)
	{
		if(!object)
			continue;
		for(int i=0; i<mViewGroupNumbers.size(); i++)
		{
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
	for(const DataPtr &object : objects)
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

void StructuresSelectionWidget::addObject(LUNG_STRUCTURES name, DataPtr object)
{
	if(!object)
		return;
	SelectableStructure structure = mSelectableStructuresMap.take(name);
	structure.mButton->setEnabled(true);
	structure.mObjects.push_back(object);
	structure.mButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
	structure.mButton->setPalette(structure.mButtonBackgroundColor);
	mSelectableStructuresMap.insert(name, structure);

	if(name == lsPET_REGISTERED)
		this->setPETImage(boost::dynamic_pointer_cast<Image>(object));
}

void StructuresSelectionWidget::setPETImage(ImagePtr image)
{
	if(image)
	{
		if(mPETImage)
			disconnect(mPETImage.get(), &Image::transferFunctionsChanged, this, &StructuresSelectionWidget::transferFunctionsChanged);

		mPETImage = image;
		mPETSlider->setImage(mPETImage);

		connect(mPETImage.get(), &Image::transferFunctionsChanged, this, &StructuresSelectionWidget::transferFunctionsChanged);
	}
}

void StructuresSelectionWidget::transferFunctionsChanged()
{
	mPETSlider->setImage(mPETImage);
}

void StructuresSelectionWidget::viewStructureSlot(LUNG_STRUCTURES name)
{
	mSelectableStructuresMap[name].mViewEnabled = !mSelectableStructuresMap[name].mViewEnabled;
	if(mSelectableStructuresMap[name].mViewEnabled)
	{
		this->displayDataObjects(mSelectableStructuresMap[name].mObjects);
		mSelectableStructuresMap[name].mButtonBackgroundColor.setColor(QPalette::Button, Qt::green);
		mSelectableStructuresMap[name].mButton->setPalette(mSelectableStructuresMap[name].mButtonBackgroundColor);
	}
	else
	{
		this->hideDataObjects(mSelectableStructuresMap[name].mObjects);
		mSelectableStructuresMap[name].mButtonBackgroundColor.setColor(QPalette::Button, Qt::red);
		mSelectableStructuresMap[name].mButton->setPalette(mSelectableStructuresMap[name].mButtonBackgroundColor);
	}
}

void StructuresSelectionWidget::onEntry()
{
	QMapIterator<LUNG_STRUCTURES, SelectableStructure> i(mSelectableStructuresMap);
	while(i.hasNext())
	{
		i.next();
		if(i.value().mButton->isEnabled())
		{
			mSelectableStructuresMap[i.key()].mViewEnabled = !i.value().mViewEnabled;
			this->viewStructureSlot(i.key());
		}
	}
}

} //namespace cx

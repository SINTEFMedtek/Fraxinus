/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxFraxinusSegmentations.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include "cxDisplayTimerWidget.h"
#include "cxContourFilter.h"
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxPatientModelService.h"
#include "cxImage.h"
#include "cxMesh.h"
#include "cxRouteToTargetFilterService.h"
#include "cxGenericScriptFilter.h"

namespace cx
{

FraxinusSegmentations::FraxinusSegmentations(CoreServicesPtr services) :
	mServices(services)
{
	mTimedAlgorithmProgressBar = new cx::TimedAlgorithmProgressBar;
}

FraxinusSegmentations::~FraxinusSegmentations()
{
	
}

void FraxinusSegmentations::close()
{
	if(mSegmentationSelectionInput)
		mSegmentationSelectionInput->close();
}

ImagePtr FraxinusSegmentations::getCTImage() const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(!it->first.contains("_copy"))
		{
			image = it->second;
			break;
		}
	}
	return image;
}

MeshPtr FraxinusSegmentations::getCenterline() const
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
		if(iter->first.contains(airwaysFilterGetNameSuffixCenterline()) && !iter->first.contains(RouteToTargetFilter::getNameSuffix()))
			return iter->second;
	}
	return MeshPtr();
}


MeshPtr FraxinusSegmentations::getAirwaysContour()
{
	return this->getMesh(airwaysFilterGetNameSuffixAirways(), ContourFilter::getNameSuffix());
}

MeshPtr FraxinusSegmentations::getAirwaysTubes()
{
	return this->getMesh(airwaysFilterGetNameSuffixTubes(), airwaysFilterGetNameSuffixAirways());
}

MeshPtr FraxinusSegmentations::getVessels()
{
	return this->getMesh(airwaysFilterGetNameSuffixVessels());
}

MeshPtr FraxinusSegmentations::getMesh(QString str_1, QString str_2)
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
		if(iter->first.contains(str_1))
			if(iter->first.contains(str_2))
				return iter->second;
	}
	return MeshPtr();
}

MeshPtr FraxinusSegmentations::getLungs()
{
	return this->getMesh("_lungs");
}

MeshPtr FraxinusSegmentations::getLymphNodes()
{
	return this->getMesh("_lymphNodes");
}

MeshPtr FraxinusSegmentations::getNodules()
{
	return this->getMesh("_nodules");
}

MeshPtr FraxinusSegmentations::getVenaCava()
{
	return this->getMesh("_mediumOrgansMediastinum", "VenaCava");
}

MeshPtr FraxinusSegmentations::getAorticArch()
{
	return this->getMesh("_mediumOrgansMediastinum", "AorticArch");
}

MeshPtr FraxinusSegmentations::getAscendingAorta()
{
	return this->getMesh("_mediumOrgansMediastinum", "AscendingAorta");
}

MeshPtr FraxinusSegmentations::getDescendingAorta()
{
	return this->getMesh("_mediumOrgansMediastinum", "DescendingAorta");
}

MeshPtr FraxinusSegmentations::getSpine()
{
	return this->getMesh("_mediumOrgansMediastinum", "Spine");
}

MeshPtr FraxinusSegmentations::getSubCarArt()
{
	return this->getMesh("_smallOrgansMediastinum", "SubCarArt");
}

MeshPtr FraxinusSegmentations::getEsophagus()
{
	return this->getMesh("_smallOrgansMediastinum", "Esophagus");
}

MeshPtr FraxinusSegmentations::getBrachiocephalicVeins()
{
	return this->getMesh("_smallOrgansMediastinum", "BrachiocephalicVeins");
}

MeshPtr FraxinusSegmentations::getAzygos()
{
	return this->getMesh("_smallOrgansMediastinum", "Azygos");
}

MeshPtr FraxinusSegmentations::getHeart()
{
	return this->getMesh("_pulmSystHeart", "Heart");
}

MeshPtr FraxinusSegmentations::getPulmonaryVeins()
{
	return this->getMesh("_pulmSystHeart", "PulmonaryVeins");
}

MeshPtr FraxinusSegmentations::getPulmonaryTrunk()
{
	return this->getMesh("_pulmSystHeart", "PulmonaryTrunk");
}

void FraxinusSegmentations::createSelectSegmentationBox()
{
	if(mActiveTimerWidget) // check that segmentation is not already running
		return;
	mSegmentationSelectionInput = new QDialog();
	mSegmentationSelectionInput->setWindowTitle(tr("Select structures for segmentation"));
	mSegmentationSelectionInput->setWindowFlags(Qt::WindowStaysOnTopHint);
	
	mCheckBoxAirways = new QCheckBox(tr("Airways (~1 min)"));
	mCheckBoxAirways->setChecked(true);
	mCheckBoxAirways->setDisabled(true);
	mCheckBoxLungs = new QCheckBox(tr("Lungs (~3 min)"));
	mCheckBoxLymphNodes = new QCheckBox(tr("Lymph Nodes  (~10 min)"));
	mCheckBoxPulmonarySystem = new QCheckBox(tr("Heart, Pulmonary Veins, Pulmonary Trunk"));
	mCheckBoxMediumOrgans = new QCheckBox(tr("Vena Cava, Aorta, Spine  (~4 min)"));
	mCheckBoxSmallOrgans = new QCheckBox(tr("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos (~4 min)"));
	mCheckBoxNodules = new QCheckBox(tr("Lesions"));
	mCheckBoxVessels = new QCheckBox(tr("Small Vessels  (~3 min)"));
	
	QPushButton* OKbutton = new QPushButton(tr("&OK"));
	QPushButton* Cancelbutton = new QPushButton(tr("&Cancel"));
	
	connect(OKbutton, &QPushButton::clicked, this, &FraxinusSegmentations::imageSelected);
	connect(Cancelbutton, &QPushButton::clicked, this, &FraxinusSegmentations::cancel);
	
	QVBoxLayout* checkBoxLayout = new QVBoxLayout;
	checkBoxLayout->addWidget(mCheckBoxAirways);
	checkBoxLayout->addWidget(mCheckBoxLungs);
	checkBoxLayout->addWidget(mCheckBoxLymphNodes);
	checkBoxLayout->addWidget(mCheckBoxPulmonarySystem);
	checkBoxLayout->addWidget(mCheckBoxMediumOrgans);
	checkBoxLayout->addWidget(mCheckBoxSmallOrgans);
	checkBoxLayout->addWidget(mCheckBoxNodules);
	checkBoxLayout->addWidget(mCheckBoxVessels);
	
	QGridLayout* mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(checkBoxLayout, 0, 0);
	mainLayout->addWidget(Cancelbutton, 1, 1);
	mainLayout->addWidget(OKbutton, 1, 2);
	
	mSegmentationSelectionInput->setLayout(mainLayout);
	mSegmentationSelectionInput->show();
	mSegmentationSelectionInput->activateWindow();
}

void FraxinusSegmentations::imageSelected()
{
	mSegmentAirways = mCheckBoxAirways->isChecked();
	mSegmentVessels = mCheckBoxVessels->isChecked();
	mSegmentLungs = mCheckBoxLungs->isChecked();
	mSegmentLymphNodes = mCheckBoxLymphNodes->isChecked();
	mSegmentPulmonarySystem = mCheckBoxPulmonarySystem->isChecked();
	mSegmentMediumOrgans = mCheckBoxMediumOrgans->isChecked();
	mSegmentSmallOrgans = mCheckBoxSmallOrgans->isChecked();
	mSegmentNodules = mCheckBoxNodules->isChecked();
	mSegmentationSelectionInput->close();
	
	this->createProcessingInfo();
	ImagePtr image = this->getCTImage();
	this->performAirwaysSegmentation(image);
}

void FraxinusSegmentations::cancel()
{
	mSegmentationSelectionInput->close();
}

void FraxinusSegmentations::createProcessingInfo()
{
	mSegmentationProcessingInfo = new QDialog();
	mSegmentationProcessingInfo->setWindowTitle(tr("Segmentation status"));
	mSegmentationProcessingInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
	
	QGridLayout* gridLayout = new QGridLayout;
	gridLayout->setColumnMinimumWidth(0,500);
	gridLayout->setColumnMinimumWidth(1,100);
	
	if (mSegmentAirways)
	{
		QWidget* timerWidget = new QWidget;
		mAirwaysTimerWidget = new DisplayTimerWidget(timerWidget);
		mAirwaysTimerWidget->setFontSize(3);
		mAirwaysTimerWidget->setFixedWidth(50);
		mAirwaysTimerWidget->show();
		QLabel* label = new QLabel("Airways:");
		gridLayout->addWidget(label,0,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,0,1);
		if(this->getCenterline())
			mAirwaysTimerWidget->stop();
	}
	if (mSegmentVessels)
	{
		QWidget* timerWidget = new QWidget;
		mVesselsTimerWidget = new DisplayTimerWidget(timerWidget);
		mVesselsTimerWidget->setFontSize(3);
		mVesselsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Small Vessels:");
		gridLayout->addWidget(label,1,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,1,1);
		if(this->getVessels())
			mVesselsTimerWidget->stop();
	}
	if (mSegmentLungs)
	{
		QWidget* timerWidget = new QWidget;
		mLungsTimerWidget = new DisplayTimerWidget(timerWidget);
		mLungsTimerWidget->setFontSize(3);
		mLungsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Lungs:");
		gridLayout->addWidget(label,2,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,2,1);
		if(this->getLungs())
			mLungsTimerWidget->stop();
	}
	if (mSegmentLymphNodes)
	{
		QWidget* timerWidget = new QWidget;
		mLymphNodesTimerWidget = new DisplayTimerWidget(timerWidget);
		mLymphNodesTimerWidget->setFontSize(3);
		mLymphNodesTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Lymph Nodes:");
		gridLayout->addWidget(label,3,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,3,1);
		if(this->getLymphNodes())
			mLymphNodesTimerWidget->stop();
	}
	if (mSegmentPulmonarySystem)
	{
		QWidget* timerWidget = new QWidget;
		mPulmonarySystemTimerWidget = new DisplayTimerWidget(timerWidget);
		mPulmonarySystemTimerWidget->setFontSize(3);
		mPulmonarySystemTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Pulmonary System:");
		gridLayout->addWidget(label,4,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,4,1);
		if(this->getHeart())
			mPulmonarySystemTimerWidget->stop();
	}
	if (mSegmentMediumOrgans)
	{
		QWidget* timerWidget = new QWidget;
		mMediumOrgansTimerWidget = new DisplayTimerWidget(timerWidget);
		mMediumOrgansTimerWidget->setFontSize(3);
		mMediumOrgansTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Vena Cava, Aorta, Spine:");
		gridLayout->addWidget(label,5,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,5,1);
		if(this->getSpine())
			mMediumOrgansTimerWidget->stop();
	}
	if (mSegmentSmallOrgans)
	{
		QWidget* timerWidget = new QWidget;
		mSmallOrgansTimerWidget = new DisplayTimerWidget(timerWidget);
		mSmallOrgansTimerWidget->setFontSize(3);
		mSmallOrgansTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos:");
		gridLayout->addWidget(label,6,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,6,1);
		if(this->getEsophagus())
			mSmallOrgansTimerWidget->stop();
	}
	if (mSegmentNodules)
	{
		QWidget* timerWidget = new QWidget;
		mNodulesTimerWidget = new DisplayTimerWidget(timerWidget);
		mNodulesTimerWidget->setFontSize(3);
		mNodulesTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Lesions:");
		gridLayout->addWidget(label,7,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,7,1,8,3);
		if(this->getNodules())
			mNodulesTimerWidget->stop();
	}
	
	
	mSegmentationProcessingInfo->setLayout(gridLayout);
	mSegmentationProcessingInfo->show();
	mSegmentationProcessingInfo->activateWindow();
}

void FraxinusSegmentations::performAirwaysSegmentation(ImagePtr image)
{
	if(!image)
		return;
	
	DataPtr centerline = this->getCenterline();
	DataPtr vessels = this->getVessels();
	if(centerline)
	{
		mAirwaysProcessed = true;
		if(vessels)
		{
			mVesselsProcessed = true;
			this->performMLSegmentation(image);
			return;
		}
		else if(!mSegmentVessels)
		{
			this->performMLSegmentation(image);
			return;
		}
	}
	
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	//dialog.show();
	
#ifndef __APPLE__
	AirwaysFilterPtr airwaysFilter = AirwaysFilterPtr(new AirwaysFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = airwaysFilter->getInputTypes();
	airwaysFilter->getOutputTypes();
	airwaysFilter->getOptions();
	if(!mAirwaysProcessed  && mSegmentAirways)
	{
		mActiveTimerWidget = mAirwaysTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		airwaysFilter->setVesselSegmentation(false);
		airwaysFilter->setAirwaySegmentation(true);
		mCurrentSegmentationType = lsAIRWAYS;
		mAirwaysProcessed = true;
	}
	else if(!mVesselsProcessed && mSegmentVessels)
	{
		mActiveTimerWidget = mVesselsTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		airwaysFilter->setVesselSegmentation(true);
		airwaysFilter->setAirwaySegmentation(false);
		mCurrentSegmentationType = lsVESSELS;
		mVesselsProcessed = true;
	}
	
	input[0]->setValue(image->getUid());
	mCurrentFilter = airwaysFilter;
	this->runAirwaysFilterSlot();
#endif
	
}

void FraxinusSegmentations::performMLSegmentation(ImagePtr image)
{
	if(!image)
		return;
	
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	//dialog.show();
	
	GenericScriptFilterPtr scriptFilter = GenericScriptFilterPtr(new GenericScriptFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = scriptFilter->getInputTypes();
	scriptFilter->getOutputTypes();
	scriptFilter->getOptions();
	
	if(mSegmentLungs && !mLungsProcessed && !this->getLungs())
	{
		mActiveTimerWidget = mLungsTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Lungs";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_Lungs.ini");
		mCurrentSegmentationType = lsLUNG;
		mLungsProcessed = true;
	}
	else if(mSegmentLymphNodes && !mLymphNodesProcessed && !this->getLymphNodes())
	{
		mActiveTimerWidget = mLymphNodesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Lymph nodes";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_LymphNodes.ini");
		mCurrentSegmentationType = lsLYMPH_NODES;
		mLymphNodesProcessed = true;
	}
	else if(mSegmentPulmonarySystem && !mPulmonarySystemProcessed && !this->getHeart())
	{
		mActiveTimerWidget = mPulmonarySystemTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting pulmonary system";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_PulmSystHeart.ini");
		mCurrentSegmentationType = lsPULMONARY_SYSTEM;
		mPulmonarySystemProcessed = true;
	}
	else if(mSegmentMediumOrgans && !mMediumOrgansProcessed && !this->getSpine())
	{
		mActiveTimerWidget = mMediumOrgansTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Vena Cava, Aorta and Spine";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_MediumOrgansMediastinum.ini");
		mCurrentSegmentationType = lsMEDIUM_ORGANS;
		mMediumOrgansProcessed = true;
	}
	else if(mSegmentSmallOrgans && !mSmallOrgansProcessed && !this->getEsophagus())
	{
		mActiveTimerWidget = mSmallOrgansTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_SmallOrgansMediastinum.ini");
		mCurrentSegmentationType = lsSMALL_ORGANS;
		mSmallOrgansProcessed = true;
	}
	else if(mSegmentNodules && !mNodulesProcessed && !this->getNodules())
	{
		mActiveTimerWidget = mNodulesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Lesions";
		scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_Nodules.ini");
		mCurrentSegmentationType = lsNODULES;
		mNodulesProcessed = true;
	}
	else
	{
		mActiveTimerWidget = NULL;
		mCurrentSegmentationType = lsUNKNOWN;
		emit segmentationFinished();
		mSegmentationProcessingInfo->close();
		return;
	} 
	
	input[0]->setValue(image->getUid());
	mCurrentFilter = scriptFilter;
	this->runMLFilterSlot();
}

void FraxinusSegmentations::runAirwaysFilterSlot()
{
	if (!mCurrentFilter)
		return;
	
	if (mThread)
	{
		reportWarning(QString("Last operation on %1 is not finished. Could not start filtering.").arg(mThread->getFilter()->getName()));
		return;
	}
	mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
	connect(mThread.get(), SIGNAL(finished()), this, SLOT(airwaysFinishedSlot()));
	mTimedAlgorithmProgressBar->attach(mThread);
	
	mThread->execute();
}

void FraxinusSegmentations::runMLFilterSlot()
{
	if (!mCurrentFilter)
		return;
	if (mThread)
	{
		reportWarning(QString("Last operation on %1 is not finished. Could not start filtering.").arg(mThread->getFilter()->getName()));
		return;
	}
	mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
	connect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
	mTimedAlgorithmProgressBar->attach(mThread);
	
	mThread->execute();
	
}

void FraxinusSegmentations::airwaysFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(airwaysFinishedSlot()));
	mThread.reset();
	//dialog.hide();
	if(mCurrentSegmentationType == lsAIRWAYS)
	{
		MeshPtr airways = this->getAirwaysContour();
		if(airways)
		{
			airways->setColor("#FFCCCC");
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
			//this->addDataToView(); //TODO: run in ProcessWorkflowState?
			QString message = "Ariway segmentation failed.\n\n"
												"Try:\n"
												"1. Click inside the airways (e.g. trachea).\n"
												"2. Select input.\n"
												"3. Select \"Use manual seed point\"\n"
												"4. Run the Airway segmantation filter again using the green start button. \n";
			QMessageBox::warning(NULL,"Airway segmentation failed", message);
		}
		if(!mVesselsProcessed && mCheckBoxVessels->isChecked())
		{
			this->performAirwaysSegmentation(this->getCTImage());
		}
		else
		{
			this->performMLSegmentation(this->getCTImage());
		}
	}
	else if(mCurrentSegmentationType == lsVESSELS)
	{
		this->checkIfSegmentationSucceeded();
		this->performMLSegmentation(this->getCTImage());
	}
}

void FraxinusSegmentations::MLFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
	mThread.reset();
	//dialog.hide();
	if(mActiveTimerWidget)
		mActiveTimerWidget->stop();
	
	this->checkIfSegmentationSucceeded();
	
	this->performMLSegmentation(this->getCTImage());
}

void FraxinusSegmentations::checkIfSegmentationSucceeded()
{
	if(mCurrentSegmentationType == lsAIRWAYS)
	{
		if(this->getAirwaysContour())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsVESSELS)
	{
		if(this->getVessels())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsLUNG)
	{
		if(this->getLungs())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsLYMPH_NODES)
	{
		if(this->getLymphNodes())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsPULMONARY_SYSTEM)
	{
		if(this->getHeart())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsMEDIUM_ORGANS)
	{
		if(this->getSpine())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsSMALL_ORGANS)
	{
		if(this->getEsophagus())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	else if(mCurrentSegmentationType == lsNODULES)
	{
		if(this->getNodules())
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->stop();
		}
		else
		{
			if(mActiveTimerWidget)
				mActiveTimerWidget->failed();
		}
	}
	
}

}//cx

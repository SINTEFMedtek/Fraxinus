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
#include "cxDataLocations.h"
#include "cxAirwaysFromCenterline.h"
#include "cxColorVariationFilter.h"
#include "cxRegistrationTransform.h"
#include "cxEnumConversion.h"


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
		if(!it->first.contains("_copy") && !it->first.contains(airwaysFilterGetNameSuffixAirways()))
		{
			image = it->second;
			break;
		}
	}
	return image;
}

ImagePtr FraxinusSegmentations::getAirwaysVolume() const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(it->first.contains(airwaysFilterGetNameSuffixAirways()))
		{
			image = it->second;
			break;
		}
	}
	return image;
}

MeshPtr FraxinusSegmentations::getRawCenterline()
{
	return this->getMesh(airwaysFilterGetNameSuffixCenterline(), "", RouteToTargetFilter::getNameSuffix(), airwaysFilterGetNameSuffixTubes());
}

MeshPtr FraxinusSegmentations::getCenterline()
{
	return this->getMesh(airwaysFilterGetNameSuffixCenterline(), airwaysFilterGetNameSuffixTubes(), RouteToTargetFilter::getNameSuffix());
}


MeshPtr FraxinusSegmentations::getAirwaysContour()
{
	return this->getMesh(airwaysFilterGetNameSuffixAirways(), "", airwaysFilterGetNameSuffixTubes(), airwaysFilterGetNameSuffixCenterline());
}

MeshPtr FraxinusSegmentations::getAirwaysTubes()
{
	return this->getMesh(airwaysFilterGetNameSuffixTubes(), airwaysFilterGetNameSuffixAirways(), airwaysFilterGetNameSuffixCenterline());
}

MeshPtr FraxinusSegmentations::getVessels()
{
	return this->getMesh(airwaysFilterGetNameSuffixVessels());
}

MeshPtr FraxinusSegmentations::getMesh(QString contain_str_1, QString contain_str_2, QString not_contain_str_1, QString not_contain_str_2)
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
		if(iter->first.contains(contain_str_1))
			if(iter->first.contains(contain_str_2))
				if(not_contain_str_1.isEmpty() || !iter->first.contains(not_contain_str_1))
					if(not_contain_str_2.isEmpty() || !iter->first.contains(not_contain_str_2))
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

MeshPtr FraxinusSegmentations::getTumors()
{
	return this->getMesh("_tumors");
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
	mCheckBoxLungs = new QCheckBox(tr("Lungs (~2 min)"));
	mCheckBoxLymphNodes = new QCheckBox(tr("Lymph Nodes (~6 min)"));
	mCheckBoxPulmonarySystem = new QCheckBox(tr("Heart, Pulmonary Veins, Pulmonary Trunk  (~20 min)"));
	mCheckBoxMediumOrgans = new QCheckBox(tr("Vena Cava, Aorta, Spine (~3 min)"));
	mCheckBoxSmallOrgans = new QCheckBox(tr("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos (~2 min)"));
	mCheckBoxNodules = new QCheckBox(tr("Lesions (~20 min)"));
	mCheckBoxTumors = new QCheckBox(tr("Tumors (~3 min)"));
	mCheckBoxVessels = new QCheckBox(tr("Small Vessels  (~1 min)"));
	
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
	checkBoxLayout->addWidget(mCheckBoxTumors);
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
	mSegmentTumors = mCheckBoxTumors->isChecked();
	mSegmentationSelectionInput->close();
	
	this->createProcessingInfo();
	ImagePtr image = this->getCTImage();
	this->performPythonSegmentation(image);
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
		gridLayout->addWidget(timerWidget,7,1);
		if(this->getNodules())
			mNodulesTimerWidget->stop();
	}
	if (mSegmentTumors)
	{
		QWidget* timerWidget = new QWidget;
		mTumorsTimerWidget = new DisplayTimerWidget(timerWidget);
		mTumorsTimerWidget->setFontSize(3);
		mTumorsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Tumors:");
		gridLayout->addWidget(label,8,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,8,1,9,3);
		if(this->getTumors())
			mTumorsTimerWidget->stop();
	}
	
	
	mSegmentationProcessingInfo->setLayout(gridLayout);
	mSegmentationProcessingInfo->show();
	mSegmentationProcessingInfo->activateWindow();
}


QString FraxinusSegmentations::getFilterScriptsPath()
{
	QString configPath = DataLocations::getRootConfigPath();
	QString retval = configPath + "/profiles/Laboratory/filter_scripts/";
	return retval;
}

void FraxinusSegmentations::performPythonSegmentation(ImagePtr image)
{
	if(!image)
		return;

	DataPtr centerline = this->getCenterline();
	DataPtr vessels = this->getVessels();
	DataPtr tumors = this->getTumors();
	if(centerline)
	{
		mAirwaysProcessed = true;
		mCenterlineProcessed = true;
		if(vessels || mVesselsProcessed || !mSegmentVessels)
		{
			if(vessels)
				mVesselsProcessed = true;
			if(tumors || mTumorsProcessed || !mSegmentTumors)
			{
				if(tumors)
					mTumorsProcessed = true;
				this->performMLSegmentation(image);
				return;
			}
		}
		else if(!mSegmentTumors)
		{
			this->performMLSegmentation(image);
			return;
		}
	}

	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	GenericScriptFilterPtr scriptFilter = GenericScriptFilterPtr(new GenericScriptFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = scriptFilter->getInputTypes();
	scriptFilter->getOutputTypes();
	scriptFilter->getOptions();

	if(!mAirwaysProcessed  && mSegmentAirways)
	{
		mActiveTimerWidget = mAirwaysTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_Airways.ini");
		mCurrentSegmentationType = lsAIRWAYS;
		mAirwaysProcessed = true;
		input[0]->setValue(image->getUid());
	}
	else if(mAirwaysProcessed && mSegmentAirways && !mCenterlineProcessed)
	{
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_AirwaysCenterline.ini");
		mCurrentSegmentationType = lsCENTERLINES;
		mCenterlineProcessed = true;
		ImagePtr airwaysVolume = this->getAirwaysVolume();
		if(airwaysVolume)
			input[0]->setValue(airwaysVolume->getUid());
	}
	else if(!mVesselsProcessed && mSegmentVessels)
	{
		mActiveTimerWidget = mVesselsTimerWidget;
		if(mActiveTimerWidget)
				mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_VesselsInLungs.ini");
		mCurrentSegmentationType = lsVESSELS;
		mVesselsProcessed = true;
		input[0]->setValue(image->getUid());
	}
	else if(!mTumorsProcessed && mSegmentTumors)
	{
		mActiveTimerWidget = mTumorsTimerWidget;
		if(mActiveTimerWidget)
				mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_Tumors.ini");
		mCurrentSegmentationType = lsTUMORS;
		mTumorsProcessed = true;
		input[0]->setValue(image->getUid());
	}
	else
		return;

	mCurrentFilter = scriptFilter;
	this->runPythonFilterSlot();
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
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_Lungs.ini");
		mCurrentSegmentationType = lsLUNG;
		mLungsProcessed = true;
	}
	else if(mSegmentLymphNodes && !mLymphNodesProcessed && !this->getLymphNodes())
	{
		mActiveTimerWidget = mLymphNodesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Lymph nodes";
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_LymphNodes.ini");
		mCurrentSegmentationType = lsLYMPH_NODES;
		mLymphNodesProcessed = true;
	}
	else if(mSegmentPulmonarySystem && !mPulmonarySystemProcessed && !this->getHeart())
	{
		mActiveTimerWidget = mPulmonarySystemTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting pulmonary system";
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_PulmSystHeart.ini");
		mCurrentSegmentationType = lsPULMONARY_SYSTEM;
		mPulmonarySystemProcessed = true;
	}
	else if(mSegmentMediumOrgans && !mMediumOrgansProcessed && !this->getSpine())
	{
		mActiveTimerWidget = mMediumOrgansTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Vena Cava, Aorta and Spine";
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_MediumOrgansMediastinum.ini");
		mCurrentSegmentationType = lsMEDIUM_ORGANS;
		mMediumOrgansProcessed = true;
	}
	else if(mSegmentSmallOrgans && !mSmallOrgansProcessed && !this->getEsophagus())
	{
		mActiveTimerWidget = mSmallOrgansTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos";
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_SmallOrgansMediastinum.ini");
		mCurrentSegmentationType = lsSMALL_ORGANS;
		mSmallOrgansProcessed = true;
	}
	else if(mSegmentNodules && !mNodulesProcessed && !this->getNodules())
	{
		mActiveTimerWidget = mNodulesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_DEBUG() << "Segmenting Lesions";
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_Nodules.ini");
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

void FraxinusSegmentations::runPythonFilterSlot()
{
	if (!mCurrentFilter)
		return;
	
	if (mThread)
	{
		reportWarning(QString("Last operation on %1 is not finished. Could not start filtering.").arg(mThread->getFilter()->getName()));
		return;
	}
	mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
	connect(mThread.get(), SIGNAL(finished()), this, SLOT(pythonFinishedSlot()));
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

void FraxinusSegmentations::pythonFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(pythonFinishedSlot()));
	mThread.reset();
	//dialog.hide();
	if(mCurrentSegmentationType == lsAIRWAYS)
	{
		MeshPtr airways = this->getAirwaysContour();
		if(airways)
		{
			airways->setColor("#FFCCCC");
		}
//		else
//		{
//			if(mActiveTimerWidget)
//				mActiveTimerWidget->failed();
//			//this->addDataToView(); //TODO: run in ProcessWorkflowState?
//			QString message = "Ariway segmentation failed.\n\n"
//												"Try:\n"
//												"1. Click inside the airways (e.g. trachea).\n"
//												"2. Select input.\n"
//												"3. Select \"Use manual seed point\"\n"
//												"4. Run the Airway segmantation filter again using the green start button. \n";
//			QMessageBox::warning(NULL,"Airway segmentation failed", message);
//		}
	}
	else if(mCurrentSegmentationType == lsCENTERLINES)
	{
		this->postProcessAirways();
		this->checkIfSegmentationSucceeded();
	}
	else
		this->checkIfSegmentationSucceeded();



	if(mCurrentSegmentationType == lsAIRWAYS)
		this->performPythonSegmentation(this->getCTImage());
	else if(mCurrentSegmentationType == lsCENTERLINES && (mSegmentVessels || mSegmentTumors))
		this->performPythonSegmentation(this->getCTImage());
	else if(mCurrentSegmentationType == lsVESSELS && mSegmentTumors)
		this->performPythonSegmentation(this->getCTImage());
	else
		this->performMLSegmentation(this->getCTImage());

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

void FraxinusSegmentations::postProcessAirways()
{
	AirwaysFromCenterlinePtr airwaysFromCLPtr = AirwaysFromCenterlinePtr(new AirwaysFromCenterline());

	ImagePtr CTimage = this->getCTImage();
	if(!CTimage)
		return;
	MeshPtr rawCenterline = this->getRawCenterline();
	if(!rawCenterline)
		return;
	ImagePtr airwaysVolume =this->getAirwaysVolume();
	if(!airwaysVolume)
		return;

	airwaysFromCLPtr->processCenterline(rawCenterline->getVtkPolyData());
	airwaysFromCLPtr->setSegmentedVolume(airwaysVolume->getBaseVtkImageData());

	// Create mesh object from the airway walls
	QString uidMesh = CTimage->getUid() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	QString nameMesh = CTimage->getName() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	MeshPtr airwayWalls = mServices->patient()->createSpecificData<Mesh>(uidMesh, nameMesh);
	airwayWalls->setColor(QColor(253, 173, 136, 255));
	airwayWalls->setVtkPolyData(airwaysFromCLPtr->generateTubes(0, true));
	airwayWalls->get_rMd_History()->setParentSpace(CTimage->getUid());
	airwayWalls->get_rMd_History()->setRegistration(CTimage->get_rMd());
	//mServices->patient()->insertData(airwayWalls);

	//Apply color varition
	VisServicesPtr visServices = boost::static_pointer_cast<VisServices>(mServices);
	ColorVariationFilterPtr coloringFilter = ColorVariationFilterPtr(new ColorVariationFilter(visServices));
	double globaleVariance = 50.0;
	double localeVariance = 5.0;
	int smoothingIterations = 5;
	airwayWalls = coloringFilter->execute(airwayWalls, globaleVariance, localeVariance, smoothingIterations);
	setMeshName(airwayWalls, lsAIRWAYS);

	//insert filtered centerline from airwaysFromCenterline
	QString uidCenterline = CTimage->getUid() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes() + airwaysFilterGetNameSuffixCenterline();
	QString nameCenterline = CTimage->getName() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes() + airwaysFilterGetNameSuffixCenterline();
	MeshPtr centerline = mServices->patient()->createSpecificData<Mesh>(uidCenterline, nameCenterline);
	centerline->setVtkPolyData(airwaysFromCLPtr->getVTKPoints());
	centerline->get_rMd_History()->setParentSpace(CTimage->getUid());
	centerline->get_rMd_History()->setRegistration(CTimage->get_rMd());
	mServices->patient()->insertData(centerline);

	mServices->patient()->removeData(airwaysVolume->getUid());
}

void FraxinusSegmentations::checkIfSegmentationSucceeded()
{
	if(mCurrentSegmentationType == lsAIRWAYS)
	{
		setMeshName(this->getAirwaysTubes(), lsAIRWAYS);
		// Not stopping timer before centerlines are created
	}
	else if(mCurrentSegmentationType == lsCENTERLINES)
	{
		setMeshNameAndStopTimer(this->getCenterline());
	}
	else if(mCurrentSegmentationType == lsVESSELS)
	{
		setMeshNameAndStopTimer(this->getVessels());
	}
	else if(mCurrentSegmentationType == lsLUNG)
	{
		setMeshNameAndStopTimer(this->getLungs());
	}
	else if(mCurrentSegmentationType == lsLYMPH_NODES)
	{
		setMeshNameAndStopTimer(this->getLymphNodes());
	}
	else if(mCurrentSegmentationType == lsPULMONARY_SYSTEM)
	{
		stopTimer(this->getHeart());
		setMeshName(this->getHeart(), lsHEART);
		setMeshName(this->getPulmonaryVeins(), lsPULMONARY_VESSELS);
		setMeshName(this->getPulmonaryTrunk(), lsPULMONARY_SYSTEM);
	}
	else if(mCurrentSegmentationType == lsMEDIUM_ORGANS)
	{
		stopTimer(this->getSpine());
		setMeshName(this->getAorticArch(), lsAORTA);
		setMeshName(this->getDescendingAorta(), lsAORTA);
		setMeshName(this->getAscendingAorta(), lsAORTA);
		setMeshName(this->getSpine(), lsSPINE);
		setMeshName(this->getVenaCava(), lsVENA_CAVA);
	}
	else if(mCurrentSegmentationType == lsSMALL_ORGANS)
	{
		stopTimer(this->getEsophagus());
		setMeshName(this->getEsophagus(), lsESOPHAGUS);
		setMeshName(this->getSubCarArt(), lsSUBCLAVIAN_ARTERY);
		setMeshName(this->getBrachiocephalicVeins(), lsVENA_CAVA);//lsPULMONARY_VESSELS
		setMeshName(this->getAzygos(), lsVENA_AZYGOS);
	}
	else if(mCurrentSegmentationType == lsNODULES)
	{
		setMeshNameAndStopTimer(this->getNodules());
	}
	else if(mCurrentSegmentationType == lsTUMORS)
	{
		setMeshNameAndStopTimer(this->getTumors());
	}
}


void FraxinusSegmentations::setMeshNameAndStopTimer(MeshPtr mesh)
{
	stopTimer(mesh);
	setMeshName(mesh, mCurrentSegmentationType);
}

void FraxinusSegmentations::setMeshName(MeshPtr mesh, LUNG_STRUCTURES segmentationType)
{
	if(mesh)
	{
		//CX_LOG_DEBUG() << "Set name on structure: " << enum2string(segmentationType);
		mesh->setName(enum2string(segmentationType));
	}
	else
		CX_LOG_DEBUG() << "Found no segmentation for: " << enum2string(segmentationType);

}

void FraxinusSegmentations::stopTimer(MeshPtr mesh)
{
	if(mActiveTimerWidget)
	{
		if(mesh)
			mActiveTimerWidget->stop();
		else
			mActiveTimerWidget->failed();
	}
}
}//cx

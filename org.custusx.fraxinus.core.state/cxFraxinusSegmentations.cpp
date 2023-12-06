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
#include "cxBinaryThinningImageFilter3DFilter.h"
#include "cxBranchList.h"

#include "cxElastixParameters.h"
#include "cxElastixExecuter.h"
#include "cxFilePathProperty.h"
#include "cxRegistrationService.h"


namespace cx
{

FraxinusSegmentations::FraxinusSegmentations(RegServicesPtr services) :
	mServices(services)
{
	mTimedAlgorithmProgressBar = new cx::TimedAlgorithmProgressBar;
}

FraxinusSegmentations::~FraxinusSegmentations()
{
	
}

void FraxinusSegmentations::close()
{
	if(!mSegmentationSelectionInput)
		return;
	disconnect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &FraxinusSegmentations::checkForPETData);
	disconnect(mCheckBoxSelectAll, &QCheckBox::toggled, this, &FraxinusSegmentations::selectAll);
	disconnect(mOKbutton, &QPushButton::clicked, this, &FraxinusSegmentations::imageSelected);
	disconnect(mCancelbutton, &QPushButton::clicked, this, &FraxinusSegmentations::cancel);
	mSegmentationSelectionInput->close();
	mSegmentationSelectionInput = nullptr;
}

ImagePtr FraxinusSegmentations::getImage(IMAGE_MODALITY modality, IMAGE_SUBTYPE subtype) const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if((it->second->getModality() == modality) && (it->second->getImageType() == subtype))
		{
			image = it->second;
			return image;
		}
	}
	return image;
}

ImagePtr FraxinusSegmentations::findAndLabelThoraxCT() const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(		 !it->first.contains("_copy")
				&& !it->first.contains(airwaysFilterGetNameSuffixAirways())
				&& !it->first.contains(airwaysFilterGetNameSuffixLungs())
				&& !(it->second->getOrganType() == otLUNGS)
				&& !(it->second->getOrganType() == otAIRWAYS)
				&& ((it->second->getModality() == imCT) || (it->second->getModality() == imUNKNOWN) || (it->second->getModality() == imCOUNT))
			 )
		{
			image = it->second;
			image->setModality(imCT);
			image->setImageType(istTHORAX_CT);
			break;
		}
	}
	return image;
}

BranchListPtr FraxinusSegmentations::getBranchList()
{
	return mBranchList;
}

ImagePtr FraxinusSegmentations::getVolume(ORGAN_TYPE organType) const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(it->second->getOrganType() == organType)
		{
			image = it->second;
			break;
		}
	}
	return image;
}

MeshPtr FraxinusSegmentations::getMesh(ORGAN_TYPE organType)
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
		if(iter->second->getOrganType() == organType)
		{
			return iter->second;
		}
	return MeshPtr();
}

void FraxinusSegmentations::createSelectSegmentationBox()
{
	if(mActiveTimerWidget) // check that segmentation is not already running
		return;
	mSegmentationSelectionInput = new QDialog();
	mSegmentationSelectionInput->setWindowTitle(tr("Select structures for segmentation"));
	mSegmentationSelectionInput->setWindowFlags(Qt::WindowStaysOnTopHint);
	
	mCheckBoxAirways = new QCheckBox(tr("Airways, Lungs (~7 min)"));
	mCheckBoxAirways->setChecked(true);
	mCheckBoxAirways->setDisabled(true);
	mCheckBoxLymphNodes = new QCheckBox(tr("Lymph Nodes (~2 min)"));
	mCheckBoxHeart = new QCheckBox(tr("Heart, Pulmonary Veins, Pulmonary Trunk  (~4 min)"));
	mCheckBoxMediumOrgans = new QCheckBox(tr("Vena Cava, Aorta, Spine (~3 min)"));
	mCheckBoxSmallOrgans = new QCheckBox(tr("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos (~2 min)"));
	mCheckBoxNodules = new QCheckBox(tr("Nodules (~2 min)"));
	mCheckBoxTumors = new QCheckBox(tr("Tumors (~3 min)"));
	mCheckBoxPET = new QCheckBox(tr("PET to CT (~2 min)"));
	this->checkForPETData();
	//mCheckBoxLungVessels = new QCheckBox(tr("Small Vessels  (<1 min)"));
	mCheckBoxSelectAll = new QCheckBox(tr("Select all"));

	connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &FraxinusSegmentations::checkForPETData);
	connect(mCheckBoxSelectAll, &QCheckBox::toggled, this, &FraxinusSegmentations::selectAll);
	
	mOKbutton = new QPushButton(tr("&OK"));
	mCancelbutton = new QPushButton(tr("&Cancel"));
	
	connect(mOKbutton, &QPushButton::clicked, this, &FraxinusSegmentations::imageSelected);
	connect(mCancelbutton, &QPushButton::clicked, this, &FraxinusSegmentations::cancel);
	
	QVBoxLayout* checkBoxLayout = new QVBoxLayout;
	checkBoxLayout->addWidget(mCheckBoxAirways);
	checkBoxLayout->addWidget(mCheckBoxLymphNodes);
	checkBoxLayout->addWidget(mCheckBoxHeart);
	checkBoxLayout->addWidget(mCheckBoxMediumOrgans);
	checkBoxLayout->addWidget(mCheckBoxSmallOrgans);
	checkBoxLayout->addWidget(mCheckBoxNodules);
	checkBoxLayout->addWidget(mCheckBoxTumors);
	checkBoxLayout->addWidget(mCheckBoxPET);
	//checkBoxLayout->addWidget(mCheckBoxLungVessels);
	checkBoxLayout->addWidget(mCheckBoxSelectAll);
	
	QGridLayout* mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addLayout(checkBoxLayout, 0, 0);
	mainLayout->addWidget(mCancelbutton, 1, 1);
	mainLayout->addWidget(mOKbutton, 1, 2);
	
	mSegmentationSelectionInput->setLayout(mainLayout);
	mSegmentationSelectionInput->show();
	mSegmentationSelectionInput->activateWindow();
}

void FraxinusSegmentations::checkForPETData()
{
	if(!( getImage(imCT, istTHORAX_CT) && getImage(imCT, istPET_CT) && getImage(imPET, istPET) ))
		mCheckBoxPET->setDisabled(true);
	else
		mCheckBoxPET->setDisabled(false);
}

void FraxinusSegmentations::selectAll(bool checked)
{
	mCheckBoxLymphNodes->setChecked(checked);
	mCheckBoxHeart->setChecked(checked);
	mCheckBoxMediumOrgans->setChecked(checked);
	mCheckBoxSmallOrgans->setChecked(checked);
	mCheckBoxNodules->setChecked(checked);
	mCheckBoxTumors->setChecked(checked);
	if(mCheckBoxPET->isEnabled())
		mCheckBoxPET->setChecked(checked);
	//mCheckBoxLungVessels->setChecked(checked);
}

void FraxinusSegmentations::imageSelected()
{
	mSegmentAirways = mCheckBoxAirways->isChecked();
	//mSegmentLungVessels = mCheckBoxLungVessels->isChecked();
	mSegmentLungVessels = false;
	mSegmentLymphNodes = mCheckBoxLymphNodes->isChecked();
	mSegmentHeart = mCheckBoxHeart->isChecked();
	mSegmentMediumOrgans = mCheckBoxMediumOrgans->isChecked();
	mSegmentSmallOrgans = mCheckBoxSmallOrgans->isChecked();
	mSegmentNodules = mCheckBoxNodules->isChecked();
	mSegmentTumors = mCheckBoxTumors->isChecked();
	mRegisterPET = mCheckBoxPET->isChecked();
	this->close();

	this->createProcessingInfo();

	ImagePtr image = this->getImage(imCT, istTHORAX_CT);

	if(mRegisterPET)
		this->performPETCTregistration();
	else
		this->performPythonSegmentation(image);
}

void FraxinusSegmentations::cancel()
{
	this->close();
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
		QLabel* label = new QLabel("Airways, Lungs:");
		gridLayout->addWidget(label,0,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,0,1);
		if(this->getMesh(otAIRWAYS_CENTERLINES))
			mAirwaysTimerWidget->stop();
	}
	if (mSegmentLungVessels)
	{
		QWidget* timerWidget = new QWidget;
		mLungVesselsTimerWidget = new DisplayTimerWidget(timerWidget);
		mLungVesselsTimerWidget->setFontSize(3);
		mLungVesselsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Small Vessels:");
		gridLayout->addWidget(label,1,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,1,1);
		if(this->getMesh(otLUNG_VESSELS))
			mLungVesselsTimerWidget->stop();
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
		if(this->getMesh(otLYMPH_NODES))
			mLymphNodesTimerWidget->stop();
	}
	if (mSegmentHeart)
	{
		QWidget* timerWidget = new QWidget;
		mHeartTimerWidget = new DisplayTimerWidget(timerWidget);
		mHeartTimerWidget->setFontSize(3);
		mHeartTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Pulmonary System:");
		gridLayout->addWidget(label,4,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,4,1);
		if(this->getMesh(otHEART))
			mHeartTimerWidget->stop();
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
		if(this->getMesh(otSPINE))
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
		if(this->getMesh(otESOPHAGUS))
			mSmallOrgansTimerWidget->stop();
	}
	if (mSegmentNodules)
	{
		QWidget* timerWidget = new QWidget;
		mNodulesTimerWidget = new DisplayTimerWidget(timerWidget);
		mNodulesTimerWidget->setFontSize(3);
		mNodulesTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Nodules:");
		gridLayout->addWidget(label,7,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,7,1);
		if(this->getMesh(otNODULES))
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
		gridLayout->addWidget(timerWidget,8,1);
		if(this->getMesh(otTUMORS))
			mTumorsTimerWidget->stop();
	}
	if (mRegisterPET)
	{
		QWidget* timerWidget = new QWidget;
		mPETTimerWidget = new DisplayTimerWidget(timerWidget);
		mPETTimerWidget->setFontSize(3);
		mPETTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("PET:");
		gridLayout->addWidget(label,9,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,9,1,10,3);
		if(this->getImage(imPET, istPET_REGISTERED))
			mPETTimerWidget->stop();
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

	DataPtr vessels = this->getMesh(otLUNG_VESSELS);
	DataPtr tumors = this->getMesh(otTUMORS);
	if(vessels || mLungVesselsProcessed || !mSegmentLungVessels)
	{
		if(vessels)
			mLungVesselsProcessed = true;
		if(tumors || mTumorsProcessed || !mSegmentTumors)
		{
			if(tumors)
				mTumorsProcessed = true;
			this->performMLSegmentation(image);
			return;
		}
	}

	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	GenericScriptFilterPtr scriptFilter = GenericScriptFilterPtr(new GenericScriptFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = scriptFilter->getInputTypes();
	scriptFilter->getOutputTypes();
	scriptFilter->getOptions();

	if(!mLungVesselsProcessed && mSegmentLungVessels)
	{
		mActiveTimerWidget = mLungVesselsTimerWidget;
		if(mActiveTimerWidget)
				mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(getFilterScriptsPath() + "python_VesselsInLungs.ini");
		mCurrentSegmentationType = lsLUNG_VESSELS;
		mLungVesselsProcessed = true;
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

QStringList FraxinusSegmentations::getRaidionicsOutputClasses(bool startTimers)
{
	QStringList retval;

	if(mSegmentAirways && !this->getMesh(otAIRWAYS_CENTERLINES))
	{
		mActiveTimerWidget = mAirwaysTimerWidget;
		retval << enum2string(otAIRWAYS);
		if(!this->getMesh(otLUNGS))
			retval << enum2string(otLUNGS);
		if(startTimers)
			mAirwaysTimerWidget->start();
	}
	if(mSegmentLymphNodes && !this->getMesh(otLYMPH_NODES))
	{
		retval << enum2string(otLYMPH_NODES);
		if(startTimers)
			mLymphNodesTimerWidget->start();
	}

	//Multiple targets, will be expanded in Raidionics::createTargetList()
	if(mSegmentHeart && !this->getMesh(otHEART))
	{
		retval << enum2string(lmPULMSYST_HEART);
		if(startTimers)
			mHeartTimerWidget->start();
	}
	if(mSegmentMediumOrgans && !this->getMesh(otVENA_CAVA))
	{
		retval << enum2string(lmMEDIUM_ORGANS_MEDIASTINUM);
		if(startTimers)
			mMediumOrgansTimerWidget->start();
	}
	if(mSegmentSmallOrgans && !this->getMesh(otAZYGOS))
	{
		retval << enum2string(lmSMALL_ORGANS_MEDIASTINUM);
		if(startTimers)
			mSmallOrgansTimerWidget->start();
	}

	return retval;
}

bool FraxinusSegmentations::runRaidionics(GenericScriptFilterPtr scriptFilter)
{
	if(mRaidionicsRun)
		return false;

	QStringList outputClasses = getRaidionicsOutputClasses();
	if(outputClasses.isEmpty())
		return false;
	scriptFilter->setParameterFilePath(getFilterScriptsPath() + "raidionics_LungAll.ini");
	scriptFilter->setOutputClasses(outputClasses);
	mCurrentSegmentationType = lsAIRWAYS;

	mRaidionicsRun = true;
	return true;
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

	if(runRaidionics(scriptFilter))
	{}
	else if(mSegmentNodules && !mNodulesProcessed && !this->getMesh(otNODULES))
	{
		mActiveTimerWidget = mNodulesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_INFO() << "Segmenting Nodules";
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

void FraxinusSegmentations::performPETCTregistration()
{
	if(this->getImage(imPET, istPET_REGISTERED))
	{
		ImagePtr CTimage = this->getImage(imCT, istTHORAX_CT);
		this->performPythonSegmentation(CTimage);
		return;
	}

	ImagePtr CTimage = this->getImage(imCT, istTHORAX_CT);
	ImagePtr PET_CTimage = this->getImage(imCT, istPET_CT);

	mActiveTimerWidget = mPETTimerWidget;
	if(mActiveTimerWidget)
		mActiveTimerWidget->start();

	//NB: Elastix creates (modified) copies of PETimage and PET_CTimage
	//Setting Image Type to istPET_REGISTERED for new PET volume in ElastixManager::addNonlinearData()

	PET_CTimage->get_rMd_History()->setParentSpace(""); //Make sure we don't move any other images
	mServices->registration()->setFixedData(CTimage);
	mServices->registration()->setMovingData(PET_CTimage);

	this->setElastixParameters();

	runElastixSlot();
}

void FraxinusSegmentations::setElastixParameters()
{
	ImagePtr PETimage = this->getImage(imPET, istPET);
	PETimage->get_rMd_History()->setParentSpace("");

	mElastixManager = ElastixManagerPtr(new ElastixManager(mServices));

	ElastixParametersPtr elastixParameters = mElastixManager->getParameters();
	elastixParameters->setDeformImage(PETimage->getUid());

	elastixParameters->getActiveParameterFile0()->setValue("elastix/par/p_Rigid.txt");
	elastixParameters->getActiveParameterFile1()->setValue("elastix/par/p_BSpline.txt");
	elastixParameters->getActiveParameterFile2()->setValue("elastix/par/p_BSpline.txt");
}

void FraxinusSegmentations::runElastixSlot()
{
	mTimedAlgorithmProgressBar->attach(mElastixManager->getExecuter());
	connect(mElastixManager->getExecuter().get(), &TimedBaseAlgorithm::finished, this, &FraxinusSegmentations::elastixFinishedSlot);
	mElastixManager->execute();
}

void FraxinusSegmentations::elastixFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mElastixManager->getExecuter().get(), &TimedBaseAlgorithm::finished, this, &FraxinusSegmentations::elastixFinishedSlot);

	mPETTimerWidget->stop();

	ImagePtr CTimage = this->getImage(imCT, istTHORAX_CT);
	this->performPythonSegmentation(CTimage);
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
	if(mCurrentSegmentationType == lsAIRWAYS)
	{
		MeshPtr airways = this->getMesh(otAIRWAYS);
		if(airways)
		{
			airways->setColor("#FFCCCC");
		}
	}
	else if(mCurrentSegmentationType == lsCENTERLINES)
	{
		this->postProcessAirways();
		this->checkIfSegmentationSucceeded();
	}
	else
		this->checkIfSegmentationSucceeded();



	if(mCurrentSegmentationType == lsAIRWAYS)
		this->performPythonSegmentation(this->getImage(imCT, istTHORAX_CT));
	else if(mCurrentSegmentationType == lsCENTERLINES && (mSegmentLungVessels || mSegmentTumors))
		this->performPythonSegmentation(this->getImage(imCT, istTHORAX_CT));
	else if(mCurrentSegmentationType == lsLUNG_VESSELS && mSegmentTumors)
		this->performPythonSegmentation(this->getImage(imCT, istTHORAX_CT));
	else
		this->performMLSegmentation(this->getImage(imCT, istTHORAX_CT));

}

void FraxinusSegmentations::MLFinishedSlot()
{
	if(mCurrentSegmentationType == lsAIRWAYS && !this->getMesh(otAIRWAYS_CENTERLINES))
		this->postProcessAirways();

	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
	mThread.reset();
	//dialog.hide();
	if(mActiveTimerWidget)
		mActiveTimerWidget->stop();
	
	this->checkIfSegmentationSucceeded();
	
	this->performMLSegmentation(getImage(imCT, istTHORAX_CT));
}

void FraxinusSegmentations::postProcessAirways()
{
	this->generateCenterline();
	AirwaysFromCenterlinePtr airwaysFromCLPtr = AirwaysFromCenterlinePtr(new AirwaysFromCenterline());
	ImagePtr CTimage = this->getImage(imCT, istTHORAX_CT);
	if(!CTimage)
		return;
	MeshPtr rawCenterline = this->getMesh(otCENTERLINES);
	if(!rawCenterline)
		return;
	ImagePtr airwaysVolume =this->getVolume(otAIRWAYS);
	if(!airwaysVolume)
		return;

	airwaysFromCLPtr->processCenterline(rawCenterline->getVtkPolyData());
	airwaysFromCLPtr->setSegmentedVolume(airwaysVolume->getBaseVtkImageData(), airwaysVolume->get_rMd());

	mBranchList = airwaysFromCLPtr->getBranchList();

	mBranchList->setRadius(airwaysVolume);

	// Create mesh object from the airway walls
	QString uidMesh = CTimage->getUid() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	QString nameMesh = CTimage->getName() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	MeshPtr airwayWalls = mServices->patient()->createSpecificData<Mesh>(uidMesh, nameMesh);
	airwayWalls->setColor(QColor(253, 173, 136, 255));
	airwayWalls->setVtkPolyData(airwaysFromCLPtr->generateTubes(0, true));
	airwayWalls->get_rMd_History()->setParentSpace(CTimage->getUid());
	airwayWalls->get_rMd_History()->setRegistration(CTimage->get_rMd());
	setMeshNameAndType(airwayWalls, otAIRWAYS_ENHANCED);

	//Apply color varition
	VisServicesPtr visServices = boost::static_pointer_cast<VisServices>(mServices);
	ColorVariationFilterPtr coloringFilter = ColorVariationFilterPtr(new ColorVariationFilter(visServices));
	double globaleVariance = 50.0;
	double localeVariance = 5.0;
	int smoothingIterations = 5;
	//NB: New object for airwayWalls
	airwayWalls = coloringFilter->execute(airwayWalls, globaleVariance, localeVariance, smoothingIterations);
	setMeshNameAndType(airwayWalls, otAIRWAYS_ENHANCED);

	//create copied airways
	QString uidAirwayWallsCopy = airwayWalls->getUid() + airwaysFilterGetNameSuffixCopy();
	QString nameAirwayWallsCopy = airwayWalls->getUid() + airwaysFilterGetNameSuffixCopy();
	MeshPtr airwayWallsCopy = mServices->patient()->createSpecificData<Mesh>(uidAirwayWallsCopy, nameAirwayWallsCopy);
	airwayWallsCopy->setVtkPolyData(airwayWalls->getVtkPolyData());
	airwayWallsCopy->setColor(airwayWalls->getColor());
	airwayWallsCopy->get_rMd_History()->setParentSpace(airwayWalls->getUid());
	airwayWallsCopy->get_rMd_History()->setRegistration(airwayWalls->get_rMd());
	setMeshNameAndType(airwayWallsCopy, otAIRWAYS_ENHANCED_COPY);
	mServices->patient()->insertData(airwayWallsCopy, true);

	//insert filtered centerline from airwaysFromCenterline
	QString uidCenterline = CTimage->getUid() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes() + airwaysFilterGetNameSuffixCenterline();
	QString nameCenterline = CTimage->getName() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes() + airwaysFilterGetNameSuffixCenterline();
	MeshPtr centerline = mServices->patient()->createSpecificData<Mesh>(uidCenterline, nameCenterline);
	centerline->setVtkPolyData(airwaysFromCLPtr->getVTKPoints());
	centerline->get_rMd_History()->setParentSpace(rawCenterline->getUid());
	centerline->get_rMd_History()->setRegistration(rawCenterline->get_rMd());
	setMeshNameAndType(centerline, otAIRWAYS_CENTERLINES);
	mServices->patient()->insertData(centerline);

	ImagePtr lungsVolume = getVolume(otLUNGS);
	if(lungsVolume)
		mServices->patient()->removeData(lungsVolume->getUid());
//	if(airwaysVolume) // do not remove, needed for airway radius calculation at Fraxinus restart
//		mServices->patient()->removeData(airwaysVolume->getUid());
}

void FraxinusSegmentations::generateCenterline()
{//using BinaryThinningImageFilter3DFilter
	VisServicesPtr visServices = boost::static_pointer_cast<VisServices>(mServices);
	BinaryThinningImageFilter3DFilterPtr binaryThinningImageFilter3DFilter =  BinaryThinningImageFilter3DFilterPtr(new BinaryThinningImageFilter3DFilter(visServices));
	std::vector<SelectDataStringPropertyBasePtr> input = binaryThinningImageFilter3DFilter->getInputTypes();
	std::vector<SelectDataStringPropertyBasePtr> output = binaryThinningImageFilter3DFilter->getOutputTypes();
	binaryThinningImageFilter3DFilter->getOptions();
	ImagePtr airwaysVolume = getVolume(otAIRWAYS);
	if(!airwaysVolume)
	{
		CX_LOG_WARNING() << "In FraxinusSegmentations::generateCenterline airways volume not found.";
		return;
	}
	input[0]->setValue(airwaysVolume->getUid());

	binaryThinningImageFilter3DFilter->preProcess();
	if(binaryThinningImageFilter3DFilter->execute())
	{
		if(binaryThinningImageFilter3DFilter->postProcess())
		{
			if(!output[0])
				return;

			MeshPtr centerline = mServices->patient()->getData<Mesh>(output[0]->getValue());
			setMeshNameAndType(centerline, otCENTERLINES);
			centerline->setColor(QColor(255,255,0,255));

			return;
		}
	}
	CX_LOG_WARNING() << "In FraxinusSegmentations::generateCenterline BinaryThinningImageFilter3DFilter failed.";
}

void FraxinusSegmentations::checkIfSegmentationSucceeded()
{
	if(mCurrentSegmentationType == lsAIRWAYS)
	{//Running Raidionics. Need to handle all meshes and timers
		// Not stopping timer before centerlines are created
		if(mSegmentAirways)
		{
			setMeshNameAndStopTimer(otAIRWAYS);
			setMeshName(otLUNGS);
		}
		if(mSegmentLymphNodes)
			setMeshNameAndStopTimer(otLYMPH_NODES);
		if(mSegmentHeart)
		{
			stopTimer(otHEART);
			setMeshName(otHEART);
			setMeshName(otPULMONARY_VEINS);
			setMeshName(otPULMONARY_TRUNK);
		}
		if(mSegmentMediumOrgans)
		{
			stopTimer(otSPINE);
			setMeshName(otAORTIC_ARCH);
			setMeshName(otDESCENDING_AORTA);
			setMeshName(otASCENDING_AORTA);
			setMeshName(otSPINE);
			setMeshName(otVENA_CAVA);
		}
		if(mSegmentSmallOrgans)
		{
			stopTimer(otESOPHAGUS);
			setMeshName(otESOPHAGUS);
			setMeshName(otSUBCLAVIAN_ARTERY);
			setMeshName(otBRACHIO_CEPHALIC_VEINS);
			setMeshName(otAZYGOS);
		}
	}
	else if(mCurrentSegmentationType == lsNODULES)
	{
		setMeshNameAndStopTimer(otNODULES);
	}
	else if(mCurrentSegmentationType == lsTUMORS)
	{
		setMeshNameAndStopTimer(otTUMORS);
	}
	mServices->patient()->autoSave();
}


void FraxinusSegmentations::setMeshNameAndStopTimer(ORGAN_TYPE target)
{
	stopTimer(target);
	setMeshName(target);
}

void FraxinusSegmentations::setMeshNameAndType(MeshPtr mesh, ORGAN_TYPE target)
{
		mesh->setOrganType(target);
		mesh->setName(convertToReadableString(target));
}

//Needs to be called after patient()->insertData to work
void FraxinusSegmentations::setMeshName(ORGAN_TYPE target)
{
	MeshPtr mesh = this->getMesh(target);
	if(mesh)
		this->setMeshNameAndType(mesh, target);
	else
		CX_LOG_WARNING() << "FraxinusSegmentations::setMeshName: Found no segmentation for: " << enum2string(target);
}

void FraxinusSegmentations::stopTimer(ORGAN_TYPE target)
{
	MeshPtr mesh = this->getMesh(target);
	DisplayTimerWidget* timer = this->getTimer(target);
	if(timer)
	{
		if(mesh)
			timer->stop();
		else
			timer->failed();
	}
}

DisplayTimerWidget* FraxinusSegmentations::getTimer(ORGAN_TYPE target)
{
	DisplayTimerWidget* timer = nullptr;

	switch(target)
	{
	case otAIRWAYS:
		timer = mAirwaysTimerWidget; break;
	case otLYMPH_NODES:
		timer = mLymphNodesTimerWidget; break;
	case otHEART:
	case otPULMONARY_VEINS:
	case otPULMONARY_TRUNK:
		timer = mHeartTimerWidget; break;
	case otVENA_CAVA:
	case otAORTIC_ARCH:
	case otASCENDING_AORTA:
	case otDESCENDING_AORTA:
	case otSPINE:
		timer = mMediumOrgansTimerWidget; break;
	case otBRACHIO_CEPHALIC_VEINS:
	case otSUBCLAVIAN_ARTERY:
	case otAZYGOS:
	case otESOPHAGUS:
		timer = mSmallOrgansTimerWidget; break;
	case otTUMORS:
		timer = mTumorsTimerWidget; break;
	case otNODULES:
		timer = mNodulesTimerWidget; break;
	default:
		timer = nullptr; break;
	}

	return timer;
}

}//cx

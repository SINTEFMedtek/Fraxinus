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
#include <vtkImageBlend.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkImageShiftScale.h>
#include <boost/math/special_functions/round.hpp>
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
#include "cxSessionStorageService.h"

#include "cxElastixParameters.h"
#include "cxElastixExecuter.h"
#include "cxFilePathProperty.h"
#include "cxRegistrationService.h"
#include "cxIslandsFilter.h"
#include "cxMeshesFromLabelsFilter.h"
#include "cxVolumeHelpers.h"

namespace cx
{

FraxinusSegmentations::FraxinusSegmentations(RegServicesPtr services) :
	mServices(services)
{
	mTimedAlgorithmProgressBar = new cx::TimedAlgorithmProgressBar;
	connect(mServices->session().get(), &SessionStorageService::sessionChanged, this, &FraxinusSegmentations::patientChanged, Qt::UniqueConnection);
}

FraxinusSegmentations::~FraxinusSegmentations()
{
	disconnect(mServices->session().get(), &SessionStorageService::sessionChanged, this, &FraxinusSegmentations::patientChanged);
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

void FraxinusSegmentations::patientChanged()
{
	if(!mServices->session()->isValid())
		return;

	mBranchList.reset();

	mAirwaysProcessed = false;
	mLungVesselsProcessed = false;
	mNodulesProcessed = false;
	mTumorsProcessed = false;
	mLymphNodesProcessed = false;
	mHeartProcessed = false;
	mMediumOrgansProcessed = false;
	mSmallOrgansProcessed = false;
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
				&& !((it->second->getImageType() == istPET) || (it->second->getImageType() == istPET_CT))
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

void FraxinusSegmentations::createSelectSegmentationBox()
{
	if(mActiveTimerWidget) // check that segmentation is not already running
		return;
	if(!mSegmentationSelectionInput)
		mSegmentationSelectionInput = new QDialog();
	mSegmentationSelectionInput->setWindowTitle(tr("Select structures for segmentation"));
	mSegmentationSelectionInput->setWindowFlags(Qt::WindowStaysOnTopHint);
	
	updateSelectSegmentationBox();

	connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &FraxinusSegmentations::checkForPETData);
	connect(mCheckBoxSelectAll, &QCheckBox::toggled, this, &FraxinusSegmentations::selectAll);
	
	if(!mOKbutton)
		mOKbutton = new QPushButton(tr("&OK"));
	if(!mCancelbutton)
		mCancelbutton = new QPushButton(tr("&Cancel"));
	
	connect(mOKbutton, &QPushButton::clicked, this, &FraxinusSegmentations::imageSelected);
	connect(mCancelbutton, &QPushButton::clicked, this, &FraxinusSegmentations::cancel);
	
	QVBoxLayout* checkBoxLayout = new QVBoxLayout;
	checkBoxLayout->addWidget(mCheckBoxAirways);
	checkBoxLayout->addWidget(mCheckBoxLymphNodes);
	checkBoxLayout->addWidget(mCheckBoxHeart);
	checkBoxLayout->addWidget(mCheckBoxMediumOrgans);
	checkBoxLayout->addWidget(mCheckBoxSmallOrgans);
	checkBoxLayout->addWidget(mCheckBoxTumors);
	checkBoxLayout->addWidget(mCheckBoxLungVessels);
	checkBoxLayout->addWidget(mCheckBoxLungLobes);
	checkBoxLayout->addWidget(mCheckBoxPET);
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

void FraxinusSegmentations::updateSelectSegmentationBox()
{
	if(!mCheckBoxAirways)
		mCheckBoxAirways = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES) || mAirwaysProcessed)
	{
		mCheckBoxAirways->setText("Airways, Lungs: Completed");
		mCheckBoxAirways->setChecked(false);
	}
	else
	{
		mCheckBoxAirways->setText("Airways, Lungs (~7 min)");
		mCheckBoxAirways->setChecked(true);
	}
	mCheckBoxAirways->setDisabled(true);

	if(!mCheckBoxLymphNodes)
		mCheckBoxLymphNodes = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otLYMPH_NODES) || mLymphNodesProcessed)
	{
		mCheckBoxLymphNodes->setText("Lymph Nodes: Completed");
		mCheckBoxLymphNodes->setDisabled(true);
	}
	else
	{
		mCheckBoxLymphNodes->setText("Lymph Nodes (~2 min)");
		mCheckBoxLymphNodes->setDisabled(false);
	}

	if(!mCheckBoxHeart)
		mCheckBoxHeart = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otHEART) || mHeartProcessed)
	{
		mCheckBoxHeart->setText("Heart, Pulmonary Veins, Pulmonary Trunk: Completed");
		mCheckBoxHeart->setDisabled(true);
	}
	else
	{
		mCheckBoxHeart->setText("Heart, Pulmonary Veins, Pulmonary Trunk  (~4 min)");
		mCheckBoxHeart->setDisabled(false);
	}

	if(!mCheckBoxMediumOrgans)
		mCheckBoxMediumOrgans = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otSPINE) || mMediumOrgansProcessed)
	{
		mCheckBoxMediumOrgans->setText("Vena Cava, Aorta, Spine: Completed");
		mCheckBoxMediumOrgans->setDisabled(true);
	}
	else
	{
		mCheckBoxMediumOrgans->setText("Vena Cava, Aorta, Spine (~3 min)");
		mCheckBoxMediumOrgans->setDisabled(false);
	}

	if(!mCheckBoxSmallOrgans)
		mCheckBoxSmallOrgans = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otESOPHAGUS) || mSmallOrgansProcessed)
	{
		mCheckBoxSmallOrgans->setText("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos: Completed");
		mCheckBoxSmallOrgans->setDisabled(true);
	}
	else
	{
		mCheckBoxSmallOrgans->setText("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos (~2 min)");
		mCheckBoxSmallOrgans->setDisabled(false);
	}

	if(!mCheckBoxTumors)
		mCheckBoxTumors = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otTUMOR) || mTumorsProcessed)
	{
		mCheckBoxTumors->setText("Tumors: Completed");
		mCheckBoxTumors->setDisabled(true);
	}
	else
	{
		mCheckBoxTumors->setText("Tumors (~5 min)");
		mCheckBoxTumors->setDisabled(false);
	}

	if(!mCheckBoxLungVessels)
		mCheckBoxLungVessels = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otLUNG_VESSELS) || mLungVesselsProcessed)
	{
		mCheckBoxLungVessels->setText("Small Vessels: Completed");
		mCheckBoxLungVessels->setDisabled(true);
	}
	else
	{
		mCheckBoxLungVessels->setText("Small Vessels (~5 min)");
		mCheckBoxLungVessels->setDisabled(false);
	}

	if(!mCheckBoxLungLobes)
		mCheckBoxLungLobes = new QCheckBox();
	if(mServices->patient()->getData<Mesh>(otLOBE_LUL) || mLungLobesProcessed)
	{
		mCheckBoxLungLobes->setText("Lung Lobes: Completed");
		mCheckBoxLungLobes->setDisabled(true);
	}
	else
	{
		mCheckBoxLungLobes->setText("Lung Lobes (~5 min)");
		mCheckBoxLungLobes->setDisabled(false);
	}

	if(!mCheckBoxPET)
		mCheckBoxPET = new QCheckBox();
	if(mServices->patient()->getImage(imPET, istPET_REGISTERED))
	{
		mCheckBoxPET->setText("PET to CT: Completed");
		mCheckBoxPET->setDisabled(true);
	}
	else
	{
		mCheckBoxPET->setText("PET to CT (~2 min)");
		this->checkForPETData();
	}

	if(!mCheckBoxSelectAll)
		mCheckBoxSelectAll = new QCheckBox();
	mCheckBoxSelectAll->setText("Select all");
}

void FraxinusSegmentations::checkForPETData()
{
	if(!( mServices->patient()->getImage(imCT, istTHORAX_CT) && mServices->patient()->getImage(imCT, istPET_CT) && mServices->patient()->getImage(imPET, istPET) ))
		mCheckBoxPET->setDisabled(true);
	else
		mCheckBoxPET->setDisabled(false);
}

void FraxinusSegmentations::selectAll(bool checked)
{
	if(!mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
		mCheckBoxAirways->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otLYMPH_NODES))
		mCheckBoxLymphNodes->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otHEART))
		mCheckBoxHeart->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otSPINE))
		mCheckBoxMediumOrgans->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otESOPHAGUS))
		mCheckBoxSmallOrgans->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otTUMOR))
		mCheckBoxTumors->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otLUNG_VESSELS))
		mCheckBoxLungVessels->setChecked(checked);
	if(!mServices->patient()->getData<Mesh>(otLOBE_LUL))
		mCheckBoxLungLobes->setChecked(checked);
	if(mCheckBoxPET->isEnabled() && !mServices->patient()->getImage(imPET, istPET_REGISTERED))
		mCheckBoxPET->setChecked(checked);
}

void FraxinusSegmentations::imageSelected()
{
	mSegmentAirways = mCheckBoxAirways->isChecked();
	mSegmentLungVessels = mCheckBoxLungVessels->isChecked();
	mSegmentLungLobes = mCheckBoxLungLobes->isChecked();
	mSegmentLymphNodes = mCheckBoxLymphNodes->isChecked();
	mSegmentHeart = mCheckBoxHeart->isChecked();
	mSegmentMediumOrgans = mCheckBoxMediumOrgans->isChecked();
	mSegmentSmallOrgans = mCheckBoxSmallOrgans->isChecked();
	mSegmentTumors = mCheckBoxTumors->isChecked();
	mRegisterPET = mCheckBoxPET->isChecked();
	this->close();

	this->createProcessingInfo();

	ImagePtr image = mServices->patient()->getImage(imCT, istTHORAX_CT);

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
		if(mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
			mAirwaysTimerWidget->stop();
	}
	if (mSegmentLungVessels)
	{
		QWidget* timerWidget = new QWidget;
		mLungVesselsTimerWidget = new DisplayTimerWidget(timerWidget);
		mLungVesselsTimerWidget->setFontSize(3);
		mLungVesselsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Small vessels:");
		gridLayout->addWidget(label,1,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,1,1);
		if(mServices->patient()->getData<Mesh>(otLUNG_VESSELS))
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
		if(mServices->patient()->getData<Mesh>(otLYMPH_NODES))
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
		if(mServices->patient()->getData<Mesh>(otHEART))
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
		if(mServices->patient()->getData<Mesh>(otSPINE))
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
		if(mServices->patient()->getData<Mesh>(otESOPHAGUS))
			mSmallOrgansTimerWidget->stop();
	}
	if (mSegmentTumors)
	{
		QWidget* timerWidget = new QWidget;
		mNodulesTimerWidget = new DisplayTimerWidget(timerWidget);
		mNodulesTimerWidget->setFontSize(3);
		mNodulesTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Small Tumors:");
		gridLayout->addWidget(label,7,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,7,1);
		if(mServices->patient()->getData<Mesh>(otTUMOR))
			mNodulesTimerWidget->stop();
	}
	if (mSegmentTumors)
	{
		QWidget* timerWidget = new QWidget;
		mTumorsTimerWidget = new DisplayTimerWidget(timerWidget);
		mTumorsTimerWidget->setFontSize(3);
		mTumorsTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Large Tumors:");
		gridLayout->addWidget(label,8,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,8,1);
		if(mServices->patient()->getData<Mesh>(otTUMOR))
			mTumorsTimerWidget->stop();
	}
	if (mSegmentLungLobes)
	{
		QWidget* timerWidget = new QWidget;
		mLungLobesTimerWidget = new DisplayTimerWidget(timerWidget);
		mLungLobesTimerWidget->setFontSize(3);
		mLungLobesTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("Lung Lobes:");
		gridLayout->addWidget(label,9,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,9,1);
		if(mServices->patient()->getData<Mesh>(otLOBE_LUL))
			mLungLobesTimerWidget->stop();
	}
	if (mRegisterPET)
	{
		QWidget* timerWidget = new QWidget;
		mPETTimerWidget = new DisplayTimerWidget(timerWidget);
		mPETTimerWidget->setFontSize(3);
		mPETTimerWidget->setFixedWidth(50);
		QLabel* label = new QLabel("PET:");
		gridLayout->addWidget(label,10,0,Qt::AlignRight);
		gridLayout->addWidget(timerWidget,10,1,11,3);
		if(mServices->patient()->getImage(imPET, istPET_REGISTERED))
			mPETTimerWidget->stop();
	}
	
	
	mSegmentationProcessingInfo->setLayout(gridLayout);
	mSegmentationProcessingInfo->show();
	mSegmentationProcessingInfo->activateWindow();
}

void FraxinusSegmentations::showProcessingInfoFinished()
{
	mSegmentationFinishedInfo = new QDialog();
	mSegmentationFinishedInfo->setWindowTitle(tr("Finished"));
	mSegmentationFinishedInfo->setWindowFlags( (Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::WindowTitleHint) & ~Qt::WindowCloseButtonHint );
	QVBoxLayout *layout = new QVBoxLayout();
	QLabel* label = new QLabel("Segmentation completed");
	layout->addWidget(label);
	if(!mOKbuttonProcessingFinished)
		mOKbuttonProcessingFinished = new QPushButton(tr("OK"));
	layout->addWidget(mOKbuttonProcessingFinished);
	mSegmentationFinishedInfo->setLayout(layout);
	mSegmentationFinishedInfo->show();
	mSegmentationFinishedInfo->activateWindow();

	connect(mOKbuttonProcessingFinished, &QPushButton::clicked, this, &FraxinusSegmentations::closeSegmentationInfo);
}

void FraxinusSegmentations::closeSegmentationInfo()
{
	disconnect(mOKbuttonProcessingFinished, &QPushButton::clicked, this, &FraxinusSegmentations::closeSegmentationInfo);
	emit segmentationFinished();
	mSegmentationProcessingInfo->close();
	mSegmentationProcessingInfo = nullptr;
	mSegmentationFinishedInfo->close();
	mSegmentationFinishedInfo = nullptr;
}

void FraxinusSegmentations::performPythonSegmentation(ImagePtr image)
{
	if(!image)
		return;

	if(mServices->patient()->getData<Mesh>(otLUNG_VESSELS))
		mLungVesselsProcessed = true;
	if(mServices->patient()->getData<Mesh>(otLOBE_LUL))
		mLungLobesProcessed = true;

	if(mLungLobesProcessed || !mSegmentLungLobes)
	{
		if(mLungVesselsProcessed || !mSegmentLungVessels)
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

	if(!mLungLobesProcessed && mSegmentLungLobes)
	{
		mActiveTimerWidget = mLungLobesTimerWidget;
		if(mActiveTimerWidget)
				mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(DataLocations::getFilterScriptsPath() + "python_LungLobes.ini");
		mCurrentSegmentationType = lsLOBE;
		mLungLobesProcessed = true;
		input[0]->setValue(image->getUid());
	}
	else if(!mLungVesselsProcessed && mSegmentLungVessels)
	{
		mActiveTimerWidget = mLungVesselsTimerWidget;
		if(mActiveTimerWidget)
				mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(DataLocations::getFilterScriptsPath() + "python_LungVessels.ini");
		mCurrentSegmentationType = lsLUNG_VESSELS;
		mLungVesselsProcessed = true;
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

	if(mSegmentAirways && !mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES) && !mAirwaysProcessed)
	{
		mActiveTimerWidget = mAirwaysTimerWidget;
		retval << enum2string(otAIRWAYS);
		if(!mServices->patient()->getData<Mesh>(otLUNGS))
			retval << enum2string(otLUNGS);
		if(startTimers)
			mAirwaysTimerWidget->start();
	}
	if(mSegmentLymphNodes && !mServices->patient()->getData<Mesh>(otLYMPH_NODES) && !mLymphNodesProcessed)
	{
		retval << enum2string(otLYMPH_NODES);
		if(startTimers)
			mLymphNodesTimerWidget->start();
	}
	if(mSegmentTumors && !mServices->patient()->getData<Mesh>(otTUMOR) && !mTumorsProcessed)
	{
		retval << enum2string(otTUMOR);
		if(startTimers)
			mTumorsTimerWidget->start();
	}

	//Multiple targets, will be expanded in Raidionics::createTargetList()
	if(mSegmentHeart && !mServices->patient()->getData<Mesh>(otHEART)&& !mHeartProcessed)
	{
		retval << enum2string(lmPULMSYST_HEART);
		if(startTimers)
			mHeartTimerWidget->start();
	}
	if(mSegmentMediumOrgans && !mServices->patient()->getData<Mesh>(otVENA_CAVA) && !mMediumOrgansProcessed)
	{
		retval << enum2string(lmMEDIUM_ORGANS_MEDIASTINUM);
		if(startTimers)
			mMediumOrgansTimerWidget->start();
	}
	if(mSegmentSmallOrgans && !mServices->patient()->getData<Mesh>(otAZYGOS)&& !mSmallOrgansProcessed)
	{
		retval << enum2string(lmSMALL_ORGANS_MEDIASTINUM);
		if(startTimers)
			mSmallOrgansTimerWidget->start();
	}

	return retval;
}

bool FraxinusSegmentations::runRaidionics(GenericScriptFilterPtr scriptFilter)
{
	QStringList outputClasses = getRaidionicsOutputClasses();
	if(outputClasses.isEmpty())
		return false;
	scriptFilter->setParameterFilePath(DataLocations::getFilterScriptsPath() + "raidionics_LungAll.ini");
	scriptFilter->setOutputClasses(outputClasses);
	mCurrentSegmentationType = lsAIRWAYS;

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
	else if(mSegmentTumors && !mNodulesProcessed && !mServices->patient()->getData<Mesh>(otTUMOR))
	{
		mActiveTimerWidget = mNodulesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		CX_LOG_INFO() << "Segmenting Nodules";
		scriptFilter->setParameterFilePath(DataLocations::getFilterScriptsPath() + "python_Nodules.ini");
		mCurrentSegmentationType = lsNODULES;
		mNodulesProcessed = true;
	}
	else
	{
		mActiveTimerWidget = NULL;
		mCurrentSegmentationType = lsUNKNOWN;
		this->showProcessingInfoFinished();
		return;
	} 
	
	input[0]->setValue(image->getUid());
	mCurrentFilter = scriptFilter;
	this->runMLFilterSlot();
}

void FraxinusSegmentations::performPETCTregistration()
{
	if(mServices->patient()->getImage(imPET, istPET_REGISTERED))
	{
		ImagePtr CTimage = mServices->patient()->getImage(imCT, istTHORAX_CT);
		this->performPythonSegmentation(CTimage);
		return;
	}

	ImagePtr CTimage = mServices->patient()->getImage(imCT, istTHORAX_CT);
	ImagePtr PET_CTimage = mServices->patient()->getImage(imCT, istPET_CT);

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
	ImagePtr PETimage = mServices->patient()->getImage(imPET, istPET);
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

	ImagePtr CTimage = mServices->patient()->getImage(imCT, istTHORAX_CT);
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

	this->checkIfSegmentationSucceeded();

	if(mCurrentSegmentationType == lsLOBE && (mSegmentLungVessels || mSegmentTumors))
		this->performPythonSegmentation(mServices->patient()->getImage(imCT, istTHORAX_CT));
	else if(mCurrentSegmentationType == lsLUNG_VESSELS && mSegmentTumors)
		this->performPythonSegmentation(this->mServices->patient()->getImage(imCT, istTHORAX_CT));
	else
		this->performMLSegmentation(mServices->patient()->getImage(imCT, istTHORAX_CT));
}

void FraxinusSegmentations::MLFinishedSlot()
{
	if(mCurrentSegmentationType == lsAIRWAYS && !mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
		this->postProcessAirways();

	if(mSegmentTumors && mTumorsProcessed && mNodulesProcessed)
		this->postProcessTumors();

	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
	mThread.reset();
	//dialog.hide();
	if(mActiveTimerWidget)
		mActiveTimerWidget->stop();
	
	this->checkIfSegmentationSucceeded();

	if(mSegmentTumors && mTumorsProcessed && mNodulesProcessed)
		deleteTumorsAndNodulesVolumes();
	
	this->performMLSegmentation(mServices->patient()->getImage(imCT, istTHORAX_CT));
}

void FraxinusSegmentations::deleteTumorsAndNodulesVolumes()
{
	ImagePtr tumorsVolume = mServices->patient()->getData<Image>(otTUMOR);
	ImagePtr nodulesVolume = mServices->patient()->getData<Image>(otNODULES);
	if(tumorsVolume)
		mServices->patient()->removeData(tumorsVolume->getUid());
	if(nodulesVolume)
		mServices->patient()->removeData(nodulesVolume->getUid());
}

void FraxinusSegmentations::postProcessAirways()
{
	this->generateCenterline();
	AirwaysFromCenterlinePtr airwaysFromCLPtr = AirwaysFromCenterlinePtr(new AirwaysFromCenterline());
	ImagePtr CTimage = mServices->patient()->getImage(imCT, istTHORAX_CT);
	if(!CTimage)
		return;
	MeshPtr rawCenterline = mServices->patient()->getData<Mesh>(otCENTERLINES);
	if(!rawCenterline)
		return;
	ImagePtr airwaysVolume = mServices->patient()->getData<Image>(otAIRWAYS);
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

	ImagePtr lungsVolume = mServices->patient()->getData<Image>(otLUNGS);
	if(lungsVolume)
		mServices->patient()->removeData(lungsVolume->getUid());
//	if(airwaysVolume) // do not remove, needed for airway radius calculation at Fraxinus restart
//		mServices->patient()->removeData(airwaysVolume->getUid());
}


void FraxinusSegmentations::postProcessTumors()
{
	ImagePtr tumorsVolume = mServices->patient()->getData<Image>(otTUMOR);
	ImagePtr nodulesVolume = mServices->patient()->getData<Image>(otNODULES);

	vtkImageDataPtr combinedVtkImage = mergeTumorVolumes(tumorsVolume, nodulesVolume);

	if(!combinedVtkImage)
		return;

	setDeepModified(combinedVtkImage);

	ImagePtr baseImage = mServices->patient()->getImage(imCT, istTHORAX_CT);
	if(!baseImage)
		return;

	VisServicesPtr visServices = boost::static_pointer_cast<VisServices>(mServices);
	IslandsFilterPtr islandsFilter = IslandsFilterPtr(new IslandsFilter(visServices));
	QString uid = baseImage->getUid() + "_Tumors_Islands%1";
	QString name = baseImage->getName()+" Tumors Islands%1";
	ImagePtr labeledImage = islandsFilter->execute(baseImage, combinedVtkImage, uid, name, 10);
	if(!labeledImage)
		return;

	std::vector<double> tumorSizes =  islandsFilter->getIslandSizes();

	MeshesFromLabelsFilterPtr meshesFromLabelsFilter = MeshesFromLabelsFilterPtr(new MeshesFromLabelsFilter(visServices));
	std::vector<vtkPolyDataPtr> rawResult = meshesFromLabelsFilter->execute(
				labeledImage->getBaseVtkImageData(),  //input
				labeledImage->getMin()+1,  //startLabel
				labeledImage->getMax(),  //endLabel
				false,  //reduceResolution
				true,  //smoothing
				true,  //preserveTopology
				0.99,  //preservedecimationTopology
				15,  //numberOfIterations
				0.03);  //passBand

	std::vector<MeshPtr> tumorMeshes = meshesFromLabelsFilter->postProcess(visServices, rawResult, labeledImage, QColor(255,255,0,255), false);
	std::vector<QString> lobeNames = getLobeOfTumors(tumorMeshes);
	setNumberAndSizeToTumorVolumes(tumorMeshes, tumorSizes, lobeNames);

	if(labeledImage)
		mServices->patient()->removeData(labeledImage->getUid());

	std::vector<ORGAN_TYPE> lobeTypes = {otLOBE_LUL, otLOBE_LLL, otLOBE_RUL, otLOBE_RML, otLOBE_RLL};
	for(int i=0; i<lobeTypes.size(); i++)
	{
		ImagePtr lobeImage = mServices->patient()->getData<Image>(lobeTypes[i]);
		if (lobeImage)
			mServices->patient()->removeData(lobeImage->getUid());
	}
}

std::vector<QString> FraxinusSegmentations::getLobeOfTumors(std::vector<MeshPtr> tumorMeshes)
{

	std::vector<QString> lobeNames;

	std::vector<ORGAN_TYPE> lobeTypes = {otLOBE_LUL, otLOBE_LLL, otLOBE_RUL, otLOBE_RML, otLOBE_RLL};
	std::vector<ImagePtr> lobesImage;
	for(int i=0; i<lobeTypes.size(); i++)
		lobesImage.push_back(mServices->patient()->getData<Image>(lobeTypes[i]));

	if(tumorMeshes.empty() || lobesImage.empty())
		return lobeNames;

	std::vector<Vector3D> centerOfTumorsVector_r;
	for(int i=0; i<tumorMeshes.size(); i++)
	{
		vtkPolyDataPtr vtkPolyDataTumor =  tumorMeshes[i]->getVtkPolyData();
		Vector3D centerOfTumor_d(vtkPolyDataTumor->GetCenter());
		Transform3D rMd = tumorMeshes[i]->get_rMd();
		Vector3D centerOfTumor_r(centerOfTumor_d(0)+rMd(0,3), centerOfTumor_d(1)+rMd(1,3), centerOfTumor_d(2)+rMd(2,3));
		centerOfTumorsVector_r.push_back(centerOfTumor_r);
	}

	std::vector<vtkImageDataPtr> lobesVtkImage;
	for(int i=0; i<lobesImage.size(); i++)
		if(lobesImage[i])
			lobesVtkImage.push_back(shiftVtkScalarToUnsignedShort(lobesImage[i]->getBaseVtkImageData()));

	for(int i=0; i<tumorMeshes.size(); i++)
	{
		for(int j=0; j<lobesVtkImage.size(); j++)
		{
			int* dim = lobesVtkImage[j]->GetDimensions();
			double* spacing = lobesVtkImage[j]->GetSpacing();
			Transform3D rMd  = lobesImage[j]->get_rMd();

			int x = (int) boost::math::round((centerOfTumorsVector_r[i](0) - rMd(0,3)) / spacing[0]);
			int y = (int) boost::math::round((centerOfTumorsVector_r[i](1) - rMd(1,3)) / spacing[1]);
			int z = (int) boost::math::round((centerOfTumorsVector_r[i](2) - rMd(2,3)) / spacing[2]);

			if(x<0 || y<0 || z<0 || x>=dim[0] || y>=dim[1] || z>=dim[2])
				continue;

			unsigned short* dataPtrImage = static_cast<unsigned short*>(lobesVtkImage[j]->GetScalarPointer(x,y,z));
			if(dataPtrImage[0] > 0)
			{
				lobeNames.push_back(enum2string(lobeTypes[j]));
				goto endOfLoop;
			}
		}

		lobeNames.push_back("");
		endOfLoop:;
	}

	return lobeNames;
}

vtkImageDataPtr FraxinusSegmentations::mergeTumorVolumes(ImagePtr tumorsVolume, ImagePtr nodulesVolume)
{
	vtkImageDataPtr combinedVtkImage = vtkImageDataPtr::New();
	if(tumorsVolume && nodulesVolume)
		combinedVtkImage =  mergeBinaryImages(tumorsVolume->getBaseVtkImageData(), nodulesVolume->getBaseVtkImageData());
	else if(tumorsVolume)
		combinedVtkImage->DeepCopy(tumorsVolume->getBaseVtkImageData());
	else if(nodulesVolume)
		combinedVtkImage->DeepCopy(nodulesVolume->getBaseVtkImageData());
	else
		combinedVtkImage = vtkImageDataPtr();

	return combinedVtkImage;
}

void FraxinusSegmentations::setNumberAndSizeToTumorVolumes(std::vector<MeshPtr> tumorMeshes, std::vector<double> tumorSizes, std::vector<QString> lobeNames)
{
	for(int i=0; i<tumorMeshes.size(); i++)
	{
		this->setMeshNameAndType(tumorMeshes[i], otTUMOR);
		QString nameWithNumber = tumorMeshes[i]->getName() + QString(" ") + QString::number(i+1);
		if(lobeNames.size()>i)
			nameWithNumber.append(QString(" ") + lobeNames[i]);
		if(tumorSizes.size()>i)
		{
			tumorMeshes[i]->setVolumeSizeMl(tumorSizes[i]);
			QString nameWithNumberAndVolumeSize = nameWithNumber + QString(":  %1 ml").arg(tumorSizes[i], 0, 'f', 2);
			tumorMeshes[i]->setName(nameWithNumberAndVolumeSize);
		}
		else
			tumorMeshes[i]->setName(nameWithNumber);
	}
}

void FraxinusSegmentations::generateCenterline()
{//using BinaryThinningImageFilter3DFilter
	VisServicesPtr visServices = boost::static_pointer_cast<VisServices>(mServices);
	BinaryThinningImageFilter3DFilterPtr binaryThinningImageFilter3DFilter =  BinaryThinningImageFilter3DFilterPtr(new BinaryThinningImageFilter3DFilter(visServices));
	std::vector<SelectDataStringPropertyBasePtr> input = binaryThinningImageFilter3DFilter->getInputTypes();
	std::vector<SelectDataStringPropertyBasePtr> output = binaryThinningImageFilter3DFilter->getOutputTypes();
	binaryThinningImageFilter3DFilter->getOptions();
	ImagePtr airwaysVolume = mServices->patient()->getData<Image>(otAIRWAYS);
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
			mAirwaysProcessed = true;
			setMeshNameAndStopTimer(otAIRWAYS);
			setMeshName(otLUNGS);
		}
		if(mSegmentLymphNodes)
		{
			mLymphNodesProcessed = true;
			setMeshNameAndStopTimer(otLYMPH_NODES);
		}
			if(mSegmentHeart)
		{
				mHeartProcessed = true;
			stopTimer(otHEART);
			setMeshName(otHEART);
			setMeshName(otPULMONARY_VEINS);
			setMeshName(otPULMONARY_TRUNK);
		}
		if(mSegmentMediumOrgans)
		{
			mMediumOrgansProcessed = true;
			stopTimer(otSPINE);
			setMeshName(otAORTIC_ARCH);
			setMeshName(otDESCENDING_AORTA);
			setMeshName(otASCENDING_AORTA);
			setMeshName(otSPINE);
			setMeshName(otVENA_CAVA);
		}
		if(mSegmentSmallOrgans)
		{
			mSmallOrgansProcessed = true;
			stopTimer(otESOPHAGUS);
			setMeshName(otESOPHAGUS);
			setMeshName(otSUBCLAVIAN_ARTERY);
			setMeshName(otBRACHIO_CEPHALIC_VEINS);
			setMeshName(otAZYGOS);
		}
		if(mSegmentTumors)
		{
			mTumorsProcessed = true;
			stopTimer(otTUMOR, true);

		}
	}
	else if(mCurrentSegmentationType == lsNODULES)
	{
		mNodulesProcessed = true;
		stopTimer(otNODULES, true);
	}
	else if(mCurrentSegmentationType == lsLUNG_VESSELS)
	{
		mLungVesselsProcessed = true;
		setMeshNameAndStopTimer(otLUNG_VESSELS);
	}
	else if(mCurrentSegmentationType == lsLOBE)
	{
		mLungLobesProcessed = true;
		stopTimer(otLOBE_LUL);
		setMeshName(otLOBE_LUL);
		setMeshName(otLOBE_LLL);
		setMeshName(otLOBE_RUL);
		setMeshName(otLOBE_RML);
		setMeshName(otLOBE_RLL);
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
	MeshPtr mesh = mServices->patient()->getData<Mesh>(target);
	if(mesh)
		this->setMeshNameAndType(mesh, target);
	else
		CX_LOG_WARNING() << "FraxinusSegmentations::setMeshName: Found no segmentation for: " << enum2string(target);
}

void FraxinusSegmentations::stopTimer(ORGAN_TYPE target, bool checkVolume)
{
	bool succeeded = false;
	if(checkVolume)
	{
		if(mServices->patient()->getData<Image>(target))
			succeeded = true;
	}
	else if(mServices->patient()->getData<Mesh>(target))
			succeeded = true;

	DisplayTimerWidget* timer = this->getTimer(target);
	if(timer)
	{
		if(succeeded)
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
	case otTUMOR:
		timer = mTumorsTimerWidget; break;
	case otNODULES:
		timer = mNodulesTimerWidget; break;
	case otLUNG_VESSELS:
		timer = mLungVesselsTimerWidget; break;
	case otLOBE_LUL:
	case otLOBE_LLL:
	case otLOBE_RUL:
	case otLOBE_RML:
	case otLOBE_RLL:
		timer = mLungLobesTimerWidget; break;
	default:
		timer = nullptr; break;
	}

	return timer;
}
}//cx

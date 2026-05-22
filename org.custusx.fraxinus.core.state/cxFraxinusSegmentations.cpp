/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxFraxinusSegmentations.h"
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QScrollArea>
#include <QTimer>
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
#include "cxFileHelpers.h"

namespace
{
const int kMinAirways      = 7;
const int kMinCenterlines  = 2;
const int kMinLungVessels  = 5;
const int kMinLymphNodes   = 2;
const int kMinHeart        = 4;
const int kMinMediumOrgans = 3;
const int kMinSmallOrgans  = 2;
const int kMinTumors       = 5;
const int kMinLungLobes    = 5;
const int kMinPET          = 5;
}

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
		mCheckBoxAirways->setText(QString("Airways, Lungs (~%1 min)").arg(kMinAirways));
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
		mCheckBoxLymphNodes->setText(QString("Lymph Nodes (~%1 min)").arg(kMinLymphNodes));
		mCheckBoxLymphNodes->setDisabled(false);
	}

	if(!mCheckBoxHeart)
	{
		mCheckBoxHeart = new QCheckBox();
		mCheckBoxHeart->setToolTip("Heart, Pulmonary Veins, Pulmonary Trunk");
	}
	if(mServices->patient()->getData<Mesh>(otHEART) || mHeartProcessed)
	{
		mCheckBoxHeart->setText("Pulmonary System: Completed");
		mCheckBoxHeart->setDisabled(true);
	}
	else
	{
		mCheckBoxHeart->setText(QString("Pulmonary System (~%1 min)").arg(kMinHeart));
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
		mCheckBoxMediumOrgans->setText(QString("Vena Cava, Aorta, Spine (~%1 min)").arg(kMinMediumOrgans));
		mCheckBoxMediumOrgans->setDisabled(false);
	}

	if(!mCheckBoxSmallOrgans)
	{
		mCheckBoxSmallOrgans = new QCheckBox();
		mCheckBoxSmallOrgans->setToolTip("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos");
	}
	if(mServices->patient()->getData<Mesh>(otESOPHAGUS) || mSmallOrgansProcessed)
	{
		mCheckBoxSmallOrgans->setText("Small Mediastinal Organs: Completed");
		mCheckBoxSmallOrgans->setDisabled(true);
	}
	else
	{
		mCheckBoxSmallOrgans->setText(QString("Small Mediastinal Organs (~%1 min)").arg(kMinSmallOrgans));
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
		mCheckBoxTumors->setText(QString("Tumors (~%1 min)").arg(kMinTumors));
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
		mCheckBoxLungVessels->setText(QString("Small Vessels (~%1 min)").arg(kMinLungVessels));
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
		mCheckBoxLungLobes->setText(QString("Lung Lobes (~%1 min)").arg(kMinLungLobes));
		mCheckBoxLungLobes->setDisabled(false);
	}

	if(!mCheckBoxSelectAll)
		mCheckBoxSelectAll = new QCheckBox();
	mCheckBoxSelectAll->setText("Select all");
}

void FraxinusSegmentations::selectAll(bool checked)
{
	if(mCheckBoxAirways->isEnabled())
		mCheckBoxAirways->setChecked(checked);
	if(mCheckBoxLymphNodes->isEnabled())
		mCheckBoxLymphNodes->setChecked(checked);
	if(mCheckBoxHeart->isEnabled())
		mCheckBoxHeart->setChecked(checked);
	if(mCheckBoxMediumOrgans->isEnabled())
		mCheckBoxMediumOrgans->setChecked(checked);
	if(mCheckBoxSmallOrgans->isEnabled())
		mCheckBoxSmallOrgans->setChecked(checked);
	if(mCheckBoxTumors->isEnabled())
		mCheckBoxTumors->setChecked(checked);
	if(mCheckBoxLungVessels->isEnabled())
		mCheckBoxLungVessels->setChecked(checked);
	if(mCheckBoxLungLobes->isEnabled())
		mCheckBoxLungLobes->setChecked(checked);
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
	mRegisterPET = mServices->patient()->getImage(imCT, istPET_CT)
	        && mServices->patient()->getImage(imPET, istPET)
	        && !mServices->patient()->getImage(imPET, istPET_REGISTERED);
	this->close();

	this->createProcessingInfo();

	ImagePtr imageCopy = mServices->patient()->getImage(imCT, istCOPY);

	if(mRegisterPET)
		this->performPETCTregistration();
	else
		this->performPythonSegmentation(imageCopy);
}

void FraxinusSegmentations::cancel()
{
	this->close();
}

void FraxinusSegmentations::startSegmentationWithOptions(
        bool airways, bool lymphNodes, bool heart,
        bool mediumOrgans, bool smallOrgans, bool tumors,
        bool lungVessels, bool lungLobes)
{
	if(mActiveTimerWidget)
		return;

	mSegmentAirways = airways;
	mSegmentLymphNodes = lymphNodes;
	mSegmentHeart = heart;
	mSegmentMediumOrgans = mediumOrgans;
	mSegmentSmallOrgans = smallOrgans;
	mSegmentTumors = tumors;
	mSegmentLungVessels = lungVessels;
	mSegmentLungLobes = lungLobes;
	mRegisterPET = mServices->patient()->getImage(imCT, istPET_CT)
	        && mServices->patient()->getImage(imPET, istPET)
	        && !mServices->patient()->getImage(imPET, istPET_REGISTERED);

	this->createProcessingInfo();

	ImagePtr imageCopy = mServices->patient()->getImage(imCT, istCOPY);
	if(mRegisterPET)
		this->performPETCTregistration();
	else
		this->performPythonSegmentation(imageCopy);
}

void FraxinusSegmentations::setProcessingInfoParentWidget(QWidget* container)
{
	mProcessingInfoParentWidget = container;
}

void FraxinusSegmentations::createProcessingInfo()
{
	if (mProcessingInfoParentWidget)
	{
		QLayout* oldLayout = mProcessingInfoParentWidget->layout();
		if (oldLayout)
		{
			mProgressBarAirways = nullptr;
			mProgressBarLungVessels = nullptr;
			mProgressBarLymphNodes = nullptr;
			mProgressBarHeart = nullptr;
			mProgressBarMediumOrgans = nullptr;
			mProgressBarSmallOrgans = nullptr;
			mProgressBarNodules = nullptr;
			mProgressBarTumors = nullptr;
			mProgressBarLungLobes = nullptr;
			mProgressBarCenterlines = nullptr;
			mAirwaysTimerWidget = nullptr;
			mLungsTimerWidget = nullptr;
			mLymphNodesTimerWidget = nullptr;
			mHeartTimerWidget = nullptr;
			mMediumOrgansTimerWidget = nullptr;
			mSmallOrgansTimerWidget = nullptr;
			mNodulesTimerWidget = nullptr;
			mTumorsTimerWidget = nullptr;
			mPETTimerWidget = nullptr;
			mProgressBarPET = nullptr;
			mLungVesselsTimerWidget = nullptr;
			mLungLobesTimerWidget = nullptr;
			mCenterlinesTimerWidget = nullptr;
			mActiveTimerWidget = nullptr;
			mCurrentRaidionicsBar = nullptr;
			mRaidionicsInInference = false;
			mPipelineCurrentStep = 1;
			mPipelineTotalSteps = 1;
			if (mCenterlineProgressTimer)
			{
				mCenterlineProgressTimer->stop();
				delete mCenterlineProgressTimer;
				mCenterlineProgressTimer = nullptr;
			}
			mCenterlineProgressTicks = 0;
			if (mPETProgressTimer)
			{
				mPETProgressTimer->stop();
				delete mPETProgressTimer;
				mPETProgressTimer = nullptr;
			}
			mPETProgressTicks = 0;
			QLayoutItem* item;
			while ((item = oldLayout->takeAt(0)) != nullptr)
			{
				delete item->widget();
				delete item;
			}
			delete oldLayout;
		}
	}

	QGridLayout* gridLayout = new QGridLayout;
	gridLayout->setColumnMinimumWidth(0, 200);
	gridLayout->setColumnMinimumWidth(1, 200);

	auto makeProgressBar = []() {
		QProgressBar* bar = new QProgressBar(nullptr);
		bar->setRange(0, 100);
		bar->setValue(0);
		return bar;
	};

	int totalMinutes = 0;
	int row = 0;
	if (mSegmentAirways)
	{
		mProgressBarAirways = makeProgressBar();
		mAirwaysTimerWidget = new DisplayTimerWidget(nullptr);
		mAirwaysTimerWidget->setFontSize(3);
		mAirwaysTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Airways, Lungs:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarAirways, row, 1);
		gridLayout->addWidget(mAirwaysTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
		{
			mAirwaysTimerWidget->stop();
			mProgressBarAirways->setValue(100);
		}
		totalMinutes += kMinAirways; row++;

		mProgressBarCenterlines = makeProgressBar();
		mCenterlinesTimerWidget = new DisplayTimerWidget(nullptr);
		mCenterlinesTimerWidget->setFontSize(3);
		mCenterlinesTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Airways centerlines:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarCenterlines, row, 1);
		gridLayout->addWidget(mCenterlinesTimerWidget, row, 2);
		if (mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
		{
			mCenterlinesTimerWidget->stop();
			mProgressBarCenterlines->setValue(100);
		}
		row++;
	}
	if (mSegmentLungVessels)
	{
		mProgressBarLungVessels = makeProgressBar();
		mLungVesselsTimerWidget = new DisplayTimerWidget(nullptr);
		mLungVesselsTimerWidget->setFontSize(3);
		mLungVesselsTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Small vessels:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarLungVessels, row, 1);
		gridLayout->addWidget(mLungVesselsTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otLUNG_VESSELS))
		{
			mLungVesselsTimerWidget->stop();
			mProgressBarLungVessels->setValue(100);
		}
		totalMinutes += kMinLungVessels; row++;
	}
	if (mSegmentLymphNodes)
	{
		mProgressBarLymphNodes = makeProgressBar();
		mLymphNodesTimerWidget = new DisplayTimerWidget(nullptr);
		mLymphNodesTimerWidget->setFontSize(3);
		mLymphNodesTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Lymph Nodes:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarLymphNodes, row, 1);
		gridLayout->addWidget(mLymphNodesTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otLYMPH_NODES))
		{
			mLymphNodesTimerWidget->stop();
			mProgressBarLymphNodes->setValue(100);
		}
		totalMinutes += kMinLymphNodes; row++;
	}
	if (mSegmentHeart)
	{
		mProgressBarHeart = makeProgressBar();
		mHeartTimerWidget = new DisplayTimerWidget(nullptr);
		mHeartTimerWidget->setFontSize(3);
		mHeartTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Pulmonary System:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarHeart, row, 1);
		gridLayout->addWidget(mHeartTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otHEART))
		{
			mHeartTimerWidget->stop();
			mProgressBarHeart->setValue(100);
		}
		totalMinutes += kMinHeart; row++;
	}
	if (mSegmentMediumOrgans)
	{
		mProgressBarMediumOrgans = makeProgressBar();
		mMediumOrgansTimerWidget = new DisplayTimerWidget(nullptr);
		mMediumOrgansTimerWidget->setFontSize(3);
		mMediumOrgansTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Vena Cava, Aorta, Spine:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarMediumOrgans, row, 1);
		gridLayout->addWidget(mMediumOrgansTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otSPINE))
		{
			mMediumOrgansTimerWidget->stop();
			mProgressBarMediumOrgans->setValue(100);
		}
		totalMinutes += kMinMediumOrgans; row++;
	}
	if (mSegmentSmallOrgans)
	{
		mProgressBarSmallOrgans = makeProgressBar();
		mSmallOrgansTimerWidget = new DisplayTimerWidget(nullptr);
		mSmallOrgansTimerWidget->setFontSize(3);
		mSmallOrgansTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Small Mediastinal Organs:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarSmallOrgans, row, 1);
		gridLayout->addWidget(mSmallOrgansTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otESOPHAGUS))
		{
			mSmallOrgansTimerWidget->stop();
			mProgressBarSmallOrgans->setValue(100);
		}
		totalMinutes += kMinSmallOrgans; row++;
	}
	if (mSegmentTumors)
	{
		mProgressBarNodules = makeProgressBar();
		mNodulesTimerWidget = new DisplayTimerWidget(nullptr);
		mNodulesTimerWidget->setFontSize(3);
		mNodulesTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Small Tumors:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarNodules, row, 1);
		gridLayout->addWidget(mNodulesTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otTUMOR))
		{
			mNodulesTimerWidget->stop();
			mProgressBarNodules->setValue(100);
		}
		row++;

		mProgressBarTumors = makeProgressBar();
		mTumorsTimerWidget = new DisplayTimerWidget(nullptr);
		mTumorsTimerWidget->setFontSize(3);
		mTumorsTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Large Tumors:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarTumors, row, 1);
		gridLayout->addWidget(mTumorsTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otTUMOR))
		{
			mTumorsTimerWidget->stop();
			mProgressBarTumors->setValue(100);
		}
		totalMinutes += kMinTumors; row++;
	}
	if (mSegmentLungLobes)
	{
		mProgressBarLungLobes = makeProgressBar();
		mLungLobesTimerWidget = new DisplayTimerWidget(nullptr);
		mLungLobesTimerWidget->setFontSize(3);
		mLungLobesTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("Lung Lobes:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarLungLobes, row, 1);
		gridLayout->addWidget(mLungLobesTimerWidget, row, 2);
		if(mServices->patient()->getData<Mesh>(otLOBE_LUL))
		{
			mLungLobesTimerWidget->stop();
			mProgressBarLungLobes->setValue(100);
		}
		totalMinutes += kMinLungLobes; row++;
	}
	if (mRegisterPET)
	{
		mProgressBarPET = makeProgressBar();
		mPETTimerWidget = new DisplayTimerWidget(nullptr);
		mPETTimerWidget->setFontSize(3);
		mPETTimerWidget->setFixedWidth(80);
		gridLayout->addWidget(new QLabel("PET:"), row, 0, Qt::AlignRight);
		gridLayout->addWidget(mProgressBarPET, row, 1);
		gridLayout->addWidget(mPETTimerWidget, row, 2);
		if(mServices->patient()->getImage(imPET, istPET_REGISTERED))
		{
			mPETTimerWidget->stop();
			mProgressBarPET->setValue(100);
		}
		totalMinutes += kMinPET; row++;
	}

	if (row > 0)
	{
		QFrame* separator = new QFrame;
		separator->setFrameShape(QFrame::HLine);
		separator->setFrameShadow(QFrame::Sunken);
		gridLayout->addWidget(separator, row, 0, 1, 3);
		row++;
		QLabel* totalLabel = new QLabel(QString("Estimated total: ~%1 min").arg(totalMinutes));
		gridLayout->addWidget(totalLabel, row, 0, 1, 3, Qt::AlignRight);
	}
	
	
	if (mProcessingInfoParentWidget)
	{
		mProcessingInfoParentWidget->setLayout(gridLayout);
		mProcessingInfoParentWidget->setVisible(true);
		QTimer::singleShot(0, [this](){
			QMainWindow* mw = nullptr;
			for (QWidget* w : qApp->topLevelWidgets())
			{
				if (w->objectName() == "main_window")
				{
					mw = qobject_cast<QMainWindow*>(w);
					break;
				}
			}
			if (!mw) return;
			QDockWidget* dock = mw->findChild<QDockWidget*>("new_load_patient_widgetDockWidget");
			if (!dock) return;
			// The dock content is a QScrollArea wrapping the NewLoadPatientWidget.
			// sizeHint() reports the current (narrow) size because setWidgetResizable(true)
			// resizes the content to fit the dock. Use minimumSizeHint() on the actual
			// content widget instead, which is computed from layout constraints.
			QScrollArea* scroller = qobject_cast<QScrollArea*>(dock->widget());
			if (!scroller || !scroller->widget()) return;
			int neededWidth = scroller->widget()->minimumSizeHint().width()
			                  + dock->contentsMargins().left()
			                  + dock->contentsMargins().right();
			if (neededWidth > dock->width())
				mw->resizeDocks({dock}, {neededWidth}, Qt::Horizontal);
		});
	}
	else
	{
		mSegmentationProcessingInfo = new QDialog();
		mSegmentationProcessingInfo->setWindowTitle(tr("Segmentation status"));
		mSegmentationProcessingInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
		mSegmentationProcessingInfo->setLayout(gridLayout);
		mSegmentationProcessingInfo->show();
		mSegmentationProcessingInfo->activateWindow();
	}
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
	if (mSegmentationProcessingInfo)
	{
		mSegmentationProcessingInfo->close();
		mSegmentationProcessingInfo = nullptr;
	}
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
	if(mServices->patient()->getData<Mesh>(otTUMOR))
		mNodulesProcessed = true;

	if(mLungLobesProcessed || !mSegmentLungLobes)
	{
		if(mLungVesselsProcessed || !mSegmentLungVessels)
		{
			if(mNodulesProcessed || !mSegmentTumors)
			{
				this->performMLSegmentation(image);
				return;
			}
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
	else if(!mNodulesProcessed && mSegmentTumors)
	{
		mActiveTimerWidget = mNodulesTimerWidget;
		if(mActiveTimerWidget)
			mActiveTimerWidget->start();
		scriptFilter->setParameterFilePath(DataLocations::getFilterScriptsPath() + "python_Nodules.ini");
		mCurrentSegmentationType = lsNODULES;
		mNodulesProcessed = true;
		input[0]->setValue(image->getUid());
	}
	else
		return;

	mCurrentFilter = scriptFilter;
	mCurrentScriptFilter = scriptFilter;
	this->runPythonFilterSlot();
}

QStringList FraxinusSegmentations::getRaidionicsOutputClasses(bool startTimers)
{
	QStringList retval;

	if(mSegmentAirways && !mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES) && !mAirwaysProcessed)
	{
		mActiveTimerWidget = mAirwaysTimerWidget;
		//retval << enum2string(otLUNGS);
		retval << enum2string(otAIRWAYS);
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
	if(!retval.isEmpty())
		retval.prepend(enum2string(otLUNGS));//always include lungs first as it is needed as a mask for all segmentations

	return retval;
}

bool FraxinusSegmentations::runRaidionics(GenericScriptFilterPtr scriptFilter)
{
	removeNonemptyDirRecursively(DataLocations::getCachePath() + "/Raidionics_temp/");
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
	else
	{
		mActiveTimerWidget = NULL;
		mCurrentSegmentationType = lsUNKNOWN;
		this->showProcessingInfoFinished();
		return;
	} 
	
	input[0]->setValue(image->getUid());
	mCurrentFilter = scriptFilter;
	mCurrentScriptFilter = scriptFilter;
	this->runMLFilterSlot();
}

void FraxinusSegmentations::performPETCTregistration()
{
	if(mServices->patient()->getImage(imPET, istPET_REGISTERED))
	{
		ImagePtr CTimageCopy = mServices->patient()->getImage(imCT, istCOPY);
		this->performPythonSegmentation(CTimageCopy);
		return;
	}

	ImagePtr CTimageCopy = mServices->patient()->getImage(imCT, istCOPY);
	ImagePtr PET_CTimage = mServices->patient()->getImage(imCT, istPET_CT);

	mActiveTimerWidget = mPETTimerWidget;
	if(mActiveTimerWidget)
		mActiveTimerWidget->start();

	mPETProgressTicks = 0;
	if (mPETProgressTimer)
	{
		mPETProgressTimer->stop();
		delete mPETProgressTimer;
	}
	mPETProgressTimer = new QTimer();
	connect(mPETProgressTimer, &QTimer::timeout, this, &FraxinusSegmentations::PETProgressTick);
	mPETProgressTimer->start(1000); //updates progressbar every second

	//NB: Elastix creates (modified) copies of PETimage and PET_CTimage
	//Setting Image Type to istPET_REGISTERED for new PET volume in ElastixManager::addNonlinearData()

	PET_CTimage->get_rMd_History()->setParentSpace(""); //Make sure we don't move any other images
	mServices->registration()->setFixedData(CTimageCopy);
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

void FraxinusSegmentations::PETProgressTick()
{
	int pauseAtPercentage = 90;
	if (!mProgressBarPET)
		return;
	++mPETProgressTicks;
	int value = std::min(pauseAtPercentage, mPETProgressTicks * pauseAtPercentage / (kMinPET*60));
	mProgressBarPET->setValue(value);
}

void FraxinusSegmentations::elastixFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mElastixManager->getExecuter().get(), &TimedBaseAlgorithm::finished, this, &FraxinusSegmentations::elastixFinishedSlot);

	if (mPETProgressTimer)
	{
		mPETProgressTimer->stop();
		delete mPETProgressTimer;
		mPETProgressTimer = nullptr;
	}
	if (mProgressBarPET)
		mProgressBarPET->setValue(100);

	mPETTimerWidget->stop();

	ImagePtr CTimageCopy = mServices->patient()->getImage(imCT, istCOPY);
	this->performPythonSegmentation(CTimageCopy);
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

	if (mCurrentScriptFilter)
		connect(mCurrentScriptFilter.get(), &GenericScriptFilter::scriptOutput,
		        this, &FraxinusSegmentations::onScriptOutput, Qt::QueuedConnection);

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
	connect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot1()));
	mTimedAlgorithmProgressBar->attach(mThread);

	if (mCurrentScriptFilter)
		connect(mCurrentScriptFilter.get(), &GenericScriptFilter::scriptOutput,
		        this, &FraxinusSegmentations::onScriptOutput, Qt::QueuedConnection);

	mThread->execute();
}

void FraxinusSegmentations::pythonFinishedSlot()
{
	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(pythonFinishedSlot()));
	mThread.reset();

	if (mCurrentScriptFilter)
	{
		disconnect(mCurrentScriptFilter.get(), &GenericScriptFilter::scriptOutput,
		           this, &FraxinusSegmentations::onScriptOutput);
		QProgressBar* bar = getProgressBar(mCurrentSegmentationType);
		if (bar)
		{
			if (bar->maximum() == 0)
				bar->setRange(0, 100);
			bar->setValue(100);
		}
	}

	this->checkIfSegmentationSucceeded();

	if(mCurrentSegmentationType == lsLOBE && (mSegmentLungVessels || mSegmentTumors))
		this->performPythonSegmentation(mServices->patient()->getImage(imCT, istCOPY));
	else if(mCurrentSegmentationType == lsLUNG_VESSELS && mSegmentTumors)
		this->performPythonSegmentation(this->mServices->patient()->getImage(imCT, istCOPY));
	else
		this->performMLSegmentation(mServices->patient()->getImage(imCT, istCOPY));
}

void FraxinusSegmentations::MLFinishedSlot1()
{
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot1()));
	if(mCurrentSegmentationType == lsAIRWAYS && !mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
	{
		connect(this, SIGNAL(centerlineReady()), this, SLOT(postProcessAirwaysSlot()));
		connect(this, SIGNAL(postProcessAirwaysFinished()), this, SLOT(MLFinishedSlot2()));
		connect(this, SIGNAL(centerlineGenerationFailed()), this, SLOT(MLFinishedSlot2()));
		this->generateCenterline();
	}
	else
		this->MLFinishedSlot2();
}

void FraxinusSegmentations::MLFinishedSlot2()
{
	disconnect(this, SIGNAL(postProcessAirwaysFinished()), this, SLOT(MLFinishedSlot2()));
	disconnect(this, SIGNAL(centerlineGenerationFailed()), this, SLOT(MLFinishedSlot2()));

	if (mCurrentScriptFilter)
		disconnect(mCurrentScriptFilter.get(), &GenericScriptFilter::scriptOutput,
		           this, &FraxinusSegmentations::onScriptOutput);

	mCurrentRaidionicsBar = nullptr;
	mRaidionicsInInference = false;

	for (QProgressBar* bar : {mProgressBarAirways, mProgressBarLymphNodes, mProgressBarHeart,
	                           mProgressBarMediumOrgans, mProgressBarSmallOrgans, mProgressBarTumors})
		if (bar)
		{
			if (bar->maximum() == 0)
				bar->setRange(0, 100);
			bar->setValue(100);
		}

	if (mCenterlineProgressTimer)
	{
		mCenterlineProgressTimer->stop();
		delete mCenterlineProgressTimer;
		mCenterlineProgressTimer = nullptr;
	}
	if (mProgressBarCenterlines)
		mProgressBarCenterlines->setValue(100);
	if (mCenterlinesTimerWidget)
		mCenterlinesTimerWidget->stop();

	if(mSegmentTumors)
		this->postProcessTumors();

	mTimedAlgorithmProgressBar->detach(mThread);
	mThread.reset();
	//dialog.hide();
	if(mActiveTimerWidget)
		mActiveTimerWidget->stop();

	this->checkIfSegmentationSucceeded();

	if(mSegmentTumors && mTumorsProcessed && mNodulesProcessed)
		deleteTumorsAndNodulesVolumes();

	this->performMLSegmentation(mServices->patient()->getImage(imCT, istCOPY));

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

void FraxinusSegmentations::postProcessAirwaysSlot()
{
	disconnect(this, SIGNAL(centerlineReady()), this, SLOT(postProcessAirwaysSlot()));

	AirwaysFromCenterlinePtr airwaysFromCLPtr = AirwaysFromCenterlinePtr(new AirwaysFromCenterline());
	ImagePtr CTimage = mServices->patient()->getImage(imCT, istTHORAX_CT);
	MeshPtr rawCenterline = mServices->patient()->getData<Mesh>(otCENTERLINES);
	ImagePtr airwaysVolume = mServices->patient()->getData<Image>(otAIRWAYS);
	if(!CTimage || !rawCenterline || !airwaysVolume)
	{
		emit postProcessAirwaysFinished();
		return;
	}

	airwaysFromCLPtr->processCenterline(rawCenterline);
	airwaysFromCLPtr->setSegmentedVolume(airwaysVolume->getBaseVtkImageData(), airwaysVolume->get_rMd());

	mBranchList = airwaysFromCLPtr->getBranchList();

	mBranchList->setRadius(airwaysVolume);

	// Create mesh object from the airway walls
	QString uidMesh = CTimage->getUid() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	QString nameMesh = CTimage->getName() + airwaysFilterGetNameSuffixAirways() + airwaysFilterGetNameSuffixTubes();
	MeshPtr airwayWalls = mServices->patient()->createSpecificData<Mesh>(uidMesh, nameMesh);
	airwayWalls->setColor(QColor(253, 173, 136, 255));
	vtkPolyDataPtr vtkPolyDataAirwayWalls_d = airwaysFromCLPtr->generateTubes(0, true);
	airwayWalls->setVtkPolyData(vtkPolyDataAirwayWalls_d);
	airwayWalls->get_rMd_History()->setParentSpace(rawCenterline->getUid());
	airwayWalls->get_rMd_History()->setRegistration(airwaysVolume->get_rMd());
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

	vtkPolyDataPtr vtkPolyDataCenterline_r = airwaysFromCLPtr->getVTKPoints();
	centerline->setVtkPolyData(vtkPolyDataCenterline_r);
	vtkPolyDataPtr vtkPolyDataCenterline_d = centerline->getTransformedPolyDataCopy(rawCenterline->get_rMd().inverse());
	centerline->setVtkPolyData(vtkPolyDataCenterline_d);
	centerline->get_rMd_History()->setParentSpace(rawCenterline->getUid());
	centerline->get_rMd_History()->setRegistration(rawCenterline->get_rMd());
	setMeshNameAndType(centerline, otAIRWAYS_CENTERLINES);
	mServices->patient()->insertData(centerline);

	ImagePtr lungsVolume = mServices->patient()->getData<Image>(otLUNGS);
	if(lungsVolume)
		mServices->patient()->removeData(lungsVolume->getUid());
//	if(airwaysVolume) // do not remove, needed for airway radius calculation at Fraxinus restart
//		mServices->patient()->removeData(airwaysVolume->getUid());

	emit postProcessAirwaysFinished();
}


void FraxinusSegmentations::postProcessTumors()
{
	ImagePtr tumorsVolume = mServices->patient()->getData<Image>(otTUMOR);
	ImagePtr nodulesVolume = mServices->patient()->getData<Image>(otNODULES);

	vtkImageDataPtr combinedVtkImage = mergeTumorVolumes(tumorsVolume, nodulesVolume);

	if(!combinedVtkImage)
		return;

	setDeepModified(combinedVtkImage);

	ImagePtr baseImage = mServices->patient()->getImage(imCT, istCOPY);
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

//	std::vector<ORGAN_TYPE> lobeTypes = {otLOBE_LUL, otLOBE_LLL, otLOBE_RUL, otLOBE_RML, otLOBE_RLL};
//	for(int i=0; i<lobeTypes.size(); i++)
//	{
//		ImagePtr lobeImage = mServices->patient()->getData<Image>(lobeTypes[i]);
//		if (lobeImage)
//			mServices->patient()->removeData(lobeImage->getUid());
//	}
}

std::vector<QString> FraxinusSegmentations::getLobeOfTumors(std::vector<MeshPtr> tumorMeshes)
{

	std::vector<QString> lobeNames;

		if(tumorMeshes.empty())
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

	lobeNames = getLobeNameFromPositions(centerOfTumorsVector_r, mServices);

	return lobeNames;
}

std::vector<QString> FraxinusSegmentations::getLobeNameFromPositions(std::vector<Vector3D> positions_r, CoreServicesPtr services)
{
	std::vector<QString> lobeNames;

	std::vector<ORGAN_TYPE> lobeTypes = {otLOBE_LUL, otLOBE_LLL, otLOBE_RUL, otLOBE_RML, otLOBE_RLL};
	std::vector<ImagePtr> lobesImage;
	for(int i=0; i<lobeTypes.size(); i++)
		lobesImage.push_back(services->patient()->getData<Image>(lobeTypes[i]));

	if(lobesImage.empty())
		return lobeNames;

	std::vector<vtkImageDataPtr> lobesVtkImage;
	for(int i=0; i<lobesImage.size(); i++)
		if(lobesImage[i])
			lobesVtkImage.push_back(shiftVtkScalarToUnsignedShort(lobesImage[i]->getBaseVtkImageData()));

	for(int i=0; i<positions_r.size(); i++)
	{
		for(int j=0; j<lobesVtkImage.size(); j++)
		{
			int* dim = lobesVtkImage[j]->GetDimensions();
			double* spacing = lobesVtkImage[j]->GetSpacing();
			Transform3D rMd  = lobesImage[j]->get_rMd();

			int x = (int) boost::math::round((positions_r[i](0) - rMd(0,3)) / spacing[0]);
			int y = (int) boost::math::round((positions_r[i](1) - rMd(1,3)) / spacing[1]);
			int z = (int) boost::math::round((positions_r[i](2) - rMd(2,3)) / spacing[2]);

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
	mBinaryThinningImageFilter3DFilter.reset(new BinaryThinningImageFilter3DFilter(visServices));
	std::vector<SelectDataStringPropertyBasePtr> input = mBinaryThinningImageFilter3DFilter->getInputTypes();
	mBinaryThinningImageFilter3DFilter->getOutputTypes(); //Needed to create output types
	mBinaryThinningImageFilter3DFilter->getOptions();
	ImagePtr airwaysVolume = mServices->patient()->getData<Image>(otAIRWAYS);
	if(!airwaysVolume)
	{
		CX_LOG_WARNING() << "In FraxinusSegmentations::generateCenterline airways volume not found.";
		emit centerlineGenerationFailed();
		return;
	}
	input[0]->setValue(airwaysVolume->getUid());

	for (QProgressBar* bar : {mProgressBarAirways, mProgressBarLymphNodes, mProgressBarHeart,
	                           mProgressBarMediumOrgans, mProgressBarSmallOrgans, mProgressBarTumors})
		if (bar)
		{
			bar->setRange(0, 100);
			bar->setValue(100);
		}
	for (DisplayTimerWidget* timer : {mAirwaysTimerWidget, mLymphNodesTimerWidget, mHeartTimerWidget,
	                                   mMediumOrgansTimerWidget, mSmallOrgansTimerWidget, mTumorsTimerWidget})
		if (timer)
			timer->stop();

	if (mProgressBarCenterlines)
	{
		mCenterlineProgressTicks = 0;
		if (mCenterlineProgressTimer)
		{
			mCenterlineProgressTimer->stop();
			delete mCenterlineProgressTimer;
		}
		mCenterlineProgressTimer = new QTimer();
		connect(mCenterlineProgressTimer, &QTimer::timeout, this, &FraxinusSegmentations::centerlineProgressTick);
		mCenterlineProgressTimer->start(1000); //updates progressbar every second
		mCenterlinesTimerWidget->start();
	}

	mCenterlineThread.reset(new FilterTimedAlgorithm(mBinaryThinningImageFilter3DFilter));
	connect(mCenterlineThread.get(), SIGNAL(finished()), this, SLOT(centerlineFinishedSlot()));
	mCenterlineThread->execute();
}

void FraxinusSegmentations::centerlineProgressTick()
{
	int pauseAtPercentage = 90;
	if (!mProgressBarCenterlines)
		return;
	++mCenterlineProgressTicks;
	int value = std::min(pauseAtPercentage, mCenterlineProgressTicks * pauseAtPercentage / (kMinCenterlines*60));
	mProgressBarCenterlines->setValue(value);
}

void FraxinusSegmentations::centerlineFinishedSlot()
{
	disconnect(mCenterlineThread.get(), SIGNAL(finished()), this, SLOT(centerlineFinishedSlot()));
	mCenterlineThread.reset();

	std::vector<SelectDataStringPropertyBasePtr> output = mBinaryThinningImageFilter3DFilter->getOutputTypes();

	if(!output[0])
	{
		emit centerlineGenerationFailed();
		CX_LOG_WARNING() << "In FraxinusSegmentations::generateCenterline BinaryThinningImageFilter3DFilter failed.";
		return;
	}

	MeshPtr centerline = mServices->patient()->getData<Mesh>(output[0]->getValue());
	setMeshNameAndType(centerline, otCENTERLINES);
	centerline->setColor(QColor(255,255,0,255));

	emit centerlineReady();

	return;
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

QProgressBar* FraxinusSegmentations::getProgressBar(LUNG_STRUCTURES type)
{
	switch(type)
	{
	case lsLUNG:
	case lsAIRWAYS:        return mProgressBarAirways;
	case lsCENTERLINES:    return mProgressBarCenterlines;
	case lsLUNG_VESSELS:   return mProgressBarLungVessels;
	case lsLOBE:           return mProgressBarLungLobes;
	case lsNODULES:        return mProgressBarNodules;
	case lsTUMOR:          return mProgressBarTumors;
	case lsLYMPH_NODES:    return mProgressBarLymphNodes;
	case lsHEART:
	case lsPULMONARY_VEINS:
	case lsPULMONARY_TRUNK: return mProgressBarHeart;
	case lsVENA_CAVA:
	case lsAORTA:
	case lsSPINE:          return mProgressBarMediumOrgans;
	case lsVENA_AZYGOS:
	case lsSUBCLAVIAN_ARTERY:
	case lsESOPHAGUS:      return mProgressBarSmallOrgans;
	case lsPET_REGISTERED: return mProgressBarPET;
	default:               return nullptr;
	}
}

QProgressBar* FraxinusSegmentations::getRaidionicsBarForLine(const QString& line)
{
	if (line.contains("Tumor"))
		return mProgressBarTumors;
	if (line.contains("Lymph"))
		return mProgressBarLymphNodes;
	if (line.contains("Respiratory") || line.contains("Pulmonary") || line.contains("Heart"))
		return mProgressBarHeart;
	if (line.contains("Large organ") || line.contains("Vena") || line.contains("Aorta") || line.contains("Spine"))
		return mProgressBarMediumOrgans;
	if (line.contains("Small organ") || line.contains("Esophagus"))
		return mProgressBarSmallOrgans;
	if (line.contains("Lungs") || line.contains("Airway"))
		return mProgressBarAirways;
	return nullptr;
}

int FraxinusSegmentations::scaledPipelineValue(int val) const
{
	if (mPipelineTotalSteps <= 1)
		return val;
	return ((mPipelineCurrentStep - 1) * 100 + val) / mPipelineTotalSteps;
}

void FraxinusSegmentations::onScriptOutput(const QString& line)
{
	// Raidionics pipeline-level progress: "LOG: Pipeline - <name> - Begin/End (N/M)"
	if (line.contains("LOG: Pipeline -") && (line.contains("- Begin") || line.contains("- End")))
	{
		bool isBegin = line.contains("- Begin");
		if (isBegin)
		{
			// Parse (N/M) to track which pipeline step we are in
			int lastParen = line.lastIndexOf('(');
			if (lastParen >= 0)
			{
				QString inner = line.mid(lastParen + 1);
				int closeParen = inner.indexOf(')');
				if (closeParen >= 0)
				{
					QStringList parts = inner.left(closeParen).split('/');
					if (parts.size() == 2)
					{
						bool ok1, ok2;
						int n = parts[0].toInt(&ok1);
						int m = parts[1].toInt(&ok2);
						if (ok1 && ok2 && m > 0)
						{
							mPipelineCurrentStep = n;
							mPipelineTotalSteps = m;
						}
					}
				}
			}

			mCurrentRaidionicsBar = getRaidionicsBarForLine(line);
			mRaidionicsInInference = false;
			if (mCurrentRaidionicsBar)
			{
				if (mCurrentRaidionicsBar->maximum() == 0)
					mCurrentRaidionicsBar->setRange(0, 100);
				mCurrentRaidionicsBar->setValue(scaledPipelineValue(5));
			}
		}
		else
		{
			if (mCurrentRaidionicsBar)
				mCurrentRaidionicsBar->setValue(scaledPipelineValue(100));
			mCurrentRaidionicsBar = nullptr;
			mRaidionicsInInference = false;
		}
	}
	// Raidionics segmentation sub-step: "LOG: Segmentation - <step> - Begin/End (K/4)"
	else if (line.contains("LOG: Segmentation -") && (line.contains("- Begin") || line.contains("- End")))
	{
		if (!mCurrentRaidionicsBar)
			return;
		bool isBegin = line.contains("- Begin");
		int val = 0;
		if (line.contains("Preprocessing"))
			val = isBegin ? 5 : 15;
		else if (line.contains("Inference"))
		{
			mRaidionicsInInference = isBegin;
			val = isBegin ? 15 : 75;
		}
		else if (line.contains("Reconstruction"))
			val = isBegin ? 75 : 90;
		else if (line.contains("Data dump"))
			val = isBegin ? 90 : 98;
		if (val > 0)
		{
			if (mCurrentRaidionicsBar->maximum() == 0)
				mCurrentRaidionicsBar->setRange(0, 100);
			mCurrentRaidionicsBar->setValue(scaledPipelineValue(val));
		}
	}
	// tqdm inside Raidionics Inference sub-step
	else if (line.contains("%|") && mRaidionicsInInference && mCurrentRaidionicsBar)
	{
		bool ok;
		int within_pct = line.split("%").first().trimmed().toInt(&ok);
		if (ok)
		{
			int overall = 15 + 60 * within_pct / 100;  // 15–75
			if (mCurrentRaidionicsBar->maximum() == 0)
				mCurrentRaidionicsBar->setRange(0, 100);
			mCurrentRaidionicsBar->setValue(scaledPipelineValue(overall));
		}
	}
	// TotalSegmentator PROGRESS: markers
	else if (line.startsWith("PROGRESS:"))
	{
		bool ok;
		int val = line.mid(9).trimmed().toInt(&ok);
		if (ok)
		{
			QProgressBar* bar = getProgressBar(mCurrentSegmentationType);
			if (bar)
			{
				if (val < 0)
				{
					bar->setRange(0, 0);
				}
				else
				{
					if (bar->maximum() == 0)
						bar->setRange(0, 100);
					bar->setValue(val);
				}
			}
		}
	}
}

}//cx

/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxFraxinusTrackingWidget.h"
#include <memory>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QListWidgetItem>
#include "cxVisServices.h"
#include "cxLogger.h"
#include "cxApplication.h"
#include "cxTrackingService.h"
#include "cxTrackingSystemService.h"
#include "cxTrackerConfigurationImpl.h"
#include "cxToolConfigureWidget.h"
#include "cxProfile.h"
#include "cxManualToolAdapter.h"
#include "cxTrackingToolConfigurer.h"
#include "cxTool.h"


namespace cx {

typedef std::shared_ptr<class ManualToolAdapter> ManualToolAdapterPtr;

FraxinusTrackingWidget::FraxinusTrackingWidget(VisServicesPtr services, FraxinusNavigationWidget *fraxinusNavigationWidget, QWidget* parent):
	BaseWidget(parent, getWidgetName(), "Tracking"),
	mFraxinusNavigationWidget(fraxinusNavigationWidget),
	mTrackingService(services->tracking()),
	mNumberOfTools(4),
	mClinicalApplication("Bronchoscopy"),
	mTrackingSystemName("aurora"),
	mEBUSProbeUid("EBUS_Bronchoscope_EXERA_III_Simulator"),
	mEBUSProbeName("EBUS EXERA III Simulator"),
	mEBUSProbeConfigurationName("WidthMaxDepth4"),
	mToolState(Tool::tsNONE)
{
	mTrackerConfiguration = mTrackingService->getConfiguration();

	if(!profile()->getToolConfigFilePath().endsWith("Fraxinus.xml"))
		this->copyToolConfigFile("Fraxinus.xml");

	mToolConfigureGroupBox = new ToolConfigureGroupBox(mTrackingService, services->state(), this);
	mToolConfigureGroupBox->setCurrentlySelectedCofiguration(profile()->getToolConfigFilePath());
	mToolConfigureGroupBox->hide();

	QStringList applications;
	applications.append(mClinicalApplication);
	QStringList trackingSystems;
	trackingSystems.append(mTrackingSystemName);
	this->addToolsToComboBoxes(mNumberOfTools, mTrackerConfiguration, applications, trackingSystems);

	mStartTrackingButton = new QPushButton("Start Tracking", this);
	connect(mStartTrackingButton, &QPushButton::clicked, this, &FraxinusTrackingWidget::startTrackingClickedSlot);
	mStopTrackingButton = new QPushButton("Stop Tracking", this);
	connect(mStopTrackingButton, &QPushButton::clicked, this, &FraxinusTrackingWidget::stopTrackingClickedSlot);

	QVBoxLayout* verticalLayout = new QVBoxLayout;
	QGridLayout* gridLayoutTools = new QGridLayout();
	QGridLayout* gridLayoutButtons = new QGridLayout();

	for (int i=0; i<mToolFilesComboBoxes.size(); i++)
	{
		QString label= "Tool ";
		label.append(QString::number(i+1));
		gridLayoutTools->addWidget(new QLabel(label), i, 0);
		gridLayoutTools->addWidget(mToolFilesComboBoxes[i], i, 1);
	}
	gridLayoutButtons->addWidget(mStartTrackingButton, 0, 0);
	gridLayoutButtons->addWidget(mStopTrackingButton, 0, 1);
	verticalLayout->setAlignment(Qt::AlignTop);
	verticalLayout->addSpacing(50);
	verticalLayout->addLayout(gridLayoutTools);
	verticalLayout->addSpacing(50);
	verticalLayout->addLayout(gridLayoutButtons);

	this->setLayout(verticalLayout);

	connect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::updateButtonStatusSlot);
	this->updateButtonStatusSlot();

	mEBUSToolConfigurer = new TrackingToolConfigurer(mTrackingService, mEBUSProbeName, this);
	connect(mEBUSToolConfigurer, &TrackingToolConfigurer::toolConfigured, this, &FraxinusTrackingWidget::setUpEBUSToolAndStartTracking);
}

FraxinusTrackingWidget::~FraxinusTrackingWidget()
{
}

void FraxinusTrackingWidget::copyToolConfigFile(QString filename)
{
	QStringList configurations = mTrackerConfiguration->getAllConfigurations();
	for(int i = 0; i < configurations.size(); ++i)
	{
		QFileInfo fileInfo(configurations[i]);
		if (fileInfo.fileName().endsWith(filename))
		{
			QString toolConfigFilePath = profile()->getApplicationToolConfigPaths().front();
			if(!QDir(toolConfigFilePath).exists())
				QDir().mkdir(toolConfigFilePath);

			QString toolConfigFileName = toolConfigFilePath + filename;
			QFile::copy(configurations[i], toolConfigFileName);
			profile()->setToolConfigFilePath(toolConfigFileName);
			break;
		}
	}
}

void FraxinusTrackingWidget::addToolsToComboBoxes(int numberOfTools, TrackerConfigurationPtr config, QStringList applicationsFilter, QStringList trackingsystemsFilter)
{
	QStringList toolPathList = config->getToolsGivenFilter(applicationsFilter, trackingsystemsFilter);
	QStringList toolsInCurrentConfig = mToolConfigureGroupBox->getConfiguration().mTools;

	mToolFilesComboBoxes.clear();
	for(int i=0; i<numberOfTools; i++)
	{
		mToolFilesComboBoxes.push_back(new QComboBox());

		for(QString toolPath : toolPathList)
		{
			if(config->getTool(toolPath).mPortNumber == i)
			{
				QString toolName = config->getTool(toolPath).mName;
				mToolFilesComboBoxes[i]->addItem(toolName);
				int index = mToolFilesComboBoxes[i]->findText(toolName);
				mToolFilesComboBoxes[i]->setItemData(index, toolPath, Qt::ToolTipRole);
				if(toolsInCurrentConfig.size()>i)
				{
					if(toolsInCurrentConfig[i].contains(toolName))
						mToolFilesComboBoxes[i]->setCurrentIndex(index);
				}
				else
					mToolFilesComboBoxes[i]->setCurrentIndex(-1);
			}
		}
		mToolFilesComboBoxes[i]->addItem("<No Tool>", Qt::ToolTipRole);

		connect(mToolFilesComboBoxes[i], SIGNAL(currentIndexChanged(int)), this, SLOT(updateTrackerConfigurationTools()));
	}
}

void FraxinusTrackingWidget::startTrackingClickedSlot(bool checked)
{
	//this->printTrackerConfiguration(); //debug
	Q_UNUSED(checked);
	this->startTracking();
}

void FraxinusTrackingWidget::startTracking()
{
	if(mToolState < Tool::tsTRACKING)
		mTrackingService->setState(Tool::tsTRACKING);
}

void FraxinusTrackingWidget::stopTrackingClickedSlot(bool checked)
{
	Q_UNUSED(checked);
	this->stopTracking();
}

void FraxinusTrackingWidget::stopTracking()
{
	if(mToolState < Tool::tsTRACKING)
		return;
	if (mFraxinusNavigationWidget)
		mFraxinusNavigationWidget->onTrackingShutDown();
	mTrackingService->setState(Tool::tsINITIALIZED);
}

void FraxinusTrackingWidget::updateButtonStatusSlot()
{
	mToolState = mTrackingService->getState();

	mStartTrackingButton->setEnabled(mToolState < Tool::tsTRACKING);
	mStopTrackingButton->setEnabled(mToolState >= Tool::tsTRACKING);
}

void FraxinusTrackingWidget::updateTrackerConfigurationTools()
{
	TrackerConfiguration::Configuration config = mToolConfigureGroupBox->getConfiguration();
	QStringList tools;
	for(int i=0; i<mNumberOfTools; i++)
	{
		QString tool;
		if(mToolFilesComboBoxes[i]->currentIndex() >= 0 )
			tool = mToolFilesComboBoxes[i]->itemData(mToolFilesComboBoxes[i]->currentIndex(), Qt::ToolTipRole).toString();

		if(!tool.isEmpty())
			tools.append(tool);
//		else
//		{
//			for(int j=i; j<mNumberOfTools; j++)
//				mToolFilesComboBoxes[j]->setCurrentIndex(-1); // if tool #n is empty, #n+1 also must be empty
//			break;
//		}
	}

	// set reference tool
	config.mReferenceTool = "";
	for(int i=0; i<tools.size(); i++)
	{
		if (mTrackerConfiguration->getTool(tools[i]).mIsReference)
			config.mReferenceTool = tools[i];
	}

	config.mTools = tools;
	mTrackerConfiguration->saveConfiguration(config);
}

void FraxinusTrackingWidget::startUltrasoundSimulation()
{
	if(!mTrackingService)
	{
		CX_LOG_WARNING() << "In FraxinusTrackingWidget::startUltrasoundSimulation(): Did not find tracking system";
		return;
	}

	int USProbeIndex = mToolFilesComboBoxes[3]->findText(mEBUSProbeUid);
	if(USProbeIndex < 0)
	{
		CX_LOG_WARNING() << "In FraxinusTrackingWidget::startUltrasoundSimulation(): Did not find the tool " << mEBUSProbeUid;
		return;
	}

	mToolFilesComboBoxes[3]->setCurrentIndex(USProbeIndex);
	mEBUSToolConfigurer->start();
}

/** The simulation follows the manual tool, so it doesn't wait for a tracking system to start tracking:
 *  the simulator is normally run without one connected. */
void FraxinusTrackingWidget::setUpEBUSToolAndStartTracking()
{
	ToolPtr tool = mEBUSToolConfigurer->getTool();
	ManualToolAdapterPtr manualToolAdapterPtr = std::static_pointer_cast<ManualToolAdapter>(mTrackingService->getManualTool());
	manualToolAdapterPtr->setBase(tool);
	manualToolAdapterPtr->startEmittingContinuousPositions(100);
	report("Manual tool imbued with properties from " + mEBUSProbeName);

	ProbePtr probe = tool->getProbe();
	if(probe)
	{
		probe->applyNewConfigurationWithId(mEBUSProbeConfigurationName);
		ProbeDefinition probeDefinition = probe->getProbeDefinition(mEBUSProbeConfigurationName);
		probe->setActiveStream(probeDefinition.getUid());
	}

	emit trackingReady();
	startTracking();
}

void FraxinusTrackingWidget::stopUltrasoundSimulation()
{
	if(!mTrackingService)
		return;

	mEBUSToolConfigurer->stop();
	stopTracking();
	mToolFilesComboBoxes[3]->setCurrentIndex(-1);
	ManualToolAdapterPtr manualToolAdapterPtr = std::static_pointer_cast<ManualToolAdapter>(mTrackingService->getManualTool());
	manualToolAdapterPtr->setBase();
	manualToolAdapterPtr->stopEmittingContinuousPositions();
	manualToolAdapterPtr->setTooltipOffset(0);

}

TrackingServicePtr FraxinusTrackingWidget::getTrackingService()
{
	return mTrackingService;
}

void FraxinusTrackingWidget::printTrackerConfiguration() //debug
{
	TrackerConfiguration::Configuration config = mToolConfigureGroupBox->getConfiguration();
	CX_LOG_DEBUG() << "-----------------Tracker configuration:------------------";
	CX_LOG_DEBUG() << "Uid: " << config.mUid;
	CX_LOG_DEBUG() << "ClinicalApplication: " << config.mClinicalApplication;
	CX_LOG_DEBUG() << "TrackingSystemImplementation: " << config.mTrackingSystemImplementation;
	CX_LOG_DEBUG() << "TrackingSystemName: " << config.mTrackingSystemName;
	CX_LOG_DEBUG() << "Tools: ";

	for(int i=0; i<config.mTools.size(); i++)
		CX_LOG_DEBUG() << "Tool " << i << ": " << config.mTools[i];

	CX_LOG_DEBUG() << "ReferenceTool: " << config.mReferenceTool;

}



} //namespace cx

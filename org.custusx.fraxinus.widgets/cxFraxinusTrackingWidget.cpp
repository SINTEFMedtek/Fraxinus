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

#include "cxFraxinusTrackingWidget.h"
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
#include "cxTool.h"


namespace cx {

FraxinusTrackingWidget::FraxinusTrackingWidget(VisServicesPtr services, FraxinusNavigationWidget *fraxinusNavigationWidget, QWidget* parent):
	BaseWidget(parent, getWidgetName(), "Tracking"),
	mFraxinusNavigationWidget(fraxinusNavigationWidget),
	mTrackingService(services->tracking()),
	mTrackerUid("Fraxinus"),
	mNumberOfTools(4),
	mClinicalApplication("Bronchoscopy"),
	mTrackingSystemImplementation("igstk"),
	mTrackingSystemName("aurora"),
	mEBUSProbeUid("EBUS_Bronchoscope_EXERA_III_Simulator"),
	mEBUSProbeName("EBUS EXERA III Simulator"),
	mEBUSProbeConfigurationName("WidthMaxDepth4")
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
		return;

	mToolFilesComboBoxes[3]->setCurrentIndex(USProbeIndex);

	if(mToolState <= Tool::tsCONFIGURED)
	{
		mTrackingService->setState(Tool::tsCONFIGURED);
		connect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::setUpEBUSToolAndStartTracking);
	}
	else
		setUpEBUSToolAndStartTracking();

}

TrackingSystemServicePtr FraxinusTrackingWidget::getIGSTKTrackingSystemService()
{
	std::vector<TrackingSystemServicePtr> trackingSystems = mTrackingService->getTrackingSystems();
	for (unsigned i = 0; i < trackingSystems.size(); ++i)
	{
		if(trackingSystems[i]->getUid() == "org.custusx.core.tracking.system.igstk")
			return trackingSystems[i];
	}
	return TrackingSystemServicePtr();
}

void FraxinusTrackingWidget::setUpEBUSToolAndStartTracking()
{
	disconnect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::setUpEBUSToolAndStartTracking);

	ManualToolAdapterPtr manualToolAdapterPtr = boost::static_pointer_cast<ManualToolAdapter>(mTrackingService->getManualTool());
	ToolMap toolMap = mTrackingService->getTools();
	ToolPtr tool;
	for (ToolMap::iterator iter = toolMap.begin(); iter != toolMap.end(); ++iter)
	{
		if (iter->first == mEBUSProbeName)
		{
		tool = iter->second;
		manualToolAdapterPtr->setBase(tool);
		manualToolAdapterPtr->startEmittingContinuousPositions(100);
		report("Manual tool imbued with properties from " + iter->first);
		break;
		}
	}

	if(tool)
	{
		ProbePtr probe = tool->getProbe();
		if(probe)
		{
			probe->applyNewConfigurationWithId(mEBUSProbeConfigurationName);
			ProbeDefinition probeDefinition = probe->getProbeDefinition(mEBUSProbeConfigurationName);
			probe->setActiveStream(probeDefinition.getUid());
		}
	}

	connect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::trackingStarted);
	startTracking();
}

void FraxinusTrackingWidget::stopUltrasoundSimulation()
{
	if(!mTrackingService)
		return;

	stopTracking();
	mToolFilesComboBoxes[3]->setCurrentIndex(-1);
	ManualToolAdapterPtr manualToolAdapterPtr = boost::static_pointer_cast<ManualToolAdapter>(mTrackingService->getManualTool());
	manualToolAdapterPtr->setBase();
	manualToolAdapterPtr->stopEmittingContinuousPositions();
	manualToolAdapterPtr->setTooltipOffset(0);

}

TrackingServicePtr FraxinusTrackingWidget::getTrackingService()
{
	return mTrackingService;
}

void FraxinusTrackingWidget::trackingStarted()
{
	disconnect(mTrackingService.get(), &TrackingService::stateChanged, this, &FraxinusTrackingWidget::trackingStarted);
	emit trackingReady();
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

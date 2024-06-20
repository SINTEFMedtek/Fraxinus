#include "cxNewLoadPatientWidget.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>

#include "cxApplication.h"
#include "cxLogger.h"
#include "cxLogicManager.h"
#include "cxDataLocations.h"
#include "cxPatientModelService.h"
#include "cxFraxinusVideoRecorderWidget.h"
#include "cxProfile.h"

namespace cx
{

NewLoadPatientWidget::NewLoadPatientWidget(QWidget *parent, VisServicesPtr services, AcquisitionServicePtr acquisitionService, PatientModelServicePtr patientModelService) :
	BaseWidget(parent, "new_load_patient_widget", "New/Load Patient"),
	mServices(services),
	mPatientModelService(patientModelService)
{
	this->setWindowTitle("Create or select patient");

	QPushButton* newButton = new QPushButton("&New Patient");
	newButton->setIcon(QIcon(":/icons/icons/add.svg"));
	connect(newButton, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatient);

	QPushButton* loadButton = new QPushButton("&Load Patient");
	loadButton->setIcon(QIcon(":/icons/icons/select.svg"));
	connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadPatient);

	QPushButton* restoreToFactorySettingsButton = new QPushButton("&Restore factory settings");
	QPalette palette = restoreToFactorySettingsButton->palette();
	palette.setColor(QPalette::Button, Qt::red);
	restoreToFactorySettingsButton->setPalette(palette);
	connect(restoreToFactorySettingsButton, &QPushButton::clicked, this, &NewLoadPatientWidget::restoreToFactorySettings);

	mSelectCTDataButton = new QPushButton("&Select CT data");
	mSelectCTDataButton->setIcon(QIcon(":/icons/icons/import.svg"));
	mSelectCTDataButton->setEnabled(false);
	connect(mSelectCTDataButton, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addSpacing(50);
	layout->addWidget(newButton);
	layout->addWidget(loadButton);
	layout->addSpacing(50);

	layout->addWidget(mSelectCTDataButton);
	layout->addStretch();

	QString profile = ProfileManager::getInstance()->activeProfile()->getUid();
	if(profile == "VideoRecording")
	{
		FraxinusVideoRecorderWidget* videoRecorderWidget = new FraxinusVideoRecorderWidget(mServices, acquisitionService, this);
		layout->addWidget(videoRecorderWidget);
		layout->addStretch();
	}

	layout->addWidget(restoreToFactorySettingsButton);
	this->setLayout(layout);
}

void NewLoadPatientWidget::createNewPatient()
{
	QString actionName = "NewPatient";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	selectCTData();
}

void NewLoadPatientWidget::loadPatient()
{
	QString actionName = "LoadFile";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
}

void NewLoadPatientWidget::enableImportDataButton()
{
	if(mPatientModelService->isPatientValid())
		mSelectCTDataButton->setEnabled(true);
	else
		mSelectCTDataButton->setEnabled(false);
}

void NewLoadPatientWidget::restoreToFactorySettings()
{
	DataLocations::deletePersistentWritablePath();
	LogicManager::getInstance()->restartServicesWithProfile("Bronchoscopy");
}

void NewLoadPatientWidget::selectCTData()
{
	if(mPatientModelService->isPatientValid())
	{
		triggerMainWindowActionWithObjectName("AddFilesForImportWithDialogCT");
		triggerMainWindowActionWithObjectName("ImportSelectedData");
	}
}

}

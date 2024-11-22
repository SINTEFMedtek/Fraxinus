#include "cxNewLoadPatientWidget.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QDialog>
#include <QLabel>
#include <QTextEdit>

#include "cxApplication.h"
#include "cxLogger.h"
#include "cxLogicManager.h"
#include "cxDataLocations.h"
#include "cxPatientModelService.h"
#include "cxFraxinusVideoRecorderWidget.h"
#include "cxProfile.h"
#include "cxVisServices.h"

namespace cx
{

NewLoadPatientWidget::NewLoadPatientWidget(QWidget *parent, VisServicesPtr services, AcquisitionServicePtr acquisitionService) :
	BaseWidget(parent, "new_load_patient_widget", "New/Load Patient"),
	mServices(services)
{
	this->setObjectName(this->getWidgetName());
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

QString NewLoadPatientWidget::getWidgetName()
{
	return "new_load_patient_widget";
}

void NewLoadPatientWidget::createNewPatient()
{
	QString actionName = "NewPatient";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	patientCreatedInfo();
}

void NewLoadPatientWidget::patientCreatedInfo()
{
	if(!mServices->patient()->isPatientValid())
		return;

	mPatientCreatedInfo = new QDialog();
	mPatientCreatedInfo->setWindowTitle(tr("Patient created"));
	mPatientCreatedInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
	QGridLayout* layout = new QGridLayout();
	QTextEdit* textBox = new QTextEdit("<b><u>A new is patient created</u></b><br>"
														 "Do you want to import CT data?<br><br>"
														 "A file dialog will open: <i>Select Madical Image file(s)</i><br>"
														 "Locate the folder containing the CT files of the patient.<br>"
														 "(A USB storage is located by clicking <i><b>media</b></i> on the left side of the dialog)<br><br>"
														 "<u>Note:</u> If a folder containing an extensive amount data (e.g from several patients)"
														 " is selected, it may be slow to load.");
	textBox->setReadOnly(true);
	textBox->setFixedWidth(600);
	layout->addWidget(textBox,0,0,1,2);
	mYesButtonPatientCreated = new QPushButton(tr("Yes"));
	mNoButtonPatientCreated = new QPushButton(tr("No"));
	layout->addWidget(mYesButtonPatientCreated,1,0);
	layout->addWidget(mNoButtonPatientCreated,1,1);
	mPatientCreatedInfo->setLayout(layout);
	mPatientCreatedInfo->show();
	mPatientCreatedInfo->activateWindow();

	connect(mYesButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	connect(mNoButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::closePatientCreatedInfo);
}

void NewLoadPatientWidget::closePatientCreatedInfo()
{
	mPatientCreatedInfo->close();
	disconnect(mNoButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::closePatientCreatedInfo);
}

void NewLoadPatientWidget::closeDataLoadedInfo()
{
	mDataLoadedInfo->close();
	disconnect(mYesButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	disconnect(mNoButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);

	if(mServices->patient()->getImage(imCT, istTHORAX_CT))
		emit dataImportCompleted();
}

void NewLoadPatientWidget::loadPatient()
{
	QString actionName = "LoadFile";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	if(mServices->patient()->getImage(imCT, istTHORAX_CT))
		emit dataImportCompleted();
	else
		dataAddedOrRemoved();
}

void NewLoadPatientWidget::enableImportDataButton()
{
	if(mServices->patient()->isPatientValid())
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
	closePatientCreatedInfo();
	disconnect(mYesButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);

	loadCTData();
}

void NewLoadPatientWidget::selectMoreCTData()
{
	mDataLoadedInfo->close();
	disconnect(mYesButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	disconnect(mNoButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	loadCTData();
}

void NewLoadPatientWidget::loadCTData()
{
	if(mServices->patient()->getImage(imCT, istTHORAX_CT))
		mThoraxCTLoaded = true;
	if(mServices->patient()->getImage(imPET, istPET) && mServices->patient()->getImage(imCT, istPET_CT))
		mPETLoaded = true;

	if(mServices->patient()->isPatientValid())
	{
		triggerMainWindowActionWithObjectName("AddFilesForImportWithDialogCT");
		triggerMainWindowActionWithObjectName("ImportSelectedData");
	}
	dataAddedOrRemoved();
}

void NewLoadPatientWidget::dataAddedOrRemoved()
{
	if(!mServices->patient()->isPatientValid())
		return;

	QString text;
	bool allDataLoaded = false;
	bool ctAvailable = false;
	bool petAvailable = false;
	bool pet_ctAvailable = false;

	if(mServices->patient()->getImage(imCT, istTHORAX_CT))
		ctAvailable = true;
	if(mServices->patient()->getImage(imPET, istPET))
		petAvailable = true;
	if(mServices->patient()->getImage(imCT, istPET_CT))
		pet_ctAvailable = true;

	if(petAvailable && !mPETLoaded && ctAvailable && pet_ctAvailable && !mThoraxCTLoaded)
		text = "CT and PET data loaded. ";
	else if(ctAvailable && !mThoraxCTLoaded)
		text = "CT data loaded. ";
	else if(petAvailable && pet_ctAvailable && !mPETLoaded)
		text = "PET data loaded. ";
	else
		text = "No valid new data loaded. ";

	if (ctAvailable && petAvailable && pet_ctAvailable)
		allDataLoaded = true;
	else
		text.append("Do you want to load more data?");

	mDataLoadedInfo = new QDialog();
	mDataLoadedInfo->setWindowTitle(tr("Data Loaded"));
	mDataLoadedInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
	QGridLayout* layout = new QGridLayout();
	QLabel* textLabel = new QLabel(text);
	layout->addWidget(textLabel,0,0,1,2);
	QLabel* ctLabel;
	QLabel* petLabel;
	QLabel* pet_ctLabel;
	if(ctAvailable)
	{
		ctLabel = new QLabel("\nThorax CT:  OK");
		ctLabel->setStyleSheet("QLabel { color : green; Qt::RichText}");
	}
	else
	{
		ctLabel = new QLabel("\nThorax CT:  Not available");
		ctLabel->setStyleSheet("QLabel { color : red; }");
	}
	if(petAvailable)
	{
		petLabel = new QLabel("PET (optional):  OK");
		petLabel->setStyleSheet("QLabel { color : green;  Qt::RichText}");
	}
	else
	{
		petLabel = new QLabel("PET (optional):  Not available");
		petLabel->setStyleSheet("QLabel { color : red; }");
	}
	if(pet_ctAvailable)
	{
		pet_ctLabel = new QLabel("PET CT (optional):  OK");
		pet_ctLabel->setStyleSheet("QLabel { color : green;  Qt::RichText}");
	}
	else
	{
		pet_ctLabel = new QLabel("PET CT (optional):  Not available");
		pet_ctLabel->setStyleSheet("QLabel { color : red; }");
	}

	layout->addWidget(ctLabel,1,0,1,2);
	layout->addWidget(petLabel,2,0,1,2);
	layout->addWidget(pet_ctLabel,3,0,1,2);


	mYesButtonDataLoaded = new QPushButton(tr("Yes"));
	if(!allDataLoaded)
	{
		layout->addWidget(mYesButtonDataLoaded,4,0);
		mNoButtonDataLoaded = new QPushButton(tr("No"));
	}
	else
		mNoButtonDataLoaded = new QPushButton(tr("Continue"));

	layout->addWidget(mNoButtonDataLoaded,4,1);

	connect(mYesButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectMoreCTData);
	connect(mNoButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::closeDataLoadedInfo);
	mDataLoadedInfo->setLayout(layout);
	mDataLoadedInfo->show();
	mDataLoadedInfo->activateWindow();

	mThoraxCTLoaded = false;
	mPETLoaded = false;
}


}

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
#include "cxStyles.h"

namespace cx
{

NewLoadPatientWidget::NewLoadPatientWidget(QWidget *parent, VisServicesPtr services, AcquisitionServicePtr acquisitionService) :
	BaseWidget(parent, "new_load_patient_widget", "New/Load Patient"),
	mServices(services)
{
	this->setObjectName(this->getWidgetName());
	this->setWindowTitle("Create or select patient");

	QPushButton* newButton = new QPushButton("&New Patient");
	const QSize BUTTON_SIZE = QSize(1, 80); //New patient button is larger as it is most important
	newButton->setMinimumSize(BUTTON_SIZE);
	newButton->setIcon(QIcon(":/icons/icons/add.svg"));
	connect(newButton, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatient);

	QPushButton* loadButton = new QPushButton("&Load Patient");
	loadButton->setIcon(QIcon(":/icons/icons/select.svg"));
	connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadPatient);

	QPushButton* restoreToFactorySettingsButton = new QPushButton("&Restore factory settings");
	QPalette palette = restoreToFactorySettingsButton->palette();
	palette.setColor(QPalette::Button, Styles::getRed());
	restoreToFactorySettingsButton->setPalette(palette);
	connect(restoreToFactorySettingsButton, &QPushButton::clicked, this, &NewLoadPatientWidget::restoreToFactorySettings);

	mSelectCTDataButton = new QPushButton("&Select CT data");
	mSelectCTDataButton->setIcon(QIcon(":/icons/icons/import.svg"));
	mSelectCTDataButton->setEnabled(false);
	connect(mSelectCTDataButton, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addSpacing(50);
	layout->addWidget(newButton);
	layout->addSpacing(25);
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
	QTextEdit* textBox = new QTextEdit(
		"<b><u>A new is patient created</u></b><br>"
		"Do you want to import CT data?<br><br>"
		"A file dialog will open: <i>Select Medical Image file(s)</i><br>"
		"Locate the folder containing the CT files of the patient.<br>"
		"(A USB storage is located by clicking <i><b>media</b></i> on the left side of the dialog)<br><br>"
		"<u>Note:</u> If a folder containing an extensive amount data (e.g from several patients)"
		" is selected, it may be slow to load.");
	textBox->setReadOnly(true);
	textBox->setFixedWidth(600);
	layout->addWidget(textBox,0,0,1,2);
	QPushButton* yesButtonPatientCreated = new QPushButton(tr("Yes"));
	QPushButton* noButtonPatientCreated = new QPushButton(tr("No"));
	layout->addWidget(yesButtonPatientCreated,1,0);
	layout->addWidget(noButtonPatientCreated,1,1);
	mPatientCreatedInfo->setLayout(layout);
	mPatientCreatedInfo->show();
	mPatientCreatedInfo->activateWindow();

	mConnectionToYesButtonPatientCreated = connect(yesButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	mConnectionToNoButtonPatientCreated = connect(noButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::closePatientCreatedInfo);
}

void NewLoadPatientWidget::closePatientCreatedInfo()
{
	disconnect(mConnectionToYesButtonPatientCreated);
	disconnect(mConnectionToNoButtonPatientCreated);
	if(mPatientCreatedInfo)
	{
		mPatientCreatedInfo->close();
		mPatientCreatedInfo->deleteLater();
	}
	mPatientCreatedInfo = nullptr;
}

void NewLoadPatientWidget::closeDataLoadedInfo(bool dataLoadingCompleted)
{
	disconnect(mConnectionToYesButtonDataLoaded);
	disconnect(mConnectionToNoButtonDataLoaded);
	if(mDataLoadedInfo)
	{
		mDataLoadedInfo->close();
		mDataLoadedInfo->deleteLater();
	}
	mDataLoadedInfo = nullptr;

	if(dataLoadingCompleted && mServices->patient()->getImage(imCT, istTHORAX_CT))
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
	loadCTData();
}

void NewLoadPatientWidget::selectMoreCTData()
{
	closeDataLoadedInfo(false);
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

	QTextEdit* textBox = new QTextEdit();
	textBox->setReadOnly(true);
	textBox->setFixedWidth(400);

	if(petAvailable && !mPETLoaded && ctAvailable && pet_ctAvailable && !mThoraxCTLoaded)
		textBox->append("<b>CT and PET data loaded</b><br>");
	else if(ctAvailable && !mThoraxCTLoaded)
		textBox->append("<b>CT data loaded</b><br>");
	else if(petAvailable && pet_ctAvailable && !mPETLoaded)
		textBox->append("<b>PET data loaded</b><br>");
	else
		textBox->append("<b>No valid new data loaded</b><br>");

	if (ctAvailable && petAvailable && pet_ctAvailable)
		allDataLoaded = true;
	else
		textBox->append("Do you want to load more data?");

	mDataLoadedInfo = new QDialog();
	mDataLoadedInfo->setWindowTitle(tr("Data Loaded"));
	mDataLoadedInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
	QGridLayout* layout = new QGridLayout();
	QLabel* textLabel = new QLabel(text);
	layout->addWidget(textLabel,0,0,1,2);
	if(ctAvailable)
		textBox->append("<ul><li><font color=green><b> Thorax CT:  OK </b></font></li>");
	else
		textBox->append("<ul><li><font color=red><b> Thorax CT:  Not available </b></font></li>");
	if(petAvailable)
		textBox->append("<li><font color=green><b> PET (optional):  OK </b></font></li>");
	else
		textBox->append("<li><font color=red><b> PET (optional):  Not available </b></font></li>");
	if(pet_ctAvailable)
		textBox->append("<li><font color=green><b> PET CT (optional):  OK </b></font></li></ul>");
	else
		textBox->append("<li><font color=red><b> PET CT (optional):  Not available </b></font></li></ul>");

	layout->addWidget(textBox,1,0,1,2);


	QPushButton* yesButtonDataLoaded = new QPushButton(tr("Yes"));
	QPushButton* noButtonDataLoaded = nullptr;
	if(!allDataLoaded)
	{
		layout->addWidget(yesButtonDataLoaded,2,0);
		noButtonDataLoaded = new QPushButton(tr("No"));
	}
	else
		noButtonDataLoaded = new QPushButton(tr("Continue"));

	layout->addWidget(noButtonDataLoaded,2,1);

	mConnectionToYesButtonDataLoaded = connect(yesButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectMoreCTData);
	mConnectionToNoButtonDataLoaded = connect(noButtonDataLoaded, &QPushButton::clicked, this, [=]() {this->closeDataLoadedInfo(true);});
	mDataLoadedInfo->setLayout(layout);
	mDataLoadedInfo->show();
	mDataLoadedInfo->activateWindow();

	mThoraxCTLoaded = false;
	mPETLoaded = false;
}


}

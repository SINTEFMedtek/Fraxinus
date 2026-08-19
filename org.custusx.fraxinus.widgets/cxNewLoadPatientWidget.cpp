#include "cxNewLoadPatientWidget.h"

#include <functional>
#include <iostream>

#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QPushButton>
#include <QCheckBox>
#include <QDockWidget>
#include <QGroupBox>
#include <QAction>
#include <QDialog>
#include <QLabel>
#include <QTextEdit>

#include "cxApplication.h"
#include "cxLogger.h"
#include "cxLogicManager.h"
#include "cxDataLocations.h"
#include "cxPatientModelService.h"
#include "cxMesh.h"
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

	QPushButton* newButton = new QPushButton("  &Create new patient");
	const QSize BUTTON_SIZE = QSize(1, 80);
	newButton->setMinimumSize(BUTTON_SIZE);
	newButton->setIcon(QIcon(":/icons/icons/add.svg"));
	connect(newButton, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatient);

	QPushButton* newButtonFromUSB = new QPushButton("  &Create new patient from USB\n  and run segmentation");
	newButtonFromUSB->setMinimumSize(BUTTON_SIZE);
	newButtonFromUSB->setIcon(QIcon(":/icons/icons/add.svg"));
	connect(newButtonFromUSB, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatientFromUSB);

	QPushButton* loadButton = new QPushButton("  &Load existing patient");
	loadButton->setMinimumSize(BUTTON_SIZE);
	loadButton->setIcon(QIcon(":/icons/icons/select.svg"));
	connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadPatient);

	mSelectCTDataButton = new QPushButton("  &Select CT data");
	mSelectCTDataButton->setIcon(QIcon(":/icons/icons/import.svg"));
	mSelectCTDataButton->setEnabled(false);
	connect(mSelectCTDataButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTDataDialog);

	// Segmentation selection checkboxes
	mCheckBoxAirways = new QCheckBox("Airways, Lungs");
	mCheckBoxAirways->setChecked(true);
	mCheckBoxAirways->setDisabled(true);
	mStatusLabelAirways = new QLabel("~7 min");

	mCheckBoxLymphNodes = new QCheckBox("Lymph Nodes");
	mCheckBoxLymphNodes->setChecked(false);
	mStatusLabelLymphNodes = new QLabel("~2 min");

	mCheckBoxHeart = new QCheckBox("Heart");
	mCheckBoxHeart->setChecked(false);
	mStatusLabelHeart = new QLabel("~4 min");

	mCheckBoxMediumOrgans = new QCheckBox("Vena Cava, Aorta, Spine");
	mCheckBoxMediumOrgans->setChecked(false);
	mStatusLabelMediumOrgans = new QLabel("~3 min");

	mCheckBoxSmallOrgans = new QCheckBox("Small Mediastinal Organs");
	mCheckBoxSmallOrgans->setToolTip("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos");
	mCheckBoxSmallOrgans->setChecked(false);
	mStatusLabelSmallOrgans = new QLabel("~2 min");

	mCheckBoxTumors = new QCheckBox("Tumors");
	mCheckBoxTumors->setChecked(false);
	mStatusLabelTumors = new QLabel("~5 min");

	mCheckBoxLungVessels = new QCheckBox("Pulmonary arteries and veins");
	mCheckBoxLungVessels->setToolTip("Pulmonary Arteries, Pulmonary Veins");
	mCheckBoxLungVessels->setChecked(false);
	mStatusLabelLungVessels = new QLabel("~5 min");

	mCheckBoxLungLobes = new QCheckBox("Lung Lobes");
	mCheckBoxLungLobes->setChecked(false);
	mStatusLabelLungLobes = new QLabel("~5 min");

	mCheckBoxSelectAll = new QCheckBox("Select all");
	mCheckBoxSelectAll->setChecked(false);
	connect(mCheckBoxSelectAll, &QCheckBox::toggled, this, &NewLoadPatientWidget::selectAll);

	QGridLayout* segLayout = new QGridLayout();
	segLayout->setColumnStretch(0, 1);
	int segRow = 0;
	segLayout->addWidget(mCheckBoxSelectAll, segRow, 0, 1, 2);
	segRow++;
	QFrame* segSeparator = new QFrame();
	segSeparator->setFrameShape(QFrame::HLine);
	segSeparator->setFrameShadow(QFrame::Sunken);
	segLayout->addWidget(segSeparator, segRow, 0, 1, 2);
	segRow++;
	segLayout->addWidget(mCheckBoxAirways,      segRow, 0); segLayout->addWidget(mStatusLabelAirways,      segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxLymphNodes,   segRow, 0); segLayout->addWidget(mStatusLabelLymphNodes,   segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxHeart,        segRow, 0); segLayout->addWidget(mStatusLabelHeart,        segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxMediumOrgans, segRow, 0); segLayout->addWidget(mStatusLabelMediumOrgans, segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxSmallOrgans,  segRow, 0); segLayout->addWidget(mStatusLabelSmallOrgans,  segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxTumors,       segRow, 0); segLayout->addWidget(mStatusLabelTumors,       segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxLungVessels,  segRow, 0); segLayout->addWidget(mStatusLabelLungVessels,  segRow, 1); segRow++;
	segLayout->addWidget(mCheckBoxLungLobes,    segRow, 0); segLayout->addWidget(mStatusLabelLungLobes,    segRow, 1);
	QGroupBox* segmentationGroup = new QGroupBox("Segmentation");
	segmentationGroup->setLayout(segLayout);

	mRunSegmentationButton = new QPushButton("Run Segmentation");
	mRunSegmentationButton->setIcon(QIcon(":/icons/icons/processing.svg"));
	mRunSegmentationButton->setEnabled(false);
	connect(mRunSegmentationButton, &QPushButton::clicked, this, &NewLoadPatientWidget::runSegmentationClicked);

	connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved,
	        this, &NewLoadPatientWidget::updateRunSegmentationButton);
	connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved,
	        this, &NewLoadPatientWidget::updateSegmentationCheckBoxes);
	connect(mServices->patient().get(), &PatientModelService::patientChanged,
	        this, &NewLoadPatientWidget::updateSegmentationCheckBoxes);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addSpacing(50);
	layout->addWidget(segmentationGroup);
	layout->addWidget(mRunSegmentationButton);
	layout->addSpacing(50);
	layout->addWidget(newButtonFromUSB);
	layout->addSpacing(25);
	layout->addWidget(newButton);
	layout->addSpacing(25);
	layout->addWidget(loadButton);
	layout->addSpacing(50);

	layout->addWidget(mSelectCTDataButton);


	mProcessingInfoGroup = new QGroupBox("Segmentation status");
	mProcessingInfoGroup->setVisible(false);
	layout->addWidget(mProcessingInfoGroup);

	layout->addStretch();

	QString profile = ProfileManager::getInstance()->activeProfile()->getUid();
	if(profile == "VideoRecording")
	{
		FraxinusVideoRecorderWidget* videoRecorderWidget = new FraxinusVideoRecorderWidget(mServices, acquisitionService, this);
		layout->addWidget(videoRecorderWidget);
		layout->addStretch();
	}

	this->setLayout(layout);
}

QString NewLoadPatientWidget::getWidgetName()
{
	return "new_load_patient_widget";
}

QGroupBox* NewLoadPatientWidget::getProcessingInfoGroup()
{
	mProcessingInfoGroup->setVisible(true);
	return mProcessingInfoGroup;
}

void NewLoadPatientWidget::createNewPatient()
{
	QString actionName = "CreatePatientWithPatientName";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	//patientCreatedInfo();
	loadCTDataDialog();
}

void NewLoadPatientWidget::createNewPatientFromUSB()
{
	QString actionName = "CreatePatientWithPatientName";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	mSkipDataLoadedInfo = true;
	loadCTDataFromUSB();
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

	mConnectionToYesButtonPatientCreated = connect(yesButtonPatientCreated, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTDataDialog);
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
	QString actionName = "LoadFileWithSimpleDialog";
	triggerMainWindowActionWithObjectName(actionName);
	enableImportDataButton();
	emit existingPatientLoaded();
}

void NewLoadPatientWidget::enableImportDataButton()
{
	if(mServices->patient()->isPatientValid())
		mSelectCTDataButton->setEnabled(true);
	else
		mSelectCTDataButton->setEnabled(false);
	this->updateRunSegmentationButton();
	this->updateSegmentationCheckBoxes();
}

void NewLoadPatientWidget::updateRunSegmentationButton()
{
	bool ctLoaded = mServices->patient()->getImage(imCT, istTHORAX_CT) != nullptr;
	bool patientValid = mServices->patient()->isPatientValid();
	mRunSegmentationButton->setEnabled(ctLoaded && patientValid && !mSegmentationRunning);
}

void NewLoadPatientWidget::segmentationStarted()
{
	mSegmentationRunning = true;
	mRunSegmentationButton->setEnabled(false);
}

void NewLoadPatientWidget::segmentationFinished()
{
	mSegmentationRunning = false;
	this->updateRunSegmentationButton();
}

void NewLoadPatientWidget::updateSegmentationCheckBoxes()
{
	bool patientValid = mServices->patient()->isPatientValid();

	std::function<void(QCheckBox*, QLabel*)> setDone = [](QCheckBox* cb, QLabel* lbl) {
		cb->setChecked(false);
		cb->setDisabled(true);
		QColor c = Styles::getGreen();
		lbl->setStyleSheet(QString("color: rgb(%1,%2,%3); font-weight: bold;")
			.arg(c.red()).arg(c.green()).arg(c.blue()));
		lbl->setText("Done");
	};
	std::function<void(QLabel*, const QString&)> setPending = [](QLabel* lbl, const QString& est) {
		lbl->setStyleSheet("");
		lbl->setText(est);
	};

	if(patientValid && mServices->patient()->getData<Mesh>(otAIRWAYS_CENTERLINES))
		setDone(mCheckBoxAirways, mStatusLabelAirways);
	else
	{
		mCheckBoxAirways->setChecked(true);
		mCheckBoxAirways->setDisabled(true);
		setPending(mStatusLabelAirways, "~7 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otLYMPH_NODES))
		setDone(mCheckBoxLymphNodes, mStatusLabelLymphNodes);
	else
	{
		mCheckBoxLymphNodes->setDisabled(false);
		setPending(mStatusLabelLymphNodes, "~2 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otHEART))
		setDone(mCheckBoxHeart, mStatusLabelHeart);
	else
	{
		mCheckBoxHeart->setDisabled(false);
		setPending(mStatusLabelHeart, "~4 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otSPINE))
		setDone(mCheckBoxMediumOrgans, mStatusLabelMediumOrgans);
	else
	{
		mCheckBoxMediumOrgans->setDisabled(false);
		setPending(mStatusLabelMediumOrgans, "~3 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otESOPHAGUS))
		setDone(mCheckBoxSmallOrgans, mStatusLabelSmallOrgans);
	else
	{
		mCheckBoxSmallOrgans->setDisabled(false);
		setPending(mStatusLabelSmallOrgans, "~2 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otTUMOR))
		setDone(mCheckBoxTumors, mStatusLabelTumors);
	else
	{
		mCheckBoxTumors->setDisabled(false);
		setPending(mStatusLabelTumors, "~5 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otPULMONARY_ARTERIES))
		setDone(mCheckBoxLungVessels, mStatusLabelLungVessels);
	else
	{
		mCheckBoxLungVessels->setDisabled(false);
		setPending(mStatusLabelLungVessels, "~5 min");
	}

	if(patientValid && mServices->patient()->getData<Mesh>(otLOBE_LUL))
		setDone(mCheckBoxLungLobes, mStatusLabelLungLobes);
	else
	{
		mCheckBoxLungLobes->setDisabled(false);
		setPending(mStatusLabelLungLobes, "~5 min");
	}
}

void NewLoadPatientWidget::selectAll(bool checked)
{
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

bool NewLoadPatientWidget::isAirwaysChecked() const { return mCheckBoxAirways->isChecked(); }
bool NewLoadPatientWidget::isLymphNodesChecked() const { return mCheckBoxLymphNodes->isChecked(); }
bool NewLoadPatientWidget::isHeartChecked() const { return mCheckBoxHeart->isChecked(); }
bool NewLoadPatientWidget::isMediumOrgansChecked() const { return mCheckBoxMediumOrgans->isChecked(); }
bool NewLoadPatientWidget::isSmallOrgansChecked() const { return mCheckBoxSmallOrgans->isChecked(); }
bool NewLoadPatientWidget::isTumorsChecked() const { return mCheckBoxTumors->isChecked(); }
bool NewLoadPatientWidget::isLungVesselsChecked() const { return mCheckBoxLungVessels->isChecked(); }
bool NewLoadPatientWidget::isLungLobesChecked() const { return mCheckBoxLungLobes->isChecked(); }

void NewLoadPatientWidget::selectCTData()
{
	closeDataLoadedInfo(false);
	loadCTDataDialog();
}

void NewLoadPatientWidget::loadCTDataDialog()
{
	closePatientCreatedInfo();

	if(!mLoadCTDialog)
		mLoadCTDialog = new QDialog();
	mLoadCTDialog->setWindowTitle(tr("Load CT data from..."));
	mLoadCTDialog->setWindowFlags(Qt::WindowStaysOnTopHint);

	if(!mUSBButton)
		mUSBButton = new QPushButton(tr("&USB drive"));
	if(!mHardDriveButton)
		mHardDriveButton = new QPushButton(tr("&Computer"));

	mUSBButton->setMinimumHeight(80);
	mUSBButton->setMinimumWidth(200);
	mHardDriveButton->setMinimumHeight(80);
	mHardDriveButton->setMinimumWidth(200);

	connect(mUSBButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTDataFromUSB);
	connect(mHardDriveButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTData);
	connect(mLoadCTDialog, &QDialog::finished, this, &NewLoadPatientWidget::loadCTDataDialogFinished);

	QGridLayout* mainLayout = new QGridLayout;
	mainLayout->setSizeConstraint(QLayout::SetFixedSize);
	mainLayout->addWidget(mUSBButton, 0, 0);
	mainLayout->addWidget(mHardDriveButton, 0, 1);
	mLoadCTDialog->setLayout(mainLayout);
	mLoadCTDialog->show();
	mLoadCTDialog->activateWindow();
}

void NewLoadPatientWidget::loadCTDataDialogFinished()
{
	if(mLoadCTDialog)
	{
		mLoadCTDialog->hide();
		mLoadCTDialog->deleteLater();
		mLoadCTDialog = nullptr;
	}
	if(mUSBButton)
	{
		disconnect(mUSBButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTDataFromUSB);
		mUSBButton = nullptr;
	}
	if(mHardDriveButton)
	{
		disconnect(mHardDriveButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadCTData);
		mHardDriveButton = nullptr;
	}
}

void NewLoadPatientWidget::loadCTDataFromUSB()
{
	loadCTData(true);
}

void NewLoadPatientWidget::loadCTData(bool fromUSB)
{
	loadCTDataDialogFinished();

	if(mServices->patient()->getImage(imCT, istTHORAX_CT))
		mThoraxCTLoaded = true;
	if(mServices->patient()->getImage(imPET, istPET) && mServices->patient()->getImage(imCT, istPET_CT))
		mPETLoaded = true;

	if(mServices->patient()->isPatientValid())
	{
		if(fromUSB)
			triggerMainWindowActionWithObjectName("AddFilesForImportFromUSB");
		else
			triggerMainWindowActionWithObjectName("AddFilesForImportWithDialogCT");
		triggerMainWindowActionWithObjectName("ImportSelectedData");
		QDockWidget* importDockWidget = findMainWindowChildWithObjectName<QDockWidget*>("import_widgetDockWidget");
		if(importDockWidget)
			importDockWidget->hide();
	}
	dataAddedOrRemoved();
}

void NewLoadPatientWidget::dataAddedOrRemoved()
{
	if(!mServices->patient()->isPatientValid())
		return;

	bool ctAvailable = mServices->patient()->getImage(imCT, istTHORAX_CT) != nullptr;
	bool petAvailable = mServices->patient()->getImage(imPET, istPET) != nullptr;
	bool pet_ctAvailable = mServices->patient()->getImage(imCT, istPET_CT) != nullptr;
	bool allDataLoaded = ctAvailable && petAvailable && pet_ctAvailable;

	bool thoraxCTJustLoaded = ctAvailable && !mThoraxCTLoaded;
	bool petJustLoaded = petAvailable && pet_ctAvailable && !mPETLoaded;

	bool skipInfo = allDataLoaded || (mSkipDataLoadedInfo && ctAvailable);

	mSkipDataLoadedInfo = false;
	mThoraxCTLoaded = false;
	mPETLoaded = false;

	if(skipInfo)
	{
		emit dataImportCompleted();
		return;
	}

	if(mDataLoadedInfo)
		return;

	QTextEdit* textBox = new QTextEdit();
	textBox->setReadOnly(true);
	textBox->setFixedWidth(400);

	if(thoraxCTJustLoaded && petJustLoaded)
		textBox->append("<b>CT and PET data loaded</b><br>");
	else if(thoraxCTJustLoaded)
		textBox->append("<b>CT data loaded</b><br>");
	else if(petJustLoaded)
		textBox->append("<b>PET data loaded</b><br>");
	else
		textBox->append("<b>No valid new data loaded</b><br>");

	textBox->append("Do you want to load more data?");

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

	mDataLoadedInfo = new QDialog();
	mDataLoadedInfo->setWindowTitle(tr("Data Loaded"));
	mDataLoadedInfo->setWindowFlags(Qt::WindowStaysOnTopHint);
	QGridLayout* layout = new QGridLayout();
	layout->addWidget(textBox, 0, 0, 1, 2);

	QPushButton* yesButtonDataLoaded = new QPushButton(tr("Yes"));
	QPushButton* noButtonDataLoaded = new QPushButton(tr("No"));
	layout->addWidget(yesButtonDataLoaded, 1, 0);
	layout->addWidget(noButtonDataLoaded, 1, 1);

	mConnectionToYesButtonDataLoaded = connect(yesButtonDataLoaded, &QPushButton::clicked, this, &NewLoadPatientWidget::selectCTData);
	mConnectionToNoButtonDataLoaded = connect(noButtonDataLoaded, &QPushButton::clicked, this, [=]() {this->closeDataLoadedInfo(true);});
	mDataLoadedInfo->setLayout(layout);
	mDataLoadedInfo->show();
	mDataLoadedInfo->activateWindow();
}


}

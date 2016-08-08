#include "cxNewLoadPatientWidget.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>

#include "cxApplication.h"
#include "cxLogger.h"
#include "cxLogicManager.h"
#include "cxDataLocations.h"

namespace cx
{

NewLoadPatientWidget::NewLoadPatientWidget(QWidget *parent, PatientModelServicePtr patient) :
	BaseWidget(parent, "new_load_patient_widget", "New/Load Patient"),
    mPatient(patient)
{
    this->setWindowTitle("Create or select patient");

    QPushButton* newButton = new QPushButton("&New Patient");
    newButton->setIcon(QIcon(":/icons/icons/add.svg"));
    connect(newButton, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatient);

    QPushButton* loadButton = new QPushButton("&Load Patient");
    loadButton->setIcon(QIcon(":/icons/icons/select.svg"));
    connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadPatient);

    QPushButton* restoreToFactorySettingsButton = new QPushButton("&Restore factory settings");
    connect(restoreToFactorySettingsButton, &QPushButton::clicked, this, &NewLoadPatientWidget::restoreToFactorySettings);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(newButton);
    layout->addWidget(loadButton);
    layout->addWidget(restoreToFactorySettingsButton);
    layout->addStretch();
    this->setLayout(layout);
}

void NewLoadPatientWidget::createNewPatient()
{
    QString actionName = "NewPatient";
    triggerMainWindowActionWithObjectName(actionName);
}

void NewLoadPatientWidget::loadPatient()
{
    QString actionName = "LoadFile";
    triggerMainWindowActionWithObjectName(actionName);
}

void NewLoadPatientWidget::restoreToFactorySettings()
{
    DataLocations::deletePersistentWritablePath();
    LogicManager::getInstance()->restartServicesWithProfile("Bronchoscopy");
}


}

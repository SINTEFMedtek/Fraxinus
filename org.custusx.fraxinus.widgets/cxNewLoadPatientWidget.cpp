#include "cxNewLoadPatientWidget.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>

#include "cxApplication.h"
#include "cxLogger.h"

namespace cx
{

NewLoadPatientWidget::NewLoadPatientWidget(QWidget *parent, PatientModelServicePtr patient) :
    BaseWidget(parent, "NewLoadPatientWidget", "New/Load Patient"),
    mPatient(patient)
{
    this->setWindowTitle("Create or select patient");

    QPushButton* newButton = new QPushButton("&New Patient");
    newButton->setIcon(QIcon(":/icons/icons/import.png"));
    connect(newButton, &QPushButton::clicked, this, &NewLoadPatientWidget::createNewPatient);

    QPushButton* loadButton = new QPushButton("&Load Patient");
    loadButton->setIcon(QIcon(":/icons/icons/import.png"));
    connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientWidget::loadPatient);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(newButton);
    layout->addWidget(loadButton);
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


}

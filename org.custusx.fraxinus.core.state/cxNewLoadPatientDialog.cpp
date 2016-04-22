#include "cxNewLoadPatientDialog.h"

#include <iostream>

#include <QHBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QAction>

#include "cxLogger.h"

namespace cx
{

NewLoadPatientDialog::NewLoadPatientDialog(QWidget *parent, PatientModelServicePtr patient) :
    QDialog(parent, Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint),
    mPatient(patient)
{
    this->setWindowTitle("Create or select patient");

    QPushButton* newButton = new QPushButton("&New");
    connect(newButton, &QPushButton::clicked, this, &NewLoadPatientDialog::createNewPatient);

    QPushButton* loadButton = new QPushButton("&Load");
    connect(loadButton, &QPushButton::clicked, this, &NewLoadPatientDialog::loadPatient);

    QPushButton* cancelButton = new QPushButton("&Cancel");
    connect(cancelButton, &QPushButton::clicked, this, &NewLoadPatientDialog::closeDialog);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(newButton);
    layout->addWidget(loadButton);
    layout->addWidget(cancelButton);
    this->setLayout(layout);
}

void NewLoadPatientDialog::createNewPatient()
{
    QString actionName = "NewPatient";
    this->triggerMainWindowActionWithObjectName(actionName);
    this->closeDialog();
}

void NewLoadPatientDialog::loadPatient()
{
    QString actionName = "LoadFile";
    this->triggerMainWindowActionWithObjectName(actionName);
    this->closeDialog();
}

void NewLoadPatientDialog::closeDialog()
{
    if(mPatient->isPatientValid())
    {
        this->close();
    } else {
        CX_LOG_WARNING() << "You have to select a patient before proceeding.";
    }
}

void NewLoadPatientDialog::triggerMainWindowActionWithObjectName(QString actionName)
{
    QWidget *mainwindow = Q_NULLPTR;
    foreach(mainwindow, QApplication::topLevelWidgets()) {
      if(mainwindow->objectName() == "MainWindow")
        break;
    }
    QAction* action = mainwindow->findChild<QAction*>(actionName);
    if(action)
    {
        action->trigger();
    }
}

}

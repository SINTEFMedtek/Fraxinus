#include "cxPinpointWidget.h"

#include <QPushButton>
#include <QLineEdit>
#include <QAction>

#include "cxApplication.h"
#include "cxMetricManager.h"
#include "cxPointMetric.h"
#include "cxVisServices.h"
#include "cxPatientModelService.h"
#include "cxSpaceProvider.h"


namespace cx {

PinpointWidget::PinpointWidget(VisServicesPtr services, QWidget *parent) :
    BaseWidget(parent, "PinpointWidget", "Pinpoint"),
    mServices(services),
    mMetricManager(new MetricManager()),
    mMetricUid("fraxinus_target"),
    mMetricName("Target")
{
    mMetricManager->setActiveUid(mMetricUid);

    QPushButton *setPointMetric = new QPushButton("&Set target", this);
    connect(setPointMetric, &QPushButton::clicked, this, &PinpointWidget::setPointMetric);
    QPushButton *centerToImage = new QPushButton(QIcon(":/icons/center_image.png"), "", this);
    connect(centerToImage, &QPushButton::clicked, this, &PinpointWidget::centerToImage);
    QLineEdit *pointMetricName = new QLineEdit(mMetricName, this);
    connect(pointMetricName, &QLineEdit::textEdited, this, &PinpointWidget::targetNameChanged);

    QVBoxLayout *v_layout = new QVBoxLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addWidget(pointMetricName);
    h_layout->addWidget(centerToImage);
    h_layout->addWidget(setPointMetric);
    h_layout->addStretch();
    v_layout->addLayout(h_layout);
    v_layout->addStretch();

    this->setLayout(v_layout);
}

void PinpointWidget::setPointMetric()
{
    if(!mServices->patient()->getData(mMetricUid))
        this->createPointMetric();
    else
        this->updateCoordinateOfPointMetric();
}

void PinpointWidget::targetNameChanged(const QString &text)
{
    mMetricName = text;
    if(!mMetricManager->getMetric(mMetricUid))
        this->createPointMetric();
    else
        this->updateNameOfPointMetric();
}

void PinpointWidget::centerToImage()
{
    triggerMainWindowActionWithObjectName("CenterToImageCenter");
}

void PinpointWidget::createPointMetric()
{
    CoordinateSystem ref = CoordinateSystem::reference();
    QColor color = QColor(250, 0, 0, 255);
    Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(ref, true);

    mMetricManager->addPoint(p_ref, ref, mMetricUid, color);

    this->updateNameOfPointMetric();
}

void PinpointWidget::updateCoordinateOfPointMetric()
{
    DataPtr data = mServices->patient()->getData(mMetricUid);
    PointMetricPtr point = boost::dynamic_pointer_cast<PointMetric>(data);
    Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(CoordinateSystem::reference(), true);
    point->setCoordinate(p_ref);
}

void PinpointWidget::updateNameOfPointMetric()
{
    DataMetricPtr data = mMetricManager->getMetric(mMetricUid);
    if(data)
        data->setName(mMetricName);
}

}


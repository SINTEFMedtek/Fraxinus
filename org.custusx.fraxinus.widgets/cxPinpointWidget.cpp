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
#include "cxLogger.h"


namespace cx {

PinpointWidget::PinpointWidget(VisServicesPtr services, QWidget *parent) :
	BaseWidget(parent, "pinpoint_widget", "Pinpoint"),
    mServices(services),
    mMetricManager(new MetricManager(services->view(), services->patient(), services->tracking(), services->spaceProvider())),
	mTargetMetricUid(this->getTargetMetricUid()),
	mTargetMetricName("Target")
{
	mMetricManager->setActiveUid(mTargetMetricUid);

    QPushButton *setPointMetric = new QPushButton("&Set target", this);
    connect(setPointMetric, &QPushButton::clicked, this, &PinpointWidget::setPointMetric);
    QPushButton *centerToImage = new QPushButton(QIcon(":/icons/center_image.png"), "", this);
    connect(centerToImage, &QPushButton::clicked, this, &PinpointWidget::centerToImage);
	mPointMetricNameLineEdit = new QLineEdit(mTargetMetricName, this);
    connect(mPointMetricNameLineEdit, &QLineEdit::textEdited, this, &PinpointWidget::targetNameChanged);

    connect(mServices->patient().get(), &PatientModelService::patientChanged, this, &PinpointWidget::loadNameOfPointMetric);

    QVBoxLayout *v_layout = new QVBoxLayout();
    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addWidget(mPointMetricNameLineEdit);
    h_layout->addWidget(centerToImage);
    h_layout->addWidget(setPointMetric);
    h_layout->addStretch();
    v_layout->addLayout(h_layout);
    v_layout->addStretch();

	this->setLayout(v_layout);
}

QString PinpointWidget::getTargetMetricUid()
{
	return "fraxinus_target";
}

QString PinpointWidget::getToolMetricUid()
{
	return "Endoscope";
}

QString PinpointWidget::getDistanceMetricUid()
{
	return "distance1";
}

void PinpointWidget::setPointMetric()
{
	if(!mServices->patient()->getData(mTargetMetricUid))
        this->createPointMetric();
    else
        this->updateCoordinateOfPointMetric();

	if(!mServices->patient()->getData(this->getToolMetricUid()))
		this->createToolMetric();

	if(!mServices->patient()->getData(this->getDistanceMetricUid()))
		this->createDistanceMetric();

	emit targetMetricSet();
}

void PinpointWidget::targetNameChanged(const QString &text)
{
	mTargetMetricName = text;

	if(mMetricManager->getMetric(mTargetMetricUid))
		this->setNameOfPointMetric();
}

void PinpointWidget::loadNameOfPointMetric()
{
	mTargetMetricName = this->getNameOfPointMetric();
    mPointMetricNameLineEdit->blockSignals(true);
	mPointMetricNameLineEdit->setText(mTargetMetricName);
    mPointMetricNameLineEdit->blockSignals(false);
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

	mMetricManager->addPoint(p_ref, ref, mTargetMetricUid, color);

	this->setNameOfPointMetric();
}

void PinpointWidget::createToolMetric()
{
	CoordinateSystem tool(COORDINATE_SYSTEM::csTOOL, "active");
	mMetricManager->addPoint(Vector3D(0,0,0), tool, this->getToolMetricUid());
}

void PinpointWidget::createDistanceMetric()
{
	mMetricManager->addDistanceButtonClickedSlot();
}

void PinpointWidget::updateCoordinateOfPointMetric()
{
	DataPtr data = mServices->patient()->getData(mTargetMetricUid);
    PointMetricPtr point = boost::dynamic_pointer_cast<PointMetric>(data);
    Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(CoordinateSystem::reference(), true);
    point->setCoordinate(p_ref);
}

void PinpointWidget::setNameOfPointMetric()
{
	DataMetricPtr data = mMetricManager->getMetric(mTargetMetricUid);
    if(data)
		data->setName(mTargetMetricName);
}

QString PinpointWidget::getNameOfPointMetric() const
{
	DataMetricPtr data = mMetricManager->getMetric(mTargetMetricUid);
	QString metricName = mTargetMetricName;
    if(data)
    {
        metricName = data->getName();
    }
    return metricName;
}

}


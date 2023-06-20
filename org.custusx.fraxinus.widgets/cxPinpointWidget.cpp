#include "cxPinpointWidget.h"

#include <QPushButton>
#include <QLineEdit>
#include <QAction>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>

#include "cxApplication.h"
#include "cxMetricManager.h"
#include "cxPointMetric.h"
#include "cxDistanceMetric.h"
#include "cxVisServices.h"
#include "cxPatientModelService.h"
#include "cxSpaceProvider.h"
#include "cxLogger.h"
#include "cxViewService.h"
#include "cxCameraControl.h"


namespace cx {

PinpointWidget::PinpointWidget(VisServicesPtr services, QWidget *parent) :
	BaseWidget(parent, "pinpoint_widget", "Set target"),
	mServices(services),
	mMetricManager(new MetricManager(services->view(), services->patient(), services->tracking(), services->spaceProvider(), services->file())),
	mTargetMetricUid(this->getTargetMetricUid()),
	mTargetMetricName("Target"),
	mViaMetricUid(this->getViaPointMetricUid()),
	mViaMetricName("Via")
{
	mMetricManager->setActiveUid(mTargetMetricUid);

	QPushButton *continueToVB = new QPushButton("&Continue to virtual bronchoscopy", this);
	connect(continueToVB, &QPushButton::clicked, this, &PinpointWidget::setTargetMetric);
	QPushButton *centerToImage = new QPushButton(QIcon(":/icons/center_image.png"), " Center Image", this);
	connect(centerToImage, &QPushButton::clicked, this, &PinpointWidget::centerToImage);
	mPointMetricNameLineEdit = new QLineEdit(mTargetMetricName, this);
	connect(mPointMetricNameLineEdit, &QLineEdit::textEdited, this, &PinpointWidget::targetNameChanged);


	// Selector for using via point in route to target
	mViaPointCheckBox = new QCheckBox(tr("Use via point"));
	mViaPointCheckBox->setChecked(false);
	QButtonGroup *viaPointSelectorGroup = new QButtonGroup(this);
	mTargetPointButton = new QPushButton("&Set target point", this);
	mViaPointButton = new QPushButton("&Set airway end point", this);
	viaPointSelectorGroup->addButton(mViaPointButton);
	viaPointSelectorGroup->addButton(mTargetPointButton);

	QGridLayout* gridLayoutSetTarget = new QGridLayout;
	gridLayoutSetTarget->addWidget(mViaPointCheckBox,1,0);
	gridLayoutSetTarget->addWidget(mTargetPointButton,0,1);
	gridLayoutSetTarget->addWidget(mViaPointButton,1,1);

	mViaPointButton->hide();


	connect(mServices->patient().get(), &PatientModelService::patientChanged, this, &PinpointWidget::loadNameOfPointMetric);

	// Selector for 2D Window type
	QButtonGroup *windowGroup = new QButtonGroup(this);
	mLungWindow = new QRadioButton(tr("Lung"));
	mAbdomenWindow = new QRadioButton(tr("Abdomen"));
	mLungWindow->setChecked(true);
	windowGroup->addButton(mLungWindow);
	windowGroup->addButton(mAbdomenWindow);
	QGridLayout* gridLayoutWindow = new QGridLayout;
	gridLayoutWindow->addWidget(mLungWindow,0,0);
	gridLayoutWindow->addWidget(mAbdomenWindow,0,1);
	QGroupBox* windowBox = new QGroupBox(tr("CT Window Type"));
	windowBox->setLayout(gridLayoutWindow);

	QVBoxLayout *v_layout = new QVBoxLayout();
	QHBoxLayout *h_layout = new QHBoxLayout();
	h_layout->addWidget(mPointMetricNameLineEdit);
	h_layout->addWidget(continueToVB);
	v_layout->addSpacing(50);
	v_layout->addLayout(h_layout);
	v_layout->addSpacing(30);
	v_layout->addLayout(gridLayoutSetTarget);
	v_layout->addSpacing(30);
	v_layout->addWidget(centerToImage);
	v_layout->addStretch();
	v_layout->addWidget(windowBox);
	v_layout->addSpacing(30);


	mStructuresSelectionWidget = new StructuresSelectionWidget(mServices,this);
	QGroupBox* structuresBox = new QGroupBox(tr("Select structures"));
	QVBoxLayout* structuresLayout = new QVBoxLayout();
	structuresLayout->addWidget(mStructuresSelectionWidget);
	structuresBox->setLayout(structuresLayout);
	v_layout->addWidget(structuresBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
	v_layout->addStretch(); //And add some more stretch

	this->setLayout(v_layout);

	connect(mViaPointCheckBox, &QCheckBox::clicked, this, &PinpointWidget::useViaPointOn);
	connect(mTargetPointButton, &QPushButton::clicked, this, &PinpointWidget::setTargetPoint);
	connect(mViaPointButton, &QPushButton::clicked, this, &PinpointWidget::setViaPoint);

	connect(mLungWindow, &QRadioButton::clicked, this, &PinpointWidget::setLungWindow);
	connect(mAbdomenWindow, &QRadioButton::clicked, this, &PinpointWidget::setAbdomenWindow);
}

QString PinpointWidget::getTargetMetricUid()
{
	return "fraxinus_target";
}

QString PinpointWidget::getViaPointMetricUid()
{
	return "fraxinus_via_point";
}

QString PinpointWidget::getEndoscopeMetricUid()
{
	return "Endoscope";
}

QString PinpointWidget::getDistanceMetricUid()
{
	return "DistanceToTarget";
}

void PinpointWidget::setTargetMetric()
{
	if(!mServices->patient()->getData(mTargetMetricUid))
		this->createPointMetric();
	else
		this->updateCoordinateOfTargetMetric();

	if(!mServices->patient()->getData(this->getEndoscopeMetricUid()))
		this->createEndoscopeMetric();

	DistanceMetricPtr distanceMetric = boost::dynamic_pointer_cast<DistanceMetric>(mServices->patient()->getData(this->getDistanceMetricUid()));
	if(!distanceMetric)
		this->createDistanceMetric();
	else if(!distanceMetric->isValid())
		this->createDistanceMetric();

	emit targetMetricSet();
}

void PinpointWidget::setViaMetric()
{
	if(!mServices->patient()->getData(mViaMetricUid))
		this->createViaMetric();
	else
		this->updateCoordinateOfViaMetric();
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

	CameraControlPtr cameraControl = mServices->view()->getCameraControl();
	ViewPtr view3D = mServices->view()->get3DView();
	cameraControl->setView(view3D);
	cameraControl->setStandard3DView(CameraControl::AnteriorDirection());
	triggerMainWindowActionWithObjectName("CenterToImageCenter");
}

void PinpointWidget::createPointMetric()
{
	CoordinateSystem ref = CoordinateSystem::reference();
	QColor color = QColor(0, 0, 250, 255);
	Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(ref, true);

	mMetricManager->addPoint(p_ref, ref, mTargetMetricUid, color);

	this->setNameOfPointMetric();
}

void PinpointWidget::createViaMetric()
{
	CoordinateSystem ref = CoordinateSystem::reference();
	QColor color = QColor(250, 0, 0, 255);
	Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(ref, true);

	mMetricManager->addPoint(p_ref, ref, mViaMetricUid, color);

	this->setNameOfViaMetric();
}

void PinpointWidget::createEndoscopeMetric()
{
	CoordinateSystem tool(COORDINATE_SYSTEM::csTOOL, "active");
	mMetricManager->addPoint(Vector3D(0,0,0), tool, this->getEndoscopeMetricUid());
}

void PinpointWidget::createDistanceMetric()
{
	DistanceMetricPtr distance = mMetricManager->addDistance(this->getDistanceMetricUid());
	distance->setName("Direct distance");

	DataPtr target = mServices->patient()->getData(mTargetMetricUid);
	DataPtr endoscope = mServices->patient()->getData(this->getEndoscopeMetricUid());

	MetricReferenceArgumentListPtr arg = distance->getArguments();
	arg->set(0, target);
	arg->set(1, endoscope);
}

void PinpointWidget::updateCoordinateOfPointMetric(QString pointMetricName)
{
	DataPtr data = mServices->patient()->getData(pointMetricName);
	PointMetricPtr point = boost::dynamic_pointer_cast<PointMetric>(data);
	if(!point)
		return;

	Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(CoordinateSystem::reference(), true);
	point->setCoordinate(p_ref);
}

void PinpointWidget::updateCoordinateOfTargetMetric()
{
	this->updateCoordinateOfPointMetric(mTargetMetricUid);
}

void PinpointWidget::updateCoordinateOfViaMetric()
{
	this->updateCoordinateOfPointMetric(mViaMetricUid);
}

void PinpointWidget::setNameOfPointMetric()
{
	DataMetricPtr data = mMetricManager->getMetric(mTargetMetricUid);
	if(data)
		data->setName(mTargetMetricName);
}

void PinpointWidget::setNameOfViaMetric()
{
	DataMetricPtr data = mMetricManager->getMetric(mViaMetricUid);
	if(data)
		data->setName(mViaMetricName);
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

StructuresSelectionWidget* PinpointWidget::getStructuresSelectionWidget()
{
	return mStructuresSelectionWidget;
}

void PinpointWidget::useViaPointOn(bool checked)
{
	mUseViaPoint = mViaPointCheckBox->isChecked();
	if(checked)
		mViaPointButton->show();
	else
		mViaPointButton->hide();

	emit updateRoute();
}

void PinpointWidget::setTargetPoint()
{
	emit updateTargetPointFromManualTool();
}

void PinpointWidget::setViaPoint()
{
	emit updateViaPointFromManualTool();
}

bool PinpointWidget::getViaOption()
{
	return mUseViaPoint;
}

void PinpointWidget::setLungWindow()
{
	emit useLungWindow();
}

void PinpointWidget::setAbdomenWindow()
{
	emit useAbdomenWindow();
}

void PinpointWidget::setLungWindowButtonOn()
{
	mLungWindow->setChecked(true);
}

}


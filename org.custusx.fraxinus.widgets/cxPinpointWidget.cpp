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
#include "cxImage.h"
#include "cxSessionStorageService.h"
#include "cxSpaceProvider.h"
#include "cxLogger.h"
#include "cxViewService.h"
#include "cxCameraControl.h"
#include "cxXmlOptionItem.h"
#include "cxXMLNodeWrapper.h"


namespace cx {

PinpointWidget::PinpointWidget(VisServicesPtr services, QWidget *parent) :
	BaseWidget(parent, "pinpoint_widget", "Set target"),
	mServices(services),
	mMetricManager(new MetricManager(services->view(), services->patient(), services->tracking(), services->spaceProvider(), services->file())),
	mTargetMetricUid(this->getTargetMetricUid()),
	mTargetMetricName("Target"),
	mAirwayPointMetricUid(this->getExtraAirwayMetricUid())
{
	mMetricManager->setActiveUid(mTargetMetricUid);

	QPushButton *continueToVB = new QPushButton("&Continue to virtual bronchoscopy", this);
	connect(continueToVB, &QPushButton::clicked, this, &PinpointWidget::setTargetMetric);
	QPushButton *centerToImage = new QPushButton(QIcon(":/icons/center_image.png"), " Center Image", this);
	connect(centerToImage, &QPushButton::clicked, this, &PinpointWidget::centerToImage);
	mPointMetricNameLineEdit = new QLineEdit(mTargetMetricName, this);
	connect(mPointMetricNameLineEdit, &QLineEdit::textEdited, this, &PinpointWidget::targetNameChanged);

	mTargetPointButton = new QPushButton("&Set target point", this);

	// Selector for adding airway points
	mAddAirwayCheckBox = new QCheckBox(tr("Add manual airway"));
	mAddAirwayCheckBox->setChecked(false);
	mAddAirwayPointButton = new QPushButton("&Add airway point", this);
	mDeletePointButton = new QPushButton("&Delete last point", this);

	QGridLayout* gridLayoutSetTarget = new QGridLayout;
	gridLayoutSetTarget->addWidget(mTargetPointButton,0,2);
	gridLayoutSetTarget->addWidget(mAddAirwayCheckBox,1,0);
	gridLayoutSetTarget->addWidget(mAddAirwayPointButton,1,1);
	gridLayoutSetTarget->addWidget(mDeletePointButton,1,2);

	mAddAirwayPointButton->hide();
	mDeletePointButton->hide();

	connect(mServices->patient().get(), &PatientModelService::patientChanged, this, &PinpointWidget::loadNameOfPointMetric);
	connect(mServices->session().get(), &SessionStorageService::isSaving, this, &PinpointWidget::onSessionSave);
	connect(mServices->session().get(), &SessionStorageService::isLoading, this, &PinpointWidget::onSessionLoad);

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

	mTumorInfoWidget = new TumorInformationWidget(mServices, this);
	QGroupBox* tumorInfoBox = new QGroupBox(tr("Tumors"));
	QVBoxLayout* tumorInfoLayout = new QVBoxLayout();
	tumorInfoLayout->addWidget(mTumorInfoWidget);
	tumorInfoBox->setLayout(tumorInfoLayout);
	v_layout->addWidget(tumorInfoBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
	v_layout->addStretch(); //And add some more stretch


	mStructuresSelectionWidget = new StructuresSelectionWidget(mServices,this);
	QGroupBox* structuresBox = new QGroupBox(tr("Select structures"));
	QVBoxLayout* structuresLayout = new QVBoxLayout();
	structuresLayout->addWidget(mStructuresSelectionWidget);
	structuresBox->setLayout(structuresLayout);
	v_layout->addWidget(structuresBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
	v_layout->addStretch(); //And add some more stretch

	this->setLayout(v_layout);

	connect(mAddAirwayCheckBox, &QCheckBox::clicked, this, &PinpointWidget::addAirwayPointOn);
	connect(mTargetPointButton, &QPushButton::clicked, this, &PinpointWidget::setTargetPoint);
	connect(mAddAirwayPointButton, &QPushButton::clicked, this, &PinpointWidget::addAirwayPoint);
	connect(mDeletePointButton, &QPushButton::clicked, this, &PinpointWidget::deleteAirwayPoint);

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

QString PinpointWidget::getExtraAirwayMetricUid()
{
	return "AirwayPoint";
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

void PinpointWidget::onSessionSave(QDomElement& root)
{
	XMLNodeAdder adder(root);
	QDomElement node = adder.descend("managers/fraxinusPinpoint").node().toElement();
	XmlOptionItem("addManualAirway", node).writeValue(mAddAirwayPoints ? "true" : "false");
}

void PinpointWidget::onSessionLoad(QDomElement& root)
{
	XMLNodeParser parser(root);
	QDomElement node = parser.descend("managers/fraxinusPinpoint").node().toElement();
	bool checked = XmlOptionItem("addManualAirway", node).readValue("false") == "true";
	mAddAirwayCheckBox->setChecked(checked);
	mAddAirwayPoints = checked;
	if(checked)
	{
		mAddAirwayPointButton->show();
		mDeletePointButton->show();
	}
	else
	{
		mAddAirwayPointButton->hide();
		mDeletePointButton->hide();
	}
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

// Used for the initial, automatically created target point: the active tool has no
// meaningful tip position yet at that point (no tracking hardware / not yet placed by
// the user), so seed it inside the CT volume instead of wherever the tool happens to be.
void PinpointWidget::createPointMetricAtImageCenter()
{
	CoordinateSystem ref = CoordinateSystem::reference();
	QColor color = QColor(0, 0, 250, 255);
	ImagePtr ctImage = mServices->patient()->getImage(imCT, istTHORAX_CT);
	Vector3D p_ref = ctImage ? ctImage->get_rMd().coord(ctImage->boundingBox().center())
	                         : mServices->spaceProvider()->getActiveToolTipPoint(ref, true);

	mMetricManager->addPoint(p_ref, ref, mTargetMetricUid, color);

	this->setNameOfPointMetric();
}

void PinpointWidget::addAirwayMetric()
{
	CoordinateSystem ref = CoordinateSystem::reference();
	QColor color = QColor(250, 0, 0, 255);
	Vector3D p_ref = mServices->spaceProvider()->getActiveToolTipPoint(ref, true);

	mMetricManager->addPoint(p_ref, ref, mAirwayPointMetricUid+"%1", color);

}

void PinpointWidget::deleteLastAirwayMetric()
{
	std::map<QString, PointMetricPtr> airwayMetrics = mMetricManager->getPointMetrics(mAirwayPointMetricUid);
	if(airwayMetrics.empty())
		return;
	std::map<QString, PointMetricPtr>::iterator it = airwayMetrics.end();
	--it;
	mServices->patient()->removeData(it->first);
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

StructuresSelectionWidget* PinpointWidget::getStructuresSelectionWidget()
{
	return mStructuresSelectionWidget;
}

TumorInformationWidget* PinpointWidget::getTumorInformationWidget()
{
	return mTumorInfoWidget;
}

void PinpointWidget::addAirwayPointOn(bool checked)
{
	mAddAirwayPoints = mAddAirwayCheckBox->isChecked();
	if(checked)
	{
		mAddAirwayPointButton->show();
		mDeletePointButton->show();
	}
	else
	{
		mAddAirwayPointButton->hide();
		mDeletePointButton->hide();
	}
	emit showViaPoints(checked);
	emit updateRoute();
}

void PinpointWidget::setTargetPoint()
{
	emit updateTargetPointFromManualTool();
}

void PinpointWidget::addAirwayPoint()
{
	this->addAirwayMetric();
	emit updateRoute();
	emit showViaPoints(true);
}

void PinpointWidget::deleteAirwayPoint()
{
	this->deleteLastAirwayMetric();
	emit updateRoute();
}

bool PinpointWidget::getAirwayPointsOption()
{
	return mAddAirwayPoints;
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

MetricManagerPtr PinpointWidget::getMetricManager()
{
	return mMetricManager;
}

}


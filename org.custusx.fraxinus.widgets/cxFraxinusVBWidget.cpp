/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#include "cxFraxinusVBWidget.h"
#include <QButtonGroup>
#include <QGroupBox>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QDial>
#include <QTimer>
#include <QPushButton>
#include <QApplication>
#include <QMainWindow>
#include "cxVisServices.h"
#include "cxViewService.h"
#include "cxViewGroupData.h"
#include "cxLogger.h"
#include "cxRouteToTarget.h"
#include "cxPinpointWidget.h"
#include "cxPointMetric.h"
#include "cxDistanceMetric.h"
#include "cxVBcameraPath.h"
#include "cxStructuresSelectionWidget.h"
#include "cxFraxinusEBUSSimulatorWidget.h"
#include "cxLogicManager.h"

namespace cx {

FraxinusVBWidget::FraxinusVBWidget(VisServicesPtr services, QWidget* parent):
	VBWidget(services, parent),
	mServices(services),
	mRouteLength(0),
	mDistanceFromPathEndToTarget(0)
{
	this->setObjectName(this->getWidgetName());

	connect(LogicManager::getInstance(), &LogicManager::pluginsStarted, this, &FraxinusVBWidget::init);
}

FraxinusVBWidget::~FraxinusVBWidget()
{

}

void FraxinusVBWidget::init()
{
	//Disable the select RTT box, as it is only confusing.
	//To use it you still need to manually change many objects in the graphics.
	mRouteToTarget->setEnabled(false);

	mEBUSSimulatorWidget = this->getFraxinusEBUSSimulatorWidget();
	if(mEBUSSimulatorWidget)
	{
		mEBUSSimulatorWidget->setVBWidget(this);
		QGroupBox* EBUSSimulatorBox = new QGroupBox(tr("EBUS Simulator"));
		EBUSSimulatorBox->setLayout(mEBUSSimulatorWidget->layout());
		mVerticalLayout->insertWidget(mVerticalLayout->count()-1, EBUSSimulatorBox);
		connect(mEBUSSimulatorWidget, &FraxinusEBUSSimulatorWidget::EBUSSimulatorStarted, this, &FraxinusVBWidget::setLeftRightOrientationTo90Deg);
		connect(mEBUSSimulatorWidget, &FraxinusEBUSSimulatorWidget::EBUSSimulatorStopped, this, &FraxinusVBWidget::resetEndoscopeSlot);
	}
	else
		CX_LOG_WARNING() << "Did not find EBUS simulator Widget";

	QGroupBox* viewBox = new QGroupBox(tr("View"));
	mViewSelectionWidget = new ViewSelectionWidget(mServices, this);
	QVBoxLayout* viewLayout = new QVBoxLayout();
	viewLayout->addWidget(mViewSelectionWidget);
	viewBox->setLayout(viewLayout);
	mVerticalLayout->insertWidget(mVerticalLayout->count()-1, viewBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.

	QVBoxLayout* routeVLayout = new QVBoxLayout();
	mStaticTotalLegth = new QLabel();
	mRemainingRttLegth = new QLabel();
	mDirectDistance = new QLabel();
	mDistanceToTarget = new QLabel();
	mWarningLabel = new QLabel();
	mDiameter = new QLabel();
	mGenerationNumber = new QLabel();
	routeVLayout->addWidget(mStaticTotalLegth);
	routeVLayout->addWidget(mDistanceToTarget);
	routeVLayout->addWidget(mWarningLabel);
	//routeVLayout->addSpacing(10);
	routeVLayout->addWidget(mRemainingRttLegth);
	routeVLayout->addWidget(mDirectDistance);
	routeVLayout->addWidget(mDiameter);
	routeVLayout->addWidget(mGenerationNumber);

	QGroupBox* routeBox = new QGroupBox(tr("Route length"));
	routeBox->setLayout(routeVLayout);
	mVerticalLayout->insertWidget(mVerticalLayout->count()-1, routeBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.

	mStructuresSelectionWidget = new StructuresSelectionWidget(mServices,this);
	QGroupBox* structuresBox = new QGroupBox(tr("Select structures"));
	QVBoxLayout* structuresLayout = new QVBoxLayout();
	structuresLayout->addWidget(mStructuresSelectionWidget);
	structuresBox->setLayout(structuresLayout);
	mVerticalLayout->insertWidget(mVerticalLayout->count()-1, structuresBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.
	mVerticalLayout->addStretch(); //And add some more stretch

	connect(mPlaybackSlider, &QSlider::valueChanged, this, &FraxinusVBWidget::playbackSliderChanged);
	connect(mRouteToTarget.get(), &SelectDataStringPropertyBase::dataChanged,
			this, &FraxinusVBWidget::calculateRouteLength);
}

void FraxinusVBWidget::playbackSliderChanged(int cameraPositionInPermill)
{
	//Using a single shot timer to wait for other prosesses to update values.
	//Using a lambda function to add the cameraPositionInPercent parameter
	mCameraPositionInPercentAdjusted = positionPercentageAdjusted(cameraPositionInPermill/10.0);
	QTimer::singleShot(0, this, [=](){this->updateRttInfo(mCameraPositionInPercentAdjusted);});
	//QTimer::singleShot(0, this, [=](){this->updateAirwaysOpacity(mCameraPositionInPercentAdjusted);});
}

void FraxinusVBWidget::updateAirwaysOpacity(double cameraPositionInPercent)
{
	double distanceThreshold = 70;
	double maxOpacity = 0.5;
	double distance = getRemainingRouteInsideAirways(cameraPositionInPercent);

	foreach(DataPtr object, mTubeViewObjects)
	{
		MeshPtr mesh = boost::dynamic_pointer_cast<Mesh>(object);
		if(mesh)
		{
			QColor color = mesh->getColor();
			color.setAlphaF(mViewSelectionWidget->getOpacity());
			if(distance < distanceThreshold)
			{
				double opacityFactor = maxOpacity*(distanceThreshold-distance)/distanceThreshold;
				if(1-opacityFactor < mViewSelectionWidget->getOpacity())
					color.setAlphaF(1-opacityFactor);
			}
			mesh->setColor(color);
		}
	}
}

double FraxinusVBWidget::getRemainingRouteInsideAirways(double cameraPositionInPercent)
{
	double position = 1 - cameraPositionInPercent / 100.0;
	double distance = mRouteLength*position;
	return distance;
}

int FraxinusVBWidget::getGenerationNumber(double cameraPositionInPercent)
{
	int generationNumber = 0;
	int index =(int) (cameraPositionInPercent * mGenerationNumbersAlongRoute.size() / 100.0);
	if(index>=0 && index<mGenerationNumbersAlongRoute.size())
		generationNumber = mGenerationNumbersAlongRoute[index];
	return generationNumber;
}

double FraxinusVBWidget::getDiameter(double cameraPositionInPercent)
{
	double radius = 0.0;
	int index =(int) (cameraPositionInPercent * mRadiusAlongRoute.size() / 100.0);
	if(index>=0 && index<mRadiusAlongRoute.size())
		radius = mRadiusAlongRoute[index];

	return std::round(2*radius*10)/10;
}

double FraxinusVBWidget::getTargetDistance()
{
	QString distanceMetricUid = PinpointWidget::getDistanceMetricUid();
	DistanceMetricPtr distanceMetric = mServices->patient()->getData<DistanceMetric>(distanceMetricUid);

	double distance = 0;
	if(distanceMetric)
		distance = distanceMetric->getDistance();

	return distance;
}
void FraxinusVBWidget::updateRttInfo(double cameraPositionInPercent)
{
	mStaticTotalLegth->setText(QString("Total route inside airways: <b>%1 mm</b> ").arg(mRouteLength, 0, 'f', 0));
	mDistanceToTarget->setText(this->createDistanceFromPathToTargetText());

	mRemainingRttLegth->setText(QString("Remaining route length: %1 mm").
															arg(getRemainingRouteInsideAirways(cameraPositionInPercent), 0, 'f', 0));
	mDirectDistance->setText(QString("Distance to target: %1 mm").
													 arg(this->getTargetDistance(), 0, 'f', 0));

	mGenerationNumber->setText(QString("Current generation: %1").
													 arg(this->getGenerationNumber(cameraPositionInPercent)));

	mDiameter->setText(QString("Airway diameter: %1 mm").
													 arg(this->getDiameter(cameraPositionInPercent)));
}

QString FraxinusVBWidget::createDistanceFromPathToTargetText()
{
	//Color the value red and print a warning if above this threshold
	double threshold = 20;
	QString distanceText = "Distance from route end to target: ";
	mWarningLabel->clear();

	if(mDistanceFromPathEndToTarget >= threshold)
		distanceText += "<font color=\"#FF0000\">";
	distanceText += QString("<b>%1 mm</b>").arg(mDistanceFromPathEndToTarget, 0, 'f', 0);
	if(mDistanceFromPathEndToTarget >= threshold)
	{
		mWarningLabel->setText("<font color=\"#FF0000\">There are no segmented airways <br>close to target!</font>");
		distanceText += "</font>";
	}
	return distanceText;
}

void FraxinusVBWidget::calculateRouteLength()
{
	MeshPtr mesh = mRouteToTarget->getMesh();
	if(!mesh)
	{
		mRouteLength = 0;
		return;
	}
	std::vector< Eigen::Vector3d > route = RouteToTarget::getRoutePositions(mesh);
	mRouteLength = RouteToTarget::calculateRouteLength(route);
	this->calculateDistanceFromRouteEndToTarget(route.back());
}

void FraxinusVBWidget::calculateDistanceFromRouteEndToTarget(Eigen::Vector3d routeEndpoint)
{
	QString pointMetricUid = PinpointWidget::getTargetMetricUid();
	PointMetricPtr pointMetric = mServices->patient()->getData<PointMetric>(pointMetricUid);
	if(!pointMetric)
		return;

	Vector3D target = pointMetric->getCoordinate();

	Vector3D direction = (target - routeEndpoint).normal();
	mDistanceFromPathEndToTarget = dot(target - routeEndpoint, direction);
}

QString FraxinusVBWidget::getWidgetName()
{
	return "fraxinus_virtual_bronchoscopy_widget";
}

//void FraxinusVBWidget::keyPressEvent(QKeyEvent* event)
//{
//	enum class Key7{NONE, SHOWVOLUME, SHOWTUBES};
//	Key7 key7 = Key7::NONE;
//	if (event->key()==Qt::Key_7)
//	{
//		if(mViewSelectionWidget->isVolumeButtonChecked())
//			key7 = Key7::SHOWTUBES;
//		else if(mViewSelectionWidget->isTubeButtonChecked())
//			key7 = Key7::SHOWVOLUME;
//	}

//	if (event->key()==Qt::Key_V || key7 == Key7::SHOWVOLUME)
//	{
//		if(mControlsEnabled) {
//			mViewSelectionWidget->setVolumeButtonChecked(true);
//			mViewSelectionWidget->displayVolume();
//			return;
//		}
//	}

//	if (event->key()==Qt::Key_T || key7 == Key7::SHOWTUBES)
//	{
//		if(mControlsEnabled) {
//			mViewSelectionWidget->setTubeButtonChecked(true);
//			mViewSelectionWidget->displayTubes();
//			return;
//		}
//	}

//	VBWidget::keyPressEvent(event);
//}

void FraxinusVBWidget::setViewGroupNumber(unsigned int viewGroupNumber)
{
	if(mViewSelectionWidget)
		mViewSelectionWidget->setViewGroupNumber(viewGroupNumber);
}

void FraxinusVBWidget::addObjectToVolumeView(DataPtr object)
{
	if(mViewSelectionWidget)
		mViewSelectionWidget->addObjectToVolumeView(object);
}

void FraxinusVBWidget::addObjectToTubeView(DataPtr object)
{
	mTubeViewObjects.push_back(object);
	if(mViewSelectionWidget)
		mViewSelectionWidget->addObjectToTubeView(object);
}

StructuresSelectionWidget* FraxinusVBWidget::getStructuresSelectionWidget()
{
	return mStructuresSelectionWidget;
}

FraxinusEBUSSimulatorWidget* FraxinusVBWidget::getEBUSSimulatorWidget()
{
	return mEBUSSimulatorWidget;
}

void FraxinusVBWidget::setGenerationNumbersAlongRoute(std::vector< int > generationNumbers)
{
	mGenerationNumbersAlongRoute = generationNumbers;
}

void FraxinusVBWidget::setRadiusAlongRoute(std::vector< double > radius)
{
	mRadiusAlongRoute = radius;
	mCameraPath->setRadiusAlongRoute(radius);
}

void FraxinusVBWidget::setNavigateAlongAirwayWall(bool navigateAlongAirwayWall)
{
	mCameraPath->setNavigateAlongAirwayWall(navigateAlongAirwayWall);
}

void FraxinusVBWidget::setLeftRightOrientationTo90Deg()
{//Used for EBUS simulator
	int angleDeg = 90;
	mViewDialLeftRight->setValue(angleDeg);
	mCameraPath->cameraViewAngleXSlot(angleDeg);
}

FraxinusEBUSSimulatorWidget* FraxinusVBWidget::getFraxinusEBUSSimulatorWidget()
{
	QMainWindow* mainWindow = this->getMainWindow();
	QString widgetName(FraxinusEBUSSimulatorWidget::getWidgetName());
	return mainWindow->findChild<FraxinusEBUSSimulatorWidget*>(widgetName);
}

QMainWindow* FraxinusVBWidget::getMainWindow()
{
	QWidgetList widgets = qApp->topLevelWidgets();
	for (QWidgetList::iterator i = widgets.begin(); i != widgets.end(); ++i)
		if ((*i)->objectName() == "main_window")
			return (QMainWindow*) (*i);
	return NULL;
}

} //namespace cx

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
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>
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

namespace cx {

FraxinusVBWidget::FraxinusVBWidget(VisServicesPtr services, QWidget* parent):
    VBWidget(services, parent),
		mServices(services),
		mRouteLength(0),
		mDistanceFromPathEndToTarget(0)
{
    this->setObjectName(this->getWidgetName());

	//Disable the select RTT box, as it is only confusing.
	//To use it you still need to manually change many objects in the graphics.
	mRouteToTarget->setEnabled(false);

    // Selector for displaying volume or artificial tubes
    QButtonGroup *displaySelectorGroup = new QButtonGroup(this);
    mTubeButton = new QRadioButton(tr("Tubes"));
    mTubeButton->setChecked(true);
    mVolumeButton = new QRadioButton(tr("Volume"));
    displaySelectorGroup->addButton(mTubeButton);
    displaySelectorGroup->addButton(mVolumeButton);

    QGroupBox* viewBox = new QGroupBox(tr("View"));
    QVBoxLayout* viewVLayout = new QVBoxLayout;
    viewVLayout->addWidget(mTubeButton);
    viewVLayout->addWidget(mVolumeButton);
    viewBox->setLayout(viewVLayout);
		mVerticalLayout->insertWidget(mVerticalLayout->count()-1, viewBox); //There is stretch at the end in the parent widget. Add the viewbox before that stretch.

    QVBoxLayout* routeVLayout = new QVBoxLayout();
		mStaticTotalLegth = new QLabel();
		mRemainingRttLegth = new QLabel();
		mDirectDistance = new QLabel();
		mDistanceToTarget = new QLabel();
		mWarningLabel = new QLabel();
		routeVLayout->addWidget(mStaticTotalLegth);
		routeVLayout->addWidget(mDistanceToTarget);
		routeVLayout->addWidget(mWarningLabel);
		//routeVLayout->addSpacing(10);
		routeVLayout->addWidget(mRemainingRttLegth);
		routeVLayout->addWidget(mDirectDistance);

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


    connect(mTubeButton, &QRadioButton::clicked, this, &FraxinusVBWidget::displayTubes);
    connect(mVolumeButton, &QRadioButton::clicked, this, &FraxinusVBWidget::displayVolume);

    connect(mPlaybackSlider, &QSlider::valueChanged, this, &FraxinusVBWidget::playbackSliderChanged);
    connect(mRouteToTarget.get(), &SelectDataStringPropertyBase::dataChanged,
						this, &FraxinusVBWidget::calculateRouteLength);
}

FraxinusVBWidget::~FraxinusVBWidget()
{

}

void FraxinusVBWidget::playbackSliderChanged(int cameraPositionInPercent)
{
	//Using a single shot timer to wait for other prosesses to update values.
	//Using a lambda function to add the cameraPositionInPercent parameter
    double cameraPositionInPercentAdjusted = positionPercentageAdjusted(cameraPositionInPercent);
    QTimer::singleShot(0, this, [=](){this->updateRttInfo(cameraPositionInPercentAdjusted);});
    QTimer::singleShot(0, this, [=](){this->updateAirwaysOpacity(cameraPositionInPercentAdjusted);});
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
			color.setAlphaF(1);
			if(distance < distanceThreshold)
			{
				double opacityFactor = maxOpacity*(distanceThreshold-distance)/distanceThreshold;
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

	mRemainingRttLegth->setText(QString("Remaining route inside airways: %1 mm").
															arg(getRemainingRouteInsideAirways(cameraPositionInPercent), 0, 'f', 0));
	mDirectDistance->setText(QString("Distance to target: %1 mm").
													 arg(this->getTargetDistance(), 0, 'f', 0));
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
	Vector3D target = pointMetric->getCoordinate();

	Vector3D direction = (target - routeEndpoint).normal();
	mDistanceFromPathEndToTarget = dot(target - routeEndpoint, direction);
}

QString FraxinusVBWidget::getWidgetName()
{
    return "fraxinus_virtual_bronchoscopy_widget";
}

void FraxinusVBWidget::keyPressEvent(QKeyEvent* event)
{
    enum class Key7{NONE, SHOWVOLUME, SHOWTUBES};
    Key7 key7 = Key7::NONE;
    if (event->key()==Qt::Key_7)
    {
        if(mVolumeButton->isChecked())
            key7 = Key7::SHOWTUBES;
        else if(mTubeButton->isChecked())
            key7 = Key7::SHOWVOLUME;
    }

    if (event->key()==Qt::Key_V || key7 == Key7::SHOWVOLUME)
    {
        if(mControlsEnabled) {
            mVolumeButton->setChecked(true);
            this->displayVolume();
            return;
        }
    }

    if (event->key()==Qt::Key_T || key7 == Key7::SHOWTUBES)
    {
        if(mControlsEnabled) {
            mTubeButton->setChecked(true);
            this->displayTubes();
            return;
        }
    }

    VBWidget::keyPressEvent(event);
}

void FraxinusVBWidget::displayVolume()
{
    this->hideDataObjects(mTubeViewObjects);
    this->displayDataObjects(mVolumeViewObjects);
}

void FraxinusVBWidget::displayTubes()
{
    this->hideDataObjects(mVolumeViewObjects);
    this->displayDataObjects(mTubeViewObjects);
}

void FraxinusVBWidget::displayDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        mServices->view()->getGroup(mViewGroupNumber)->addData(object->getUid());
        CX_LOG_DEBUG() << "Display data object. Uid: " << object->getUid();
    }
}

void FraxinusVBWidget::hideDataObjects(std::vector<DataPtr> objects)
{
    foreach(DataPtr object, objects)
    {
        if(!object)
            continue;
        mServices->view()->getGroup(mViewGroupNumber)->removeData(object->getUid());
    }
}

void FraxinusVBWidget::setViewGroupNumber(unsigned int viewGroupNumber)
{
    mViewGroupNumber = viewGroupNumber;
}

void FraxinusVBWidget::addObjectToVolumeView(DataPtr object)
{
    mVolumeViewObjects.push_back(object);
}

void FraxinusVBWidget::addObjectToTubeView(DataPtr object)
{
    mTubeViewObjects.push_back(object);
}

StructuresSelectionWidget* FraxinusVBWidget::getStructuresSelectionWidget()
{
    return mStructuresSelectionWidget;
}


} //namespace cx

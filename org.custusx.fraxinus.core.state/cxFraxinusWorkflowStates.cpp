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

#include "cxFraxinusWorkflowStates.h"
#include <QProgressDialog>
#include <QDialog>
#include <QApplication>
#include <QMainWindow>
#include "cxStateService.h"
#include "cxSettings.h"
#include "cxTrackingService.h"
#include "cxTool.h"
#include "cxVideoService.h"
#include "cxPatientModelService.h"
#include "cxLogger.h"
#include "cxVisServices.h"
#include "cxClippers.h"
#include "cxInteractiveClipper.h"
#include "cxAirwaysFilterService.h"
#include "cxActiveData.h"
#include "cxImage.h"
#include "cxMesh.h"
#include "cxSelectDataStringPropertyBase.h"
#include "cxPointMetric.h"
#include "cxRouteToTargetFilterService.h"
#include "cxVBWidget.h"
#include "cxViewGroupData.h"

namespace cx
{

FraxinusWorkflowState::FraxinusWorkflowState(QState* parent, QString uid, QString name, CoreServicesPtr services, bool enableAction) :
	WorkflowState(parent, uid, name, services, enableAction)
{}

void FraxinusWorkflowState::setCameraStyleInGroup(CAMERA_STYLE_TYPE style, int groupIdx)
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    if(services)
        services->view()->setCameraStyle(style, groupIdx);
}

void FraxinusWorkflowState::useClipper(bool on)
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    if(services)
    {
        ClippersPtr clippers = services->view()->getClippers();
        InteractiveClipperPtr anyplaneClipper = clippers->getClipper("Any");
        anyplaneClipper->useClipper(on);
        anyplaneClipper->setData(this->getActiveImage());
        anyplaneClipper->invertPlane(true);
    }
}

ImagePtr FraxinusWorkflowState::getActiveImage()
{
    ActiveDataPtr activeData = mServices->patient()->getActiveData();
    if(!activeData)
        return ImagePtr();

    ImagePtr activeImage = activeData->getActive<Image>();
    return activeImage;
}

void FraxinusWorkflowState::onEntryDefault(QEvent * event)
{
	WorkflowState::onEntry(event);
	this->useClipper(false);

    //Hack to make sure camera style is set correnyly
    //This is needed as set camera style needs the views to be shown before trying to set style
    QTimer::singleShot(0, this, SLOT(setDefaultCameraStyle()));
}

void FraxinusWorkflowState::setDefaultCameraStyle()
{
    this->setCameraStyleInGroup(cstDEFAULT_STYLE, 0);
    this->setCameraStyleInGroup(cstDEFAULT_STYLE, 1);
    this->setCameraStyleInGroup(cstDEFAULT_STYLE, 2);
}

void FraxinusWorkflowState::setVBCameraStyle()
{
    this->setCameraStyleInGroup(cstANGLED_TOOL_STYLE, 0);
    this->setCameraStyleInGroup(cstTOOL_STYLE, 2);
}

void FraxinusWorkflowState::onEntry(QEvent * event)
{
	this->onEntryDefault(event);
}

MeshPtr FraxinusWorkflowState::getCenterline()
{
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(iter->first.contains("centerline") && !iter->first.contains("_rtt_cl"))
            return iter->second;
    }
    return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getRouteTotarget()
{
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(iter->first.contains("_rtt_cl"))
            return iter->second;
    }
    return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getAirwaysContour()
{
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(iter->first.contains("_ge"))
            return iter->second;
    }
    return MeshPtr();
}

ImagePtr FraxinusWorkflowState::getCTImage()
{
    std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();;

    ImagePtr image;

    if (!images.empty())
        image = images.begin()->second;

    return image;
}

QMainWindow* FraxinusWorkflowState::getMainWindow()
{
    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidgetList::iterator i = widgets.begin(); i != widgets.end(); ++i)
        if ((*i)->objectName() == "MainWindow")
            return (QMainWindow*) (*i);
    return NULL;
}

VBWidget* FraxinusWorkflowState::getVBWidget()
{
    QMainWindow* mainWindow = this->getMainWindow();

    QString widgetName("Virtual Bronchoscopy Widget");
    return mainWindow->findChild<VBWidget*>(widgetName);
}

// --------------------------------------------------------
// --------------------------------------------------------

PatientWorkflowState::PatientWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "PatientUid", "New/Load Patient", services)
{}

PatientWorkflowState::~PatientWorkflowState()
{}

QIcon PatientWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/patient.svg");
}

bool PatientWorkflowState::canEnter() const
{
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

ImportWorkflowState::ImportWorkflowState(QState* parent, VisServicesPtr services) :
	FraxinusWorkflowState(parent, "ImportUid", "Import", services, false)
{
}

ImportWorkflowState::~ImportWorkflowState()
{}

void ImportWorkflowState::onEntry(QEvent * event)
{
	this->onEntryDefault(event);
}

QIcon ImportWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/import.svg");
}

bool ImportWorkflowState::canEnter() const
{
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

ProcessWorkflowState::ProcessWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "ProcessUid", "Process", services, false)
{
    connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

ProcessWorkflowState::~ProcessWorkflowState()
{}

QIcon ProcessWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/airwaysegmentation.svg");
}

void ProcessWorkflowState::onEntry(QEvent * event)
{
	this->onEntryDefault(event);
	this->autoStartHardware();

    //Hack to make sure file is present for AirwaysSegmentation as this loads file from disk instead of using the image
    QTimer::singleShot(0, this, SLOT(imageSelected()));
}

void ProcessWorkflowState::imageSelected()
{
    DataPtr centerline = this->getCenterline();
    if(centerline)
        return;

    ImagePtr image = this->getCTImage();
    this->performAirwaysSegmentation(image);
}

void ProcessWorkflowState::performAirwaysSegmentation(ImagePtr image)
{
    if(!image)
        return;

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

    QProgressDialog progress("Please wait a few minutes for airways segmentation.\n(Program may appear frozen while processing.)", QString(), 0, 0);
    progress.show();

    AirwaysFilterPtr airwaysFilter = AirwaysFilterPtr(new AirwaysFilter(services));
    std::vector <cx::SelectDataStringPropertyBasePtr> input = airwaysFilter->getInputTypes();
    airwaysFilter->getOutputTypes();
    airwaysFilter->getOptions();

    input[0]->setValue(image->getUid());

    if(!airwaysFilter->preProcess())
        CX_LOG_WARNING() << "Airway segmentation preProcess failed";
    if(airwaysFilter->execute())
    {
        airwaysFilter->postProcess();
        progress.hide();
        emit airwaysSegmented();
    }
    else
        CX_LOG_WARNING() << "Airway segmentation failed";

}

bool ProcessWorkflowState::canEnter() const
{
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

PinpointWorkflowState::PinpointWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "PinpointUid", "Pinpoint", services, false)
{
    connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
    connect(mServices->patient().get(), &PatientModelService::dataAddedOrRemoved, this, &PinpointWorkflowState::dataAddedOrRemovedSlot);
}

PinpointWorkflowState::~PinpointWorkflowState()
{}

QIcon PinpointWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/pinpoint.svg");
}

bool PinpointWorkflowState::canEnter() const
{
    return true;
}

void PinpointWorkflowState::dataAddedOrRemovedSlot()
{
    PointMetricPtr targetPoint = this->getTargetPoint();
    MeshPtr centerline = this->getCenterline();
    MeshPtr routeToTarget = this->getRouteTotarget();

    if(targetPoint && centerline && !routeToTarget)
        this->createRouteToTarget();
}

void PinpointWorkflowState::createRouteToTarget()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    RouteToTargetFilterPtr routeToTargetFilter = RouteToTargetFilterPtr(new RouteToTargetFilter(services));
    std::vector<SelectDataStringPropertyBasePtr> input = routeToTargetFilter->getInputTypes();
    routeToTargetFilter->getOutputTypes();
    routeToTargetFilter->getOptions();

    PointMetricPtr targetPoint = this->getTargetPoint();
    MeshPtr centerline = this->getCenterline();

    input[0]->setValue(centerline->getUid());
    input[1]->setValue(targetPoint->getUid());

    if(routeToTargetFilter->execute())
    {
        routeToTargetFilter->postProcess();
        emit routeToTargetCreated();
    }
}

PointMetricPtr PinpointWorkflowState::getTargetPoint()
{
    std::map<QString, PointMetricPtr> metrics = mServices->patient()->getDataOfType<PointMetric>();
    PointMetricPtr metric;
    if (!metrics.empty())
        metric = metrics.begin()->second;
    return metric;
}

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyFlyThroughWorkflowState::VirtualBronchoscopyFlyThroughWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "VirtualBronchoscopyFlyThroughUid", "Virtual Bronchoscopy Fly Through", services, false)
{
    connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

VirtualBronchoscopyFlyThroughWorkflowState::~VirtualBronchoscopyFlyThroughWorkflowState()
{}

QIcon VirtualBronchoscopyFlyThroughWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/vbflythrough.svg");
}

void VirtualBronchoscopyFlyThroughWorkflowState::onEntry(QEvent * event)
{
	WorkflowState::onEntry(event);
	this->useClipper(true);

    VBWidget* widget = this->getVBWidget();

    if(widget)
    {
        MeshPtr routeToTarget = this->getRouteTotarget();
        if(routeToTarget)
            widget->setRouteToTarget(routeToTarget->getUid());
    }

    this->showAirwaysAndRouteToTarget();

    QTimer::singleShot(0, this, SLOT(setVBCameraStyle()));
}

void VirtualBronchoscopyFlyThroughWorkflowState::showAirwaysAndRouteToTarget()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    ViewGroupDataPtr viewGroup = services->view()->getGroup(2);

    MeshPtr routeToTarget = this->getRouteTotarget();
    if(routeToTarget)
        viewGroup->addData(routeToTarget->getUid());

    MeshPtr airways = this->getAirwaysContour();
    if(airways)
        viewGroup->addData(airways->getUid());
}

bool VirtualBronchoscopyFlyThroughWorkflowState::canEnter() const
{
    return true;
}

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyCutPlanesWorkflowState::VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services) :
	FraxinusWorkflowState(parent, "VirtualBronchoscopyCutPlanesUid", "Virtual Bronchoscopy Cut Planes", services, false)
{
    connect(mServices->patient().get(), SIGNAL(patientChanged()), this, SLOT(canEnterSlot()));
}

VirtualBronchoscopyCutPlanesWorkflowState::~VirtualBronchoscopyCutPlanesWorkflowState()
{}

QIcon VirtualBronchoscopyCutPlanesWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/vbcutplane.svg");
}

void VirtualBronchoscopyCutPlanesWorkflowState::onEntry(QEvent * event)
{
	WorkflowState::onEntry(event);
	this->useClipper(true);
	QTimer::singleShot(0, this, SLOT(setVBCameraStyle()));
}

bool VirtualBronchoscopyCutPlanesWorkflowState::canEnter() const
{
    return true;
}

} //namespace cx


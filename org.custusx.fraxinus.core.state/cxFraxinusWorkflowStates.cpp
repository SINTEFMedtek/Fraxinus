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
#include <QApplication>
#include <QMainWindow>
#include <QLayout>
#include <QList>
#include <QMessageBox>
#include <QCheckBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QListWidgetItem>
#include <QLabel>
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
#include "cxActiveData.h"
#include "cxImage.h"
#include "cxMesh.h"
#include "cxSelectDataStringPropertyBase.h"
#include "cxPointMetric.h"
#include "cxDistanceMetric.h"
#include "cxRouteToTargetFilterService.h"
#include "cxViewGroupData.h"
#include "cxProfile.h"
#include "cxDataLocations.h"
#include "cxTransferFunctions3DPresets.h"
#include "cxApplication.h"
#include "cxSyncedValue.h"
#include "cxCameraControl.h"
#include <vtkCamera.h>
#include "cxVBCameraZoomSetting3D.h"
#include "cxPinpointWidget.h"
#include "cxContourFilter.h"
#include "cxFraxinusVBWidget.h"
#include "cxGenericScriptFilter.h"
#include "cxDisplayTimerWidget.h"
#include "cxProcedurePlanningWidget.h"

#ifndef __APPLE__
#include "cxAirwaysFilterService.h"
#endif

namespace cx
{

FraxinusWorkflowState::FraxinusWorkflowState(QState* parent, QString uid, QString name, CoreServicesPtr services, bool enableAction) :
	WorkflowState(parent, uid, name, services, enableAction)
{
	connect(mServices->patient().get(), &PatientModelService::patientChanged, this, &FraxinusWorkflowState::canEnterSlot);
}

void FraxinusWorkflowState::setCameraStyleInGroup(CAMERA_STYLE_TYPE style, int groupIdx)
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	if(services)
		services->view()->setCameraStyle(style, groupIdx);
}

InteractiveClipperPtr FraxinusWorkflowState::enableInvertedClipper(QString clipper_name, bool on)
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	InteractiveClipperPtr anyplaneClipper;
	if(services)
	{
		ClippersPtr clippers = services->view()->getClippers();
		anyplaneClipper = clippers->getClipper(clipper_name);
		anyplaneClipper->useClipper(on);
		anyplaneClipper->invertPlane(true);
	}

	return anyplaneClipper;
}

void FraxinusWorkflowState::removeAllDataFromClipper(InteractiveClipperPtr clipper)
{
	std::map<QString, DataPtr> datas = clipper->getDatas();
	std::map<QString, DataPtr>::iterator it = datas.begin();
	for(; it != datas.end(); ++it)
	{
		clipper->removeData(it->second);
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
	this->enableAction(true);

	//Reset clipping
	InteractiveClipperPtr anyplaneClipper = this->enableInvertedClipper("Any", false);
	this->removeAllDataFromClipper(anyplaneClipper);

	//Reset viewgroups
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	for(int i=0; i<3; ++i)
	{
		ViewGroupDataPtr viewgroup = services->view()->getGroup(i);
		//Clear
		viewgroup->clearData();

		//Set default zoomfactor
		viewgroup->getGroup2DZoom()->set(0.3);
		viewgroup->getGlobal2DZoom()->set(0.3);
	}

	CameraControlPtr camera_control = services->view()->getCameraControl();
	if(camera_control)
	{
        camera_control->setSuperiorView();
    }

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

void FraxinusWorkflowState::setVBFlythroughCameraStyle()
{
	this->setCameraStyleInGroup(cstANGLED_TOOL_STYLE, 0);
	this->setCameraStyleInGroup(cstTOOL_STYLE, 2);
}

void FraxinusWorkflowState::setVBCutplanesCameraStyle()
{
	this->setCameraStyleInGroup(cstANGLED_TOOL_STYLE, 0);
	this->setCameraStyleInGroup(cstTOOL_STYLE, 2);
}

void FraxinusWorkflowState::onEntry(QEvent * event)
{
	this->onEntryDefault(event);
}

MeshPtr FraxinusWorkflowState::getCenterline() const
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
				if(iter->first.contains(airwaysFilterGetNameSuffixCenterline()) && !iter->first.contains(RouteToTargetFilter::getNameSuffix()))
			return iter->second;
	}
	return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getTubeCenterline() const
{
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(iter->first.contains(airwaysFilterGetNameSuffixCenterline()) && !iter->first.contains(RouteToTargetFilter::getNameSuffix())
                && iter->first.contains(airwaysFilterGetNameSuffixTubes()))
            return iter->second;
    }
    return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getRouteToTarget() const
{
	QStringList allRoutes;
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();

	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
        if(this->getTargetPoint())
            if(iter->first.contains(this->getTargetPoint()->getName()) && iter->first.contains(RouteToTargetFilter::getNameSuffix()) && !iter->first.contains(RouteToTargetFilter::getNameSuffixExtension()))
				return iter->second;
	}

	return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getExtendedRouteToTarget() const
{
    QStringList allRoutes;
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();

    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(this->getTargetPoint())
            if(iter->first.contains(this->getTargetPoint()->getName()) && iter->first.contains(RouteToTargetFilter::getNameSuffix()) && iter->first.contains(RouteToTargetFilter::getNameSuffixExtension()))
                return iter->second;
    }

    return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getAirwaysContour()
{
    return this->getMesh(airwaysFilterGetNameSuffixAirways(), ContourFilter::getNameSuffix());
}

MeshPtr FraxinusWorkflowState::getAirwaysTubes()
{
    return this->getMesh(airwaysFilterGetNameSuffixTubes(), airwaysFilterGetNameSuffixAirways());
}

MeshPtr FraxinusWorkflowState::getVessels()
{
    return this->getMesh(airwaysFilterGetNameSuffixVessels());
}

MeshPtr FraxinusWorkflowState::getMesh(QString str_1, QString str_2)
{
    std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
    for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
    {
        if(iter->first.contains(str_1))
            if(iter->first.contains(str_2))
                return iter->second;
    }
    return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getLungs()
{
    return this->getMesh("_lungs");
}

MeshPtr FraxinusWorkflowState::getLymphNodes()
{
    return this->getMesh("_lymphNodes");
}

MeshPtr FraxinusWorkflowState::getNodules()
{
    return this->getMesh("_nodules");
}

MeshPtr FraxinusWorkflowState::getVenaCava()
{
    return this->getMesh("_mediumOrgansMediastinum", "VenaCava");
}

MeshPtr FraxinusWorkflowState::getAorticArch()
{
    return this->getMesh("_mediumOrgansMediastinum", "AorticArch");
}

MeshPtr FraxinusWorkflowState::getAscendingAorta()
{
    return this->getMesh("_mediumOrgansMediastinum", "AscendingAorta");
}

MeshPtr FraxinusWorkflowState::getSpine()
{
    return this->getMesh("_mediumOrgansMediastinum", "Spine");
}

MeshPtr FraxinusWorkflowState::getSubCarArt()
{
    return this->getMesh("_smallOrgansMediastinum", "SubCarArt");
}

MeshPtr FraxinusWorkflowState::getEsophagus()
{
    return this->getMesh("_smallOrgansMediastinum", "Esophagus");
}

MeshPtr FraxinusWorkflowState::getBrachiocephalicVeins()
{
    return this->getMesh("_smallOrgansMediastinum", "BrachiocephalicVeins");
}

MeshPtr FraxinusWorkflowState::getAzygos()
{
    return this->getMesh("_smallOrgansMediastinum", "Azygos");
}

MeshPtr FraxinusWorkflowState::getHeart()
{
    return this->getMesh("_pulmSystHeart", "Heart");
}

MeshPtr FraxinusWorkflowState::getPulmonaryVeins()
{
    return this->getMesh("_pulmSystHeart", "PulmonaryVeins");
}

MeshPtr FraxinusWorkflowState::getPulmonaryTrunk()
{
    return this->getMesh("_pulmSystHeart", "PulmonaryTrunk");
}


ImagePtr FraxinusWorkflowState::getCTImage() const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(!it->first.contains("_copy"))
		{
			image = it->second;
			break;
		}
	}
	return image;
}

ImagePtr FraxinusWorkflowState::getCTImageCopied() const
{
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();

    if(images.empty())
        return ImagePtr();

	std::map<QString, ImagePtr>::iterator it = images.begin();
    ImagePtr imageCopied;
	for( ; it != images.end(); ++it)
	{
		if(it->first.contains("_copy"))
		{
            imageCopied = it->second;
			break;
		}
	}

    if (!imageCopied)
        imageCopied = createCopiedImage(images.begin()->second);

    return imageCopied;
}

ImagePtr FraxinusWorkflowState::createCopiedImage(ImagePtr originalImage) const
{
    ImagePtr imageCopied = originalImage->copy();
    imageCopied->setName(originalImage->getName()+"_copy");
    imageCopied->setUid(originalImage->getUid()+"_copy");
    mServices->patient()->insertData(imageCopied);

    return imageCopied;
}

PointMetricPtr FraxinusWorkflowState::getTargetPoint() const
{
	std::map<QString, PointMetricPtr> metrics = mServices->patient()->getDataOfType<PointMetric>();
	std::map<QString, PointMetricPtr>::iterator it = metrics.begin();
	PointMetricPtr metric;
	for( ; it != metrics.end(); ++it)
	{
		if(it->first.contains(PinpointWidget::getTargetMetricUid()))
		{
			metric = it->second;
			break;
		}
	}

	return metric;
}

PointMetricPtr FraxinusWorkflowState::getEndoscopePoint() const
{
	std::map<QString, PointMetricPtr> metrics = mServices->patient()->getDataOfType<PointMetric>();
	std::map<QString, PointMetricPtr>::iterator it = metrics.begin();
	PointMetricPtr metric;
	for( ; it != metrics.end(); ++it)
	{
		if(it->first.contains(PinpointWidget::getEndoscopeMetricUid()))
		{
			metric = it->second;
			break;
		}
	}

	return metric;
}

DistanceMetricPtr FraxinusWorkflowState::getDistanceToTargetMetric() const
{
	std::map<QString, DistanceMetricPtr> metrics = mServices->patient()->getDataOfType<DistanceMetric>();
	std::map<QString, DistanceMetricPtr>::iterator it = metrics.begin();
	DistanceMetricPtr metric;
	for( ; it != metrics.end(); ++it)
	{
		if(it->first.contains(PinpointWidget::getDistanceMetricUid()))
		{
			metric = it->second;
			break;
		}
	}

	return metric;
}

QMainWindow* FraxinusWorkflowState::getMainWindow()
{
	QWidgetList widgets = qApp->topLevelWidgets();
	for (QWidgetList::iterator i = widgets.begin(); i != widgets.end(); ++i)
		if ((*i)->objectName() == "main_window")
			return (QMainWindow*) (*i);
	return NULL;
}

FraxinusVBWidget* FraxinusWorkflowState::getVBWidget()
{
	QMainWindow* mainWindow = this->getMainWindow();

	QString widgetName(FraxinusVBWidget::getWidgetName());
	return mainWindow->findChild<FraxinusVBWidget*>(widgetName);
}

StructuresSelectionWidget* FraxinusWorkflowState::getStructturesSelectionWidget()
{
    QMainWindow* mainWindow = this->getMainWindow();

    QString widgetName(StructuresSelectionWidget::getWidgetName());
    return mainWindow->findChild<StructuresSelectionWidget*>(widgetName);
}

ProcedurePlanningWidget* FraxinusWorkflowState::getProcedurePlanningWidget()
{
    QMainWindow* mainWindow = this->getMainWindow();

    QString widgetName(ProcedurePlanningWidget::getWidgetName());
    return mainWindow->findChild<ProcedurePlanningWidget*>(widgetName);
}

PinpointWidget* FraxinusWorkflowState::getPinpointWidget()
{
	QMainWindow* mainWindow = this->getMainWindow();

	QString widgetName("pinpoint_widget");
	PinpointWidget* ret = mainWindow->findChild<PinpointWidget*>(widgetName);
	return ret;
}

TransferFunctions3DPresetsPtr FraxinusWorkflowState::getTransferfunctionPresets()
{
	XmlOptionFile preset(DataLocations::findConfigFilePath("presets.xml", "/transferFunctions"));
	XmlOptionFile custom = profile()->getXmlSettings().descend("presetTransferFunctions");
	TransferFunctions3DPresetsPtr transferFunctionPresets = TransferFunctions3DPresetsPtr(new TransferFunctions3DPresets(preset, custom));

	return transferFunctionPresets;
}

void FraxinusWorkflowState::setTransferfunction3D(QString transferfunction, ImagePtr image)
{
	if(!image)
		return;
	TransferFunctions3DPresetsPtr transferFunctionPresets = getTransferfunctionPresets();
	transferFunctionPresets->load3D(transferfunction, image);
}

void FraxinusWorkflowState::setTransferfunction2D(QString transferfunction, ImagePtr image)
{
	TransferFunctions3DPresetsPtr transferFunctionPresets = getTransferfunctionPresets();
	transferFunctionPresets->load2D(transferfunction, image);
}

void FraxinusWorkflowState::setRTTInVBWidget()
{
	FraxinusVBWidget* widget = this->getVBWidget();

	if(widget)
    {
        this->createRouteToTarget();
        widget->setRoutePositions(mRouteToTargetPositions);
        widget->setCameraRotationAlongRoute(mRouteToTargetCameraRotations);

        MeshPtr routeToTarget = this->getRouteToTarget();
        if(routeToTarget)
            widget->setRouteToTarget(routeToTarget->getUid());
	}
}

void FraxinusWorkflowState::setupViewOptionsINVBWidget(int flyThrough3DViewGroupNumber)
{
    ImagePtr ctImage_copied = this->getCTImageCopied();
	std::vector<DataPtr> volumeViewObjects;
    volumeViewObjects.push_back(ctImage_copied);

    std::vector<DataPtr> tubeViewObjects;
    MeshPtr tubes = this->getAirwaysTubes();
    tubeViewObjects.push_back(tubes);

    FraxinusVBWidget* widget = this->getVBWidget();
    foreach(DataPtr object, tubeViewObjects)
        widget->addObjectToTubeView(object);
    foreach(DataPtr object, volumeViewObjects)
		widget->addObjectToVolumeView(object);

    FraxinusVBWidget* VBWidget = this->getVBWidget();
    if (VBWidget)
        this->setupViewOptionsForStructuresSelection(VBWidget->getStructturesSelectionWidget(), flyThrough3DViewGroupNumber);

	widget->setViewGroupNumber(flyThrough3DViewGroupNumber);

}

void FraxinusWorkflowState::setupViewOptionsForStructuresSelection(StructuresSelectionWidget* widget, int viewGroupNumber)
{
    //StructuresSelectionWidget* widget = this->getStructturesSelectionWidget();

    std::vector<DataPtr> lungObjects;
    MeshPtr lungs = this->getLungs();
    if(lungs)
        lungObjects.push_back(lungs);

    std::vector<DataPtr> lesionObjects;
    MeshPtr lesions = this->getNodules();
    if(lesions)
        lesionObjects.push_back(this->getNodules());

    std::vector<DataPtr> lymphNodeObjects;
    MeshPtr lymphNodes = this->getLymphNodes();
    if(lymphNodes)
        lymphNodeObjects.push_back(lymphNodes);

    std::vector<DataPtr> spineObjects;
    MeshPtr spine = this->getSpine();
    if(spine)
        spineObjects.push_back(spine);

    std::vector<DataPtr> largeVesselsObjects;
    MeshPtr venaCava = this->getVenaCava();
    if(venaCava)
        largeVesselsObjects.push_back(venaCava);
    MeshPtr aorticArch = this->getAorticArch();
    if(aorticArch)
        largeVesselsObjects.push_back(aorticArch);
    MeshPtr ascendingAorta = this->getAscendingAorta();
    if(ascendingAorta)
        largeVesselsObjects.push_back(ascendingAorta);
    MeshPtr subCarArt = this->getSubCarArt();
    if(subCarArt)
        largeVesselsObjects.push_back(subCarArt);
    MeshPtr brachiocephalicVeins = this->getBrachiocephalicVeins();
    if(brachiocephalicVeins)
        largeVesselsObjects.push_back(brachiocephalicVeins);
    MeshPtr azygos = this->getAzygos();
    if(azygos)
        largeVesselsObjects.push_back(azygos);

    std::vector<DataPtr> smallVesselsObjects;
    MeshPtr smallVessels = this->getVessels();
    if(smallVessels)
        smallVesselsObjects.push_back(smallVessels);

    std::vector<DataPtr> heartObjects;
    MeshPtr heart = this->getHeart();
    if(heart)
        heartObjects.push_back(heart);
    MeshPtr pulmonaryTrunk = this->getPulmonaryTrunk();
    if(pulmonaryTrunk)
        heartObjects.push_back(pulmonaryTrunk);
    MeshPtr pulmonaryVeins = this->getPulmonaryVeins();
    if(pulmonaryVeins)
        heartObjects.push_back(pulmonaryVeins);

    std::vector<DataPtr> esophagusObjects;
    MeshPtr esophagus = this->getEsophagus();
    if(esophagus)
        esophagusObjects.push_back(esophagus);

    CX_LOG_DEBUG() << "Adding lung object - lungObjects.size(): " << lungObjects.size();
    foreach(DataPtr object, lungObjects)
        widget->addLungObject(object);
    foreach(DataPtr object, lesionObjects)
        widget->addLesionObject(object);
    foreach(DataPtr object, lymphNodeObjects)
        widget->addLymphNodeObject(object);
    foreach(DataPtr object, spineObjects)
        widget->addSpineObject(object);
    foreach(DataPtr object, smallVesselsObjects)
        widget->addSmallVesselObject(object);
    foreach(DataPtr object, largeVesselsObjects)
        widget->addLargeVesselObject(object);
    foreach(DataPtr object, heartObjects)
        widget->addHeartObject(object);
    foreach(DataPtr object, esophagusObjects)
        widget->addEsophagusObject(object);

    widget->setViewGroupNumber(viewGroupNumber);
}


void FraxinusWorkflowState::setupVBWidget(int flyThrough3DViewGroupNumber)
{
	this->setRTTInVBWidget();
	this->setupViewOptionsINVBWidget(flyThrough3DViewGroupNumber);
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	if(services)
		services->view()->zoomCamera3D(flyThrough3DViewGroupNumber, VB3DCameraZoomSetting::getZoomFactor());

	this->getVBWidget()->grabKeyboard(); //NB! This make this widget take all keyboard input. E.g. "R" doesn't work in this workflow step.
	//Actually, "R" seems to be a special case since it is from VTK. Other key input might work, but maybe not if the menu bar is off.
	//this->getVBWidget()->setFocus(); // Can't seem to get any affect from this regarding key input.
}

void FraxinusWorkflowState::setupProcedurePlanningWidget(int viewGroupNumber)
{
    ProcedurePlanningWidget* procedurePlanningWidget = this->getProcedurePlanningWidget();
    if (procedurePlanningWidget)
        this->setupViewOptionsForStructuresSelection(procedurePlanningWidget->getStructuresSelectionWidget(), viewGroupNumber);
}

void FraxinusWorkflowState::createRouteToTarget()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    RouteToTargetFilterPtr routeToTargetFilter = RouteToTargetFilterPtr(new RouteToTargetFilter(services, true));
    std::vector<SelectDataStringPropertyBasePtr> input = routeToTargetFilter->getInputTypes();
    routeToTargetFilter->getOutputTypes();
    routeToTargetFilter->getOptions();

    routeToTargetFilter->setSmoothing(false);

    PointMetricPtr targetPoint = this->getTargetPoint();
    MeshPtr centerline = this->getTubeCenterline();

    input[0]->setValue(centerline->getUid());
    input[1]->setValue(targetPoint->getUid());

    routeToTargetFilter->setTargetName(targetPoint->getName());
    if(routeToTargetFilter->execute())
    {
        routeToTargetFilter->postProcess();
        mRouteToTargetPositions = routeToTargetFilter->getRoutePositions();
        mRouteToTargetCameraRotations = routeToTargetFilter->getCameraRotation();
        emit routeToTargetCreated();
    }
}

void FraxinusWorkflowState::cleanupVBWidget()
{
	this->getVBWidget()->releaseKeyboard();
	//this->getVBWidget()->clearFocus();
}

// --------------------------------------------------------
// --------------------------------------------------------

PatientWorkflowState::PatientWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "PatientUid", "New/Load Patient", services, true)
{}

PatientWorkflowState::~PatientWorkflowState()
{}

QIcon PatientWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/patient.svg");
}

void PatientWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();
}

bool PatientWorkflowState::canEnter() const
{
	return true;
}

void PatientWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();

	//Assuming 3D
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(ctImage)
	{
		this->setTransferfunction3D("Default", ctImage);
		viewGroup0_3D->addData(ctImage->getUid());
	}
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
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();
}

void ImportWorkflowState::onExit(QEvent * event)
{
	ImagePtr ctImage = this->getCTImage();
	if(ctImage)
	{
		ctImage->setInitialWindowLevel(-1, -1);
	}
	WorkflowState::onExit(event);
}

QIcon ImportWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/import.svg");
}

bool ImportWorkflowState::canEnter() const
{
	return mServices->patient()->isPatientValid();
}

void ImportWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();

	//Assuming 3D ACS
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(ctImage)
	{
		ctImage->setInitialWindowLevel(-1, -1);
		this->setTransferfunction3D("Default", ctImage);
		this->setTransferfunction2D("2D CT Lung", ctImage);
		viewGroup0_3D->addData(ctImage->getUid());
	}

	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	viewGroup1_2D->getGroup2DZoom()->set(0.1);
	viewGroup1_2D->getGlobal2DZoom()->set(0.1);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());
}

// --------------------------------------------------------
// --------------------------------------------------------

ProcessWorkflowState::ProcessWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "ProcessUid", "Process", services, false)
{
	mTimedAlgorithmProgressBar = new cx::TimedAlgorithmProgressBar;

    //QHBoxLayout *layout = new QHBoxLayout();
    //layout->addWidget(mTimedAlgorithmProgressBar);
    //dialog.setLayout(layout);
}

ProcessWorkflowState::~ProcessWorkflowState()
{}

QIcon ProcessWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/airwaysegmentation.svg");
}

void ProcessWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();

    this->createSelectSegmentationBox();

	//Hack to make sure file is present for AirwaysSegmentation as this loads file from disk instead of using the image
    //QTimer::singleShot(0, this, SLOT(imageSelected()));

    //Setting Pinpoint workflow active here, in case segmentation is run manuelly if automatic segmentation fails.
    QObject* parentWorkFlow = this->parent();
    QList<FraxinusWorkflowState *> allWorkflows = parentWorkFlow->findChildren<FraxinusWorkflowState *>();
     for (int i = 0; i < allWorkflows.size(); i++)
         if (allWorkflows[i]->getName() == "Set target")
         {
             allWorkflows[i]->enableAction(true);
             break;
         }
}

void ProcessWorkflowState::createSelectSegmentationBox()
{
    mSegmentationSelectionInput = new QDialog();
    mSegmentationSelectionInput->setWindowTitle(tr("Select structures for segmentation"));
    mSegmentationSelectionInput->setWindowFlag(Qt::WindowStaysOnTopHint);

    mCheckBoxAirways = new QCheckBox(tr("Airways (~1 min)"));
    mCheckBoxAirways->setChecked(true);
    mCheckBoxAirways->setDisabled(true);
    mCheckBoxLungs = new QCheckBox(tr("Lungs (~3 min)"));
    mCheckBoxLymphNodes = new QCheckBox(tr("Lymph Nodes  (~10 min)"));
    mCheckBoxPulmonarySystem = new QCheckBox(tr("Pulmonary System"));
    mCheckBoxMediumOrgans = new QCheckBox(tr("Vena Cava, Aorta, Spine  (~4 min)"));
    mCheckBoxSmallOrgans = new QCheckBox(tr("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos (~4 min)"));
    mCheckBoxNodules = new QCheckBox(tr("Lesions"));
    mCheckBoxVessels = new QCheckBox(tr("Small Vessels  (~3 min)"));

    QPushButton* OKbutton = new QPushButton(tr("&OK"));
    connect(OKbutton, SIGNAL(clicked()), this, SLOT(imageSelected()));

    QPushButton* Cancelbutton = new QPushButton(tr("&Cancel"));
    connect(Cancelbutton, SIGNAL(clicked()), this, SLOT(cancel()));

    QVBoxLayout* checkBoxLayout = new QVBoxLayout;
    checkBoxLayout->addWidget(mCheckBoxAirways);
    checkBoxLayout->addWidget(mCheckBoxLungs);
    checkBoxLayout->addWidget(mCheckBoxLymphNodes);
    checkBoxLayout->addWidget(mCheckBoxPulmonarySystem);
    checkBoxLayout->addWidget(mCheckBoxMediumOrgans);
    checkBoxLayout->addWidget(mCheckBoxSmallOrgans);
    checkBoxLayout->addWidget(mCheckBoxNodules);
    checkBoxLayout->addWidget(mCheckBoxVessels);

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(checkBoxLayout, 0, 0);
    mainLayout->addWidget(Cancelbutton, 1, 1);
    mainLayout->addWidget(OKbutton, 1, 2);

    mSegmentationSelectionInput->setLayout(mainLayout);
    mSegmentationSelectionInput->show();
    mSegmentationSelectionInput->activateWindow();
}

void ProcessWorkflowState::imageSelected()
{
    mSegmentAirways = mCheckBoxAirways->isChecked();
    mSegmentVessels = mCheckBoxVessels->isChecked();
    mSegmentLungs = mCheckBoxLungs->isChecked();
    mSegmentLymphNodes = mCheckBoxLymphNodes->isChecked();
    mSegmentPulmonarySystem = mCheckBoxPulmonarySystem->isChecked();
    mSegmentMediumOrgans = mCheckBoxMediumOrgans->isChecked();
    mSegmentSmallOrgans = mCheckBoxSmallOrgans->isChecked();
    mSegmentNodules = mCheckBoxNodules->isChecked();
    mSegmentationSelectionInput->close();

    this->createProcessingInfo();
    ImagePtr image = this->getCTImage();
    this->performAirwaysSegmentation(image);
}

void ProcessWorkflowState::cancel()
{
    mSegmentationSelectionInput->close();
}

void ProcessWorkflowState::createProcessingInfo()
{
    mSegmentationProcessingInfo = new QDialog();
    mSegmentationProcessingInfo->setWindowTitle(tr("Segmentation status"));
    mSegmentationProcessingInfo->setWindowFlag(Qt::WindowStaysOnTopHint);

    QGridLayout* gridLayout = new QGridLayout;
    gridLayout->setColumnMinimumWidth(0,500);
    gridLayout->setColumnMinimumWidth(1,100);

    if (mSegmentAirways)
    {
        QWidget* timerWidget = new QWidget;
        mAirwaysTimerWidget = new DisplayTimerWidget(timerWidget);
        mAirwaysTimerWidget->setFontSize(3);
        mAirwaysTimerWidget->setFixedWidth(50);
        mAirwaysTimerWidget->show();
        QLabel* label = new QLabel("Airways:");
        gridLayout->addWidget(label,0,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,0,1);
        if(this->getCenterline())
            mAirwaysTimerWidget->stop();
    }
    if (mSegmentVessels)
    {
        QWidget* timerWidget = new QWidget;
        mVesselsTimerWidget = new DisplayTimerWidget(timerWidget);
        mVesselsTimerWidget->setFontSize(3);
        mVesselsTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Small Vessels:");
        gridLayout->addWidget(label,1,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,1,1);
        if(this->getVessels())
            mVesselsTimerWidget->stop();
    }
    if (mSegmentLungs)
    {
        QWidget* timerWidget = new QWidget;
        mLungsTimerWidget = new DisplayTimerWidget(timerWidget);
        mLungsTimerWidget->setFontSize(3);
        mLungsTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Lungs:");
        gridLayout->addWidget(label,2,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,2,1);
        if(this->getLungs())
            mLungsTimerWidget->stop();
    }
    if (mSegmentLymphNodes)
    {
        QWidget* timerWidget = new QWidget;
        mLymphNodesTimerWidget = new DisplayTimerWidget(timerWidget);
        mLymphNodesTimerWidget->setFontSize(3);
        mLymphNodesTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Lymph Nodes:");
        gridLayout->addWidget(label,3,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,3,1);
        if(this->getLymphNodes())
            mLymphNodesTimerWidget->stop();
    }
    if (mSegmentPulmonarySystem)
    {
        QWidget* timerWidget = new QWidget;
        mPulmonarySystemTimerWidget = new DisplayTimerWidget(timerWidget);
        mPulmonarySystemTimerWidget->setFontSize(3);
        mPulmonarySystemTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Pulmonary System:");
        gridLayout->addWidget(label,4,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,4,1);
        if(this->getHeart())
            mPulmonarySystemTimerWidget->stop();
    }
    if (mSegmentMediumOrgans)
    {
        QWidget* timerWidget = new QWidget;
        mMediumOrgansTimerWidget = new DisplayTimerWidget(timerWidget);
        mMediumOrgansTimerWidget->setFontSize(3);
        mMediumOrgansTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Vena Cava, Aorta, Spine:");
        gridLayout->addWidget(label,5,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,5,1);
        if(this->getSpine())
            mMediumOrgansTimerWidget->stop();
    }
    if (mSegmentSmallOrgans)
    {
        QWidget* timerWidget = new QWidget;
        mSmallOrgansTimerWidget = new DisplayTimerWidget(timerWidget);
        mSmallOrgansTimerWidget->setFontSize(3);
        mSmallOrgansTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos:");
        gridLayout->addWidget(label,6,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,6,1);
        if(this->getEsophagus())
            mSmallOrgansTimerWidget->stop();
    }
    if (mSegmentNodules)
    {
        QWidget* timerWidget = new QWidget;
        mNodulesTimerWidget = new DisplayTimerWidget(timerWidget);
        mNodulesTimerWidget->setFontSize(3);
        mNodulesTimerWidget->setFixedWidth(50);
        QLabel* label = new QLabel("Lesions:");
        gridLayout->addWidget(label,7,0,Qt::AlignRight);
        gridLayout->addWidget(timerWidget,7,1,8,3);
        if(this->getNodules())
            mNodulesTimerWidget->stop();
    }


    mSegmentationProcessingInfo->setLayout(gridLayout);
    mSegmentationProcessingInfo->show();
    mSegmentationProcessingInfo->activateWindow();
}

void ProcessWorkflowState::performAirwaysSegmentation(ImagePtr image)
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::performAirwaysSegmentation()";
    if(!image)
        return;

    DataPtr centerline = this->getCenterline();
    DataPtr vessels = this->getVessels();
    if(centerline)
    {
        mAirwaysProcessed = true;
        if(vessels)
        {
            mVesselsProcessed = true;
            this->performMLSegmentation(image);
            return;
        }
        else if(!mSegmentVessels)
        {
            this->performMLSegmentation(image);
            return;
        }
    }

	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    //dialog.show();

#ifndef __APPLE__
	AirwaysFilterPtr airwaysFilter = AirwaysFilterPtr(new AirwaysFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = airwaysFilter->getInputTypes();
	airwaysFilter->getOutputTypes();
	airwaysFilter->getOptions();
    if(!mAirwaysProcessed  && mSegmentAirways)
    {
        mActiveTimerWidget = mAirwaysTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        airwaysFilter->setVesselSegmentation(false);
        airwaysFilter->setAirwaySegmentation(true);
        mCurrentSegmentationType = "Airways";
        mAirwaysProcessed = true;
    }
    else if(!mVesselsProcessed && mSegmentVessels)
    {
        mActiveTimerWidget = mVesselsTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        airwaysFilter->setVesselSegmentation(true);
        airwaysFilter->setAirwaySegmentation(false);
        mCurrentSegmentationType = "Vessels";
        mVesselsProcessed = true;
    }

    input[0]->setValue(image->getUid());
	mCurrentFilter = airwaysFilter;
    this->runAirwaysFilterSlot();
#endif

}

void ProcessWorkflowState::performMLSegmentation(ImagePtr image)
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::performMLSegmentation()";
    if(!image)
        return;

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    //dialog.show();

    GenericScriptFilterPtr scriptFilter = GenericScriptFilterPtr(new GenericScriptFilter(services));
    std::vector <cx::SelectDataStringPropertyBasePtr> input = scriptFilter->getInputTypes();
    scriptFilter->getOutputTypes();
    scriptFilter->getOptions();

    if(mSegmentLungs && !mLungsProcessed && !this->getLungs())
    {
        mActiveTimerWidget = mLungsTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting Lungs";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_Lungs.ini");
        mCurrentSegmentationType = "Lungs";
        mLungsProcessed = true;
    }
    else if(mSegmentLymphNodes && !mLymphNodesProcessed && !this->getLymphNodes())
    {
        mActiveTimerWidget = mLymphNodesTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting Lymph nodes";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_LymphNodes.ini");
        mCurrentSegmentationType = "LymphNodes";
        mLymphNodesProcessed = true;
    }
    else if(mSegmentPulmonarySystem && !mPulmonarySystemProcessed && !this->getHeart())
    {
        mActiveTimerWidget = mPulmonarySystemTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting pulmonary system";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_PulmSystHeart.ini");
        mCurrentSegmentationType = "PulmonarySystem";
        mPulmonarySystemProcessed = true;
    }
    else if(mSegmentMediumOrgans && !mMediumOrgansProcessed && !this->getSpine())
    {
        mActiveTimerWidget = mMediumOrgansTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting Vena Cava, Aorta and Spine";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_MediumOrgansMediastinum.ini");
        mCurrentSegmentationType = "MediumOrgans";
        mMediumOrgansProcessed = true;
    }
    else if(mSegmentSmallOrgans && !mSmallOrgansProcessed && !this->getEsophagus())
    {
        mActiveTimerWidget = mSmallOrgansTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting Subcarinal Artery, Esophagus, Brachiocephalic Veins, Azygos";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_SmallOrgansMediastinum.ini");
        mCurrentSegmentationType = "SmallOrgans";
        mSmallOrgansProcessed = true;
    }
    else if(mSegmentNodules && !mNodulesProcessed && !this->getNodules())
    {
        mActiveTimerWidget = mNodulesTimerWidget;
        if(mActiveTimerWidget)
            mActiveTimerWidget->start();
        CX_LOG_DEBUG() << "Segmenting Lesions";
        scriptFilter->setParameterFilePath("/home/ehof/dev/fraxinus/CX/CX/config/profiles/Laboratory/filter_scripts/python_Nodules.ini");
        mCurrentSegmentationType = "Nodules";
        mNodulesProcessed = true;
    }
    else
    {
        mCurrentSegmentationType = "";
        emit segmentationFinished();
        mSegmentationProcessingInfo->close();
        return;
    } 

    input[0]->setValue(image->getUid());
    mCurrentFilter = scriptFilter;
    this->runMLFilterSlot();
}

void ProcessWorkflowState::runAirwaysFilterSlot()
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::runAirwaysFilterSlot()";
	if (!mCurrentFilter)
		return;

	if (mThread)
	{
        reportWarning(QString("Last operation on %1 is not finished. Could not start filtering.").arg(mThread->getFilter()->getName()));
		return;
	}
	mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
    connect(mThread.get(), SIGNAL(finished()), this, SLOT(airwaysFinishedSlot()));
	mTimedAlgorithmProgressBar->attach(mThread);

	mThread->execute();
}

void ProcessWorkflowState::runMLFilterSlot()
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::runMLFilterSlot()";
    if (!mCurrentFilter)
        return;
    if (mThread)
    {
        reportWarning(QString("Last operation on %1 is not finished. Could not start filtering.").arg(mThread->getFilter()->getName()));
        return;
    }
    mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
   connect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
   mTimedAlgorithmProgressBar->attach(mThread);

    mThread->execute();

}

void ProcessWorkflowState::airwaysFinishedSlot()
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::airwaysFinishedSlot()";
	mTimedAlgorithmProgressBar->detach(mThread);
    disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(airwaysFinishedSlot()));
	mThread.reset();
    //dialog.hide();
    if(mCurrentSegmentationType == "Airways")
    {
        MeshPtr airways = this->getAirwaysContour();
        if(airways)
        {
            airways->setColor("#FFCCCC");
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
            this->addDataToView();
            QString message = "Ariway segmentation failed.\n\n"
                              "Try:\n"
                              "1. Click inside the airways (e.g. trachea).\n"
                              "2. Select input.\n"
                              "3. Select \"Use manual seed point\"\n"
                              "4. Run the Airway segmantation filter again using the green start button. \n";
            QMessageBox::warning(NULL,"Airway segmentation failed", message);
        }
        if(!mVesselsProcessed && mCheckBoxVessels->isChecked())
        {
            this->performAirwaysSegmentation(this->getCTImage());
        }
        else
        {
            this->performMLSegmentation(this->getCTImage());
        }
    }
    else if(mCurrentSegmentationType == "Vessels")
    {
        this->checkIfSegmentationSucceeded();
        this->performMLSegmentation(this->getCTImage());
    }
}

void ProcessWorkflowState::MLFinishedSlot()
{
    CX_LOG_DEBUG() << "ProcessWorkflowState::MLFinishedSlot()";
    mTimedAlgorithmProgressBar->detach(mThread);
    disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(MLFinishedSlot()));
    mThread.reset();
    //dialog.hide();
    if(mActiveTimerWidget)
        mActiveTimerWidget->stop();

    this->checkIfSegmentationSucceeded();

    this->performMLSegmentation(this->getCTImage());
}

bool ProcessWorkflowState::checkIfSegmentationSucceeded()
{
    if(mCurrentSegmentationType == "Airways")
    {
        if(this->getAirwaysContour())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "Vessels")
    {
        if(this->getVessels())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "Lungs")
    {
        if(this->getLungs())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "LymphNodes")
    {
        if(this->getLymphNodes())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "PulmonarySystem")
    {
        if(this->getHeart())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "MediumOrgans")
    {
        if(this->getSpine())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "SmallOrgans")
    {
        if(this->getEsophagus())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }
    else if(mCurrentSegmentationType == "Nodules")
    {
        if(this->getNodules())
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->stop();
        }
        else
        {
            if(mActiveTimerWidget)
                mActiveTimerWidget->failed();
        }
    }

}

bool ProcessWorkflowState::canEnter() const
{
	if(this->getCTImage())
		return true;
	else
		return false;
}

void ProcessWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	MeshPtr airways = this->getAirwaysContour();
    ImagePtr ctImage = this->getCTImage();


    //Assuming 3D
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(airways)
		viewGroup0_3D->addData(airways->getUid());
    else if(ctImage)
    {
        ctImage->setInitialWindowLevel(-1, -1);
        this->setTransferfunction3D("Default", ctImage);
        this->setTransferfunction2D("2D CT Lung", ctImage);
        viewGroup0_3D->addData(ctImage->getUid());
    }

    //Assuming ACS
    ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    viewGroup1_2D->getGroup2DZoom()->set(0.2);
    viewGroup1_2D->getGlobal2DZoom()->set(0.2);
    if(ctImage)
        viewGroup1_2D->addData(ctImage->getUid());
}

void ProcessWorkflowState::onExit(QEvent * event)
{
    if(mSegmentationSelectionInput)
        mSegmentationSelectionInput->close();

    WorkflowState::onExit(event);
}

// --------------------------------------------------------
// --------------------------------------------------------

PinpointWorkflowState::PinpointWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "PinpointUid", "Set target", services, false),
	mPointChanged(false)
{
	connect(mServices->patient().get(), &PatientModelService::patientChanged, this, &PinpointWorkflowState::dataAddedOrRemovedSlot, Qt::UniqueConnection);
}

PinpointWorkflowState::~PinpointWorkflowState()
{}

QIcon PinpointWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/pinpoint.svg");
}

void PinpointWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
    this->addDataToView();

    connect(this->getPinpointWidget(), &PinpointWidget::targetMetricSet, this, &PinpointWorkflowState::dataAddedOrRemovedSlot, Qt::UniqueConnection);

	PointMetricPtr targetPoint = this->getTargetPoint();
	if(targetPoint)
	{
		connect(targetPoint.get(), &PointMetric::transformChanged, this, &PinpointWorkflowState::pointChanged, Qt::UniqueConnection);
    }

    this->setDefaultCameraStyle();
}

bool PinpointWorkflowState::canEnter() const
{
	if(this->getCenterline())
		return true;
	else
		return false;

}

void PinpointWorkflowState::dataAddedOrRemovedSlot()
{
	PointMetricPtr targetPoint = this->getTargetPoint();
	MeshPtr centerline = this->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();

	if(targetPoint && centerline && !routeToTarget)
		this->createRoute();
}

void PinpointWorkflowState::createRoute()
{
	MeshPtr oldRouteToTarget = this->getRouteToTarget();
	if(!oldRouteToTarget)
	{
		this->createRouteToTarget();
		PointMetricPtr targetPoint = this->getTargetPoint();
		mPointChanged = false;
		connect(targetPoint.get(), &PointMetric::transformChanged, this, &PinpointWorkflowState::pointChanged, Qt::UniqueConnection);
	}
	else if(mPointChanged)
	{
		this->deleteOldRouteToTarget();
		this->createRouteToTarget();
	}
}

void PinpointWorkflowState::pointChanged()
{
	mPointChanged = true;
	this->createRoute();
}


void PinpointWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();

    MeshPtr airways = this->getAirwaysContour();

	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImage());

    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
    if(airways)
    {
        viewGroup0_3D->addData(airways->getUid());
        CameraControlPtr camera_control = services->view()->getCameraControl();
        if(camera_control)
        {
            camera_control->setAnteriorView();
            QTimer::singleShot(0, this, SLOT(setDefaultCameraStyle()));
        }
    }
    ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    this->setTransferfunction2D("2D CT Lung", ctImage);
    viewGroup1_2D->getGroup2DZoom()->set(0.3);
    viewGroup1_2D->getGlobal2DZoom()->set(0.3);
    if(ctImage)
        viewGroup1_2D->addData(ctImage->getUid());
}

void PinpointWorkflowState::deleteOldRouteToTarget()
{
	QString targetName = this->getTargetPoint()->getName();
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
		QString meshName = iter->first;
		if(meshName.contains(targetName))
			mServices->patient()->removeData(iter->second->getUid());
	}
}

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyFlyThroughWorkflowState::VirtualBronchoscopyFlyThroughWorkflowState(QState* parent, CoreServicesPtr services)
  : FraxinusWorkflowState(parent, "VirtualBronchoscopyFlyThroughUid", "Virtual Bronchoscopy Fly Through", services, false)
  , mFlyThrough3DViewGroupNumber(2)
  , mSurfaceModel3DViewGroupNumber(0)
{

}

VirtualBronchoscopyFlyThroughWorkflowState::~VirtualBronchoscopyFlyThroughWorkflowState()
{}

QIcon VirtualBronchoscopyFlyThroughWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/vbflythrough.svg");
}

void VirtualBronchoscopyFlyThroughWorkflowState::onEntry(QEvent * event)
{
    FraxinusWorkflowState::onEntry(event);
	this->setupVBWidget(mFlyThrough3DViewGroupNumber); 
    this->addDataToView();

    QTimer::singleShot(0, this, SLOT(setVBFlythroughCameraStyle()));
}

void VirtualBronchoscopyFlyThroughWorkflowState::onExit(QEvent * event)
{
	this->cleanupVBWidget();
	WorkflowState::onExit(event);
}

void VirtualBronchoscopyFlyThroughWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();
	ImagePtr ctImage_copied = this->getCTImageCopied();
	MeshPtr routeToTarget = this->getRouteToTarget();
    MeshPtr extendedRouteToTarget = this->getExtendedRouteToTarget();
	MeshPtr airways = this->getAirwaysContour();
    MeshPtr airwaysTubes = this->getAirwaysTubes();
	PointMetricPtr targetPoint = this->getTargetPoint();
    //DistanceMetricPtr distanceToTargetMetric = this->getDistanceToTargetMetric();


	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImage());

	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(mSurfaceModel3DViewGroupNumber);
	this->setTransferfunction3D("Default", ctImage);
	if(ctImage)
		viewGroup0_3D->addData(ctImage->getUid());
	if(airways)
	{
		QColor c = airways->getColor();
		c.setAlphaF(1);
		airways->setColor(c);
		viewGroup0_3D->addData(airways->getUid());
	}
	if(targetPoint)
		viewGroup0_3D->addData(targetPoint->getUid());
	if(extendedRouteToTarget)
		viewGroup0_3D->addData(extendedRouteToTarget->getUid());
	//if(distanceToTargetMetric)
	//	viewGroup0_3D->addData(distanceToTargetMetric->getUid());

	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    viewGroup1_2D->getGroup2DZoom()->set(0.4);
    viewGroup1_2D->getGlobal2DZoom()->set(0.4);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());

	ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(mFlyThrough3DViewGroupNumber);
	this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage_copied);
    if(targetPoint)
        viewGroup2_3D->addData(targetPoint->getUid());
    if(airwaysTubes)
        viewGroup2_3D->addData(airwaysTubes->getUid());
    if(extendedRouteToTarget)
        viewGroup2_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
        viewGroup2_3D->addData(routeToTarget->getUid());
}

bool VirtualBronchoscopyFlyThroughWorkflowState::canEnter() const
{
	PointMetricPtr targetPoint = this->getTargetPoint();
	MeshPtr centerline = this->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();
	return targetPoint && centerline && routeToTarget;
}


// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyCutPlanesWorkflowState::VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services) :
	FraxinusWorkflowState(parent, "VirtualBronchoscopyCutPlanesUid", "Virtual Bronchoscopy Cut Planes", services, false)
  , mFlyThrough3DViewGroupNumber(2)
{

}

VirtualBronchoscopyCutPlanesWorkflowState::~VirtualBronchoscopyCutPlanesWorkflowState()
{}

QIcon VirtualBronchoscopyCutPlanesWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/vbcutplane.svg");
}

void VirtualBronchoscopyCutPlanesWorkflowState::onEntry(QEvent * event)
{
    FraxinusWorkflowState::onEntry(event);
    this->addDataToView();
	this->setupVBWidget(mFlyThrough3DViewGroupNumber);

	QTimer::singleShot(0, this, SLOT(setVBCutplanesCameraStyle()));
}

void VirtualBronchoscopyCutPlanesWorkflowState::onExit(QEvent *event)
{
	this->cleanupVBWidget();
	WorkflowState::onExit(event);
}

void VirtualBronchoscopyCutPlanesWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();
	ImagePtr ctImage_copied = this->getCTImageCopied();
	MeshPtr routeToTarget = this->getRouteToTarget();
	MeshPtr extendedRouteToTarget = this->getExtendedRouteToTarget();
	MeshPtr airways = this->getAirwaysContour();
    MeshPtr airwaysTubes = this->getAirwaysTubes();
	PointMetricPtr targetPoint = this->getTargetPoint();
	//DistanceMetricPtr distanceToTargetMetric = this->getDistanceToTargetMetric();

	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImage());

	//assuming layout: LAYOUT_VB_CUT_PLANES

	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	this->setTransferfunction3D("Default", ctImage);
	if(ctImage)
		viewGroup0_3D->addData(ctImage->getUid());
	if(airways)
	{
		QColor c = airways->getColor();
		c.setAlphaF(1);
		airways->setColor(c);
		viewGroup0_3D->addData(airways->getUid());
	}
	if(targetPoint)
		viewGroup0_3D->addData(targetPoint->getUid());
	if(extendedRouteToTarget)
		viewGroup0_3D->addData(extendedRouteToTarget->getUid());

	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	this->setTransferfunction2D("2D CT Lung", ctImage);
    viewGroup1_2D->getGroup2DZoom()->set(0.4);
    viewGroup1_2D->getGlobal2DZoom()->set(0.4);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());
	//if(distanceToTargetMetric)
	//	viewGroup0_3D->addData(distanceToTargetMetric->getUid());

	ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(mFlyThrough3DViewGroupNumber);
    this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage_copied);
    if(airwaysTubes)
        viewGroup2_3D->addData(airwaysTubes->getUid());
	if(extendedRouteToTarget)
		viewGroup2_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
		viewGroup2_3D->addData(routeToTarget->getUid());
	if(targetPoint)
		viewGroup2_3D->addData(targetPoint->getUid());
}

bool VirtualBronchoscopyCutPlanesWorkflowState::canEnter() const
{
	PointMetricPtr targetPoint = this->getTargetPoint();
	MeshPtr centerline = this->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();
	return targetPoint && centerline && routeToTarget;
}

// --------------------------------------------------------
// --------------------------------------------------------

ProcedurePlanningWorkflowState::ProcedurePlanningWorkflowState(QState* parent, CoreServicesPtr services) :
    FraxinusWorkflowState(parent, "ProcedurePlanningUid", "Procedure Plannig", services, true)
  , m3DViewGroupNumber(0)
  , m2DViewGroupNumber(1)
{

}

ProcedurePlanningWorkflowState::~ProcedurePlanningWorkflowState()
{}

QIcon ProcedurePlanningWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/airwaysegmentation.svg");// TO DO: change icon
}

void ProcedurePlanningWorkflowState::onEntry(QEvent * event)
{
    FraxinusWorkflowState::onEntry(event);
    this->addDataToView();
    this->setupProcedurePlanningWidget(m3DViewGroupNumber);
//    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
//    if(services)
//        services->view()->zoomCamera3D(m3DViewGroupNumber, 1);

    QTimer::singleShot(0, this, SLOT(setDefaultCameraStyle()));
}

void ProcedurePlanningWorkflowState::onExit(QEvent * event)
{
    WorkflowState::onExit(event);
}

void ProcedurePlanningWorkflowState::addDataToView()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

    ImagePtr ctImage = this->getCTImage();
    MeshPtr airwaysTubes = this->getAirwaysTubes();
    PointMetricPtr targetPoint = this->getTargetPoint();

    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(m3DViewGroupNumber);
    this->setTransferfunction3D("Default", ctImage);
    if(targetPoint)
        viewGroup0_3D->addData(targetPoint->getUid());
    if(airwaysTubes)
        viewGroup0_3D->addData(airwaysTubes->getUid());

    ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(m2DViewGroupNumber);
    viewGroup1_2D->getGroup2DZoom()->set(0.4);
    viewGroup1_2D->getGlobal2DZoom()->set(0.4);
    if(ctImage)
        viewGroup1_2D->addData(ctImage->getUid());
}

bool ProcedurePlanningWorkflowState::canEnter() const
{
    return mServices->patient()->isPatientValid();;
}

} //namespace cx


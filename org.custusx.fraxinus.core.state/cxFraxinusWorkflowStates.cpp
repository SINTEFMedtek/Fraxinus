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
#include "cxFraxinusVBWidget.h"
#include "cxProcedurePlanningWidget.h"
#include "cxFraxinusSegmentations.h"

namespace cx
{

FraxinusWorkflowState::FraxinusWorkflowState(QState* parent, QString uid, QString name, CoreServicesPtr services, bool enableAction) :
	WorkflowState(parent, uid, name, services, enableAction)
{
	mFraxinusSegmentations = FraxinusSegmentationsPtr(new FraxinusSegmentations(services));
	
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

void FraxinusWorkflowState::setAnyplaneCameraStyle()
{
    this->setCameraStyleInGroup(cstDEFAULT_STYLE, 0);
    this->setCameraStyleInGroup(cstTOOL_STYLE, 2);
}

void FraxinusWorkflowState::onEntry(QEvent * event)
{
	this->onEntryDefault(event);
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

ImagePtr FraxinusWorkflowState::getCTImage() const
{
	return mFraxinusSegmentations->getCTImage();
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

void FraxinusWorkflowState::setupViewOptionsInVBWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber)
{
	ImagePtr ctImage_copied = this->getCTImageCopied();
	std::vector<DataPtr> volumeViewObjects;
	volumeViewObjects.push_back(ctImage_copied);
	
	std::vector<DataPtr> tubeViewObjects;
	MeshPtr tubes = mFraxinusSegmentations->getAirwaysTubes();
	tubeViewObjects.push_back(tubes);
	
	FraxinusVBWidget* VBWidget = this->getVBWidget();
	for(DataPtr object : tubeViewObjects)
		VBWidget->addObjectToTubeView(object);
	for(DataPtr object : volumeViewObjects)
		VBWidget->addObjectToVolumeView(object);
	
	std::vector<unsigned int> viewGroupNumbers;
	viewGroupNumbers.push_back(flyThrough3DViewGroupNumber);
	if (surfaceModel3DViewGroupNumber >= 0)
		viewGroupNumbers.push_back(surfaceModel3DViewGroupNumber);
	if (VBWidget)
		this->setupViewOptionsForStructuresSelection(VBWidget->getStructuresSelectionWidget(), viewGroupNumbers);
	
	VBWidget->setViewGroupNumber(flyThrough3DViewGroupNumber);
	
}

void FraxinusWorkflowState::setupViewOptionsForStructuresSelection(StructuresSelectionWidget* widget, std::vector<unsigned int> viewGroupNumbers)
{
	std::vector<DataPtr> lungObjects;
	MeshPtr lungs = mFraxinusSegmentations->getLungs();
	if(lungs)
		lungObjects.push_back(lungs);
	
	std::vector<DataPtr> lesionObjects;
	MeshPtr lesions = mFraxinusSegmentations->getNodules();
	if(lesions)
		lesionObjects.push_back(mFraxinusSegmentations->getNodules());
	
	std::vector<DataPtr> lymphNodeObjects;
	MeshPtr lymphNodes = mFraxinusSegmentations->getLymphNodes();
	if(lymphNodes)
		lymphNodeObjects.push_back(lymphNodes);
	
	std::vector<DataPtr> spineObjects;
	MeshPtr spine = mFraxinusSegmentations->getSpine();
	if(spine)
		spineObjects.push_back(spine);
	
	std::vector<DataPtr> VenaCavaObjects;
	MeshPtr venaCava = mFraxinusSegmentations->getVenaCava();
	if(venaCava)
		VenaCavaObjects.push_back(venaCava);
	MeshPtr brachiocephalicVeins = mFraxinusSegmentations->getBrachiocephalicVeins();
	if(brachiocephalicVeins)
		VenaCavaObjects.push_back(brachiocephalicVeins);
	
	std::vector<DataPtr> AortaObjects;
	MeshPtr aorticArch = mFraxinusSegmentations->getAorticArch();
	if(aorticArch)
		AortaObjects.push_back(aorticArch);
	MeshPtr ascendingAorta = mFraxinusSegmentations->getAscendingAorta();
	if(ascendingAorta)
		AortaObjects.push_back(ascendingAorta);
	MeshPtr descendingAorta = mFraxinusSegmentations->getDescendingAorta();
	if(descendingAorta)
		AortaObjects.push_back(descendingAorta);
	
	std::vector<DataPtr> AzygosObjects;
	MeshPtr azygos = mFraxinusSegmentations->getAzygos();
	if(azygos)
		AzygosObjects.push_back(azygos);
	
	std::vector<DataPtr> SubclavianObjects;
	MeshPtr subCarArt = mFraxinusSegmentations->getSubCarArt();
	if(subCarArt)
		SubclavianObjects.push_back(subCarArt);
	
	std::vector<DataPtr> smallVesselsObjects;
	MeshPtr smallVessels = mFraxinusSegmentations->getVessels();
	if(smallVessels)
		smallVesselsObjects.push_back(smallVessels);
	
	std::vector<DataPtr> heartObjects;
	MeshPtr heart = mFraxinusSegmentations->getHeart();
	if(heart)
		heartObjects.push_back(heart);
	MeshPtr pulmonaryTrunk = mFraxinusSegmentations->getPulmonaryTrunk();
	if(pulmonaryTrunk)
		heartObjects.push_back(pulmonaryTrunk);
	MeshPtr pulmonaryVeins = mFraxinusSegmentations->getPulmonaryVeins();
	if(pulmonaryVeins)
		heartObjects.push_back(pulmonaryVeins);
	
	std::vector<DataPtr> esophagusObjects;
	MeshPtr esophagus = mFraxinusSegmentations->getEsophagus();
	if(esophagus)
		esophagusObjects.push_back(esophagus);
	
	for(DataPtr object : lungObjects)
		widget->addObject(lsLUNG, object);
	for(DataPtr object : lesionObjects)
		widget->addObject(lsLESIONS, object);
	for(DataPtr object : lymphNodeObjects)
		widget->addObject(lsLYMPH_NODES, object);
	for(DataPtr object : spineObjects)
		widget->addObject(lsSPINE, object);
	for(DataPtr object : smallVesselsObjects)
		widget->addObject(lsPULMONARY_VESSELS, object);
	for(DataPtr object : VenaCavaObjects)
		widget->addObject(lsVENA_CAVA, object);
	for(DataPtr object : AzygosObjects)
		widget->addObject(lsVENA_AZYGOS, object);
	for(DataPtr object : AortaObjects)
		widget->addObject(lsAORTA, object);
	for(DataPtr object : SubclavianObjects)
		widget->addObject(lsSUBCLAVIAN_ARTERY, object);
	for(DataPtr object : heartObjects)
		widget->addObject(lsHEART, object);
	for(DataPtr object : esophagusObjects)
		widget->addObject(lsESOPHAGUS, object);
	
	widget->setViewGroupNumbers(viewGroupNumbers);
	
}


void FraxinusWorkflowState::setupVBWidget(int flyThrough3DViewGroupNumber, int surfaceModel3DViewGroupNumber)
{
	this->setRTTInVBWidget();
	this->setupViewOptionsInVBWidget(flyThrough3DViewGroupNumber, surfaceModel3DViewGroupNumber);
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	if(services)
		services->view()->zoomCamera3D(flyThrough3DViewGroupNumber, VB3DCameraZoomSetting::getZoomFactor());
	
	this->getVBWidget()->grabKeyboard(); //NB! This make this widget take all keyboard input. E.g. "R" doesn't work in this workflow step.
	//Actually, "R" seems to be a special case since it is from VTK. Other key input might work, but maybe not if the menu bar is off.
	//this->getVBWidget()->setFocus(); // Can't seem to get any affect from this regarding key input.
}

void FraxinusWorkflowState::setupProcedurePlanningWidget(int viewGroupNumber)
{
	std::vector<unsigned int> viewGroupNumbers;
	viewGroupNumbers.push_back(viewGroupNumber);
	ProcedurePlanningWidget* procedurePlanningWidget = this->getProcedurePlanningWidget();
	if (procedurePlanningWidget)
		this->setupViewOptionsForStructuresSelection(procedurePlanningWidget->getStructuresSelectionWidget(), viewGroupNumbers);
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
}

ProcessWorkflowState::~ProcessWorkflowState()
{}

QIcon ProcessWorkflowState::getIcon() const
{
	return QIcon(":/icons/icons/processing.svg");
}

void ProcessWorkflowState::onEntry(QEvent * event)
{
	FraxinusWorkflowState::onEntry(event);
	this->addDataToView();
	
	//TODO: connect to mFraxinusSegmentations, to run addDataToView() if airways segmentation fails? - Is this needed?
	mFraxinusSegmentations->createSelectSegmentationBox();
	
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
	MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
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
	mFraxinusSegmentations->close();
	
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

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        int viewGroupNumber3D = 0;
        ViewPtr view_3D = services->view()->get3DView(viewGroupNumber3D);
        camera_control->setView(view_3D);
        camera_control->setAnteriorView();
    }
	
	this->setDefaultCameraStyle();
}

bool PinpointWorkflowState::canEnter() const
{
	if(mFraxinusSegmentations->getCenterline())
		return true;
	else
		return false;
	
}

void PinpointWorkflowState::dataAddedOrRemovedSlot()
{
	PointMetricPtr targetPoint = this->getTargetPoint();
	MeshPtr centerline = mFraxinusSegmentations->getCenterline();
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
	
	MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
	
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
	this->setupVBWidget(mFlyThrough3DViewGroupNumber, mSurfaceModel3DViewGroupNumber);
	this->addDataToView();
	
	FraxinusVBWidget* FraxinusVBWidgetPtr = this->getVBWidget();
	if(FraxinusVBWidgetPtr)
	{
		StructuresSelectionWidget* structureSelectionWidget = FraxinusVBWidgetPtr->getStructuresSelectionWidget();
		if(structureSelectionWidget)
			structureSelectionWidget->onEntry();
	}
	
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
	MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
	MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
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
    if(targetPoint)
        viewGroup1_2D->addData(targetPoint->getUid());
	
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
	MeshPtr centerline = mFraxinusSegmentations->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();
	return targetPoint && centerline && routeToTarget;
}


// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyCutPlanesWorkflowState::VirtualBronchoscopyCutPlanesWorkflowState(QState* parent, VisServicesPtr services) :
	FraxinusWorkflowState(parent, "VirtualBronchoscopyCutPlanesUid", "Virtual Bronchoscopy Cut Planes", services, false)
, mFlyThrough3DViewGroupNumber(2)
, mSurfaceModel3DViewGroupNumber(0)
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
	this->setupVBWidget(mFlyThrough3DViewGroupNumber, mSurfaceModel3DViewGroupNumber);
	
	FraxinusVBWidget* FraxinusVBWidgetPtr = this->getVBWidget();
	if(FraxinusVBWidgetPtr)
	{
		StructuresSelectionWidget* structureSelectionWidget = FraxinusVBWidgetPtr->getStructuresSelectionWidget();
		if(structureSelectionWidget)
			structureSelectionWidget->onEntry();
	}
	
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
	MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
	MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
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
    if(targetPoint)
        viewGroup1_2D->addData(targetPoint->getUid());
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
	MeshPtr centerline = mFraxinusSegmentations->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();
	return targetPoint && centerline && routeToTarget;
}

// --------------------------------------------------------
// --------------------------------------------------------

VirtualBronchoscopyAnyplaneWorkflowState::VirtualBronchoscopyAnyplaneWorkflowState(QState* parent, CoreServicesPtr services)
  : FraxinusWorkflowState(parent, "VirtualBronchoscopyAnyplaneUid", "Virtual Bronchoscopy Anyplane", services, false)
  , mFlyThrough3DViewGroupNumber(2)
  , mSurfaceModel3DViewGroupNumber(0)
{

}

VirtualBronchoscopyAnyplaneWorkflowState::~VirtualBronchoscopyAnyplaneWorkflowState()
{}

QIcon VirtualBronchoscopyAnyplaneWorkflowState::getIcon() const
{
    return QIcon(":/icons/icons/vbflythrough.svg");
}

void VirtualBronchoscopyAnyplaneWorkflowState::onEntry(QEvent * event)
{
    FraxinusWorkflowState::onEntry(event);
    this->setupVBWidget(mFlyThrough3DViewGroupNumber, mSurfaceModel3DViewGroupNumber);
    this->addDataToView();

    FraxinusVBWidget* FraxinusVBWidgetPtr = this->getVBWidget();
    if(FraxinusVBWidgetPtr)
    {
        StructuresSelectionWidget* structureSelectionWidget = FraxinusVBWidgetPtr->getStructuresSelectionWidget();
        if(structureSelectionWidget)
            structureSelectionWidget->onEntry();
    }

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        ViewPtr viewSurface_3D = services->view()->get3DView(mSurfaceModel3DViewGroupNumber);
        camera_control->setView(viewSurface_3D);
        camera_control->setAnteriorView();
        viewSurface_3D->setZoomFactor(1.5);
    }

    QTimer::singleShot(0, this, SLOT(setAnyplaneCameraStyle()));
}

void VirtualBronchoscopyAnyplaneWorkflowState::onExit(QEvent * event)
{
    this->cleanupVBWidget();
    WorkflowState::onExit(event);
}

void VirtualBronchoscopyAnyplaneWorkflowState::addDataToView()
{
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

    ImagePtr ctImage = this->getCTImage();
    MeshPtr routeToTarget = this->getRouteToTarget();
    MeshPtr extendedRouteToTarget = this->getExtendedRouteToTarget();
    MeshPtr airways = mFraxinusSegmentations->getAirwaysContour();
    MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
    PointMetricPtr targetPoint = this->getTargetPoint();
    //DistanceMetricPtr distanceToTargetMetric = this->getDistanceToTargetMetric();


    ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(mSurfaceModel3DViewGroupNumber);
    this->setTransferfunction3D("Default", ctImage);
//    if(ctImage)
//        viewGroup0_3D->addData(ctImage->getUid());
    if(airways)
    {
        QColor c = airways->getColor();
        c.setAlphaF(0.6);
        airways->setColor(c);
        viewGroup0_3D->addData(airways->getUid());
    }
    if(targetPoint)
        viewGroup0_3D->addData(targetPoint->getUid());
    if(extendedRouteToTarget)
        viewGroup0_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
        viewGroup0_3D->addData(routeToTarget->getUid());
    //if(distanceToTargetMetric)
    //	viewGroup0_3D->addData(distanceToTargetMetric->getUid());

    ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
    viewGroup1_2D->getGroup2DZoom()->set(0.4);
    viewGroup1_2D->getGlobal2DZoom()->set(0.4);

    if(ctImage)
        viewGroup1_2D->addData(ctImage->getUid());
    if(targetPoint)
        viewGroup1_2D->addData(targetPoint->getUid());

    ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(mFlyThrough3DViewGroupNumber);
    //this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage_copied);
    if(targetPoint)
        viewGroup2_3D->addData(targetPoint->getUid());
    if(airwaysTubes)
        viewGroup2_3D->addData(airwaysTubes->getUid());
    if(extendedRouteToTarget)
        viewGroup2_3D->addData(extendedRouteToTarget->getUid());
    if(routeToTarget)
        viewGroup2_3D->addData(routeToTarget->getUid());
}

bool VirtualBronchoscopyAnyplaneWorkflowState::canEnter() const
{
    PointMetricPtr targetPoint = this->getTargetPoint();
    MeshPtr centerline = mFraxinusSegmentations->getCenterline();
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
    ProcedurePlanningWidget* procedurePlanningWidget = this->getProcedurePlanningWidget();
    if(procedurePlanningWidget)
    {
        StructuresSelectionWidget* structureSelectionWidget = procedurePlanningWidget->getStructuresSelectionWidget();
        if(structureSelectionWidget)
            structureSelectionWidget->onEntry();
    }

    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
    CameraControlPtr camera_control = services->view()->getCameraControl();
    if(camera_control)
    {
        ViewPtr view_3D = services->view()->get3DView(m3DViewGroupNumber);
        camera_control->setView(view_3D);
        camera_control->setAnteriorView();
    }

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
	MeshPtr airwaysTubes = mFraxinusSegmentations->getAirwaysTubes();
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


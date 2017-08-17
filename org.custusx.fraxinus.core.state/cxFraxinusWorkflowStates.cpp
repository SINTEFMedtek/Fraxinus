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
        if(iter->first.contains(AirwaysFilter::getNameSuffix()) && !iter->first.contains(RouteToTargetFilter::getNameSuffix()))
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
            if(iter->first.contains(this->getTargetPoint()->getName()) && iter->first.contains(RouteToTargetFilter::getNameSuffixExtension()))
				return iter->second;
	}

	return MeshPtr();
}

MeshPtr FraxinusWorkflowState::getAirwaysContour() const
{
	std::map<QString, MeshPtr> datas = mServices->patient()->getDataOfType<Mesh>();
	for (std::map<QString, MeshPtr>::const_iterator iter = datas.begin(); iter != datas.end(); ++iter)
	{
        if(iter->first.contains(ContourFilter::getNameSuffix()))
			return iter->second;
	}
	return MeshPtr();
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
	std::map<QString, ImagePtr>::iterator it = images.begin();
	ImagePtr image;
	for( ; it != images.end(); ++it)
	{
		if(it->first.contains("_copy"))
		{
			image = it->second;
			break;
		}
	}
	return image;
}

PointMetricPtr FraxinusWorkflowState::getTargetPoint() const
{
	std::map<QString, PointMetricPtr> metrics = mServices->patient()->getDataOfType<PointMetric>();
	PointMetricPtr metric;
	if (!metrics.empty())
	{
		metric = metrics.begin()->second;
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

VBWidget* FraxinusWorkflowState::getVBWidget()
{
	QMainWindow* mainWindow = this->getMainWindow();

	QString widgetName("virtual_bronchoscopy_widget");
    return mainWindow->findChild<VBWidget*>(widgetName);
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
	VBWidget* widget = this->getVBWidget();

	if(widget)
	{
		MeshPtr routeToTarget = this->getRouteToTarget();
		if(routeToTarget)
			widget->setRouteToTarget(routeToTarget->getUid());
	}
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
		viewGroup0_3D->addData(ctImage->getUid());
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
	std::map<QString, ImagePtr> images = mServices->patient()->getDataOfType<Image>();
	if (images.size() == 1)
	{
		ImagePtr image = images.begin()->second;
		ImagePtr image_copy = image->copy();
		image_copy->setName(image->getName()+"_copy");
		image_copy->setUid(image->getUid()+"_copy");
		mServices->patient()->insertData(image_copy);
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

	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(mTimedAlgorithmProgressBar);
	dialog.setLayout(layout);
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
	dialog.show();

	AirwaysFilterPtr airwaysFilter = AirwaysFilterPtr(new AirwaysFilter(services));
	std::vector <cx::SelectDataStringPropertyBasePtr> input = airwaysFilter->getInputTypes();
	airwaysFilter->getOutputTypes();
	airwaysFilter->getOptions();

	input[0]->setValue(image->getUid());

	mCurrentFilter = airwaysFilter;
	this->runFilterSlot();

}

void ProcessWorkflowState::runFilterSlot()
{
	if (!mCurrentFilter)
		return;
	if (mThread)
	{
		reportWarning(QString("Last operation on %1 is not finished. Could not start filtering").arg(mThread->getFilter()->getName()));
		return;
	}
	mThread.reset(new FilterTimedAlgorithm(mCurrentFilter));
	connect(mThread.get(), SIGNAL(finished()), this, SLOT(finishedSlot()));
	mTimedAlgorithmProgressBar->attach(mThread);

	mThread->execute();
}

void ProcessWorkflowState::finishedSlot()
{

	mTimedAlgorithmProgressBar->detach(mThread);
	disconnect(mThread.get(), SIGNAL(finished()), this, SLOT(finishedSlot()));
	mThread.reset();
	dialog.hide();
	MeshPtr airways = this->getAirwaysContour();
	if(airways)
	{
		airways->setColor("#FFCCCC");
		emit airwaysSegmented();
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

	//Assuming 3D
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	if(airways)
		viewGroup0_3D->addData(airways->getUid());
}

// --------------------------------------------------------
// --------------------------------------------------------

PinpointWorkflowState::PinpointWorkflowState(QState* parent, CoreServicesPtr services) :
	FraxinusWorkflowState(parent, "PinpointUid", "Pinpoint", services, false),
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

	routeToTargetFilter->setTargetName(targetPoint->getName());
	if(routeToTargetFilter->execute())
	{
		routeToTargetFilter->postProcess();
		emit routeToTargetCreated();
	}
}

void PinpointWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();
	ImagePtr ctImage_copied = this->getCTImageCopied();

	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImage());

	//assuming layout: LAYOUT_ACAS
	ViewGroupDataPtr viewGroup0_2D = services->view()->getGroup(0);
	this->setTransferfunction2D("2D CT Abdomen", ctImage_copied);
	viewGroup0_2D->getGroup2DZoom()->set(0.3);
	viewGroup0_2D->getGlobal2DZoom()->set(0.3);
	if(ctImage_copied)
		viewGroup0_2D->addData(ctImage_copied->getUid());

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
  , m3DViewGroupNumber(2)
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
    this->addDataToView();
    this->setRTTInVBWidget();
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	if(services)
		services->view()->zoomCamera3D(m3DViewGroupNumber, VB3DCameraZoomSetting::getZoomFactor());

    QTimer::singleShot(0, this, SLOT(setVBFlythroughCameraStyle()));
}

void VirtualBronchoscopyFlyThroughWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();
	MeshPtr routeToTarget = this->getRouteToTarget();
	MeshPtr airways = this->getAirwaysContour();
	PointMetricPtr targetPoint = this->getTargetPoint();

	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImageCopied());

	//assuming layout: LAYOUT_VB_FLY_THROUGH
	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	this->setTransferfunction3D("Default", ctImage_copied);
	if(ctImage_copied)
		viewGroup0_3D->addData(ctImage_copied->getUid());
	if(routeToTarget)
		viewGroup0_3D->addData(routeToTarget->getUid());
	if(targetPoint)
		viewGroup0_3D->addData(targetPoint->getUid());

	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	viewGroup1_2D->getGroup2DZoom()->set(0.2);
	viewGroup1_2D->getGlobal2DZoom()->set(0.2);
	if(ctImage_copied)
		viewGroup1_2D->addData(ctImage_copied->getUid());

	ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(m3DViewGroupNumber);
    this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage);
    if(targetPoint)
        viewGroup2_3D->addData(targetPoint->getUid());
    if(ctImage)
        viewGroup2_3D->addData(ctImage->getUid());
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
  , mFlyThroughViewGroupNumber3D(2)
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
    this->setRTTInVBWidget();
    VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);
	if(services)
		services->view()->zoomCamera3D(mFlyThroughViewGroupNumber3D, VB3DCameraZoomSetting::getZoomFactor());

    QTimer::singleShot(0, this, SLOT(setVBCutplanesCameraStyle()));
}

void VirtualBronchoscopyCutPlanesWorkflowState::addDataToView()
{
	VisServicesPtr services = boost::static_pointer_cast<VisServices>(mServices);

	ImagePtr ctImage = this->getCTImage();
	ImagePtr ctImage_copied = this->getCTImageCopied();
	MeshPtr routeToTarget = this->getRouteToTarget();
	MeshPtr airways = this->getAirwaysContour();

	InteractiveClipperPtr clipper = this->enableInvertedClipper("Any", true);
	clipper->addData(this->getCTImage());

	//assuming layout: LAYOUT_VB_CUT_PLANES

	ViewGroupDataPtr viewGroup0_3D = services->view()->getGroup(0);
	this->setTransferfunction3D("Default", ctImage);
	if(ctImage)
		viewGroup0_3D->addData(ctImage->getUid());
	if(routeToTarget)
		viewGroup0_3D->addData(routeToTarget->getUid());
	if(airways)
		viewGroup0_3D->addData(airways->getUid());

	ViewGroupDataPtr viewGroup1_2D = services->view()->getGroup(1);
	this->setTransferfunction2D("2D CT Lung", ctImage);
	viewGroup1_2D->getGroup2DZoom()->set(0.2);
	viewGroup1_2D->getGlobal2DZoom()->set(0.2);
	if(ctImage)
		viewGroup1_2D->addData(ctImage->getUid());

    ViewGroupDataPtr viewGroup2_3D = services->view()->getGroup(mFlyThroughViewGroupNumber3D);
    this->setTransferfunction3D("3D CT Virtual Bronchoscopy", ctImage_copied);
    if(ctImage_copied)
        viewGroup2_3D->addData(ctImage_copied->getUid());
    if(routeToTarget)
        viewGroup2_3D->addData(routeToTarget->getUid());
}

bool VirtualBronchoscopyCutPlanesWorkflowState::canEnter() const
{
	PointMetricPtr targetPoint = this->getTargetPoint();
	MeshPtr centerline = this->getCenterline();
	MeshPtr routeToTarget = this->getRouteToTarget();
	return targetPoint && centerline && routeToTarget;
}

} //namespace cx


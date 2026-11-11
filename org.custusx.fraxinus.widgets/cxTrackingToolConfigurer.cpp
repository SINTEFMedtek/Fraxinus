/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxTrackingToolConfigurer.h"
#include "cxTrackingService.h"
#include "cxLogger.h"

namespace cx {

TrackingToolConfigurer::TrackingToolConfigurer(TrackingServicePtr trackingService, QString toolUid, QObject* parent) :
	QObject(parent),
	mTrackingService(trackingService),
	mToolUid(toolUid)
{
}

void TrackingToolConfigurer::start()
{
	if(this->getTool())
	{
		emit toolConfigured();
		return;
	}

	connect(mTrackingService.get(), &TrackingService::stateChanged, this, &TrackingToolConfigurer::stateChangedSlot, Qt::UniqueConnection);
	if(mTrackingService->getState() >= Tool::tsCONFIGURED)
		mTrackingService->setState(Tool::tsNONE);
	mTrackingService->setState(Tool::tsCONFIGURED);
}

void TrackingToolConfigurer::stop()
{
	disconnect(mTrackingService.get(), &TrackingService::stateChanged, this, &TrackingToolConfigurer::stateChangedSlot);
}

ToolPtr TrackingToolConfigurer::getTool() const
{
	TrackingService::ToolMap toolMap = mTrackingService->getTools();
	TrackingService::ToolMap::iterator iter = toolMap.find(mToolUid);
	if(iter == toolMap.end())
		return ToolPtr();
	return iter->second;
}

void TrackingToolConfigurer::stateChangedSlot()
{
	if(mTrackingService->getState() < Tool::tsCONFIGURED)
		return; // still configuring
	this->stop();

	if(!this->getTool())
	{
		CX_LOG_WARNING() << "TrackingToolConfigurer: The tracking system was configured without the tool " << mToolUid;
		return;
	}
	emit toolConfigured();
}

} //namespace cx

/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXTRACKINGTOOLCONFIGURER_H
#define CXTRACKINGTOOLCONFIGURER_H

#include "org_custusx_fraxinus_widgets_Export.h"
#include <QObject>
#include "cxForwardDeclarations.h"

namespace cx {

/** \brief Configures the tracking system with a tool from the tool configuration, and reports when it has the tool.
 *
 * Doesn't wait for tracking: the tool is reported as soon as the tracking system is configured with it,
 * also when no tracking device is connected. A configured tracking system doesn't read a changed tool
 * configuration again, so it is configured again when it doesn't have the tool.
 */
class org_custusx_fraxinus_widgets_EXPORT TrackingToolConfigurer : public QObject
{
	Q_OBJECT
public:
	TrackingToolConfigurer(TrackingServicePtr trackingService, QString toolUid, QObject* parent = nullptr);

	void start(); ///< Emits toolConfigured, at once if the tracking system already has the tool
	void stop(); ///< Stops waiting for the tool
	ToolPtr getTool() const;

signals:
	void toolConfigured();

private slots:
	void stateChangedSlot();

private:
	TrackingServicePtr mTrackingService;
	QString mToolUid;
};

} //namespace cx

#endif //CXTRACKINGTOOLCONFIGURER_H

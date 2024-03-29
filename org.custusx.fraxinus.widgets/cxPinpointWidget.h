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

#ifndef PINPOINTWIDGET_H
#define PINPOINTWIDGET_H

#include "org_custusx_fraxinus_widgets_Export.h"
#include "cxBaseWidget.h"
#include <cxStructuresSelectionWidget.h>

class QLineEdit;

namespace cx {

typedef boost::shared_ptr<class MetricManager> MetricManagerPtr;
typedef boost::shared_ptr<class PointMetric> PointMetricPtr;
typedef boost::shared_ptr<class VisServices> VisServicesPtr;

class org_custusx_fraxinus_widgets_EXPORT PinpointWidget : public BaseWidget
{
	Q_OBJECT
public:
	PinpointWidget(VisServicesPtr services, QWidget *parent);
	static QString getTargetMetricUid();
	static QString getEndoscopeMetricUid();
	static QString getDistanceMetricUid();
	StructuresSelectionWidget* getStructuresSelectionWidget();

signals:
	void targetMetricSet();

private slots:
	void setPointMetric();
	void centerToImage();
	void targetNameChanged(const QString &text);
	void loadNameOfPointMetric();

private:
	void createPointMetric();
	void createEndoscopeMetric();
	void createDistanceMetric();
	void updateCoordinateOfPointMetric();
	void setNameOfPointMetric();
	QString getNameOfPointMetric() const;

	QLineEdit *mPointMetricNameLineEdit;
	VisServicesPtr mServices;
	MetricManagerPtr mMetricManager;
	QString mTargetMetricUid;
	QString mTargetMetricName;
	StructuresSelectionWidget* mStructuresSelectionWidget;
};

}

#endif // PINPOINTWIDGET_H

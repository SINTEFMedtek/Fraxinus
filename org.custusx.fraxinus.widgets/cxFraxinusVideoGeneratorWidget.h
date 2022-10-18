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

#ifndef FRAXINUSVIDEOGENERATORWIDGET_H
#define FRAXINUSVIDEOGENERATORWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include <cxVBWidget.h>

namespace cx {

typedef boost::shared_ptr<class Data> DataPtr;
typedef boost::shared_ptr<class RouteToTargetFilter> RouteToTargetFilterPtr;
typedef boost::shared_ptr<class MetricManager> MetricManagerPtr;

class org_custusx_fraxinus_widgets_EXPORT FraxinusVideoGeneratorWidget : public VBWidget
{
	Q_OBJECT
public:
	FraxinusVideoGeneratorWidget(VisServicesPtr services, QWidget *parent = 0);
	virtual ~FraxinusVideoGeneratorWidget();

	static QString getWidgetName();

private slots:
	void startSimulationClickedSlot();
	void navigateNextRoute();

private:
	void makeRoute(PointMetricPtr targetPoint, MeshPtr centerline);

	VisServicesPtr mServices;
	int mFlyThrough3DViewGroupNumber;
	int mZoomSetting;
	std::vector<DataPtr> mTubeViewObjects;
	double mCameraPositionInPercentAdjusted;
	QPushButton* mStartButton;
	StringPropertySelectMeshPtr	mCenterline;
	MetricManagerPtr mMetricManager;
	QString mTargetUid;
	std::vector<Eigen::Vector3d> mEndPositions;

};

} //namespace cx


#endif //FRAXINUSVIDEOGENERATORWIDGET_H

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

#ifndef FRAXINUSVBWIDGET_H
#define FRAXINUSVBWIDGET_H


#include "org_custusx_fraxinus_widgets_Export.h"
#include "cxVBWidget.h"
#include "cxStructuresSelectionWidget.h"
#include "cxViewSelectionWidget.h"
#include "cxFraxinusEBUSSimulatorWidget.h"

class QRadioButton;
class QLabel;
class QPushButton;
class QMainWindow;

namespace cx {

typedef boost::shared_ptr<class Data> DataPtr;
class FraxinusEBUSSimulatorWidget;

class org_custusx_fraxinus_widgets_EXPORT FraxinusVBWidget : public VBWidget
{
	Q_OBJECT
public:
	FraxinusVBWidget(VisServicesPtr services, QWidget *parent = 0);
	virtual ~FraxinusVBWidget();
	void init();

	static QString getWidgetName();
	void setViewGroupNumber(unsigned int viewGroupNumber);
	void addObjectToVolumeView(DataPtr object);
	void addObjectToTubeView(DataPtr object);
	StructuresSelectionWidget *getStructuresSelectionWidget();
	FraxinusEBUSSimulatorWidget* getEBUSSimulatorWidget();
	void setGenerationNumbersAlongRoute(std::vector< int > generationNumbers);
	void setRadiusAlongRoute(std::vector<double> radius);
	void setNavigateAlongAirwayWall(bool navigateAlongAirwayWall);


protected slots:
//	virtual void keyPressEvent(QKeyEvent* event);
	void calculateRouteLength();
	void playbackSliderChanged(int cameraPositionInPermill);
	void setLeftRightOrientationTo90Deg();

private:
	void updateRttInfo(double cameraPositionInPercent);
	void updateAirwaysOpacity(double cameraPositionInPercent);
	void calculateDistanceFromRouteEndToTarget(Eigen::Vector3d routeEndpoint);
	QString createDistanceFromPathToTargetText();
	double getTargetDistance();
	double getRemainingRouteInsideAirways(double cameraPositionInPercent);
	int getGenerationNumber(double cameraPositionInPercent);
	double getDiameter(double cameraPositionInPercent);
	FraxinusEBUSSimulatorWidget* getFraxinusEBUSSimulatorWidget();
	QMainWindow* getMainWindow();

	ViewSelectionWidget* mViewSelectionWidget;
	StructuresSelectionWidget* mStructuresSelectionWidget;
	FraxinusEBUSSimulatorWidget* mEBUSSimulatorWidget;
	VisServicesPtr mServices;
	std::vector<DataPtr> mTubeViewObjects;
	QLabel* mStaticTotalLegth;
	QLabel* mRemainingRttLegth;
	QLabel* mDirectDistance;
	QLabel* mDistanceToTarget;
	QLabel* mWarningLabel;
	QLabel* mGenerationNumber;
	QLabel* mDiameter;
	double mRouteLength;
	double mDistanceFromPathEndToTarget;
	double mCameraPositionInPercentAdjusted;
	std::vector<int> mGenerationNumbersAlongRoute;
	std::vector<double> mRadiusAlongRoute;

};

} //namespace cx


#endif //FRAXINUSVBWIDGET_H

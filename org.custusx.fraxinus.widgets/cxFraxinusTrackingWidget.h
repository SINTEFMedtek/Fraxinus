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

#ifndef FRAXINUSTRACKINGWIDGET_H
#define FRAXINUSTRACKINGWIDGET_H

#include "org_custusx_fraxinus_widgets_Export.h"
#include "cxStructuresSelectionWidget.h"
#include "cxFraxinusPatientOrientationWidget.h"
#include "cxFraxinusNavigationWidget.h"
#include "cxBaseWidget.h"
#include "cxForwardDeclarations.h"
#include "cxTrackerConfiguration.h"
#include "cxTrackingService.h"

class QRadioButton;
class QLabel;
class QPushButton;
class QComboBox;

namespace cx {

class ToolConfigureGroupBox;

class org_custusx_fraxinus_widgets_EXPORT FraxinusTrackingWidget : public BaseWidget
{
	Q_OBJECT
public:
	FraxinusTrackingWidget(VisServicesPtr services, FraxinusNavigationWidget* fraxinusNavigationWidget = nullptr, QWidget *parent = nullptr);
	virtual ~FraxinusTrackingWidget();

	static QString getWidgetName(){return "fraxinus_tracking_widget";}
	void startTracking();
	void stopTracking();
	void startUltrasoundSimulation();
	void stopUltrasoundSimulation();
	TrackingSystemServicePtr getIGSTKTrackingSystemService();
	TrackingServicePtr getTrackingService();

signals:
	void trackingReady();

private slots:
	void updateButtonStatusSlot();
	void updateTrackerConfigurationTools();
	void startTrackingClickedSlot(bool);
	void stopTrackingClickedSlot(bool);
	void setUpEBUSToolAndStartTracking();

private:
	TrackingServicePtr mTrackingService;
	ToolConfigureGroupBox* mToolConfigureGroupBox;
	QString mClinicalApplication;
	QString mTrackingSystemName;
	QString mEBUSProbeUid;
	QString mEBUSProbeName;
	QString mEBUSProbeConfigurationName;
	int mNumberOfTools;
	TrackerConfigurationPtr mTrackerConfiguration;
	std::vector<QComboBox*> mToolFilesComboBoxes;
	QPushButton* mStartTrackingButton;
	QPushButton* mStopTrackingButton;
	FraxinusNavigationWidget* mFraxinusNavigationWidget;
	Tool::State mToolState;

	void copyToolConfigFile(QString filename);
	void addToolsToComboBoxes(int numberOfTools, TrackerConfigurationPtr configuration, QStringList applicationsFilter, QStringList trackingsystemsFilter);
	void printTrackerConfiguration(); //debug
};


} //namespace cx


#endif //FRAXINUSTRACKINGWIDGET_H

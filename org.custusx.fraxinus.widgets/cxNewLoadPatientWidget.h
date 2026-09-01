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

#ifndef NEWLOADPATIENTWIDGET_H
#define NEWLOADPATIENTWIDGET_H

#include "org_custusx_fraxinus_widgets_Export.h"
#include "cxBaseWidget.h"
#include <QKeyEvent>
#include "cxForwardDeclarations.h"

class QPushButton;
class QCheckBox;
class QGroupBox;
class QLabel;

namespace cx
{

class org_custusx_fraxinus_widgets_EXPORT NewLoadPatientWidget : public BaseWidget
{
	Q_OBJECT
public:
	NewLoadPatientWidget(QWidget *parent, VisServicesPtr services, AcquisitionServicePtr acquisitionService);
	static QString getWidgetName();

	QGroupBox* getProcessingInfoGroup();

	bool isAirwaysChecked() const;
	bool isLymphNodesChecked() const;
	bool isHeartChecked() const;
	bool isMediumOrgansChecked() const;
	bool isSmallOrgansChecked() const;
	bool isTumorsChecked() const;
	bool isLungVesselsChecked() const;
	bool isLungLobesChecked() const;

public slots:
	void segmentationStarted();
	void segmentationFinished();

private slots:
	void createNewPatient();
	void createNewPatientFromUSB();
	void loadPatient();
	void selectCTData();
	void patientCreatedInfo();
	void closePatientCreatedInfo();
	void closeDataLoadedInfo(bool dataLoadingCompleted);
	void loadCTDataDialog();
	void loadCTDataFromUSB();
	void loadCTDataDialogFinished();
	void selectAll(bool checked);
	void updateRunSegmentationButton();
	void updateSegmentationCheckBoxes();

signals:
	void dataImportCompleted();
	void runSegmentationClicked();
	void existingPatientLoaded();

private:
	void enableImportDataButton();
	void loadCTData(bool fromUSB = false);
	void dataAddedOrRemoved();

	VisServicesPtr mServices;
	QPushButton* mSelectCTDataButton = nullptr;
	QDialog* mPatientCreatedInfo = nullptr;
	QDialog* mDataLoadedInfo = nullptr;
	QDialog* mLoadCTDialog = nullptr;
	QPushButton* mUSBButton = nullptr;
	QPushButton* mHardDriveButton = nullptr;
	QMetaObject::Connection mConnectionToYesButtonPatientCreated, mConnectionToNoButtonPatientCreated;
	QMetaObject::Connection mConnectionToYesButtonDataLoaded, mConnectionToNoButtonDataLoaded;

	QCheckBox* mCheckBoxAirways = nullptr;
	QCheckBox* mCheckBoxLymphNodes = nullptr;
	QCheckBox* mCheckBoxHeart = nullptr;
	QCheckBox* mCheckBoxMediumOrgans = nullptr;
	QCheckBox* mCheckBoxSmallOrgans = nullptr;
	QCheckBox* mCheckBoxTumors = nullptr;
	QCheckBox* mCheckBoxLungVessels = nullptr;
	QCheckBox* mCheckBoxLungLobes = nullptr;
	QCheckBox* mCheckBoxSelectAll = nullptr;
	QLabel* mStatusLabelAirways = nullptr;
	QLabel* mStatusLabelLymphNodes = nullptr;
	QLabel* mStatusLabelHeart = nullptr;
	QLabel* mStatusLabelMediumOrgans = nullptr;
	QLabel* mStatusLabelSmallOrgans = nullptr;
	QLabel* mStatusLabelTumors = nullptr;
	QLabel* mStatusLabelLungVessels = nullptr;
	QLabel* mStatusLabelLungLobes = nullptr;
	QPushButton* mRunSegmentationButton = nullptr;
	QGroupBox* mSegmentationGroup = nullptr;
	QGroupBox* mProcessingInfoGroup = nullptr;

	bool mThoraxCTLoaded = false;
	bool mPETLoaded = false;
	bool mSkipDataLoadedInfo = false;
	bool mSegmentationRunning = false;
};

}

#endif // NEWLOADPATIENTWIDGET_H

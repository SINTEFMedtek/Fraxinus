/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXFRAXINUSSEGMENTATIONS_H
#define CXFRAXINUSSEGMENTATIONS_H

#include "org_custusx_fraxinus_core_state_Export.h"

#include <QDialog>
#include <QCheckBox>
#include "cxFilterTimedAlgorithm.h"
#include "cxTimedAlgorithmProgressBar.h"
#include "cxDefinitions.h"

namespace cx
{
typedef boost::shared_ptr<class FraxinusSegmentations> FraxinusSegmentationsPtr;
class DisplayTimerWidget;

class org_custusx_fraxinus_core_state_EXPORT FraxinusSegmentations : public QObject
{
	Q_OBJECT
public:
	FraxinusSegmentations(CoreServicesPtr services);
	~FraxinusSegmentations();
	
	ImagePtr getCTImage() const;
	MeshPtr getCenterline() const;
	
	MeshPtr getAirwaysContour();
	MeshPtr getAirwaysTubes();
	MeshPtr getAirwaysTubesColored();
	MeshPtr getVessels();
	MeshPtr getMesh(QString contain_str_1, QString contain_str_2 = "", QString not_contain_str="");
	MeshPtr getLungs();
	MeshPtr getLymphNodes();
	MeshPtr getNodules();
	MeshPtr getVenaCava();
	MeshPtr getAorticArch();
	MeshPtr getAscendingAorta();
	MeshPtr getDescendingAorta();
	MeshPtr getSpine();
	MeshPtr getSubCarArt();
	MeshPtr getEsophagus();
	MeshPtr getBrachiocephalicVeins();
	MeshPtr getAzygos();
	MeshPtr getHeart();
	MeshPtr getPulmonaryVeins();
	MeshPtr getPulmonaryTrunk();
	
	void createSelectSegmentationBox();
	void createProcessingInfo();
	void performAirwaysSegmentation(ImagePtr image);
	void performMLSegmentation(ImagePtr image);
	QString getFilterScriptsPath();
	void checkIfSegmentationSucceeded();
	void close();
	
signals:
  void segmentationFinished();
  
private slots:
	void imageSelected();
	void cancel();
	void runAirwaysFilterSlot();
	void runMLFilterSlot();
	void airwaysFinishedSlot();
	void MLFinishedSlot();
	
private:
	CoreServicesPtr mServices;
	
	FilterPtr mCurrentFilter;
	FilterTimedAlgorithmPtr mThread;
	TimedAlgorithmProgressBar* mTimedAlgorithmProgressBar;
	
	QDialog* mSegmentationSelectionInput;
	QDialog* mSegmentationProcessingInfo;
	DisplayTimerWidget* mAirwaysTimerWidget;
	DisplayTimerWidget* mLungsTimerWidget;
	DisplayTimerWidget* mLymphNodesTimerWidget;
	DisplayTimerWidget* mPulmonarySystemTimerWidget;
	DisplayTimerWidget* mMediumOrgansTimerWidget;
	DisplayTimerWidget* mSmallOrgansTimerWidget;
	DisplayTimerWidget* mNodulesTimerWidget;
	DisplayTimerWidget* mVesselsTimerWidget;
	DisplayTimerWidget* mActiveTimerWidget = NULL;
	QCheckBox* mCheckBoxAirways;
	QCheckBox* mCheckBoxLungs;
	QCheckBox* mCheckBoxLymphNodes;
	QCheckBox* mCheckBoxPulmonarySystem;
	QCheckBox* mCheckBoxMediumOrgans;
	QCheckBox* mCheckBoxSmallOrgans;
	QCheckBox* mCheckBoxNodules;
	QCheckBox* mCheckBoxVessels;
	bool mAirwaysProcessed = false;
	bool mVesselsProcessed = false;
	bool mLungsProcessed = false;
	bool mLymphNodesProcessed = false;
	bool mPulmonarySystemProcessed = false;
	bool mMediumOrgansProcessed = false;
	bool mSmallOrgansProcessed = false;
	bool mNodulesProcessed = false;
	bool mSegmentAirways;
	bool mSegmentVessels;
	bool mSegmentLungs;
	bool mSegmentLymphNodes;
	bool mSegmentPulmonarySystem;
	bool mSegmentSmallOrgans;
	bool mSegmentMediumOrgans;
	bool mSegmentNodules;
	LUNG_STRUCTURES mCurrentSegmentationType;	
};
}//cx
#endif // CXFRAXINUSSEGMENTATIONS_H

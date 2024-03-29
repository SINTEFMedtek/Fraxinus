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
	ImagePtr getAirwaysVolume() const;

	MeshPtr getRawCenterline();
	MeshPtr getCenterline();
	MeshPtr getAirwaysContour();
	MeshPtr getAirwaysTubes();
	MeshPtr getLungVessels();
	MeshPtr getMesh(QString contain_str_1, QString contain_str_2 = "", QString not_contain_str_1="", QString not_contain_str_2="");
	MeshPtr getLungs();
	MeshPtr getLymphNodes();
	MeshPtr getNodules();
	MeshPtr getTumors();
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
	void performPythonSegmentation(ImagePtr image);
	void performMLSegmentation(ImagePtr image);
	QString getFilterScriptsPath();
	void postProcessAirways();
	void checkIfSegmentationSucceeded();
	void close();

signals:
	void segmentationFinished();

private slots:
	void imageSelected();
	void cancel();
	void runPythonFilterSlot();
	void runMLFilterSlot();
	void pythonFinishedSlot();
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
	DisplayTimerWidget* mHeartTimerWidget;
	DisplayTimerWidget* mMediumOrgansTimerWidget;
	DisplayTimerWidget* mSmallOrgansTimerWidget;
	DisplayTimerWidget* mNodulesTimerWidget;
	DisplayTimerWidget* mTumorsTimerWidget;
	DisplayTimerWidget* mLungVesselsTimerWidget;
	DisplayTimerWidget* mActiveTimerWidget = NULL;
	QCheckBox* mCheckBoxAirways;
	QCheckBox* mCheckBoxLungs;
	QCheckBox* mCheckBoxLymphNodes;
	QCheckBox* mCheckBoxHeart;
	QCheckBox* mCheckBoxMediumOrgans;
	QCheckBox* mCheckBoxSmallOrgans;
	QCheckBox* mCheckBoxNodules;
	QCheckBox* mCheckBoxTumors;
	QCheckBox* mCheckBoxLungVessels;
	bool mAirwaysProcessed = false;
	bool mCenterlineProcessed = false;
	bool mLungVesselsProcessed = false;
	bool mLungsProcessed = false;
	bool mLymphNodesProcessed = false;
	bool mHeartProcessed = false;
	bool mMediumOrgansProcessed = false;
	bool mSmallOrgansProcessed = false;
	bool mNodulesProcessed = false;
	bool mTumorsProcessed = false;
	bool mSegmentAirways;
	bool mSegmentLungVessels;
	bool mSegmentLungs;
	bool mSegmentLymphNodes;
	bool mSegmentHeart;
	bool mSegmentSmallOrgans;
	bool mSegmentMediumOrgans;
	bool mSegmentNodules;
	bool mSegmentTumors;
	LUNG_STRUCTURES mCurrentSegmentationType;

	void setMeshNameAndStopTimer(MeshPtr mesh);
	void setMeshName(MeshPtr mesh, LUNG_STRUCTURES segmentationType);
	void stopTimer(MeshPtr mesh);

};
}//cx
#endif // CXFRAXINUSSEGMENTATIONS_H

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
typedef boost::shared_ptr<class BinaryThinningImageFilter3DFilter> BinaryThinningImageFilter3DFilterPtr;
typedef boost::shared_ptr<class GenericScriptFilter> GenericScriptFilterPtr;
class DisplayTimerWidget;

class org_custusx_fraxinus_core_state_EXPORT FraxinusSegmentations : public QObject
{
	Q_OBJECT
public:
	FraxinusSegmentations(CoreServicesPtr services);
	~FraxinusSegmentations();
	
	ImagePtr getCTImage() const;
	ImagePtr getAirwaysVolume() const;

//	MeshPtr getRawCenterline();
	MeshPtr getCenterline();
//	MeshPtr getAirwaysContour();
	MeshPtr getAirwaysTubes();
	MeshPtr getLungVessels();
	MeshPtr getMesh(QString contain_str_1, QString contain_str_2 = "", QString not_contain_str_1="", QString not_contain_str_2="");
	MeshPtr getMesh(ORGAN_TYPE organType);
	
	void createSelectSegmentationBox();
	void createProcessingInfo();
	void performPythonSegmentation(ImagePtr image);
	void performMLSegmentation(ImagePtr image);
	QString getFilterScriptsPath();
	void postProcessAirways();
	void checkIfSegmentationSucceeded();
	void close();

	//Utility functions
	static QString getReadableString(ORGAN_TYPE target);
	static QString getReadableString(QString string);///< Adds space to CamelCase strings. All words, including first must start with Uppercase setters

signals:
	void segmentationFinished();

protected:
	bool mSegmentAirways = false;
	bool mSegmentLungs = false;
	bool mSegmentLymphNodes = false;
	bool mSegmentHeart = false;
	bool mSegmentMediumOrgans = false;
	bool mSegmentSmallOrgans = false;

	QStringList getRaidionicsOutputClasses(bool startTimers = true);

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
	bool mRaidionicsRun = false;
//	bool mAirwaysProcessed = false;
//	bool mCenterlineProcessed = false;
	bool mLungVesselsProcessed = false;
//	bool mLungsProcessed = false;
//	bool mLymphNodesProcessed = false;
//	bool mHeartProcessed = false;
//	bool mMediumOrgansProcessed = false;
//	bool mSmallOrgansProcessed = false;
	bool mNodulesProcessed = false;
	bool mTumorsProcessed = false;
	bool mSegmentLungVessels = false;
	bool mSegmentNodules = false;
	bool mSegmentTumors = false;
	LUNG_STRUCTURES mCurrentSegmentationType;

	void setMeshNameAndStopTimer(ORGAN_TYPE target);
	void setMeshName(ORGAN_TYPE target);
	void stopTimer(ORGAN_TYPE target);
	void generateCenterline();
	bool runRaidionics(GenericScriptFilterPtr scriptFilter);
	DisplayTimerWidget *getTimer(ORGAN_TYPE target);
};
}//cx
#endif // CXFRAXINUSSEGMENTATIONS_H

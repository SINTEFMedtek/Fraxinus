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
#include "cxElastixManager.h"

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
	FraxinusSegmentations(RegServicesPtr services);
	~FraxinusSegmentations();
	
	ImagePtr findAndLabelThoraxCT() const;

	BranchListPtr getBranchList();
	
	void createSelectSegmentationBox();
	void createProcessingInfo();
	void performPETCTregistration();
	void performPythonSegmentation(ImagePtr image);
	void performMLSegmentation(ImagePtr image);
	void postProcessTumors();
	void checkIfSegmentationSucceeded();
	void close();
	static std::vector<QString> getLobeNameFromPositions(std::vector<Vector3D> positions_r, CoreServicesPtr services);

signals:
	void segmentationFinished();
	void postProcessAirwaysFinished();
	void centerlineReady();
	void centerlineGenerationFailed();

protected:
	bool mSegmentAirways = false;
	bool mSegmentLymphNodes = false;
	bool mSegmentHeart = false;
	bool mSegmentMediumOrgans = false;
	bool mSegmentSmallOrgans = false;
	bool mSegmentTumors = false;
	bool mSegmentLungVessels = false;
	bool mSegmentLungLobes = false;
	//bool mSegmentNodules = false;
	bool mRegisterPET = false;

	QStringList getRaidionicsOutputClasses(bool startTimers = true);
	void setElastixParameters();

public slots:
	void updateSelectSegmentationBox();

private slots:
	void patientChanged();
	void selectAll(bool checked);
	void imageSelected();
	void cancel();
	void runPythonFilterSlot();
	void runMLFilterSlot();
	void pythonFinishedSlot();
	void MLFinishedSlot1();
	void MLFinishedSlot2();
	void runElastixSlot();
	void elastixFinishedSlot();
	void checkForPETData();
	void showProcessingInfoFinished();
	void closeSegmentationInfo();
	void postProcessAirwaysSlot();
	void centerlineFinishedSlot();
	
private:
	void deleteTumorsAndNodulesVolumes();
	std::vector<QString> getLobeOfTumors(std::vector<MeshPtr> tumorMeshes);
	vtkImageDataPtr mergeTumorVolumes(ImagePtr tumorsVolume, ImagePtr nodulesVolume);
	void setNumberAndSizeToTumorVolumes(std::vector<MeshPtr> tumorMeshes, std::vector<double> tumorSizes, std::vector<QString> lobeNames);

	RegServicesPtr mServices;
	
	FilterPtr mCurrentFilter;
	FilterTimedAlgorithmPtr mThread;
	FilterTimedAlgorithmPtr mCenterlineThread;
	TimedAlgorithmProgressBar* mTimedAlgorithmProgressBar;
	BinaryThinningImageFilter3DFilterPtr mBinaryThinningImageFilter3DFilter;
	
	QDialog* mSegmentationSelectionInput = nullptr;
	QDialog* mSegmentationProcessingInfo = nullptr;
	QDialog* mSegmentationFinishedInfo = nullptr;
	DisplayTimerWidget* mAirwaysTimerWidget;
	DisplayTimerWidget* mLungsTimerWidget;
	DisplayTimerWidget* mLymphNodesTimerWidget;
	DisplayTimerWidget* mHeartTimerWidget;
	DisplayTimerWidget* mMediumOrgansTimerWidget;
	DisplayTimerWidget* mSmallOrgansTimerWidget;
	DisplayTimerWidget* mNodulesTimerWidget;
	DisplayTimerWidget* mTumorsTimerWidget;
	DisplayTimerWidget* mPETTimerWidget;
	DisplayTimerWidget* mLungVesselsTimerWidget;
	DisplayTimerWidget* mLungLobesTimerWidget;
	DisplayTimerWidget* mActiveTimerWidget = NULL;
	QCheckBox* mCheckBoxAirways = nullptr;
	QCheckBox* mCheckBoxLungs = nullptr;
	QCheckBox* mCheckBoxLymphNodes = nullptr;
	QCheckBox* mCheckBoxHeart = nullptr;
	QCheckBox* mCheckBoxMediumOrgans = nullptr;
	QCheckBox* mCheckBoxSmallOrgans = nullptr;
	QCheckBox* mCheckBoxNodules = nullptr;
	QCheckBox* mCheckBoxTumors = nullptr;
	QCheckBox* mCheckBoxPET = nullptr;
	QCheckBox* mCheckBoxLungVessels = nullptr;
	QCheckBox* mCheckBoxLungLobes = nullptr;
	QCheckBox* mCheckBoxSelectAll = nullptr;
	bool mAirwaysProcessed = false;
	bool mLungVesselsProcessed = false;
	bool mLungLobesProcessed = false;
	bool mNodulesProcessed = false;
	bool mTumorsProcessed = false;
	bool mLymphNodesProcessed = false;
	bool mHeartProcessed = false;
	bool mMediumOrgansProcessed = false;
	bool mSmallOrgansProcessed = false;
	LUNG_STRUCTURES mCurrentSegmentationType;
	BranchListPtr mBranchList;
	ElastixManagerPtr mElastixManager;
	QPushButton* mOKbutton = nullptr;
	QPushButton* mCancelbutton = nullptr;
	QPushButton *mOKbuttonProcessingFinished = nullptr;

	void setMeshNameAndStopTimer(ORGAN_TYPE target);
	void setMeshName(ORGAN_TYPE target);///< Needs to be called after patient()->insertData to work. Better to use: setMeshNameAndType(MeshPtr mesh, ORGAN_TYPE target)
	void setMeshNameAndType(MeshPtr mesh, ORGAN_TYPE target);
	void stopTimer(ORGAN_TYPE target, bool checkVolume = false);
	void generateCenterline();
	bool runRaidionics(GenericScriptFilterPtr scriptFilter);
	DisplayTimerWidget *getTimer(ORGAN_TYPE target);
};
}//cx
#endif // CXFRAXINUSSEGMENTATIONS_H

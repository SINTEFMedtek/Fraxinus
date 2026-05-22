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
#include <QMap>
#include "cxFilterTimedAlgorithm.h"
#include "cxTimedAlgorithmProgressBar.h"
#include "cxDefinitions.h"
#include "cxElastixManager.h"

class QProgressBar;

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
	void startSegmentationWithOptions(bool airways, bool lymphNodes, bool heart,
	                                   bool mediumOrgans, bool smallOrgans, bool tumors,
	                                   bool lungVessels, bool lungLobes);
	void setProcessingInfoParentWidget(QWidget* container);
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

	void showProcessingInfoFinished();
	void closeSegmentationInfo();
	void postProcessAirwaysSlot();
	void centerlineFinishedSlot();
	void onScriptOutput(const QString& line);
	void centerlineProgressTick();
	void PETProgressTick();
	
private:
	void deleteTumorsAndNodulesVolumes();
	std::vector<QString> getLobeOfTumors(std::vector<MeshPtr> tumorMeshes);
	vtkImageDataPtr mergeTumorVolumes(ImagePtr tumorsVolume, ImagePtr nodulesVolume);
	void setNumberAndSizeToTumorVolumes(std::vector<MeshPtr> tumorMeshes, std::vector<double> tumorSizes, std::vector<QString> lobeNames);

	RegServicesPtr mServices;
	
	FilterPtr mCurrentFilter;
	GenericScriptFilterPtr mCurrentScriptFilter;
	FilterTimedAlgorithmPtr mThread;
	FilterTimedAlgorithmPtr mCenterlineThread;
	TimedAlgorithmProgressBar* mTimedAlgorithmProgressBar;
	BinaryThinningImageFilter3DFilterPtr mBinaryThinningImageFilter3DFilter;
	
	QDialog* mSegmentationSelectionInput = nullptr;
	QDialog* mSegmentationProcessingInfo = nullptr;
	QDialog* mSegmentationFinishedInfo = nullptr;
	QWidget* mProcessingInfoParentWidget = nullptr;
	QMap<LUNG_STRUCTURES, QProgressBar*> mProgressBars;
	DisplayTimerWidget* mAirwaysTimerWidget = nullptr;
	DisplayTimerWidget* mLungsTimerWidget = nullptr;
	DisplayTimerWidget* mLymphNodesTimerWidget = nullptr;
	DisplayTimerWidget* mHeartTimerWidget = nullptr;
	DisplayTimerWidget* mMediumOrgansTimerWidget = nullptr;
	DisplayTimerWidget* mSmallOrgansTimerWidget = nullptr;
	DisplayTimerWidget* mNodulesTimerWidget = nullptr;
	DisplayTimerWidget* mTumorsTimerWidget = nullptr;
	DisplayTimerWidget* mPETTimerWidget = nullptr;
	DisplayTimerWidget* mLungVesselsTimerWidget = nullptr;
	DisplayTimerWidget* mLungLobesTimerWidget = nullptr;
	DisplayTimerWidget* mCenterlinesTimerWidget = nullptr;
	DisplayTimerWidget* mActiveTimerWidget = NULL;
	QCheckBox* mCheckBoxAirways = nullptr;
	QCheckBox* mCheckBoxLungs = nullptr;
	QCheckBox* mCheckBoxLymphNodes = nullptr;
	QCheckBox* mCheckBoxHeart = nullptr;
	QCheckBox* mCheckBoxMediumOrgans = nullptr;
	QCheckBox* mCheckBoxSmallOrgans = nullptr;
	QCheckBox* mCheckBoxNodules = nullptr;
	QCheckBox* mCheckBoxTumors = nullptr;

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
	QProgressBar* getProgressBar(LUNG_STRUCTURES type);
	QProgressBar* getRaidionicsBarForLine(const QString& line);
	int scaledPipelineValue(int val) const;

	QProgressBar* mCurrentRaidionicsBar = nullptr;
	bool mRaidionicsInInference = false;
	int mPipelineCurrentStep = 1;
	int mPipelineTotalSteps = 1;
	QTimer* mCenterlineProgressTimer = nullptr;
	int mCenterlineProgressTicks = 0;
	QTimer* mPETProgressTimer = nullptr;
	int mPETProgressTicks = 0;
};
}//cx
#endif // CXFRAXINUSSEGMENTATIONS_H

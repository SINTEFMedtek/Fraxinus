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
	
	ImagePtr getImage(IMAGE_MODALITY modality, IMAGE_SUBTYPE subtype) const;
	ImagePtr findAndLabelThoraxCT() const;
	ImagePtr getVolume(ORGAN_TYPE organType) const;

	BranchListPtr getBranchList();

	MeshPtr getMesh(ORGAN_TYPE organType);
	std::vector<MeshPtr> getMeshes(ORGAN_TYPE organType);
	
	void createSelectSegmentationBox();
	void createProcessingInfo();
	void performPETCTregistration();
	void performPythonSegmentation(ImagePtr image);
	void performMLSegmentation(ImagePtr image);
	QString getFilterScriptsPath();
	void postProcessAirways();
	void postProcessTumors();
	void checkIfSegmentationSucceeded();
	void close();

signals:
	void segmentationFinished();

protected:
	bool mSegmentAirways = false;
	bool mSegmentLymphNodes = false;
	bool mSegmentHeart = false;
	bool mSegmentMediumOrgans = false;
	bool mSegmentSmallOrgans = false;
	bool mSegmentTumors = false;

	QStringList getRaidionicsOutputClasses(bool startTimers = true);
	void setElastixParameters();

private slots:
	void patientChanged();
	void selectAll(bool checked);
	void imageSelected();
	void cancel();
	void runPythonFilterSlot();
	void runMLFilterSlot();
	void pythonFinishedSlot();
	void MLFinishedSlot();
	void runElastixSlot();
	void elastixFinishedSlot();
	void checkForPETData();
	
private:
	vtkImageDataPtr mergeBinaryVolumes(vtkImageDataPtr imageA, vtkImageDataPtr imageB);
	vtkImageDataPtr shiftVtkScalarToUnsignedShort(vtkImageDataPtr input);

	RegServicesPtr mServices;
	
	FilterPtr mCurrentFilter;
	FilterTimedAlgorithmPtr mThread;
	TimedAlgorithmProgressBar* mTimedAlgorithmProgressBar;
	
	QDialog* mSegmentationSelectionInput = nullptr;
	QDialog* mSegmentationProcessingInfo;
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
	DisplayTimerWidget* mActiveTimerWidget = NULL;
	QCheckBox* mCheckBoxAirways;
	QCheckBox* mCheckBoxLungs;
	QCheckBox* mCheckBoxLymphNodes;
	QCheckBox* mCheckBoxHeart;
	QCheckBox* mCheckBoxMediumOrgans;
	QCheckBox* mCheckBoxSmallOrgans;
	QCheckBox* mCheckBoxNodules;
	QCheckBox* mCheckBoxTumors;
	QCheckBox* mCheckBoxPET;
	QCheckBox* mCheckBoxLungVessels;
	QCheckBox* mCheckBoxSelectAll;
	bool mAirwaysProcessed = false;
	bool mLungVesselsProcessed = false;
	bool mNodulesProcessed = false;
	bool mTumorsProcessed = false;
	bool mLymphNodesProcessed = false;
	bool mHeartProcessed = false;
	bool mMediumOrgansProcessed = false;
	bool mSmallOrgansProcessed = false;
	bool mSegmentLungVessels = false;
	bool mRegisterPET = false;
	LUNG_STRUCTURES mCurrentSegmentationType;
	BranchListPtr mBranchList;
	ElastixManagerPtr mElastixManager;
	QPushButton* mOKbutton;
	QPushButton* mCancelbutton;

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

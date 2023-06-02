/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "catch.hpp"
#include <QFileInfo>
#include "cxFraxinusSegmentations.h"
#include "cxCoreServices.h"
#include "cxRaidionics.h"
#include "cxEnumConverter.h"

namespace
{
class TestFraxinusSegmentations : public cx::FraxinusSegmentations
{
public:
	TestFraxinusSegmentations() :
		cx::FraxinusSegmentations(cx::CoreServices::getNullObjects())
	{}

	QStringList testGetRaidionicsOutputClasses()
	{
		return getRaidionicsOutputClasses(false);
	}
	int setSingleTargetSegmentations()
	{
		mSegmentAirways = true;
		mSegmentLungs = true;
		mSegmentLymphNodes = true;
		return 3;
	}
	int setMultipleTargetsSegmentations()
	{
		mSegmentHeart = true;
		mSegmentMediumOrgans = true;
		mSegmentSmallOrgans = true;
		return 3;
	}
	int getNumExpandedTargets()
	{
		QStringList targets = cx::Raidionics::createTargetList(enum2string(cx::lmPULMSYST_HEART));
		int retval = targets.size();
		targets = cx::Raidionics::createTargetList(enum2string(cx::lmMEDIUM_ORGANS_MEDIASTINUM));
		retval += targets.size();
		targets = cx::Raidionics::createTargetList(enum2string(cx::lmSMALL_ORGANS_MEDIASTINUM));
		retval += targets.size();
		return retval;
	}

};
}//namespace

TEST_CASE("FraxinusSegmentations: getFilterScriptsPath", "[unit]")
{
	cx::FraxinusSegmentations segmentations(cx::CoreServices::getNullObjects());
	QString scriptFilePath = segmentations.getFilterScriptsPath();
	CHECK(QFileInfo::exists(scriptFilePath));
	
	QString iniFilePath = scriptFilePath + "python_Lungs_test.ini";
	CHECK(QFileInfo::exists(iniFilePath));
}

TEST_CASE("FraxinusSegmentations: getRaidionicsOutputClasses", "[unit]")
{
	TestFraxinusSegmentations segmentations;
	QStringList outputClasses = segmentations.testGetRaidionicsOutputClasses();
	{
		INFO(outputClasses.join(", ").toStdString());
		CHECK(outputClasses.isEmpty());
	}

	int numSegmentations = segmentations.setSingleTargetSegmentations();
	outputClasses = segmentations.testGetRaidionicsOutputClasses();
	{
		INFO(outputClasses.join(", ").toStdString());
		CHECK(outputClasses.size() == numSegmentations);
	}

	numSegmentations += segmentations.setMultipleTargetsSegmentations();
	outputClasses = segmentations.testGetRaidionicsOutputClasses();
	{
		INFO(outputClasses.join(", ").toStdString());
		CHECK(outputClasses.size() == numSegmentations);
	}

	numSegmentations -= segmentations.setMultipleTargetsSegmentations();//These will be replaced in Raidionics::expandOutputClasses() below
	numSegmentations += segmentations.getNumExpandedTargets();
	outputClasses = cx::Raidionics::expandOutputClasses(outputClasses);
	{
		INFO(outputClasses.join(", ").toStdString());
		CHECK(outputClasses.size() == numSegmentations);
	}

	//Verify that all lung targets are used, and covered by this test
	//The test must be updated when adding new targets (LUNG_MODEL_TARGETS)
	CHECK(outputClasses.size() == cx::otRAIDIONICS_END);
}

TEST_CASE("FraxinusSegmentations: getReadableString", "[unit]")
{
	QString string = cx::FraxinusSegmentations::getReadableString("");
	{
		INFO(string.toStdString());
		CHECK((string == ""));
	}

	string = cx::FraxinusSegmentations::getReadableString("test");
	{
		INFO(string.toStdString());
		CHECK((string == ""));
	}
	string = cx::FraxinusSegmentations::getReadableString("Test");
	{
		INFO(string.toStdString());
		CHECK((string == "Test"));
	}
	string = cx::FraxinusSegmentations::getReadableString("TestString");
	{
		INFO(string.toStdString());
		CHECK((string == "Test String"));
	}
	string = cx::FraxinusSegmentations::getReadableString("TestTestString");
	{
		INFO(string.toStdString());
		CHECK((string == "Test Test String"));
	}
	string = cx::FraxinusSegmentations::getReadableString(cx::otLYMPH_NODES);
	{
		INFO(string.toStdString());
		CHECK((string == "Lymph Nodes"));
	}

	for(int target = cx::otAIRWAYS; target < cx::organtypeCOUNT; ++target)
	{
		cx::ORGAN_TYPE targetEnum = cx::ORGAN_TYPE(target);
		string = cx::FraxinusSegmentations::getReadableString(targetEnum);
		if(!string.contains(" "))
			CHECK(string == enum2string(targetEnum));
		else if(targetEnum == cx::otLYMPH_NODES)
			CHECK(string == "Lymph Nodes");
		else if(targetEnum == cx::otVENA_CAVA)
			CHECK(string == "Vena Cava");
	}

}

/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "catch.hpp"

#include <vector>
#include "cxTrackingToolConfigurer.h"
#include "cxTrackingServiceNull.h"
#include "cxDummyTool.h"

namespace
{
const QString kToolUid = "EBUS EXERA III Simulator";

/** A tracking system without a device: it configures in a thread, finished with finishConfiguring(), and never tracks */
class TrackingServiceWithoutDevice : public cx::TrackingServiceNull
{
public:
	std::vector<cx::Tool::State> mRequestedStates;

	TrackingServiceWithoutDevice(cx::Tool::State state = cx::Tool::tsNONE, bool hasTool = false) : mState(state)
	{
		if (hasTool)
			this->addTool();
	}
	cx::Tool::State getState() const override { return mState; }
	void setState(const cx::Tool::State val) override
	{
		mRequestedStates.push_back(val);
		if (val == cx::Tool::tsNONE && mState != cx::Tool::tsNONE)
		{
			mState = cx::Tool::tsNONE;
			mTools.clear();
			emit stateChanged();
		}
	}
	ToolMap getTools() override { return mTools; }

	void finishConfiguring(bool withTool)
	{
		if (withTool)
			this->addTool();
		mState = cx::Tool::tsCONFIGURED;
		emit stateChanged();
	}

private:
	cx::Tool::State mState;
	ToolMap mTools;

	void addTool() { mTools[kToolUid] = cx::ToolPtr(new cx::DummyTool(kToolUid)); }
};
typedef std::shared_ptr<TrackingServiceWithoutDevice> TrackingServiceWithoutDevicePtr;

struct ConfigurerFixture
{
	TrackingServiceWithoutDevicePtr mTracking;
	cx::TrackingToolConfigurer* mConfigurer;
	int mToolConfiguredCount;

	ConfigurerFixture(TrackingServiceWithoutDevicePtr tracking) :
		mTracking(tracking),
		mConfigurer(new cx::TrackingToolConfigurer(tracking, kToolUid)),
		mToolConfiguredCount(0)
	{
		QObject::connect(mConfigurer, &cx::TrackingToolConfigurer::toolConfigured, [this]() { ++mToolConfiguredCount; });
	}
	~ConfigurerFixture() { delete mConfigurer; }
};
} // namespace

TEST_CASE("TrackingToolConfigurer: the tool is reported when the tracking system is configured, without waiting for tracking", "[unit][plugins][org.custusx.fraxinus.widgets]")
{
	ConfigurerFixture fixture(TrackingServiceWithoutDevicePtr(new TrackingServiceWithoutDevice()));

	fixture.mConfigurer->start();
	REQUIRE(fixture.mTracking->mRequestedStates == std::vector<cx::Tool::State>({cx::Tool::tsCONFIGURED}));
	CHECK(fixture.mToolConfiguredCount == 0);

	fixture.mTracking->finishConfiguring(true);
	CHECK(fixture.mToolConfiguredCount == 1);
	CHECK(fixture.mConfigurer->getTool());
	CHECK(fixture.mTracking->getState() == cx::Tool::tsCONFIGURED);
}

TEST_CASE("TrackingToolConfigurer: the tool is reported at once when the tracking system has it", "[unit][plugins][org.custusx.fraxinus.widgets]")
{
	ConfigurerFixture fixture(TrackingServiceWithoutDevicePtr(new TrackingServiceWithoutDevice(cx::Tool::tsCONFIGURED, true)));

	fixture.mConfigurer->start();
	CHECK(fixture.mToolConfiguredCount == 1);
	CHECK(fixture.mTracking->mRequestedStates.empty());
}

TEST_CASE("TrackingToolConfigurer: a tracking system configured without the tool is configured again", "[unit][plugins][org.custusx.fraxinus.widgets]")
{
	ConfigurerFixture fixture(TrackingServiceWithoutDevicePtr(new TrackingServiceWithoutDevice(cx::Tool::tsCONFIGURED, false)));

	fixture.mConfigurer->start();
	CHECK(fixture.mTracking->mRequestedStates == std::vector<cx::Tool::State>({cx::Tool::tsNONE, cx::Tool::tsCONFIGURED}));
	CHECK(fixture.mToolConfiguredCount == 0);

	SECTION("with the tool")
	{
		fixture.mTracking->finishConfiguring(true);
		CHECK(fixture.mToolConfiguredCount == 1);
	}
	SECTION("without the tool: not reported, and no longer waited for")
	{
		fixture.mTracking->finishConfiguring(false);
		CHECK(fixture.mToolConfiguredCount == 0);
		fixture.mTracking->finishConfiguring(true);
		CHECK(fixture.mToolConfiguredCount == 0);
	}
}

TEST_CASE("TrackingToolConfigurer: the tool is not reported after stopping", "[unit][plugins][org.custusx.fraxinus.widgets]")
{
	ConfigurerFixture fixture(TrackingServiceWithoutDevicePtr(new TrackingServiceWithoutDevice()));

	fixture.mConfigurer->start();
	fixture.mConfigurer->stop();
	fixture.mTracking->finishConfiguring(true);
	CHECK(fixture.mToolConfiguredCount == 0);
}

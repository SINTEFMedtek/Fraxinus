/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxScopeExposure.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "cxBranch.h"
#include "cxBranchList.h"
#include "cxTool.h"
#include "cxTrackingService.h"
#include "cxSpaceProvider.h"
#include "cxCoordinateSystemHelpers.h"
#include "cxViewGroupData.h"

namespace cx
{

double ScopeExposure::exposureFromRadius(double radius)
{
	double exposure = 1.0;
	if (radius > 0)
	{
		exposure = std::pow(radius / referenceRadius, radiusExponent);
		exposure = std::max(minExposure, std::min(maxExposure, exposure));
	}
	return exposure;
}

void ScopeExposure::setAirways(const std::vector<Vector3D>& positions, const std::vector<double>& radius)
{
	mPositions.clear();
	mRadius.clear();
	size_t count = std::min(positions.size(), radius.size());
	for (size_t i = 0; i < count; ++i)
	{
		if (radius[i] > 0)
		{
			mPositions.push_back(positions[i]);
			mRadius.push_back(radius[i]);
		}
	}
}

void ScopeExposure::setAirways(BranchListPtr branches)
{
	std::vector<Vector3D> positions;
	std::vector<double> radius;
	if (branches)
	{
		for (BranchPtr branch : branches->getBranches())
		{
			Eigen::MatrixXd branchPositions = branch->getPositions();
			Eigen::VectorXd measuredRadius = branch->getRadius();
			double radiusOfBranch = branchRadius(std::vector<double>(measuredRadius.data(), measuredRadius.data() + measuredRadius.size()));
			for (int i = 0; i < branchPositions.cols(); ++i)
			{
				positions.push_back(Vector3D(branchPositions.col(i)));
				radius.push_back(radiusOfBranch);
			}
		}
	}
	this->setAirways(positions, radius);
}

double ScopeExposure::branchRadius(const std::vector<double>& measuredRadius)
{
	std::vector<double> valid;
	for (double radius : measuredRadius)
	{
		if (radius > 0)
		{
			valid.push_back(radius);
		}
	}
	double median = 0;
	if (!valid.empty())
	{
		size_t middle = valid.size() / 2;
		std::sort(valid.begin(), valid.end());
		median = (valid.size() % 2 == 1) ? valid[middle] : (valid[middle-1] + valid[middle]) / 2;
	}
	return median;
}

bool ScopeExposure::hasAirways() const
{
	return !mPositions.empty();
}

double ScopeExposure::getRadius(const Vector3D& position) const
{
	std::vector<double> nearbyRadius;
	double closestDistance = std::numeric_limits<double>::max();
	double closestRadius = 0;
	for (size_t i = 0; i < mPositions.size(); ++i)
	{
		double distance = (mPositions[i] - position).norm();
		if (distance <= averagingDistance)
		{
			nearbyRadius.push_back(mRadius[i]);
		}
		if (distance < closestDistance)
		{
			closestDistance = distance;
			closestRadius = mRadius[i];
		}
	}
	double radius = 0; // Outside the airways, e.g. before registration
	if (!nearbyRadius.empty())
	{
		radius = interquartileMean(nearbyRadius);
	}
	else if (closestDistance <= closestRadius)
	{
		radius = closestRadius; // Inside a wide airway, away from its centerline
	}
	return radius;
}

double ScopeExposure::interquartileMean(std::vector<double> values)
{
	double mean = 0;
	if (!values.empty())
	{
		std::sort(values.begin(), values.end());
		size_t quarter = values.size() / 4;
		double sum = 0;
		for (size_t i = quarter; i < values.size() - quarter; ++i)
		{
			sum += values[i];
		}
		mean = sum / (values.size() - 2*quarter);
	}
	return mean;
}

double ScopeExposure::getExposure(const Vector3D& position) const
{
	return exposureFromRadius(this->getRadius(position));
}

// --------------------------------------------------------

ScopeExposureController::ScopeExposureController(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider, ViewGroupDataPtr viewGroup, const ScopeExposure& exposure) :
	mTracking(tracking),
	mSpaceProvider(spaceProvider),
	mViewGroup(viewGroup),
	mExposure(exposure)
{
	if (mTracking)
	{
		connect(mTracking.get(), &TrackingService::activeToolChanged, this, &ScopeExposureController::activeToolChangedSlot);
	}
	this->activeToolChangedSlot();
}

ScopeExposureController::~ScopeExposureController()
{
	if (mViewGroup)
	{
		mViewGroup->setScopeExposure(1.0);
	}
}

void ScopeExposureController::activeToolChangedSlot()
{
	if (mTool)
	{
		disconnect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &ScopeExposureController::update);
	}
	mTool = mTracking ? mTracking->getActiveTool() : ToolPtr();
	if (mTool)
	{
		connect(mTool.get(), &Tool::toolTransformAndTimestamp, this, &ScopeExposureController::update);
	}
	this->update();
}

void ScopeExposureController::update()
{
	double exposure = 1.0;
	if (mTool && mSpaceProvider)
	{
		Vector3D toolTip_r = mSpaceProvider->getActiveToolTipPoint(CoordinateSystem::reference(), true);
		exposure = mExposure.getExposure(toolTip_r);
	}
	if (mViewGroup)
	{
		mViewGroup->setScopeExposure(exposure);
	}
}

} // namespace cx

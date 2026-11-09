/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "cxRouteGuidance.h"
#include <algorithm>
#include <limits>

namespace cx
{

void RouteGuidance::setRoute(const std::vector<Vector3D>& positions, const std::vector<int>& generationNumbers, const std::vector<double>& radius)
{
	// Generation numbers and radius may be shorter than the positions (no values for via points);
	// they are aligned from the start of the route.
	mPositions = positions;
	mRadius = (radius.size() <= positions.size()) ? radius : std::vector<double>();

	mDistances.assign(mPositions.size(), 0.0);
	for (size_t i = 1; i < mPositions.size(); ++i)
		mDistances[i] = mDistances[i-1] + (mPositions[i] - mPositions[i-1]).norm();

	mBranchingIndices.clear();
	mBranchingPoints.clear();
	if (mPositions.empty() || generationNumbers.size() > mPositions.size())
		return;
	for (size_t i = 1; i < generationNumbers.size(); ++i)
		if (generationNumbers[i] > generationNumbers[i-1])
			mBranchingIndices.push_back(static_cast<int>(i));

	double previousBranchingDistance = 0;
	double previousHideFrom = 0;
	for (int index : mBranchingIndices)
	{
		double branchingDistance = mDistances[index-1];
		double branchLength = branchingDistance - previousBranchingDistance;

		BranchingPoint point;
		point.arrow = this->createArrow(index, previousBranchingDistance);
		point.showFrom = branchingDistance - std::max(minShowDistance, showFractionOfBranch * branchLength);
		double tailDistance = branchingDistance - tailFraction * point.arrow.size;
		// Hide after the previous arrow is hidden, so that each arrow is shown for a while (at least half its branch).
		point.hideFrom = std::max(tailDistance - std::max(minHideDistance, point.arrow.size), previousHideFrom + 0.5 * branchLength);
		mBranchingPoints.push_back(point);

		previousBranchingDistance = branchingDistance;
		previousHideFrom = point.hideFrom;
	}
}

std::vector<int> RouteGuidance::getBranchingIndices() const
{
	return mBranchingIndices;
}

RouteGuidanceArrow RouteGuidance::getArrow(const Vector3D& position) const
{
	if (mBranchingPoints.empty())
		return RouteGuidanceArrow();

	int closest = this->findClosestIndex(position);
	if ((mPositions[closest] - position).norm() > maxDistanceFromRoute + this->getRadius(closest))
		return RouteGuidanceArrow();
	double distanceAlongRoute = mDistances[closest];

	// The first arrow not yet hidden is the next one. Arrows are hidden in route order,
	// and each one after it has appeared.
	for (const BranchingPoint& point : mBranchingPoints)
	{
		if (distanceAlongRoute >= point.hideFrom)
			continue;
		if (distanceAlongRoute < point.showFrom)
			return RouteGuidanceArrow();
		return point.arrow;
	}
	return RouteGuidanceArrow();
}

RouteGuidanceArrow RouteGuidance::createArrow(int branchingIndex, double previousBranchingDistance) const
{
	RouteGuidanceArrow arrow;
	double size = this->getArrowSize(branchingIndex);
	double branchingDistance = mDistances[branchingIndex-1];
	double tailDistance = std::max(branchingDistance - tailFraction * size, previousBranchingDistance);

	arrow.branchingIndex = branchingIndex;
	arrow.tail = this->pointAtDistance(tailDistance);
	arrow.corner = mPositions[branchingIndex-1];
	arrow.tip = this->pointAtDistance(branchingDistance + size);
	arrow.size = size;
	arrow.visible = (arrow.tip - arrow.corner).norm() > 1e-6;
	return arrow;
}

int RouteGuidance::findClosestIndex(const Vector3D& position) const
{
	int closest = 0;
	double minDistance = std::numeric_limits<double>::max();
	for (size_t i = 0; i < mPositions.size(); ++i)
	{
		double distance = (mPositions[i] - position).squaredNorm();
		if (distance < minDistance)
		{
			minDistance = distance;
			closest = static_cast<int>(i);
		}
	}
	return closest;
}

Vector3D RouteGuidance::pointAtDistance(double distance) const
{
	if (distance <= 0)
		return mPositions.front();
	if (distance >= mDistances.back())
		return mPositions.back();

	size_t upper = std::upper_bound(mDistances.begin(), mDistances.end(), distance) - mDistances.begin();
	size_t lower = upper - 1;
	double segmentLength = mDistances[upper] - mDistances[lower];
	double t = (segmentLength > 0) ? (distance - mDistances[lower]) / segmentLength : 0;
	return mPositions[lower] + t * (mPositions[upper] - mPositions[lower]);
}

double RouteGuidance::getRadius(int index) const
{
	return (index >= 0 && index < static_cast<int>(mRadius.size())) ? mRadius[index] : 0;
}

double RouteGuidance::getArrowSize(int index) const
{
	return std::min(std::max(2 * this->getRadius(index), minArrowSize), maxArrowSize);
}

} // namespace cx

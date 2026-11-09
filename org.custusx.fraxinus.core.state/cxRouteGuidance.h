/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXROUTEGUIDANCE_H
#define CXROUTEGUIDANCE_H

#include "org_custusx_fraxinus_core_state_Export.h"
#include <vector>
#include "cxVector3D.h"

namespace cx
{

/**
 * The bent turn arrow for the next branching point: from the tail in the parent branch,
 * bending at the branching point, to the tip in the branch to take. All positions in r space.
 */
struct org_custusx_fraxinus_core_state_EXPORT RouteGuidanceArrow
{
	bool visible = false;
	int branchingIndex = -1; ///< Route index of the first position in the branch to take
	Vector3D tail = Vector3D::Zero();
	Vector3D corner = Vector3D::Zero(); ///< The branching point
	Vector3D tip = Vector3D::Zero();
	double size = 0; ///< mm; arrow length in the branch to take, sets the arrow thickness
};

/**
 * GPS-like guidance along a route to target: finds the branching points on the route
 * and gives an arrow into the branch to take when approaching the next one.
 *
 * The route goes from the trachea towards the target. A branching point is where the
 * airway generation number increases along the route.
 */
class org_custusx_fraxinus_core_state_EXPORT RouteGuidance
{
public:
	static constexpr double minShowDistance = 30.0; ///< mm; an arrow is always shown at least this far before its branching point
	static constexpr double showFractionOfBranch = 2.0/3.0; ///< On long branches, show the arrow over this part of the branch leading to it
	static constexpr double minHideDistance = 3.0; ///< mm; hide the arrow at least this far before its tail (otherwise one arrow size)
	static constexpr double maxDistanceFromRoute = 10.0; ///< mm outside the airway radius; no arrow when further away from the route
	static constexpr double minArrowSize = 2.0; ///< mm
	static constexpr double maxArrowSize = 8.0; ///< mm
	static constexpr double tailFraction = 1.0; ///< Length of the arrow part in the parent branch, relative to the arrow size

	/** Generation numbers and radius are aligned with positions from the route start, and may be shorter (e.g. via points). */
	void setRoute(const std::vector<Vector3D>& positions, const std::vector<int>& generationNumbers, const std::vector<double>& radius);
	std::vector<int> getBranchingIndices() const;
	RouteGuidanceArrow getArrow(const Vector3D& position) const;

private:
	struct BranchingPoint
	{
		double showFrom; ///< Distance along the route where the arrow appears
		double hideFrom; ///< Distance along the route where the arrow disappears
		RouteGuidanceArrow arrow;
	};

	int findClosestIndex(const Vector3D& position) const;
	Vector3D pointAtDistance(double distance) const;
	double getRadius(int index) const;
	double getArrowSize(int index) const;
	RouteGuidanceArrow createArrow(int branchingIndex, double previousBranchingDistance) const;

	std::vector<Vector3D> mPositions;
	std::vector<double> mDistances; ///< Distance along the route from its start, per position
	std::vector<double> mRadius;
	std::vector<int> mBranchingIndices; ///< First position in each new branch along the route
	std::vector<BranchingPoint> mBranchingPoints;
};

} // namespace cx

#endif // CXROUTEGUIDANCE_H

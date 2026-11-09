/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXSCOPEEXPOSURE_H
#define CXSCOPEEXPOSURE_H

#include "org_custusx_fraxinus_core_state_Export.h"
#include <memory>
#include <vector>
#include <QObject>
#include "cxVector3D.h"

namespace cx
{
typedef std::shared_ptr<class BranchList> BranchListPtr;
typedef std::shared_ptr<class TrackingService> TrackingServicePtr;
typedef std::shared_ptr<class SpaceProvider> SpaceProviderPtr;
typedef std::shared_ptr<class Tool> ToolPtr;
typedef std::shared_ptr<class ViewGroupData> ViewGroupDataPtr;

/**
 * Automatic exposure for the virtual bronchoscopy, imitating the automatic brightness
 * control of a bronchoscope: brighter light in wide airways, where the walls are far
 * from the scope, and dimmer light in narrow airways, where the walls are close.
 *
 * The exposure is a factor on the scope light intensity, found from the airway radius
 * at the scope tip.
 *
 * Each branch gets one radius: the median of the radius measured along it. The measured
 * radius varies a lot in the periphery (it is measured in whole voxels), and is too large
 * near the bifurcations, where the measurement reaches into the junction or the other
 * branch. Using the median per branch avoids that the light turns up and down again
 * when passing a bifurcation.
 *
 * The radius at the tip is the interquartile mean of the centerline points close to
 * it, so that it changes gradually from one branch to the next.
 * All positions in r space.
 */
class org_custusx_fraxinus_core_state_EXPORT ScopeExposure
{
public:
	static constexpr double referenceRadius = 4.0; ///< mm; airway radius with exposure 1
	static constexpr double radiusExponent = 0.5; ///< exposure = (radius/referenceRadius)^radiusExponent
	static constexpr double minExposure = 0.6;
	static constexpr double maxExposure = 1.6;
	static constexpr double averagingDistance = 5.0; ///< mm; use the radius of the centerline points this close to the tip

	static double exposureFromRadius(double radius);
	static double interquartileMean(std::vector<double> values); ///< Mean without the lowest and highest quarter; 0 when empty

	static double branchRadius(const std::vector<double>& measuredRadius); ///< Median of the valid (>0) values; 0 when none

	/** Centerline positions with their airway radius. Positions without a radius (<=0) are ignored. */
	void setAirways(const std::vector<Vector3D>& positions, const std::vector<double>& radius);
	void setAirways(BranchListPtr branches); ///< One radius per branch, see branchRadius()
	double getRadius(const Vector3D& position) const; ///< 0 when there are no airways
	double getExposure(const Vector3D& position) const; ///< 1 when there are no airways

private:
	std::vector<Vector3D> mPositions;
	std::vector<double> mRadius;
};

/**
 * Updates the scope exposure of a view group from the active tool position,
 * and resets it to 1 when deleted.
 */
class org_custusx_fraxinus_core_state_EXPORT ScopeExposureController : public QObject
{
	Q_OBJECT
public:
	ScopeExposureController(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider, ViewGroupDataPtr viewGroup, const ScopeExposure& exposure);
	~ScopeExposureController();

private:
	void activeToolChangedSlot();
	void update();

	TrackingServicePtr mTracking;
	SpaceProviderPtr mSpaceProvider;
	ViewGroupDataPtr mViewGroup;
	ScopeExposure mExposure;
	ToolPtr mTool;
};
typedef std::shared_ptr<ScopeExposureController> ScopeExposureControllerPtr;

} // namespace cx

#endif // CXSCOPEEXPOSURE_H

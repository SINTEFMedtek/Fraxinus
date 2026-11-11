/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#ifndef CXROUTEGUIDANCEREP_H
#define CXROUTEGUIDANCEREP_H

#include "org_custusx_fraxinus_core_state_Export.h"
#include "cxRepImpl.h"
#include "cxRouteGuidance.h"
#include "vtkForwardDeclarations.h"

namespace cx
{
typedef std::shared_ptr<class TrackingService> TrackingServicePtr;
typedef std::shared_ptr<class SpaceProvider> SpaceProviderPtr;
typedef std::shared_ptr<class Tool> ToolPtr;
typedef std::shared_ptr<class RouteGuidanceRep> RouteGuidanceRepPtr;

/** Shows a bent 3D arrow into the branch to take at the next branching point ahead of the active tool. */
class org_custusx_fraxinus_core_state_EXPORT RouteGuidanceRep : public RepImpl
{
	Q_OBJECT
public:
	static RouteGuidanceRepPtr New(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider, const QString& uid = "");
	virtual ~RouteGuidanceRep();
	virtual QString getType() const { return "RouteGuidanceRep"; }

	/** Route from the trachea towards the target, in r space. */
	void setRoute(const std::vector<Vector3D>& positions, const std::vector<int>& generationNumbers, const std::vector<double>& radius);

protected:
	RouteGuidanceRep(TrackingServicePtr tracking, SpaceProviderPtr spaceProvider);
	virtual void addRepActorsToViewRenderer(ViewPtr view);
	virtual void removeRepActorsFromViewRenderer(ViewPtr view);
	virtual void onModifiedStartRender();

private slots:
	void activeToolChangedSlot();

private:
	void updateArrow();
	void updateArrowGeometry(const RouteGuidanceArrow& arrow);

	TrackingServicePtr mTracking;
	SpaceProviderPtr mSpaceProvider;
	ToolPtr mTool;
	RouteGuidance mGuidance;
	vtkActorPtr mActor;
	vtkPolyDataMapperPtr mMapper;
	int mShownBranchingIndex = -1;
};

} // namespace cx

#endif // CXROUTEGUIDANCEREP_H

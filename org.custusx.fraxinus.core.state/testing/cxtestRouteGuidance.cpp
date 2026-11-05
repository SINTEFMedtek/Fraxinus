/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "catch.hpp"
#include "cxRouteGuidance.h"

namespace
{
using cx::Vector3D;

// 1 mm steps: 50 mm along +z (generation 0), 20 mm along +x (generation 1), 20 mm along +y (generation 2).
struct TestRoute
{
	std::vector<Vector3D> positions;
	std::vector<int> generations;
	std::vector<double> radius;

	TestRoute()
	{
		for (int i = 0; i <= 50; ++i)
			add(Vector3D(0, 0, i), 0, 5.0);
		for (int i = 1; i <= 20; ++i)
			add(Vector3D(i, 0, 50), 1, 0.5);
		for (int i = 1; i <= 20; ++i)
			add(Vector3D(20, i, 50), 2, 0.5);
	}
	void add(Vector3D position, int generation, double r)
	{
		positions.push_back(position);
		generations.push_back(generation);
		radius.push_back(r);
	}
};

cx::RouteGuidance createGuidance()
{
	TestRoute route;
	cx::RouteGuidance guidance;
	guidance.setRoute(route.positions, route.generations, route.radius);
	return guidance;
}
} // namespace

TEST_CASE("RouteGuidance: Branching points are where the generation increases", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	std::vector<int> branchingIndices = guidance.getBranchingIndices();
	REQUIRE(branchingIndices.size() == 2);
	CHECK(branchingIndices[0] == 51);
	CHECK(branchingIndices[1] == 71);
}

TEST_CASE("RouteGuidance: Via points at the end of the route have no generation numbers", "[unit][fraxinus]")
{
	// As from RouteToTarget with extra airway points: positions after the airway part have no generation number or radius.
	TestRoute route;
	for (int i = 1; i <= 10; ++i)
		route.positions.push_back(Vector3D(20, 20, 50 + i));
	cx::RouteGuidance guidance;
	guidance.setRoute(route.positions, route.generations, route.radius);

	std::vector<int> branchingIndices = guidance.getBranchingIndices();
	REQUIRE(branchingIndices.size() == 2);
	CHECK(branchingIndices[0] == 51);
	CHECK(branchingIndices[1] == 71);
	CHECK(guidance.getArrow(Vector3D(0, 0, 40)).visible);
}

TEST_CASE("RouteGuidance: No branching points with more generation numbers than positions", "[unit][fraxinus]")
{
	TestRoute route;
	route.positions.pop_back();
	cx::RouteGuidance guidance;
	guidance.setRoute(route.positions, route.generations, route.radius);
	CHECK(guidance.getBranchingIndices().empty());
	CHECK_FALSE(guidance.getArrow(Vector3D(0, 0, 40)).visible);
}

TEST_CASE("RouteGuidance: Allowed distance from the route grows with the airway radius", "[unit][fraxinus]")
{
	TestRoute route;
	for (double& r : route.radius)
		r = 9.0; // Trachea-like
	cx::RouteGuidance guidance;
	guidance.setRoute(route.positions, route.generations, route.radius);
	CHECK(guidance.getArrow(Vector3D(0, 15, 30)).visible); // 15 mm off the centerline, within radius + margin
	CHECK_FALSE(guidance.getArrow(Vector3D(0, 25, 30)).visible);
}

TEST_CASE("RouteGuidance: Arrow is hidden far before the next branching point", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	CHECK_FALSE(guidance.getArrow(Vector3D(0, 0, 5)).visible);
}

TEST_CASE("RouteGuidance: Arrow bends from the parent into the branch to take", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();

	cx::RouteGuidanceArrow arrow = guidance.getArrow(Vector3D(0, 0, 40));
	REQUIRE(arrow.visible);
	CHECK(arrow.branchingIndex == 51);
	CHECK(arrow.corner.isApprox(Vector3D(0, 0, 50)));
	CHECK(arrow.tail.isApprox(Vector3D(0, 0, 48))); // One arrow size back in the parent branch
	CHECK(arrow.tip.isApprox(Vector3D(2, 0, 50)));
	CHECK(arrow.size == Approx(cx::RouteGuidance::minArrowSize)); // Diameter 1 mm is below the minimum

	arrow = guidance.getArrow(Vector3D(10, 0, 50));
	REQUIRE(arrow.visible);
	CHECK(arrow.branchingIndex == 71);
	CHECK(arrow.corner.isApprox(Vector3D(20, 0, 50)));
	CHECK(arrow.tail.isApprox(Vector3D(18, 0, 50)));
	CHECK(arrow.tip.isApprox(Vector3D(20, 2, 50)));
}

TEST_CASE("RouteGuidance: Arrow appears earlier on a long branch", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	// 32 mm before the first branching point: beyond the minimum show distance, but within 2/3 of the 50 mm branch.
	CHECK(guidance.getArrow(Vector3D(0, 0, 18)).visible);
}

TEST_CASE("RouteGuidance: Arrow is hidden before the camera reaches it", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	cx::RouteGuidanceArrow arrow = guidance.getArrow(Vector3D(0, 0, 46)); // 2 mm before the arrow tail
	CHECK_FALSE((arrow.visible && arrow.branchingIndex == 51));
}

TEST_CASE("RouteGuidance: Every arrow is shown, also on short branches", "[unit][fraxinus]")
{
	// 0.5 mm steps: 50 mm trachea, then six 2 mm branches, then a 20 mm branch. Narrow and wide branches
	// alternate, so a short arrow is followed by an arrow longer than the branch in between.
	std::vector<Vector3D> positions;
	std::vector<int> generations;
	Vector3D position(0, 0, 0);
	auto addBranch = [&](Vector3D direction, double length, int generation)
	{
		for (double travelled = 0.5; travelled <= length + 1e-9; travelled += 0.5)
		{
			position += 0.5 * direction;
			positions.push_back(position);
			generations.push_back(generation);
		}
	};
	positions.push_back(position);
	generations.push_back(0);
	addBranch(Vector3D(0, 0, 1), 50, 0);
	for (int generation = 1; generation <= 6; ++generation)
		addBranch((generation % 2) ? Vector3D(1, 0, 0) : Vector3D(0, 1, 0), 2, generation);
	addBranch(Vector3D(0, 0, 1), 20, 7);
	std::vector<double> radius;
	for (int generation : generations)
		radius.push_back((generation % 2) ? 1.0 : 10.0);

	cx::RouteGuidance guidance;
	guidance.setRoute(positions, generations, radius);
	std::vector<int> branchingIndices = guidance.getBranchingIndices();
	REQUIRE(branchingIndices.size() == 7);

	std::vector<bool> shown(branchingIndices.size(), false);
	for (const Vector3D& cameraPosition : positions)
	{
		cx::RouteGuidanceArrow arrow = guidance.getArrow(cameraPosition);
		for (size_t i = 0; i < branchingIndices.size(); ++i)
			if (arrow.visible && arrow.branchingIndex == branchingIndices[i])
				shown[i] = true;
	}
	for (size_t i = 0; i < shown.size(); ++i)
	{
		INFO("Branching point " << i);
		CHECK(shown[i]);
	}
}

TEST_CASE("RouteGuidance: Arrow is hidden after the last branching point", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	CHECK_FALSE(guidance.getArrow(Vector3D(20, 10, 50)).visible);
}

TEST_CASE("RouteGuidance: Arrow is hidden far away from the route", "[unit][fraxinus]")
{
	cx::RouteGuidance guidance = createGuidance();
	CHECK_FALSE(guidance.getArrow(Vector3D(0, 30, 40)).visible);
}

TEST_CASE("RouteGuidance: Arrow size follows the airway diameter within limits", "[unit][fraxinus]")
{
	TestRoute route;
	for (double& r : route.radius)
		r = 2.0;
	cx::RouteGuidance guidance;
	guidance.setRoute(route.positions, route.generations, route.radius);
	CHECK(guidance.getArrow(Vector3D(0, 0, 30)).size == Approx(4.0));

	for (double& r : route.radius)
		r = 20.0;
	guidance.setRoute(route.positions, route.generations, route.radius);
	CHECK(guidance.getArrow(Vector3D(0, 0, 30)).size == Approx(cx::RouteGuidance::maxArrowSize));
}

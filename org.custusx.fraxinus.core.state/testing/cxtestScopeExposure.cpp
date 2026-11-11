/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "catch.hpp"
#include "cxScopeExposure.h"

namespace
{
using cx::Vector3D;
using cx::ScopeExposure;

// 1 mm steps: a wide airway along +z (radius 9 mm), then a narrow one along +x (radius 1.5 mm).
ScopeExposure createTestAirways()
{
	std::vector<Vector3D> positions;
	std::vector<double> radius;
	for (int i = 0; i <= 50; ++i)
	{
		positions.push_back(Vector3D(0, 0, i));
		radius.push_back(9.0);
	}
	for (int i = 1; i <= 50; ++i)
	{
		positions.push_back(Vector3D(i, 0, 50));
		radius.push_back(1.5);
	}
	ScopeExposure exposure;
	exposure.setAirways(positions, radius);
	return exposure;
}
} // namespace

TEST_CASE("ScopeExposure: Exposure is 1 at the reference radius", "[fraxinus][unit]")
{
	CHECK(ScopeExposure::exposureFromRadius(ScopeExposure::referenceRadius) == Approx(1.0));
}

TEST_CASE("ScopeExposure: Wide airways get more light than narrow airways", "[fraxinus][unit]")
{
	CHECK(ScopeExposure::exposureFromRadius(8.0) > 1.0);
	CHECK(ScopeExposure::exposureFromRadius(2.0) < 1.0);
}

TEST_CASE("ScopeExposure: Exposure is kept within limits", "[fraxinus][unit]")
{
	CHECK(ScopeExposure::exposureFromRadius(100.0) == Approx(ScopeExposure::maxExposure));
	CHECK(ScopeExposure::exposureFromRadius(0.1) == Approx(ScopeExposure::minExposure));
}

TEST_CASE("ScopeExposure: Exposure is 1 without airways", "[fraxinus][unit]")
{
	ScopeExposure exposure;
	CHECK_FALSE(exposure.hasAirways());
	CHECK(exposure.getExposure(Vector3D(0, 0, 0)) == Approx(1.0));
	CHECK(ScopeExposure::exposureFromRadius(0) == Approx(1.0));
}

TEST_CASE("ScopeExposure: Has no airways when no position has a radius", "[fraxinus][unit]")
{
	ScopeExposure exposure;
	exposure.setAirways({Vector3D(0, 0, 0)}, {0.0});
	CHECK_FALSE(exposure.hasAirways());
	exposure.setAirways({Vector3D(0, 0, 0)}, {6.0});
	CHECK(exposure.hasAirways());
}

TEST_CASE("ScopeExposure: Radius is found from the closest airway", "[fraxinus][unit]")
{
	ScopeExposure exposure = createTestAirways();
	CHECK(exposure.getRadius(Vector3D(0, 0, 10)) == Approx(9.0));
	CHECK(exposure.getRadius(Vector3D(30, 0, 50)) == Approx(1.5));
	CHECK(exposure.getRadius(Vector3D(2, 3, 10)) == Approx(9.0)); // off the centerline
	CHECK(exposure.getRadius(Vector3D(8, 0, 10)) == Approx(9.0)); // inside the wide airway, away from the centerline
}

TEST_CASE("ScopeExposure: Exposure is 1 outside the airways", "[fraxinus][unit]")
{
	ScopeExposure exposure = createTestAirways();
	CHECK(exposure.getRadius(Vector3D(30, 20, 50)) == Approx(0.0));
	CHECK(exposure.getExposure(Vector3D(30, 20, 50)) == Approx(1.0));
	CHECK(exposure.getRadius(Vector3D(0, 0, -100)) == Approx(0.0));
}

TEST_CASE("ScopeExposure: Radius changes gradually at a branching point", "[fraxinus][unit]")
{
	ScopeExposure exposure = createTestAirways();
	double before = exposure.getRadius(Vector3D(0, 0, 47));
	double at = exposure.getRadius(Vector3D(0, 0, 50));
	double after = exposure.getRadius(Vector3D(3, 0, 50));
	CHECK(before < 9.0);
	CHECK(at < before);
	CHECK(after < at);
	CHECK(after > 1.5);
}

TEST_CASE("ScopeExposure: Positions without a radius are ignored", "[fraxinus][unit]")
{
	ScopeExposure exposure;
	exposure.setAirways({Vector3D(0, 0, 0), Vector3D(0, 0, 1)}, {0.0, 6.0});
	CHECK(exposure.getRadius(Vector3D(0, 0, 0)) == Approx(6.0));
}

TEST_CASE("ScopeExposure: Interquartile mean ignores the lowest and highest quarter", "[fraxinus][unit]")
{
	CHECK(ScopeExposure::interquartileMean({3.0, 1.0, 2.0}) == Approx(2.0));
	CHECK(ScopeExposure::interquartileMean({4.0, 1.0, 3.0, 2.0}) == Approx(2.5));
	CHECK(ScopeExposure::interquartileMean({100.0, 1.0, 2.0, 3.0, 0.1, 2.0, 2.0, 2.0}) == Approx(2.0));
	CHECK(ScopeExposure::interquartileMean({}) == Approx(0.0));
}

TEST_CASE("ScopeExposure: A single bad radius reading is ignored", "[fraxinus][unit]")
{
	std::vector<Vector3D> positions;
	std::vector<double> radius;
	for (int i = 0; i <= 20; ++i)
	{
		positions.push_back(Vector3D(0, 0, i));
		radius.push_back(i == 10 ? 15.0 : 1.5);
	}
	ScopeExposure exposure;
	exposure.setAirways(positions, radius);
	CHECK(exposure.getRadius(Vector3D(0, 0, 10)) == Approx(1.5));
}

TEST_CASE("ScopeExposure: Branch radius ignores too large readings at the bifurcations", "[fraxinus][unit]")
{
	// Readings near the branch start and end reach into the junctions
	std::vector<double> measured = {6.0, 4.5, 2.0, 2.1, 1.9, 2.0, 0.0, 2.2, 1.8, 3.5, 7.0};
	CHECK(ScopeExposure::branchRadius(measured) == Approx(2.15));
}

TEST_CASE("ScopeExposure: Branch radius is 0 without valid readings", "[fraxinus][unit]")
{
	CHECK(ScopeExposure::branchRadius({}) == Approx(0.0));
	CHECK(ScopeExposure::branchRadius({0.0, 0.0}) == Approx(0.0));
}

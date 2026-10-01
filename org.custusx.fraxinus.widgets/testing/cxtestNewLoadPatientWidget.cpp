/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) SINTEF Department of Medical Technology.
All rights reserved.

CustusX is released under a BSD 3-Clause license.

See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
=========================================================================*/

#include "catch.hpp"

#include <vector>
#include <QCheckBox>
#include <QWidget>
#include "cxNewLoadPatientWidget.h"

TEST_CASE("NewLoadPatientWidget: Select all is checked only when every enabled checkbox is checked", "[unit][plugins][org.custusx.fraxinus.widgets]")
{
	QWidget parent;
	QCheckBox* first = new QCheckBox(&parent);
	QCheckBox* second = new QCheckBox(&parent);
	const std::vector<QCheckBox*> checkBoxes = {first, second};

	SECTION("All enabled checkboxes checked")
	{
		first->setChecked(true);
		second->setChecked(true);
		CHECK(cx::NewLoadPatientWidget::allEnabledChecked(checkBoxes));
	}
	SECTION("An enabled checkbox unchecked")
	{
		first->setChecked(true);
		CHECK_FALSE(cx::NewLoadPatientWidget::allEnabledChecked(checkBoxes));
	}
	SECTION("Unchecked disabled checkbox is ignored")
	{
		first->setChecked(true);
		second->setDisabled(true);
		CHECK(cx::NewLoadPatientWidget::allEnabledChecked(checkBoxes));
	}
	SECTION("No enabled checkboxes")
	{
		first->setDisabled(true);
		second->setDisabled(true);
		CHECK_FALSE(cx::NewLoadPatientWidget::allEnabledChecked(checkBoxes));
	}
}

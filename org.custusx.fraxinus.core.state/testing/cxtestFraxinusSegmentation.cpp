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


TEST_CASE("FraxinusSegmentations: getFilterScriptsPath", "[unit]")
{
	cx::FraxinusSegmentations segmentations(cx::CoreServices::getNullObjects());
	QString scriptFilePath = segmentations.getFilterScriptsPath();
	CHECK(QFileInfo(scriptFilePath).exists());
	
	QString iniFilePath = scriptFilePath + "python_Lungs_test.ini";
	CHECK(QFileInfo(iniFilePath).exists());
}

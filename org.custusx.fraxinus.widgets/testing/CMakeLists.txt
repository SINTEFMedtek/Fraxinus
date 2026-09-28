# =========================================================================
# This file is part of CustusX, an Image Guided Therapy Application.
#
# Copyright (c) SINTEF Department of Medical Technology.
# All rights reserved.
#
# CustusX is released under a BSD 3-Clause license.
#
# See Lisence.txt (https://github.com/SINTEFMedtek/CustusX/blob/master/License.txt) for details.
# =========================================================================

if(BUILD_TESTING)

    set(CXTEST_SOURCES_TO_MOC
    )
    set(CXTEST_PLUGINWIDGETS_SOURCES
        cxtestNewLoadPatientWidget.cpp
        cxtestExportDummyClassForLinkingOnWindowsInLibWithoutExportedClass.cpp
    )

    qt5_wrap_cpp(CXTEST_SOURCES_TO_MOC ${CXTEST_SOURCES_TO_MOC})
    add_library(cxtest_org_custusx_fraxinus_widgets ${CXTEST_PLUGINWIDGETS_SOURCES} ${CXTEST_SOURCES_TO_MOC})
    include(GenerateExportHeader)
    generate_export_header(cxtest_org_custusx_fraxinus_widgets)
    target_include_directories(cxtest_org_custusx_fraxinus_widgets
        PUBLIC
        .
        ${CMAKE_CURRENT_BINARY_DIR}
    )
    target_link_libraries(cxtest_org_custusx_fraxinus_widgets
        PRIVATE
        org_custusx_fraxinus_widgets
        cxCatch
        cxtestUtilities
    )
    cx_add_tests_to_catch(cxtest_org_custusx_fraxinus_widgets)

endif(BUILD_TESTING)

project(org_custusx_fraxinus)

set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)

###############################################################################
#
# Define variables used by various installer generators to customize
# the installer look&feel.
#
###############################################################################
macro(cxsetup_install_decorate_generators)

        set(CX_LICENSE_FILE "${SOURCE_DIR}/doc/License.txt")
        set_property(GLOBAL PROPERTY CX_LICENSE_FILE "${CX_LICENSE_FILE}")

        set_property(GLOBAL PROPERTY CPACK_RESOURCE_FILE_WELCOME "${SOURCE_DIR}/doc/install_text/install_welcome.txt")
        set_property(GLOBAL PROPERTY CPACK_RESOURCE_FILE_README "${SOURCE_DIR}/doc/install_text/install_readme.rtf")
        set_property(GLOBAL PROPERTY CPACK_RESOURCE_FILE_LICENSE "${CX_LICENSE_FILE}")
endmacro()

###############################################################################
#
# Install all configuration files
#
###############################################################################
function(cxsetup_install_configuration_files)
    install(DIRECTORY ${SOURCE_DIR}/config/
            DESTINATION ./${CX_INSTALL_CONFIG_DIR}
            FILE_PERMISSIONS ${CX_FULL_PERMISSIONS}
            DIRECTORY_PERMISSIONS ${CX_FULL_PERMISSIONS}
            )
endfunction()

###############################################################################
#
# Install all documentation files
#
###############################################################################
function(cxsetup_install_documentation_files)
    set(DOC_FOLDER "${SOURCE_DIR}/doc/user_manual")
    cx_doc_define_user_docs("${DOC_FOLDER}")
    set_property(GLOBAL PROPERTY CX_DOC_USER_MAINPAGE_OVERRIDE "${DOC_FOLDER}/Fraxinus_doc_mainpage.md")
endfunction()


message(STATUS "Configuring plugin org.custusx.fraxinus...")

cxsetup_install_decorate_generators()
cxsetup_install_documentation_files()
cxsetup_install_configuration_files()

# ---------------------------------------------------------------------------
# Optional: silence CMP0177 warning (path normalization for install/DESTINATION)
# ---------------------------------------------------------------------------
if(CX_WINDOWS)
  if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
  endif()
endif()

install(FILES
${CMAKE_CURRENT_LIST_DIR}/installRaidionics.ps1
DESTINATION .
COMPONENT RaidionicsSetup)

install(FILES
${CMAKE_CURRENT_LIST_DIR}/installTotalsegmentator.ps1
DESTINATION .
COMPONENT TotalSegSetup)

install(FILES
${CMAKE_CURRENT_LIST_DIR}/installElastix.ps1
DESTINATION .
COMPONENT ElastixSetup)

# Components and labels
set(CPACK_COMPONENTS_ALL Core RaidionicsSetup TotalSegSetup ElastixSetup)

set(CPACK_NSIS_DISPLAY_NAME_CORE              "Fraxinus Core")
set(CPACK_NSIS_DISPLAY_NAME_RAIDIONICSSETUP   "Raidionics Setup")
set(CPACK_NSIS_DISPLAY_NAME_TOTALSEGSETUP     "TotalSegmentator Setup")
set(CPACK_NSIS_DISPLAY_NAME_ELASTIXSETUP      "Elastix Setup")

set(CPACK_COMPONENT_RAIDIONICSSETUP_DISPLAY_NAME   "Raidionics setup")
set(CPACK_COMPONENT_RAIDIONICSSETUP_DESCRIPTION    "Setup a Raidionics venv and download/unpack model files")

set(CPACK_COMPONENT_TOTALSEGSETUP_DISPLAY_NAME     "TotalSegmentator setup")
set(CPACK_COMPONENT_TOTALSEGSETUP_DESCRIPTION      "Create a venv and download weights")

set(CPACK_COMPONENT_ELASTIXSETUP_DISPLAY_NAME     "Elastix setup")
set(CPACK_COMPONENT_ELASTIXSETUP_DESCRIPTION      "Install Elastix")

# Show details checkbox and route plugin output to the log window
set(CPACK_NSIS_ENABLE_LOG ON)
#set(CPACK_NSIS_LOG_FILE "$LOCALAPPDATA/Fraxinus/Logs") #TODO: Use?


# add the application
cx_add_optional_app_subdirectory("Fraxinus")


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

        # Override the CustusX defaults set in cx_install_initialize_customizable_properties()
        # so the Fraxinus installer/uninstaller show the Fraxinus icon, not CustusX's.
        set_property(GLOBAL PROPERTY CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/Fraxinus/icons/Fraxinus.png")
        if(CX_WINDOWS)
                set_property(GLOBAL PROPERTY CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/Fraxinus/icons\\\\Fraxinus.ico")
        endif()
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

    # Replace CustusX shared pages with Fraxinus-specific versions
    cx_doc_exclude_user_page("${CustusX_SOURCE_DIR}/doc/shared_manual/cx_user_supported_platforms.md")
    cx_doc_exclude_user_page("${CustusX_SOURCE_DIR}/doc/shared_manual/installation/cx_user_installation.md")

    # Add Fraxinus developer documentation and replace CustusX build instructions
    cx_doc_define_dev_docs("${SOURCE_DIR}/doc/dev_manual")
    cx_doc_exclude_dev_page("${CustusX_SOURCE_DIR}/doc/dev_manual/cx_dev_build_instructions.md")
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

if(CX_WINDOWS)

  # Ensure NSIS can access the custom include file
  configure_file(
	${CMAKE_CURRENT_LIST_DIR}/FraxinusPostInstall.nsh
    ${CMAKE_BINARY_DIR}/FraxinusPostInstall.nsh
    COPYONLY
  )

  install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/installRaidionics.ps1
    ${CMAKE_CURRENT_LIST_DIR}/FraxinusInstallHelpers.ps1
    DESTINATION .
    COMPONENT RaidionicsSetup)
  install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/installTotalsegmentator.ps1
    ${CMAKE_CURRENT_LIST_DIR}/FraxinusInstallHelpers.ps1
    DESTINATION .
    COMPONENT TotalSegSetup)
  install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/installElastix.ps1
    ${CMAKE_CURRENT_LIST_DIR}/FraxinusInstallHelpers.ps1
    DESTINATION .
    COMPONENT ElastixSetup)

  # Add components and labels
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

endif()

if(CX_LINUX)
  install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/Fraxinus.desktop
    DESTINATION ${CX_INSTALL_ROOT_DIR})
  install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/Fraxinus/icons/Fraxinus.png
    DESTINATION ${CX_INSTALL_ROOT_DIR}/icons)
endif()

# add the application
cx_add_optional_app_subdirectory("Fraxinus")


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
                # NSIS's File instruction (used to embed MUI_HEADERIMAGE_BITMAP) needs a
                # backslash right before the filename, same as cx_install_initialize_customizable_properties()
                # does for CustusX's own icons - an all-forward-slash path fails with
                # "no files found" even though the file exists.
                set_property(GLOBAL PROPERTY CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}/Fraxinus/icons\\\\Fraxinus.ico")
                set_property(GLOBAL PROPERTY CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/Fraxinus/icons\\\\Fraxinus.png")
        endif()
endmacro()

###############################################################################
#
# Pull a pinned version string out of one of the install*.ps1 scripts, by
# matching a line of the form `$<PS1_VAR_NAME> = "<value>"` (single or double
# quotes). Used to keep the NSIS pre-check logic and the actual pip/git/download
# pin in installRaidionics.ps1 / installTotalsegmentator.ps1 / installElastix.ps1
# from drifting apart.
#
###############################################################################
function(cxsetup_extract_ps1_version SCRIPT_FILE PS1_VAR_NAME OUT_VERSION)
  file(STRINGS "${SCRIPT_FILE}" _matched_lines REGEX "[$]${PS1_VAR_NAME}[ \t]*=")
  if(_matched_lines)
    list(GET _matched_lines 0 _line)
    string(REGEX REPLACE ".*=[ \t]*['\"]([^'\"]+)['\"].*" "\\1" _version "${_line}")
    set(${OUT_VERSION} "${_version}" PARENT_SCOPE)
  else()
    message(WARNING "cxsetup_extract_ps1_version: could not find \$${PS1_VAR_NAME} in ${SCRIPT_FILE}")
    set(${OUT_VERSION} "unknown" PARENT_SCOPE)
  endif()
endfunction()

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

  # Ensure NSIS can access the custom include files
  configure_file(
	${CMAKE_CURRENT_LIST_DIR}/FraxinusPostInstall.nsh
    ${CMAKE_BINARY_DIR}/FraxinusPostInstall.nsh
    COPYONLY
  )
  configure_file(
	${CMAKE_CURRENT_LIST_DIR}/FraxinusPostUninstall.nsh
    ${CMAKE_BINARY_DIR}/FraxinusPostUninstall.nsh
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

  # Add components and labels.
  #
  # NOTE: these must be relayed as GLOBAL PROPERTYs, not plain set(), and picked
  # back up in CX/CMake/cxSetupAdditionalNSISWindowsComponents.cmake just before
  # include(CPack) runs (in CX/CMake/cxInstallCustusX.cmake). This directory is a
  # sibling of, not an ancestor of, the scope include(CPack) runs in, so a plain
  # set() here never reaches it -- the same reason CPACK_PACKAGE_ICON etc. above
  # use set_property(GLOBAL PROPERTY ...) instead of set(). Without this, CPack
  # still discovers the components themselves (auto-detected from the install()
  # COMPONENT names below), but every display-name/description override here is
  # silently dropped -- which is why the NSIS component page's tooltips were
  # blank despite these being "set" correctly.
  set_property(GLOBAL PROPERTY CPACK_COMPONENTS_ALL Core RaidionicsSetup TotalSegSetup ElastixSetup)
  set_property(GLOBAL PROPERTY CPACK_FRAX_FAMILY_FOLDER_NAME "${CX_FAMILY_FOLDER_NAME}")

  if(CX_FRAXINUS_PRIVATE)
    set_property(GLOBAL PROPERTY CPACK_COMPONENT_CORE_DISPLAY_NAME "FraxinusExcelsior Core")
  else()
    set_property(GLOBAL PROPERTY CPACK_COMPONENT_CORE_DISPLAY_NAME "Fraxinus Core")
  endif()
  set_property(GLOBAL PROPERTY CPACK_COMPONENT_CORE_DESCRIPTION "Core application files")

  # Kept to one short sentence each: the NSIS components-page description box
  # is a fixed-size static control with no scrollbar, so longer text risks
  # being clipped rather than wrapping. It also only wraps at whitespace, not
  # at '/' or '\', so a long unbroken path (e.g. a full nested install path)
  # just runs past the edge and gets clipped instead of wrapping -- keep any
  # path short enough to read as a single "word". Paths use forward slashes
  # rather than backslashes -- still readable as a Windows path in tooltip
  # text, but a literal backslash here (e.g. \F, \T) trips CMake's "Invalid
  # escape sequence" dev warning when CPack's own generated CPackConfig.cmake
  # is re-parsed by cpack.exe (harmless, but noisy in the build log).
  set_property(GLOBAL PROPERTY CPACK_COMPONENT_RAIDIONICSSETUP_DISPLAY_NAME "Raidionics setup")
  set_property(GLOBAL PROPERTY CPACK_COMPONENT_RAIDIONICSSETUP_DESCRIPTION  "Slow - opens a console window. Installs into the shared Fraxinus folder in your user folder.")

  set_property(GLOBAL PROPERTY CPACK_COMPONENT_TOTALSEGSETUP_DISPLAY_NAME "TotalSegmentator setup")
  set_property(GLOBAL PROPERTY CPACK_COMPONENT_TOTALSEGSETUP_DESCRIPTION  "Slow - opens a console window. Installs into the shared Fraxinus folder in your user folder.")

  set_property(GLOBAL PROPERTY CPACK_COMPONENT_ELASTIXSETUP_DISPLAY_NAME "Elastix setup")
  set_property(GLOBAL PROPERTY CPACK_COMPONENT_ELASTIXSETUP_DESCRIPTION  "Opens a console window. Installs to C:/Elastix.")

  # Show details checkbox and route plugin output to the log window
  set_property(GLOBAL PROPERTY CPACK_NSIS_ENABLE_LOG ON)

  # Pinned versions of each optional external tool, read straight out of the
  # matching install*.ps1 script (single source of truth -- the same value
  # that script actually installs). Relayed as GLOBAL PROPERTYs the same way
  # as the component descriptions above, and substituted into NSIS.template.in
  # as @CPACK_FRAX_..._VERSION@ so Function .onInit can tell an already
  # up-to-date component apart from one that needs a (re)install; see
  # installed_version.txt written by each script on success.
  cxsetup_extract_ps1_version("${CMAKE_CURRENT_LIST_DIR}/installRaidionics.ps1" RaidionicsVersion _raidionicsVersion)
  cxsetup_extract_ps1_version("${CMAKE_CURRENT_LIST_DIR}/installTotalsegmentator.ps1" TotalSegmentatorVersion _totalSegVersion)
  cxsetup_extract_ps1_version("${CMAKE_CURRENT_LIST_DIR}/installElastix.ps1" ElastixVersion _elastixVersion)
  set_property(GLOBAL PROPERTY CPACK_FRAX_RAIDIONICS_VERSION "${_raidionicsVersion}")
  set_property(GLOBAL PROPERTY CPACK_FRAX_TOTALSEG_VERSION "${_totalSegVersion}")
  set_property(GLOBAL PROPERTY CPACK_FRAX_ELASTIX_VERSION "${_elastixVersion}")

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


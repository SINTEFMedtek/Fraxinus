###########################################################
##
##               Fraxinus Application
##
###########################################################

#cx_add_class_qt_moc(SOURCES
#    cxFraxinusMainWindow
#)

#cx_add_class(SOURCES
#)


qt5_add_resources(fraxinus_resources
    cxfraxinus.qrc
)

# The private plugin build produces a distinctly-branded FraxinusExcelsior.exe
# instead of Fraxinus.exe, so it installs alongside (not on top of) plain
# public Fraxinus -- same executable-target-per-product pattern CustusS uses
# relative to CustusX. CX_FRAXINUS_PRIVATE is set by cxPrivateComponentAssembly.py
# and already gates whether the private plugin is built at all (see
# org.custusx.fraxinus.core.state/CMakeLists.txt).
if(CX_FRAXINUS_PRIVATE)
    set(FRAXINUS_TARGET_NAME FraxinusExcelsior)
else()
    set(FRAXINUS_TARGET_NAME Fraxinus)
endif()

set(SOURCES
    ${SOURCES}
    ${fraxinus_resources}
    cxMain.cpp
    cxFraxinusMainWindowApplicationComponent.h
    cxFraxinusMainWindowApplicationComponent.cpp
)

if(APPLE)
    # For Apple set the icns file containing icons
    # set where in the bundle to put the icns file
	set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/icons/Fraxinus.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

    # include the icns file in the target
	set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/icons/Fraxinus.icns)

    # NOTE: Don't include the path in MACOSX_BUNDLE_ICON_FILE -- this is
    # the property added to Info.plist
	set(MACOSX_BUNDLE_ICON_FILE "Fraxinus.icns")
endif(APPLE)

#------------------------------------------------------------------------------
# Executable
#-----------------------------------------------------------------------------
if(APPLE)
    add_executable(${FRAXINUS_TARGET_NAME} MACOSX_BUNDLE ${SOURCES} ) #APPLE
else(APPLE)
    if(CX_WINDOWS)
        # CustusX.rc only references a local .ico file, but adding it as a normal
        # source of the Fraxinus target (CMake's native RC-language handling) makes
        # it inherit the target's entire aggregated include-directory list (every
        # linked VTK/ITK/CTK/... module), same as any .cpp file. Unlike cl.exe,
        # rc.exe has no response-file fallback for long command lines, so once
        # enough dependencies are linked in (e.g. the private plugins) this trips
        # Windows' ~32767-char command-line limit ("CreateProcess failed...
        # command line too long"). Compile it via a plain custom command instead,
        # so it only ever sees the two paths it actually needs.
        set(CX_RC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CustusX.rc")
        set(CX_RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/CustusX.res")
        add_custom_command(
            OUTPUT ${CX_RES_FILE}
            COMMAND ${CMAKE_RC_COMPILER} /fo ${CX_RES_FILE} ${CX_RC_FILE}
            DEPENDS ${CX_RC_FILE} "${CMAKE_CURRENT_SOURCE_DIR}/icons/Fraxinus.ico"
            COMMENT "Compiling CustusX.rc"
            VERBATIM
        )
        # Without EXTERNAL_OBJECT, the Ninja/Makefiles generators don't recognize
        # the precompiled .res as something to feed to the linker (unlike the
        # Visual Studio generator, which understands .res natively) and silently
        # drop it from the link line -- the exe still links fine, just with no
        # icon resource embedded.
        set_source_files_properties(${CX_RES_FILE} PROPERTIES EXTERNAL_OBJECT TRUE)
        add_executable(${FRAXINUS_TARGET_NAME} ${SOURCES} ${CX_RES_FILE}) #WINDOWS
        set_target_properties(${FRAXINUS_TARGET_NAME} PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"")
    else()
        add_executable(${FRAXINUS_TARGET_NAME} ${SOURCES} ) #LINUX
    endif()
endif(APPLE)

cx_set_target_high_resolution(${FRAXINUS_TARGET_NAME})

#------------------------------------------------------------------------------
# Linking
#------------------------------------------------------------------------------

target_link_libraries(${FRAXINUS_TARGET_NAME}
    PRIVATE
    cxResource
    cxLogicManager
    cxResourceWidgets
    cxResourceVisualization
    cxGUIExtenderService
    cxPluginUtilities
    cxResourceFilter
    cxGui
    cxLogicManager
    ${SSC_GCOV_LIBRARY} #for coverage testing
)

#------------------------------------------------------------------------------
# Install
#------------------------------------------------------------------------------

install(TARGETS ${FRAXINUS_TARGET_NAME}
    BUNDLE DESTINATION ${CX_INSTALL_ROOT_DIR}
    RUNTIME DESTINATION ${CX_INSTALL_ROOT_DIR}/bin
)

message(STATUS "Configuring application ${FRAXINUS_TARGET_NAME}...")


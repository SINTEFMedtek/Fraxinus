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

set(SOURCES
    ${SOURCES}
    ${fraxinus_resources}
    cxMain.cpp
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
    add_executable(Fraxinus MACOSX_BUNDLE ${SOURCES} ) #APPLE
else(APPLE)
    if(CX_WINDOWS)
        add_executable(Fraxinus ${SOURCES} Fraxinus.rc) #WINDOWS
        set_target_properties(Fraxinus PROPERTIES LINK_FLAGS    "/MANIFESTUAC:\"level='asInvoker' uiAccess='false'\"")
    else()
        add_executable(Fraxinus ${SOURCES} ) #LINUX
    endif()
endif(APPLE)

#------------------------------------------------------------------------------
# Linking
#------------------------------------------------------------------------------

target_link_libraries(Fraxinus
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

install(TARGETS Fraxinus
    BUNDLE DESTINATION ${CX_INSTALL_ROOT_DIR}
    RUNTIME DESTINATION ${CX_INSTALL_ROOT_DIR}/bin
)

message(STATUS "Configuring application Fraxinus...")


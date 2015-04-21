###########################################################
##
##               Fraxinus Application
##
###########################################################

cx_add_class_qt_moc(SOURCES
    cxFraxinusMainWindow
)

#cx_add_class(SOURCES
#)

set(SOURCES
    ${SOURCES}
    cxMain.cpp
)

if(APPLE)
	# For Apple set the icns file containing icons
	set_source_files_properties(${CustusX_SOURCE_DIR}/source/gui/icons/application.icns
                                    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)  # set where in the bundle to put the icns file
	set(SOURCES ${SOURCES} ${CustusX_SOURCE_DIR}/source/gui/icons/application.icns)  # include the icns file in the target
endif(APPLE)

#------------------------------------------------------------------------------
# Executable
#-----------------------------------------------------------------------------
if(APPLE)
    add_executable(Fraxinus MACOSX_BUNDLE ${SOURCES} ) #APPLE
else(APPLE)
    if(CX_WINDOWS)
        add_executable(Fraxinus ${SOURCES} CustusX.rc) #WINDOWS
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


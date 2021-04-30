###########################################################
##               Fraxinus State Service
##
###########################################################

project(org_custusx_fraxinus_core_state)

if(NOT CX_APPLE)
    cx_initialize_FAST()
endif()

set(PLUGIN_export_directive "${PROJECT_NAME}_EXPORT")

qt5_add_resources(fraxinus_state_resources
    cxFraxinusState.qrc
)

set(PLUGIN_SRCS
    ${fraxinus_state_resources}
    cxStateServiceImpl.cpp
    cxFraxinusWorkflowStateMachine.h
    cxFraxinusWorkflowStateMachine.cpp
    cxFraxinusWorkflowStates.h
    cxFraxinusWorkflowStates.cpp
    cxFraxinusWorkflowStatesNavigation.h
    cxFraxinusWorkflowStatesNavigation.cpp
    cxStatePluginActivator.cpp
    cxApplicationsParser.h
    cxApplicationsParser.cpp
    cxVBCameraZoomSetting3D.h
    cxVBCameraZoomSetting3D.cpp
    cxFraxinusSegmentations.h
    cxFraxinusSegmentations.cpp
)


if(NOT CX_APPLE)
    # Make sure OpenCL kernel files are copied to the installer
    # Install OpenCL kernels
    install(DIRECTORY ${FAST_SOURCE_DIR}
            DESTINATION ${CX_INSTALL_CONFIG_DIR}/FAST/
            FILES_MATCHING PATTERN "*.cl"
    )
endif()

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)
# Z lib comes with many names, needed for FAST atm
if(WIN32)
    set(ZLIB zlib.lib)
elseif(APPLE)
    set(ZLIB libz.dylib)
else()
    set(ZLIB libz.so)
endif(WIN32)


# Files which should be processed by Qts moc
set(PLUGIN_MOC_SRCS
    cxFraxinusWorkflowStateMachine.h
    cxFraxinusWorkflowStates.h
    cxFraxinusWorkflowStatesNavigation.h
    cxStateServiceImpl.h
    cxStatePluginActivator.h
    cxFraxinusSegmentations.h
)

# Qt Designer files which should be processed by Qts uic
set(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
set(PLUGIN_resources
)

if(NOT CX_APPLE)
    find_package(FAST REQUIRED)
    link_directories("${FAST_DIR}/lib/")
endif()

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)
set(PLUGIN_target_libraries 
    PUBLIC
    ${PLUGIN_target_libraries}
    Qt5::Widgets
    cxResourceFilter

    #PRIVATE
    #FAST
    cxPluginUtilities
    cxResource
    cxResourceWidgets
    cxResourceVisualization
    cxGui
    org_custusx_filter_routetotarget
    org_custusx_virtualbronchoscopy
    org_custusx_fraxinus_widgets
    )

if(NOT CX_APPLE)
  set(PLUGIN_target_libraries
    ${PLUGIN_target_libraries}
    FAST
    org_custusx_filter_airways
    )
endif()

set(PLUGIN_OUTPUT_DIR "")
if(CX_WINDOWS)
    #on windows we want dlls to be placed with the executables
    set(PLUGIN_OUTPUT_DIR "../")
endif(CX_WINDOWS)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries}
  OUTPUT_DIR ${PLUGIN_OUTPUT_DIR}
  ${CX_CTK_PLUGIN_NO_INSTALL}
)
target_include_directories(org_custusx_fraxinus_core_state
    PUBLIC
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(testing)

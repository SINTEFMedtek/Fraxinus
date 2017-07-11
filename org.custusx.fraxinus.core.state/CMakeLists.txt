###########################################################
##               Fraxinus State Service
##
###########################################################

project(org_custusx_fraxinus_core_state)

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
    cxStatePluginActivator.cpp
    cxApplicationsParser.h
    cxApplicationsParser.cpp
    cxVBCameraZoomSetting3D.h
    cxVBCameraZoomSetting3D.cpp
)

# Files which should be processed by Qts moc
set(PLUGIN_MOC_SRCS
    cxFraxinusWorkflowStateMachine.h
    cxFraxinusWorkflowStates.h
    cxStateServiceImpl.h
    cxStatePluginActivator.h
)

# Qt Designer files which should be processed by Qts uic
set(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
set(PLUGIN_resources
)

find_package(FAST REQUIRED)
link_directories("${FAST_DIR}/lib/")

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)
set(PLUGIN_target_libraries 
    PUBLIC
    ${PLUGIN_target_libraries}

    PRIVATE
    cxPluginUtilities
    Qt5::Widgets
    cxResource
	cxResourceWidgets
    cxResourceVisualization
    cxResourceFilter
    cxGui
    org_custusx_filter_airways
    org_custusx_filter_routetotarget
    org_custusx_virtualbronchoscopy
    )

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

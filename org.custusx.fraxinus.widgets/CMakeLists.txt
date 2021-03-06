project(org_custusx_fraxinus_widgets)

#### Enable C++11
set (CMAKE_CXX_STANDARD 11)

set(PLUGIN_export_directive "${PROJECT_NAME}_EXPORT")

qt5_add_resources(fraxinus_widgets_resources
    cxFraxinusWidgets.qrc
)

set(PLUGIN_SRCS
    ${fraxinus_widgets_resources}
    cxFraxinusWidgetsPluginActivator.cpp
    cxFraxinusWidgetsGUIExtenderService.cpp
    cxPinpointWidget.cpp
    cxNewLoadPatientWidget.cpp
    cxFraxinusVBWidget.cpp
)

# Files which should be processed by Qts moc
set(PLUGIN_MOC_SRCS
    cxFraxinusWidgetsPluginActivator.h
    cxFraxinusWidgetsGUIExtenderService.h
    cxPinpointWidget.h
    cxNewLoadPatientWidget.h
    cxFraxinusVBWidget.h
)

# Qt Designer files which should be processed by Qts uic
set(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
set(PLUGIN_resources
)


#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)
set(PLUGIN_target_libraries 
    ${PLUGIN_target_libraries}   
    cxGUIExtenderService
    cxPluginUtilities
    Qt5::Widgets
    cxResourceVisualization #todo: remove (for VisServices, for FilterSetupWidget)
    cxGui #todo: remove (for FilterSetupWidget)
    cxResourceWidgets
    cxResource #todo: remove?
    cxResourceFilter
	cxLogicManager
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

target_include_directories(${PROJECT_NAME}
    PUBLIC
    .
    ${CMAKE_CURRENT_BINARY_DIR}
)


#add_subdirectory(testing)


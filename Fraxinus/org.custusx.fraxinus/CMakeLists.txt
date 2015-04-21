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
                    DESTINATION ${CX_INSTALL_ROOT_DIR}/config
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

    #TODO

endfunction()


if(APPLE)
    # Install folder for storage of igstk<->CustusX symlinks.
    # The symlinks must be in an absolute location.
    # Apple only (Win uses COM-ports, Linux installs with OS)
    # Because CPACK_PACKAGING_INSTALL_PREFIX is "/", this will install
    # in /Library/CustusX.
    install(DIRECTORY ${SOURCE_DIR}/install/Apple/igstk.links
            DESTINATION Library/CustusX
            FILE_PERMISSIONS ${CX_FULL_PERMISSIONS}
            DIRECTORY_PERMISSIONS ${CX_FULL_PERMISSIONS}
            )
endif(APPLE)


message(STATUS "Configuring plugin org.custusx.fraxinus...")

cxsetup_install_decorate_generators()
cxsetup_install_documentation_files()
cxsetup_install_configuration_files()

# add the application
cx_add_optional_app_subdirectory("Fraxinus")


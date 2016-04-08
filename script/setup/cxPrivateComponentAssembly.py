#!/usr/bin/env python

import platform

import cx.build.cxComponents
import cx.build.cxComponentAssembly
import cxPrivateComponents
import cx.build.cxInstallData
import cx.utils.cxSSH

class PrivateControlData(cx.build.cxInstallData.Common):
    def __init__(self):
        super(PrivateControlData, self).__init__()

        self.main_branch = "master"
        user = "custusx"
        server = "sintefweb07.sintef.no"
        self.publish_release_target                 = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/releases", user)
        self.publish_developer_documentation_target = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/developer_doc", user)
        self.publish_user_documentation_target      = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/user_doc", user)
        self.publish_coverage_info_target           = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/gcov", user)
	self.gitrepo_internal_site_base = "ssh://git@git.code.sintef.no/mt"
	self.gitrepo_main_site_base = "ssh://git@git.code.sintef.no/mt"

	self.system_base_name = "Fraxinus"

class LibraryAssembly(cx.build.cxComponentAssembly.LibraryAssembly):
    '''
	Contains all components
    '''
    def __init__(self):
	''
        controlData = PrivateControlData()
        super(LibraryAssembly, self).__init__(controlData)

        self.addComponent(cxPrivateComponents.Fraxinus())
        self.addComponent(cxPrivateComponents.org_custusx_virtualbronchoscopy())
        self.libraries.remove(self.custusx)
        self.addComponent(self.custusx)

        
        



#!/usr/bin/env python

import os
import platform

import cx.build.cxComponents
import cx.build.cxComponentAssembly
from . import cxPrivateComponents
import cx.build.cxInstallData
import cx.utils.cxSSH
from . import cxCustusXFinder
from . import cxRepoHandler

class PrivateControlData(cx.build.cxInstallData.Common):
    def __init__(self):
        ''
        super(PrivateControlData, self).__init__()
        
        
        self.main_repo_folder = cxCustusXFinder.RepoLocations().getPrivateRepoPath()
        self.main_branch = cxRepoHandler.getBranchForRepo(self.main_repo_folder, fallback='develop')
        user = "custusx"
        server = "mtwiki.sintef.no"
        self.publish_release_target                 = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/releases", user)
        self.publish_developer_documentation_target = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/developer_doc", user)
        self.publish_user_documentation_target      = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/user_doc", user)
        self.publish_coverage_info_target           = cx.utils.cxSSH.RemoteServerID(server, "uploads/fraxinus/gcov", user)
        self.gitrepo_open_site_base = "https://github.com/SINTEFMedtek"
        self.gitrepo_main_site_base = "git@gitlab.sintef.no:custusx"

        self.system_base_name = "Fraxinus"
        self.mBuildIGSTK = self._igstk_supported() # Build with IGSK tracking for Ubuntu 20 and 22

    def _igstk_supported(self):
        # CI sets BUILD_IGSTK explicitly; honour it so the Python installer
        # stays in sync with the shell-side IGSTK_LIBS / tarball selection.
        build_igstk = os.environ.get('BUILD_IGSTK', '').lower()
        if build_igstk == 'true':
            return True
        if build_igstk in ('false', '0'):
            return False
        # Fall back to OS detection for local builds where BUILD_IGSTK is not set.
        if platform.system() != 'Linux':
            return False
        try:
            with open('/etc/os-release') as f:
                for line in f:
                    if line.startswith('VERSION_ID='):
                        version_id = float(line.strip().split('=')[1].strip('"'))
                        return version_id < 24.0
        except Exception as e:
            print('WARNING: _igstk_supported: could not read /etc/os-release (%s), defaulting IGSTK to off.' % e)
            return False
        print('WARNING: _igstk_supported: VERSION_ID not found in /etc/os-release, defaulting IGSTK to off.')
        return False

class LibraryAssembly(cx.build.cxComponentAssembly.LibraryAssembly):
    '''
    Contains all components
    '''
    def __init__(self, controlData=None):
        ''
        if controlData is None:
            controlData = PrivateControlData()
        super(LibraryAssembly, self).__init__(controlData)

        self.addComponent(cxPrivateComponents.Fraxinus())
        self.addComponent(cxPrivateComponents.org_custusx_fraxinus_private())

        if not self.controlData.mBuildIGSTK:
            self.addComponent(cxPrivateComponents.org_custusx_core_tracking_system_ndi())

	#self.addComponent(cxPrivateComponents.thoraxCTdata())
        self.libraries.remove(self.custusx)
        self.addComponent(self.custusx)

        
        



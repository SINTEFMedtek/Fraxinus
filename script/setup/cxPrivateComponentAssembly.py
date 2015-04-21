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
        
        self.system_base_name = "Fraxinus"

class LibraryAssembly(cx.build.cxComponentAssembly.LibraryAssembly):
    def __init__(self):
        controlData = PrivateControlData()
        super(LibraryAssembly, self).__init__(controlData)

        self.addComponent(cxPrivateComponents.Fraxinus())
        self.libraries.remove(self.custusx)
        self.addComponent(self.custusx)


        
        



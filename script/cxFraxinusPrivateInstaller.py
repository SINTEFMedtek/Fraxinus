#!/usr/bin/env python

import cxsetup.cxFindCustusX

import cx.script.cxInstallScript
import cxsetup.cxPrivateComponentAssembly

class Controller(cx.script.cxInstallScript.Controller):
    def __init__(self, assembly=None):
        ''                
        assembly = cxsetup.cxPrivateComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)

if __name__ == '__main__':
    controller = Controller()
    controller.run()



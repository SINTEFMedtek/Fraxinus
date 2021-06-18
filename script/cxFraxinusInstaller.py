#!/usr/bin/env python

import cxsetup.cxFindCustusX

import cx.script.cxInstallScript
import cxsetup.cxPublicComponentAssembly

class Controller(cx.script.cxInstallScript.Controller):
    def __init__(self, assembly=None):
        ''                
        assembly = cxsetup.cxPublicComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)

if __name__ == '__main__':
    controller = Controller()
    controller.run()



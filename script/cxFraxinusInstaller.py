#!/usr/bin/env python

import setup.cxFindCustusX

import cx.script.cxInstallScript
import setup.cxPrivateComponentAssembly

class Controller(cx.script.cxInstallScript.Controller):
    def __init__(self, assembly=None):
        ''                
        assembly = setup.cxPrivateComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)

if __name__ == '__main__':
    controller = Controller()
    controller.run()



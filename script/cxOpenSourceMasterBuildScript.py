#!/usr/bin/env python

#####################################################
# Unix jenkins script
# Author: Ole Vegard Solberg, SINTEF Medical Technology
# Date:   2022.06.10
#
# Description:
# Open source build for Fraxinus, used by Bamboo
#
#####################################################

import cxsetup.cxUpdateToLatestCustusX

import cxsetup.cxPublicComponentAssembly
import cx.script.cxJenkinsMasterBuildScript

class Controller(cx.script.cxJenkinsMasterBuildScript.Controller):
    '''
    '''
    def __init__(self):
        ''
        assembly = cxsetup.cxPublicComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)
        
if __name__ == '__main__':
    controller = Controller()
    controller.run()

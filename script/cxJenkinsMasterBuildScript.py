#!/usr/bin/env python

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2013.10.13
#
# Description:
#
#
#####################################################

import cxsetup.cxUpdateToLatestCustusX

import cxsetup.cxPrivateComponentAssembly
import cx.script.cxJenkinsMasterBuildScript

class Controller(cx.script.cxJenkinsMasterBuildScript.Controller):
    '''
    '''
    def __init__(self):
        ''                
        assembly = cxsetup.cxPrivateComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)
        
if __name__ == '__main__':
    controller = Controller()
    controller.run()

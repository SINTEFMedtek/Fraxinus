#!/usr/bin/env python

#####################################################
# Unix jenkins script
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2015.02.16
#
# Description:
#
#
#####################################################

import setup.cxUpdateToLatestCustusX

import setup.cxPrivateComponentAssembly
import cx.script.cxJenkinsBuildScript_JobDefinitions

class Controller(cx.script.cxJenkinsBuildScript_JobDefinitions.Controller):
    '''
    '''
    def __init__(self):
        ''                
        assembly = setup.cxPrivateComponentAssembly.LibraryAssembly()
        super(Controller, self).__init__(assembly)
        
if __name__ == '__main__':
    controller = Controller()
    controller.run()

        

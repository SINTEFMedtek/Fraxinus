#####################################################
# 
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2014.07.13
#
# Import at the beginning of a setup script,
# will checkout CustusX and connect to the scripts there.
#
#####################################################

from . import cxCustusXFinder

finder = cxCustusXFinder.CustusXFinder()    

finder.checkoutMaster()
finder.addCustusXPythonScriptFolderToPath()


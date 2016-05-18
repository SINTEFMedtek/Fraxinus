#####################################################
# 
# Author: Christian Askeland, SINTEF Medical Technology
# Date:   2014.07.13
#
# Import at the beginning of a setup script,
# will checkout CustusX and connect to the scripts there.
#
#####################################################
import os.path
import os
import sys
import subprocess
import pprint
import argparse
import cxRepoHandler

import argparse

class RepoLocations:
    '''
    '''    
    def getRootPath(self):
        '''
        find the root path, i.e. common root for both public and private repos
        '''
        loc = self.getPrivateRepositoryLocation()
        rootPath = loc[0]
        return rootPath

    def getPrivateRepoPath(self):
        '''
        Look at the file system, find the name of the folder the repo resides in.
        '''
        loc = self.getPrivateRepositoryLocation()
        return '/'.join(loc)

    def getPrivateRepoFolder(self):
        '''
        Look at the file system, find the name of the folder the repo resides in.
        '''
        loc = self.getPrivateRepositoryLocation()
        repoFolder = loc[2]
        return repoFolder
    
    def getPrivateRepositoryLocation(self):
        '''
        Look at the file system, find the name of the folder the repo resides in.
        Return a list containing:
        r = [root_dir, custusx_base, custusx_repo]
        such that the full path to the private repo is
        p = '/'.join(r)
        
        '''
        # assuming module path is root/base/repo/script/cxsetup
        moduleFile = os.path.realpath(__file__)
        modulePath = os.path.dirname(moduleFile)
        repoPath = '%s/../..' % modulePath 
        repoPath = os.path.abspath(repoPath) # root/base/repo
        (basePath, repoFolder) = os.path.split(repoPath)
        (rootPath, baseFolder) = os.path.split(basePath)
        return [rootPath, baseFolder, repoFolder]
    
    def getPublicRepoPath(self):
        r = self.getRootPath()
        p = self.getPublicRepoFolder()
        return '%s/%s/%s' % (r, p, p)
    
    def getPublicRepoFolder(self):
        '''
        If this repo is named old-style, return old-style CustusX name,
        otherwise return new style (2015-02-25)
        '''
        cs_name = self.getPrivateRepoFolder()
        if cs_name=='CustusXSetup':
            return 'CustusX'
        else:
            return 'CX'
###########################################################    


class CustusXFinder:
    '''
    '''    
    def __init__(self, silent=True):
        self.silent = silent
        self.locations = RepoLocations()
                    
    def getCustusXRepo(self, forceCheckout):
        root = self.locations.getRootPath()
        print '===== ensure CustusX is checked out [root: %s] ===' % root
        cx_name = self.locations.getPublicRepoFolder()

        cx_root = '%s/%s' % (root, cx_name)
        url_base = 'git@github.com:SINTEFMedtek/'
        url_name = 'custusx.git'
        cx_repo_path='%s/%s' % (cx_root, cx_name)

        repo = cxRepoHandler.RepoHandler(silent=self.silent)

        repo.setRepoInfo(url_base=url_base, url_name=url_name, root_path=cx_root, repo_path=cx_repo_path)
        repo.cloneRepoWithPrompt()
        
        defaultBranch = cxRepoHandler.getBranchForRepo(self.locations.getPrivateRepoPath())
        repo.setBranchDefault(defaultBranch)
        repo.syncToGitRef()
        
        print '===== ensure CustusX is checked out completed [location: %s] ===' % cx_repo_path
        
    def addCustusXPythonScriptFolderToPath(self):
        cx_path = "%s/install" % (self.locations.getPublicRepoPath())
        if os.path.exists(cx_path):
            sys.path.append(cx_path)
            print "Added %s to pythonpath" % cx_path
        else:
            print "ERROR: Could not locate CustusX python script folder %s" % cx_path
            
    def checkoutMasterIfNotFound(self):
        self.getCustusXRepo(forceCheckout=False)

    def checkoutMaster(self):
        self.getCustusXRepo(forceCheckout=True)

        

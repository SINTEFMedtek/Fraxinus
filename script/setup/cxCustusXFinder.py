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

    def runShell(self, cmd, path):
        if not os.path.exists(path):
            os.makedirs(path)
        print '[shell cmd] %s [%s]' % (cmd, path)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, cwd=path)
        out, err = p.communicate("") # wait for process to complete
        if p.returncode == 0:
            return out.strip()
        return None
                    
    def getCustusXRepo(self, forceCheckout):
        root = self.locations.getRootPath()
        print '===== ensure CustusX is checked out [root: %s] ===' % root
        cx_name = self.locations.getPublicRepoFolder()

        cx_root = '%s/%s' % (root, cx_name)
        repository = 'ssh://git@git.code.sintef.no/mt/custusx.git'

        args = self._parseArgs()
        branch = args.main_branch        
        
        cx_repo_path = '%s/%s' % (cx_root, cx_name)
        pathfound = os.path.exists(cx_repo_path)
        if not pathfound:
            print '*** CustusX will be cloned in [%s]' % cx_root
            doprompt = not (self.silent or args.silent_mode)
            self._promptToContinue(doprompt)
            self.runShell('git clone %s %s' % (repository, cx_name), cx_root)
            self.syncTagOrBranchToParent(cx_repo_path)                
        elif forceCheckout:
            self.syncTagOrBranchToParent(cx_repo_path)                
        print '===== ensure CustusX is checked out completed [location: %s] ===' % cx_repo_path
    
    def syncTagOrBranchToParent(self, cx_path):
        '''
        Attempt to guess what git is pointing at:
         * Are we on a tag? If 'git describe --tags --exact-match'
           succeeeds, then YES.
         * Are we on a branch? If 'git rev-parse --abbrev-ref HEAD' != HEAD, 
           then YES
         * otherwise, we depend on input command-line parameters 
         * last resort: master branch
        '''
        cxsetup_path = self.locations.getPrivateRepoPath()
        self.runShell('git fetch', cx_path)
        
        args = self._parseArgs()

        tag = args.git_tag        
        if not tag or tag=="":
            tag = self.runShell('git describe --tags --exact-match', cxsetup_path)
        
        if tag:
            print 'Checking out CustusX to same tag=%s as Fraxinus' % tag
            self.runShell('git checkout %s' % tag, cx_path)
        else:
            branch = args.main_branch        
            if not branch or branch=="":
                branch = self.runShell('git rev-parse --abbrev-ref HEAD', cxsetup_path)
            if branch=="HEAD": # jenkins always checkout a SHA, thus the repo always start out detached
                print "CustusXSetup repo is detached, guess branch=master as we have no more info"
                branch = 'master'

            print 'Checking out/pulling CustusX to same branch=%s as Fraxinus' % branch
            if not self.runShell('git checkout %s' % branch, cx_path):
                self.runShell('git checkout %s' % 'develop', cx_path)
            if not self.runShell('git pull origin %s' % branch, cx_path):
                self.runShell('git pull origin %s' % 'develop', cx_path)

    
    def _parseArgs(self):
        parser = argparse.ArgumentParser(add_help=False, conflict_handler='resolve')
        parser.add_argument('-g', '--git_tag', default=None, metavar='TAG', dest='git_tag')
        parser.add_argument('--main_branch', default='master', dest='main_branch')
        parser.add_argument('-s', '--silent_mode', action='store_true', help='execute script without user interaction')
        args = parser.parse_known_args()[0]
        return args

    def _promptToContinue(self, do_it):
        if do_it:
            raw_input("\nPress enter to continue or ctrl-C to quit:")

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

        

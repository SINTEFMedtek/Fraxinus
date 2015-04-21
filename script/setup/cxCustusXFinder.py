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


class CustusXFinder:
    '''
    '''    
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
        print '===== ensure CustusX is checked out ==='
        root = self.getRootPath()
        cx_name = 'CX'

        cx_root = '%s/%s' % (root, cx_name)
        repository = 'git@github.com:SINTEFMedtek/CustusX.git'

        args = self._parseArgs()
        branch = args.main_branch        
        
        cx_repo_path = '%s/%s' % (cx_root, cx_name)
        pathfound = os.path.exists(cx_repo_path)
        if not pathfound:
            self.runShell('git clone %s %s' % (repository, cx_name), cx_root)
        if forceCheckout:
            self.syncTagOrBranchToParent(cx_repo_path)                
        print '===== ensure CustusX is checked out completed [location: %s] ===' % cx_repo_path
    
    def syncTagOrBranchToParent(self, cx_path):
        cxsetup_path = self.getPrivateRepoPath()
        self.runShell('git fetch', cx_path)
        
        args = self._parseArgs()

        tag = args.git_tag        
        if not tag or tag=="":
            tag = self.runShell('git describe --tags --exact-match', cxsetup_path)
        
        if tag:
            print 'Checking out CustusX to same tag=%s as CustusXSetup' % tag
            self.runShell('git checkout %s' % tag, cx_path)
        else:
            branch = args.main_branch        
            if not branch or branch=="":
                branch = self.runShell('git rev-parse --abbrev-ref HEAD', cxsetup_path)
            if branch=="HEAD": # jenkins always checkout a SHA, thus the repo always start out detached
                print "The projects repo is detached, guess branch=master as we have no more info"
                branch = 'master'

            print 'Checking out/pulling CustusX to same branch=%s as the project' % branch
            self.runShell('git checkout %s' % branch, cx_path)
            self.runShell('git pull origin %s' % branch, cx_path)

    def getRootPath(self):
        '''
        find the root path, i.e. common root for both public and private repos
        '''
        moduleFile = os.path.realpath(__file__)
        modulePath = os.path.dirname(moduleFile)
        repoPath = '%s/../..' % modulePath 
        rootPath = '%s/../..' % repoPath
        rootPath = os.path.abspath(rootPath)
        #print "********* Found Root path: %s, " % rootPath
        return rootPath

    def getPrivateRepoPath(self):
        '''
        Look at the file system, find the name of the folder the repo resides in.
        '''
        r = self.getRootPath()
        p = self.getPrivateRepoFolder()
        return '%s/%s/%s' % (r, p, p)

    def getPrivateRepoFolder(self):
        '''
        Look at the file system, find the name of the folder the repo resides in.
        '''
        moduleFile = os.path.realpath(__file__)
        modulePath = os.path.dirname(moduleFile)
        repoPath = '%s/../../..' % modulePath 
        repoPath = os.path.abspath(repoPath)
        repoFolder = os.path.split(repoPath)[1]
        #print "********* Found Private path: %s, folder=%s" % (repoPath, repoFolder)
        return repoFolder
    
    def getPublicRepoPath(self):
        r = self.getRootPath()
        p = 'CX'
        return '%s/%s/%s' % (r, p, p)
    
    def _parseArgs(self):
        parser = argparse.ArgumentParser(add_help=False, conflict_handler='resolve')
        parser.add_argument('-g', '--git_tag', default=None, metavar='TAG', dest='git_tag')
        parser.add_argument('--main_branch', default='master', dest='main_branch')
        args = parser.parse_known_args()[0]
        return args

    def addCustusXPythonScriptFolderToPath(self):
        cx_path = "%s/install" % (self.getPublicRepoPath())
        if os.path.exists(cx_path):
            sys.path.append(cx_path)
            print "Added %s to pythonpath" % cx_path
        else:
            print "ERROR: Could not locate CustusX python script folder %s" % cx_path
            
    def checkoutMasterIfNotFound(self):
        self.getCustusXRepo(forceCheckout=False)

    def checkoutMaster(self):
        self.getCustusXRepo(forceCheckout=True)

        

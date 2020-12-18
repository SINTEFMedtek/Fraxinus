#!/usr/bin/env python

import platform

import cx.build.cxComponents
import cxCustusXFinder

class Fraxinus(cx.build.cxComponents.CppComponent):
    def name(self):
        return "Fraxinus"
    def help(self):
        return 'Info and setup for Fraxinus'
    def path(self):
        return '%s/%s' % (self.controlData.getWorkingPath(), self.sourceFolder())    
    def sourceFolder(self):
	return cxCustusXFinder.RepoLocations().getPrivateRepoFolder()
    def update(self):
        self._getBuilder().gitCheckoutDefaultBranch()    
    def configure(self):
        pass
    def build(self):
        pass
    def repository(self):
        base = self.controlData.gitrepo_main_site_base
        return '%s/fraxinus.git' % base
    def makeClean(self):
        pass
    def pluginPath(self):
        return '%s/org.custusx.fraxinus' % self.sourcePath()
    def addConfigurationToDownstreamLib(self, builder):
        add = builder.addCMakeOption
        add('CX_APP_CustusX:BOOL', 'OFF');
        add('CX_APP_Fraxinus:BOOL', 'ON');
        add('CX_OPTIONAL_CONFIG_ROOT:PATH', '%s/config'%self.sourcePath());
        '''Replacing the default state plugin from CustusX with Fraxinus specific setup'''
        add('CX_PLUGIN_org.custusx.core.state', 'OFF')
        add('CX_EXTERNAL_PLUGIN_org_custusx_fraxinus_core_state', '%s/org.custusx.fraxinus.core.state' % self.sourcePath())
        add('CX_EXTERNAL_PLUGIN_org_custusx_fraxinus_widgets', '%s/org.custusx.fraxinus.widgets' % self.sourcePath())

# ---------------------------------------------------------

class medtekAI(cx.build.cxComponents.CppComponent):
    def name(self):
        return "medtekAI"
    def help(self):
        return 'Machine Learning tools related to CustusS (SINTEF private)'
    #def path(self):
    #    custusx = self._createSibling(cx.build.cxComponents.CustusX)
    #    return '%s/%s' % (custusx.path(), custusx.sourceFolder())
    def sourcePath(self):
        return '%s/%s/%s' % (self.controlData.getWorkingPath(), self.sourceFolder(), self.sourceFolder())
    def sourceFolder(self):
        return 'medtekAI'
    def repository(self):
        #return '%s/medtekAI.git' % self.controlData.gitrepo_main_site_base
        return 'ssh://git@git.code.sintef.no/mt/medtekAI.git'
    def update(self):
        self._getBuilder().gitSetRemoteURL(self.repository())
        self._getBuilder().gitCheckoutSha('d8646bd6968e6d05884e48e04d79bce88f507e1a')
    def configure(self):
        pass
    def build(self):
        pass
    def makeClean(self):
        pass
    def useInIntegrationTesting(self):
        'use during integration test'
        return True
# ---------------------------------------------------------

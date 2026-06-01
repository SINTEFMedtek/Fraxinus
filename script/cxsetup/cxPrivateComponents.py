#!/usr/bin/env python

import platform

import cx.build.cxComponents
from . import cxCustusXFinder

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
        self._getBuilder().gitSetRemoteURL(self.repository())
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

class org_custusx_fraxinus_tracking(cx.build.cxComponents.CppComponent):
    def name(self):
        return "FraxinusTracking"
    def help(self):
        return 'Tracking and navigation in Fraxinus (SINTEF private)'
    def path(self):
        fraxinus = self._createSibling(Fraxinus)
        #return '%s/FX/FX/%s' % (self.controlData.getWorkingPath(), self.sourceFolder())
        return fraxinus.path() + "/" + fraxinus.sourceFolder()
    def sourceFolder(self):
        return 'org.custusx.fraxinus.tracking'
    def repository(self):
        return '%s/org.custusx.fraxinus.tracking.git' % self.controlData.gitrepo_main_site_base
    def update(self):
        self._getBuilder().gitSetRemoteURL(self.repository())
        # self._getBuilder().gitCheckoutSha('c98f39a1f5ab11d24ae9547391f0fd4118a2cc8c')
        self._getBuilder().gitCheckoutDefaultBranch()
    def configure(self):
        pass
    def build(self):
        pass
    def makeClean(self):
        pass
    def useInIntegrationTesting(self):
        'use during integration test'
        return True
    def addConfigurationToDownstreamLib(self, builder):
        add = builder.addCMakeOption
        add('CX_FRAXINUS_TRACKING:BOOL', 'ON');
        add('CX_EXTERNAL_PLUGIN_org_custusx_fraxinus_tracking', self.path() + '/' + self.sourceFolder())
# ---------------------------------------------------------

class org_custusx_ussimulator(cx.build.cxComponents.CppComponent):

    def name(self):
        return "USSimulator"
    def help(self):
        return 'Plugin for simulating streaming of ultrasound images from any kinda of volume.'
    def path(self):
        custusx = self._createSibling(cx.build.cxComponents.CustusX)
        return '%s/%s/source/plugins' % (custusx.path(), custusx.sourceFolder())
#        return self.controlData.getWorkingPath() + "/CustusX/CustusX/source/plugins"
    def sourceFolder(self):
        return 'org.custusx.ussimulator'
    #def _rawCheckout(self):
    #    self._getBuilder().gitClone(self.gitRepository(), self.sourceFolder())
    def update(self):
        self._getBuilder().gitSetRemoteURL(self.repository())
        self._getBuilder().gitCheckout('fcafd0a547e36c0c5a5d9a2067eec8385360f71e')
    def configure(self):
        pass
    def build(self):
        pass
    def repository(self):
        base = self.controlData.gitrepo_main_site_base
        return '%s/org.custusx.ussimulator.git' % base
    def makeClean(self):
        pass
    def pluginPath(self):
        return '%s' % self.sourcePath()
    def addConfigurationToDownstreamLib(self, builder):
        add = builder.addCMakeOption
        add('CX_PLUGIN_org.custusx.ussimulator:BOOL', 'ON');
# ---------------------------------------------------------

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
        return cxCustusXFinder.CustusXFinder().getPrivateRepoFolder()
    def _rawCheckout(self):
        self._getBuilder().gitClone(self.gitRepository(), self.sourceFolder())
    def update(self):
        self._getBuilder().gitCheckoutDefaultBranch()    
    def configure(self):
        pass
    def build(self):
        pass
    def gitRepository(self):
        base = self.controlData.gitrepo_open_site_base
        return '%s/Fraxinus.git' % base
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
        add('CX_PLUGIN_org.custusx.filter.tubesegmentation', 'ON')
        add('CX_PLUGIN_org.custusx.filter.airways', 'ON')

# ---------------------------------------------------------

class org_custusx_virtualbronchoscopy(cx.build.cxComponents.CppComponent):

    def name(self):
        return "org.custusx.virtualbronchoscopy"
    def help(self):
        return 'Plugin virtualbronchoscopy'
    def path(self):
        custusx = self._createSibling(cx.build.cxComponents.CustusX)
        return '%s/%s/source/plugins' % (custusx.path(), custusx.sourceFolder())
    def sourceFolder(self):
        return 'org.custusx.virtualbronchoscopy'
    def _rawCheckout(self):
        self._getBuilder().gitClone(self.gitRepository(), self.sourceFolder())
    def update(self):
        self._getBuilder().gitCheckoutBranch('feature/Virtual_Bronchoscopy')
    def configure(self):
        pass
    def build(self):
        pass
    def gitRepository(self):
        base = self.controlData.gitrepo_open_site_base
        return '%s/org.custusx.virtualbronchoscopy.git' % base
    def makeClean(self):
        pass
    def pluginPath(self):
        return '%s' % self.sourcePath()

# ---------------------------------------------------------

class org_custusx_bronchoscopynavigation(cx.build.cxComponents.CppComponent):

    def name(self):
        return "org.custusx.bronchoscopynavigation"
    def help(self):
        return 'Plugin bronchoscopynavigation'
    def path(self):
        custusx = self._createSibling(cx.build.cxComponents.CustusX)
        return '%s/%s/source/plugins' % (custusx.path(), custusx.sourceFolder())
    def sourceFolder(self):
        return 'org.custusx.bronchoscopynavigation'
    def _rawCheckout(self):
        self._getBuilder().gitClone(self.gitRepository(), self.sourceFolder())
    def update(self):
        self._getBuilder().gitCheckoutDefaultBranch()
    def configure(self):
        pass
    def build(self):
        pass
    def gitRepository(self):
        base = self.controlData.gitrepo_open_site_base
        return '%s/org.custusx.bronchoscopynavigation.git' % base
    def makeClean(self):
        pass
    def pluginPath(self):
        return '%s' % self.sourcePath()

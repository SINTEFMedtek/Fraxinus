#!/usr/bin/env python

import argparse

import cxsetup.cxFindCustusX

import cx.script.cxInstallScript
import cxsetup.cxPrivateComponentAssembly

class Controller(cx.script.cxInstallScript.Controller):
    def __init__(self, assembly=None):
        ''
        # Pre-parse --igstk / --skip_igstk before LibraryAssembly is assembled so
        # that an explicit CLI flag overrides the OS auto-detection in
        # PrivateControlData._igstk_supported(). Both flags are registered here
        # regardless of platform so either direction works on any machine.
        _pre = argparse.ArgumentParser(add_help=False)
        _pre.add_argument('--igstk', action='store_true', dest='igstk_on', default=False)
        _pre.add_argument('--skip_igstk', action='store_true', dest='igstk_off', default=False)
        _pre_args, _ = _pre.parse_known_args()

        controlData = cxsetup.cxPrivateComponentAssembly.PrivateControlData()
        if _pre_args.igstk_on:
            controlData.mBuildIGSTK = True
        elif _pre_args.igstk_off:
            controlData.mBuildIGSTK = False
        # Neither flag → keep the value auto-detected by _igstk_supported()

        assembly = cxsetup.cxPrivateComponentAssembly.LibraryAssembly(controlData)
        super(Controller, self).__init__(assembly)

if __name__ == '__main__':
    controller = Controller()
    controller.run()



#
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Test for the fdt modules

import os
import sys
import tempfile
import unittest

from fdt_select import FdtScan
import fdt_util
import tools

class TestFdt(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self._binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._indir = tempfile.mkdtemp(prefix='binmant.')
        tools.PrepareOutputDir(self._indir, True)

    def TestFile(self, fname):
        return os.path.join(self._binman_dir, 'test', fname)

    def GetCompiled(self, fname):
        return fdt_util.EnsureCompiled(self.TestFile(fname))

    def _DeleteProp(self, fdt):
        node = fdt.GetNode('/microcode/update@0')
        node.DeleteProp('data')

    def testFdtNormal(self):
        fname = self.GetCompiled('34_x86_ucode.dts')
        fdt = FdtScan(fname)
        self._DeleteProp(fdt)

    def testFdtFallback(self):
        fname = self.GetCompiled('34_x86_ucode.dts')
        fdt = FdtScan(fname, True)
        fdt.GetProp('/microcode/update@0', 'data')
        self.assertEqual('fred',
            fdt.GetProp('/microcode/update@0', 'none', default='fred'))
        self.assertEqual('12345678 12345679',
            fdt.GetProp('/microcode/update@0', 'data', typespec='x'))
        self._DeleteProp(fdt)

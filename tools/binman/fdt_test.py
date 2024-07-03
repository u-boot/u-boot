# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the fdt modules

import os
import sys
import tempfile
import unittest

from dtoc import fdt
from dtoc import fdt_util
from dtoc.fdt import FdtScan
from u_boot_pylib import tools

class TestFdt(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self._binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        self._indir = tempfile.mkdtemp(prefix='binmant.')
        tools.prepare_output_dir(self._indir, True)

    @classmethod
    def tearDownClass(self):
        tools._finalise_for_test()

    def TestFile(self, fname):
        return os.path.join(self._binman_dir, 'test', fname)

    def GetCompiled(self, fname):
        return fdt_util.EnsureCompiled(self.TestFile(fname))

    def _DeleteProp(self, dt):
        node = dt.GetNode('/microcode/update@0')
        node.DeleteProp('data')

    def testFdtNormal(self):
        fname = self.GetCompiled('034_x86_ucode.dts')
        dt = FdtScan(fname)
        self._DeleteProp(dt)

    def testFdtNormalProp(self):
        fname = self.GetCompiled('045_prop_test.dts')
        dt = FdtScan(fname)
        node = dt.GetNode('/binman/intel-me')
        self.assertEqual('intel-me', node.name)
        val = fdt_util.GetString(node, 'filename')
        self.assertEqual(str, type(val))
        self.assertEqual('me.bin', val)

        prop = node.props['intval']
        self.assertEqual(fdt.Type.INT, prop.type)
        self.assertEqual(3, fdt_util.GetInt(node, 'intval'))

        prop = node.props['intarray']
        self.assertEqual(fdt.Type.INT, prop.type)
        self.assertEqual(list, type(prop.value))
        self.assertEqual(2, len(prop.value))
        self.assertEqual([5, 6],
                          [fdt_util.fdt32_to_cpu(val) for val in prop.value])

        prop = node.props['byteval']
        self.assertEqual(fdt.Type.BYTE, prop.type)
        self.assertEqual(chr(8), prop.value)

        prop = node.props['bytearray']
        self.assertEqual(fdt.Type.BYTE, prop.type)
        self.assertEqual(list, type(prop.value))
        self.assertEqual(str, type(prop.value[0]))
        self.assertEqual(3, len(prop.value))
        self.assertEqual([chr(1), '#', '4'], prop.value)

        prop = node.props['longbytearray']
        self.assertEqual(fdt.Type.INT, prop.type)
        self.assertEqual(0x090a0b0c, fdt_util.GetInt(node, 'longbytearray'))

        prop = node.props['stringval']
        self.assertEqual(fdt.Type.STRING, prop.type)
        self.assertEqual('message2', fdt_util.GetString(node, 'stringval'))

        prop = node.props['stringarray']
        self.assertEqual(fdt.Type.STRING, prop.type)
        self.assertEqual(list, type(prop.value))
        self.assertEqual(3, len(prop.value))
        self.assertEqual(['another', 'multi-word', 'message'], prop.value)

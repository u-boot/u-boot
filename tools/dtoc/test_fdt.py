#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

from optparse import OptionParser
import glob
import os
import sys
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
for dirname in ['../patman', '..']:
    sys.path.insert(0, os.path.join(our_path, dirname))

import command
import fdt
from fdt import TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL
from fdt_util import fdt32_to_cpu
import libfdt
import test_util
import tools

class TestFdt(unittest.TestCase):
    """Tests for the Fdt module

    This includes unit tests for some functions and functional tests for the fdt
    module.
    """
    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools._FinaliseForTest()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')

    def testFdt(self):
        """Test that we can open an Fdt"""
        self.dtb.Scan()
        root = self.dtb.GetRoot()
        self.assertTrue(isinstance(root, fdt.Node))

    def testGetNode(self):
        """Test the GetNode() method"""
        node = self.dtb.GetNode('/spl-test')
        self.assertTrue(isinstance(node, fdt.Node))
        node = self.dtb.GetNode('/i2c@0/pmic@9')
        self.assertTrue(isinstance(node, fdt.Node))
        self.assertEqual('pmic@9', node.name)

    def testFlush(self):
        """Check that we can flush the device tree out to its file"""
        fname = self.dtb._fname
        with open(fname) as fd:
            data = fd.read()
        os.remove(fname)
        with self.assertRaises(IOError):
            open(fname)
        self.dtb.Flush()
        with open(fname) as fd:
            data = fd.read()

    def testPack(self):
        """Test that packing a device tree works"""
        self.dtb.Pack()

    def testGetFdt(self):
        """Tetst that we can access the raw device-tree data"""
        self.assertTrue(isinstance(self.dtb.GetFdt(), bytearray))

    def testGetProps(self):
        """Tests obtaining a list of properties"""
        node = self.dtb.GetNode('/spl-test')
        props = self.dtb.GetProps(node)
        self.assertEqual(['boolval', 'bytearray', 'byteval', 'compatible',
                          'intarray', 'intval', 'longbytearray',
                          'stringarray', 'stringval', 'u-boot,dm-pre-reloc'],
                         sorted(props.keys()))

    def testCheckError(self):
        """Tests the ChecKError() function"""
        with self.assertRaises(ValueError) as e:
            self.dtb.CheckErr(-libfdt.NOTFOUND, 'hello')
        self.assertIn('FDT_ERR_NOTFOUND: hello', str(e.exception))


class TestNode(unittest.TestCase):
    """Test operation of the Node class"""

    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools._FinaliseForTest()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')
        self.node = self.dtb.GetNode('/spl-test')

    def testOffset(self):
        """Tests that we can obtain the offset of a node"""
        self.assertTrue(self.node.Offset() > 0)

    def testDelete(self):
        """Tests that we can delete a property"""
        node2 = self.dtb.GetNode('/spl-test2')
        offset1 = node2.Offset()
        self.node.DeleteProp('intval')
        offset2 = node2.Offset()
        self.assertTrue(offset2 < offset1)
        self.node.DeleteProp('intarray')
        offset3 = node2.Offset()
        self.assertTrue(offset3 < offset2)

    def testFindNode(self):
        """Tests that we can find a node using the _FindNode() functoin"""
        node = self.dtb.GetRoot()._FindNode('i2c@0')
        self.assertEqual('i2c@0', node.name)
        subnode = node._FindNode('pmic@9')
        self.assertEqual('pmic@9', subnode.name)


class TestProp(unittest.TestCase):
    """Test operation of the Prop class"""

    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools._FinaliseForTest()

    def setUp(self):
        self.dtb = fdt.FdtScan('tools/dtoc/dtoc_test_simple.dts')
        self.node = self.dtb.GetNode('/spl-test')
        self.fdt = self.dtb.GetFdtObj()

    def testGetEmpty(self):
        """Tests the GetEmpty() function for the various supported types"""
        self.assertEqual(True, fdt.Prop.GetEmpty(fdt.TYPE_BOOL))
        self.assertEqual(chr(0), fdt.Prop.GetEmpty(fdt.TYPE_BYTE))
        self.assertEqual(chr(0) * 4, fdt.Prop.GetEmpty(fdt.TYPE_INT))
        self.assertEqual('', fdt.Prop.GetEmpty(fdt.TYPE_STRING))

    def testGetOffset(self):
        """Test we can get the offset of a property"""
        prop = self.node.props['longbytearray']

        # Add 12, which is sizeof(struct fdt_property), to get to start of data
        offset = prop.GetOffset() + 12
        data = self.dtb._fdt[offset:offset + len(prop.value)]
        bytes = [chr(x) for x in data]
        self.assertEqual(bytes, prop.value)

    def testWiden(self):
        """Test widening of values"""
        node2 = self.dtb.GetNode('/spl-test2')
        prop = self.node.props['intval']

        # No action
        prop2 = node2.props['intval']
        prop.Widen(prop2)
        self.assertEqual(fdt.TYPE_INT, prop.type)
        self.assertEqual(1, fdt32_to_cpu(prop.value))

        # Convert singla value to array
        prop2 = self.node.props['intarray']
        prop.Widen(prop2)
        self.assertEqual(fdt.TYPE_INT, prop.type)
        self.assertTrue(isinstance(prop.value, list))

        # A 4-byte array looks like a single integer. When widened by a longer
        # byte array, it should turn into an array.
        prop = self.node.props['longbytearray']
        prop2 = node2.props['longbytearray']
        self.assertFalse(isinstance(prop2.value, list))
        self.assertEqual(4, len(prop2.value))
        prop2.Widen(prop)
        self.assertTrue(isinstance(prop2.value, list))
        self.assertEqual(9, len(prop2.value))

        # Similarly for a string array
        prop = self.node.props['stringval']
        prop2 = node2.props['stringarray']
        self.assertFalse(isinstance(prop.value, list))
        self.assertEqual(7, len(prop.value))
        prop.Widen(prop2)
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(3, len(prop.value))

        # Enlarging an existing array
        prop = self.node.props['stringarray']
        prop2 = node2.props['stringarray']
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(2, len(prop.value))
        prop.Widen(prop2)
        self.assertTrue(isinstance(prop.value, list))
        self.assertEqual(3, len(prop.value))


def RunTests(args):
    """Run all the test we have for the fdt model

    Args:
        args: List of positional args provided to fdt. This can hold a test
            name to execute (as in 'fdt -t testFdt', for example)
    """
    result = unittest.TestResult()
    sys.argv = [sys.argv[0]]
    test_name = args and args[0] or None
    for module in (TestFdt, TestNode, TestProp):
        if test_name:
            try:
                suite = unittest.TestLoader().loadTestsFromName(test_name, module)
            except AttributeError:
                continue
        else:
            suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    print result
    for _, err in result.errors:
        print err
    for _, err in result.failures:
        print err

if __name__ != '__main__':
    sys.exit(1)

parser = OptionParser()
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
(options, args) = parser.parse_args()

# Run our meagre tests
if options.test:
    RunTests(args)

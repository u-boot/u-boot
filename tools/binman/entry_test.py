# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Test for the Entry class

import collections
import os
import sys
import unittest

import entry
import fdt
import fdt_util
import tools

class TestEntry(unittest.TestCase):
    def setUp(self):
        tools.PrepareOutputDir(None)

    def tearDown(self):
        tools.FinaliseOutputDir()

    def GetNode(self):
        binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        fname = fdt_util.EnsureCompiled(
            os.path.join(binman_dir,('test/005_simple.dts')))
        dtb = fdt.FdtScan(fname)
        return dtb.GetNode('/binman/u-boot')

    def _ReloadEntry(self):
        global entry
        if entry:
            if sys.version_info[0] >= 3:
                import importlib
                importlib.reload(entry)
            else:
                reload(entry)
        else:
            import entry

    def test1EntryNoImportLib(self):
        """Test that we can import Entry subclassess successfully"""
        sys.modules['importlib'] = None
        global entry
        self._ReloadEntry()
        entry.Entry.Create(None, self.GetNode(), 'u-boot')
        self.assertFalse(entry.have_importlib)

    def test2EntryImportLib(self):
        del sys.modules['importlib']
        global entry
        self._ReloadEntry()
        entry.Entry.Create(None, self.GetNode(), 'u-boot-spl')
        self.assertTrue(entry.have_importlib)

    def testEntryContents(self):
        """Test the Entry bass class"""
        import entry
        base_entry = entry.Entry(None, None, None)
        self.assertEqual(True, base_entry.ObtainContents())

    def testUnknownEntry(self):
        """Test that unknown entry types are detected"""
        Node = collections.namedtuple('Node', ['name', 'path'])
        node = Node('invalid-name', 'invalid-path')
        with self.assertRaises(ValueError) as e:
            entry.Entry.Create(None, node, node.name)
        self.assertIn("Unknown entry type 'invalid-name' in node "
                      "'invalid-path'", str(e.exception))

    def testUniqueName(self):
        """Test Entry.GetUniqueName"""
        Node = collections.namedtuple('Node', ['name', 'parent'])
        base_node = Node('root', None)
        base_entry = entry.Entry(None, None, base_node)
        self.assertEqual('root', base_entry.GetUniqueName())
        sub_node = Node('subnode', base_node)
        sub_entry = entry.Entry(None, None, sub_node)
        self.assertEqual('root.subnode', sub_entry.GetUniqueName())

    def testGetDefaultFilename(self):
        """Trivial test for this base class function"""
        base_entry = entry.Entry(None, None, None)
        self.assertIsNone(base_entry.GetDefaultFilename())

    def testBlobFdt(self):
        """Test the GetFdtEtype() method of the blob-dtb entries"""
        base = entry.Entry.Create(None, self.GetNode(), 'blob-dtb')
        self.assertIsNone(base.GetFdtEtype())

        dtb = entry.Entry.Create(None, self.GetNode(), 'u-boot-dtb')
        self.assertEqual('u-boot-dtb', dtb.GetFdtEtype())

    def testWriteChildData(self):
        """Test the WriteChildData() method of the base class"""
        base = entry.Entry.Create(None, self.GetNode(), 'blob-dtb')
        self.assertTrue(base.WriteChildData(base))


if __name__ == "__main__":
    unittest.main()

#
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Test for the Entry class

import collections
import os
import sys
import unittest

import fdt
import fdt_util
import tools

class TestEntry(unittest.TestCase):
    def GetNode(self):
        binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
        tools.PrepareOutputDir(None)
        fname = fdt_util.EnsureCompiled(
            os.path.join(binman_dir,('test/05_simple.dts')))
        dtb = fdt.FdtScan(fname)
        return dtb.GetNode('/binman/u-boot')

    def test1EntryNoImportLib(self):
        """Test that we can import Entry subclassess successfully"""

        sys.modules['importlib'] = None
        global entry
        import entry
        entry.Entry.Create(None, self.GetNode(), 'u-boot')

    def test2EntryImportLib(self):
        del sys.modules['importlib']
        global entry
        reload(entry)
        entry.Entry.Create(None, self.GetNode(), 'u-boot-spl')
        tools._RemoveOutputDir()
        del entry

    def testEntryContents(self):
        """Test the Entry bass class"""
        import entry
        base_entry = entry.Entry(None, None, None, read_node=False)
        self.assertEqual(True, base_entry.ObtainContents())

    def testUnknownEntry(self):
        """Test that unknown entry types are detected"""
        import entry
        Node = collections.namedtuple('Node', ['name', 'path'])
        node = Node('invalid-name', 'invalid-path')
        with self.assertRaises(ValueError) as e:
            entry.Entry.Create(None, node, node.name)
        self.assertIn("Unknown entry type 'invalid-name' in node "
                      "'invalid-path'", str(e.exception))


if __name__ == "__main__":
    unittest.main()

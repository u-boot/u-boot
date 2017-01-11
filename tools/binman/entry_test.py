#
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Test for the Entry class

import collections
import unittest

import entry

class TestEntry(unittest.TestCase):
    def testEntryContents(self):
        """Test the Entry bass class"""
        base_entry = entry.Entry(None, None, None, read_node=False)
        self.assertEqual(True, base_entry.ObtainContents())

    def testUnknownEntry(self):
        """Test that unknown entry types are detected"""
        Node = collections.namedtuple('Node', ['name', 'path'])
        node = Node('invalid-name', 'invalid-path')
        with self.assertRaises(ValueError) as e:
            entry.Entry.Create(None, node, node.name)
        self.assertIn("Unknown entry type 'invalid-name' in node "
                      "'invalid-path'", str(e.exception))

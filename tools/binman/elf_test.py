#
# Copyright (c) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Test for the elf module

import os
import sys
import unittest

import elf

binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
fname = os.path.join(binman_dir, 'test', 'u_boot_ucode_ptr')

class TestElf(unittest.TestCase):
    def testAllSymbols(self):
        syms = elf.GetSymbols(fname, [])
        self.assertIn('.ucode', syms)

    def testRegexSymbols(self):
        syms = elf.GetSymbols(fname, ['ucode'])
        self.assertIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing'])
        self.assertNotIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing', 'ucode'])
        self.assertIn('.ucode', syms)

if __name__ == '__main__':
    unittest.main()

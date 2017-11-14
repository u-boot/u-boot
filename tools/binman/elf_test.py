#
# Copyright (c) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Test for the elf module

from contextlib import contextmanager
import os
import sys
import unittest

try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO

import elf

binman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))

# Use this to suppress stdout/stderr output:
# with capture_sys_output() as (stdout, stderr)
#   ...do something...
@contextmanager
def capture_sys_output():
  capture_out, capture_err = StringIO(), StringIO()
  old_out, old_err = sys.stdout, sys.stderr
  try:
    sys.stdout, sys.stderr = capture_out, capture_err
    yield capture_out, capture_err
  finally:
    sys.stdout, sys.stderr = old_out, old_err


class FakeEntry:
    def __init__(self, contents_size):
        self.contents_size = contents_size
        self.data = 'a' * contents_size

    def GetPath(self):
        return 'entry_path'

class FakeImage:
    def __init__(self, sym_value=1):
        self.sym_value = sym_value

    def GetPath(self):
        return 'image_path'

    def LookupSymbol(self, name, weak, msg):
        return self.sym_value

class TestElf(unittest.TestCase):
    def testAllSymbols(self):
        fname = os.path.join(binman_dir, 'test', 'u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, [])
        self.assertIn('.ucode', syms)

    def testRegexSymbols(self):
        fname = os.path.join(binman_dir, 'test', 'u_boot_ucode_ptr')
        syms = elf.GetSymbols(fname, ['ucode'])
        self.assertIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing'])
        self.assertNotIn('.ucode', syms)
        syms = elf.GetSymbols(fname, ['missing', 'ucode'])
        self.assertIn('.ucode', syms)

    def testMissingFile(self):
        entry = FakeEntry(10)
        image = FakeImage()
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols('missing-file', entry, image)
        self.assertIn("Filename 'missing-file' not found in input path",
                      str(e.exception))

    def testOutsideFile(self):
        entry = FakeEntry(10)
        image = FakeImage()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, image)
        self.assertIn('entry_path has offset 4 (size 8) but the contents size '
                      'is a', str(e.exception))

    def testMissingImageStart(self):
        entry = FakeEntry(10)
        image = FakeImage()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms_bad')
        self.assertEqual(elf.LookupAndWriteSymbols(elf_fname, entry, image),
                         None)

    def testBadSymbolSize(self):
        entry = FakeEntry(10)
        image = FakeImage()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms_size')
        with self.assertRaises(ValueError) as e:
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, image)
        self.assertIn('has size 1: only 4 and 8 are supported',
                      str(e.exception))

    def testNoValue(self):
        entry = FakeEntry(20)
        image = FakeImage(sym_value=None)
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        syms = elf.LookupAndWriteSymbols(elf_fname, entry, image)
        self.assertEqual(chr(255) * 16 + 'a' * 4, entry.data)

    def testDebug(self):
        elf.debug = True
        entry = FakeEntry(20)
        image = FakeImage()
        elf_fname = os.path.join(binman_dir, 'test', 'u_boot_binman_syms')
        with capture_sys_output() as (stdout, stderr):
            syms = elf.LookupAndWriteSymbols(elf_fname, entry, image)
        elf.debug = False
        self.assertTrue(len(stdout.getvalue()) > 0)


if __name__ == '__main__':
    unittest.main()

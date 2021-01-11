# SPDX-License-Identifier: GPL-2.0+
# Copyright 2020 Google LLC
#

"""Tests for the src_scan module

This includes unit tests for scanning of the source code
"""

import os
import shutil
import tempfile
import unittest
from unittest import mock

from dtoc import src_scan
from patman import test_util
from patman import tools

# This is a test so is allowed to access private things in the module it is
# testing
# pylint: disable=W0212

class TestSrcScan(unittest.TestCase):
    """Tests for src_scan"""
    @classmethod
    def setUpClass(cls):
        tools.PrepareOutputDir(None)

    @classmethod
    def tearDownClass(cls):
        tools.FinaliseOutputDir()

    def test_simple(self):
        """Simple test of scanning drivers"""
        scan = src_scan.Scanner(None, True, None)
        scan.scan_drivers()
        self.assertIn('sandbox_gpio', scan._drivers)
        self.assertIn('sandbox_gpio_alias', scan._driver_aliases)
        self.assertEqual('sandbox_gpio',
                         scan._driver_aliases['sandbox_gpio_alias'])
        self.assertNotIn('sandbox_gpio_alias2', scan._driver_aliases)

    def test_additional(self):
        """Test with additional drivers to scan"""
        scan = src_scan.Scanner(
            None, True, [None, '', 'tools/dtoc/dtoc_test_scan_drivers.cxx'])
        scan.scan_drivers()
        self.assertIn('sandbox_gpio_alias2', scan._driver_aliases)
        self.assertEqual('sandbox_gpio',
                         scan._driver_aliases['sandbox_gpio_alias2'])

    def test_unicode_error(self):
        """Test running dtoc with an invalid unicode file

        To be able to perform this test without adding a weird text file which
        would produce issues when using checkpatch.pl or patman, generate the
        file at runtime and then process it.
        """
        driver_fn = '/tmp/' + next(tempfile._get_candidate_names())
        with open(driver_fn, 'wb+') as fout:
            fout.write(b'\x81')

        scan = src_scan.Scanner(None, True, [driver_fn])
        with test_util.capture_sys_output() as (stdout, _):
            scan.scan_drivers()
        self.assertRegex(stdout.getvalue(),
                         r"Skipping file '.*' due to unicode error\s*")

    def test_driver(self):
        """Test the Driver class"""
        drv1 = src_scan.Driver('fred')
        drv2 = src_scan.Driver('mary')
        drv3 = src_scan.Driver('fred')
        self.assertEqual("Driver(name='fred')", str(drv1))
        self.assertEqual(drv1, drv3)
        self.assertNotEqual(drv1, drv2)
        self.assertNotEqual(drv2, drv3)

    def test_scan_dirs(self):
        """Test scanning of source directories"""
        def add_file(fname):
            pathname = os.path.join(indir, fname)
            dirname = os.path.dirname(pathname)
            os.makedirs(dirname, exist_ok=True)
            tools.WriteFile(pathname, '', binary=False)
            fname_list.append(pathname)

        try:
            indir = tempfile.mkdtemp(prefix='dtoc.')

            fname_list = []
            add_file('fname.c')
            add_file('dir/fname2.c')

            # Mock out scan_driver and check that it is called with the
            # expected files
            with mock.patch.object(src_scan.Scanner, "scan_driver")  as mocked:
                scan = src_scan.Scanner(indir, True, None)
                scan.scan_drivers()
            self.assertEqual(2, len(mocked.mock_calls))
            self.assertEqual(mock.call(fname_list[0]),
                             mocked.mock_calls[0])
            self.assertEqual(mock.call(fname_list[1]),
                             mocked.mock_calls[1])
        finally:
            shutil.rmtree(indir)

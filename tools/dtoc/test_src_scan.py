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

OUR_PATH = os.path.dirname(os.path.realpath(__file__))

class FakeNode:
    """Fake Node object for testing"""
    def __init__(self):
        self.name = None
        self.props = {}

class FakeProp:
    """Fake Prop object for testing"""
    def __init__(self):
        self.name = None
        self.value = None

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
        i2c = 'I2C_UCLASS'
        compat = {'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF',
                  'rockchip,rk3288-srf': None}
        drv1 = src_scan.Driver('fred', 'fred.c')
        drv2 = src_scan.Driver('mary', 'mary.c')
        drv3 = src_scan.Driver('fred', 'fred.c')
        drv1.uclass_id = i2c
        drv1.compat = compat
        drv2.uclass_id = i2c
        drv2.compat = compat
        drv3.uclass_id = i2c
        drv3.compat = compat
        self.assertEqual(
            "Driver(name='fred', uclass_id='I2C_UCLASS', "
            "compat={'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF', "
            "'rockchip,rk3288-srf': None}, priv=)", str(drv1))
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

    def test_scan(self):
        """Test scanning of a driver"""
        fname = os.path.join(OUR_PATH, '..', '..', 'drivers/i2c/tegra_i2c.c')
        buff = tools.ReadFile(fname, False)
        scan = src_scan.Scanner(None, False, None)
        scan._parse_driver(fname, buff)
        self.assertIn('i2c_tegra', scan._drivers)
        drv = scan._drivers['i2c_tegra']
        self.assertEqual('i2c_tegra', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual(
            {'nvidia,tegra114-i2c': 'TYPE_114',
             'nvidia,tegra20-i2c': 'TYPE_STD',
             'nvidia,tegra20-i2c-dvc': 'TYPE_DVC'}, drv.compat)
        self.assertEqual('i2c_bus', drv.priv)
        self.assertEqual(1, len(scan._drivers))

    def test_normalized_name(self):
        """Test operation of get_normalized_compat_name()"""
        prop = FakeProp()
        prop.name = 'compatible'
        prop.value = 'rockchip,rk3288-grf'
        node = FakeNode()
        node.props = {'compatible': prop}
        scan = src_scan.Scanner(None, False, None)
        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual([], aliases)
        self.assertEqual(
            'WARNING: the driver rockchip_rk3288_grf was not found in the driver list',
            stdout.getvalue().strip())

        i2c = 'I2C_UCLASS'
        compat = {'rockchip,rk3288-grf': 'ROCKCHIP_SYSCON_GRF',
                  'rockchip,rk3288-srf': None}
        drv = src_scan.Driver('fred', 'fred.c')
        drv.uclass_id = i2c
        drv.compat = compat
        scan._drivers['rockchip_rk3288_grf'] = drv

        scan._driver_aliases['rockchip_rk3288_srf'] = 'rockchip_rk3288_grf'

        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('', stdout.getvalue().strip())
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual([], aliases)

        prop.value = 'rockchip,rk3288-srf'
        with test_util.capture_sys_output() as (stdout, _):
            name, aliases = scan.get_normalized_compat_name(node)
        self.assertEqual('', stdout.getvalue().strip())
        self.assertEqual('rockchip_rk3288_grf', name)
        self.assertEqual(['rockchip_rk3288_srf'], aliases)

    def test_scan_errors(self):
        """Test detection of scanning errors"""
        buff = '''
static const struct udevice_id tegra_i2c_ids2[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = tegra_i2c_ids,
};
'''
        scan = src_scan.Scanner(None, False, None)
        with self.assertRaises(ValueError) as exc:
            scan._parse_driver('file.c', buff)
        self.assertIn(
            "file.c: Unknown compatible var 'tegra_i2c_ids' (found: tegra_i2c_ids2)",
            str(exc.exception))

    def test_of_match(self):
        """Test detection of of_match_ptr() member"""
        buff = '''
static const struct udevice_id tegra_i2c_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(i2c_tegra) = {
	.name	= "i2c_tegra",
	.id	= UCLASS_I2C,
	.of_match = of_match_ptr(tegra_i2c_ids),
};
'''
        scan = src_scan.Scanner(None, False, None)
        scan._parse_driver('file.c', buff)
        self.assertIn('i2c_tegra', scan._drivers)
        drv = scan._drivers['i2c_tegra']
        self.assertEqual('i2c_tegra', drv.name)

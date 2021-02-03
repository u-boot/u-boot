# SPDX-License-Identifier: GPL-2.0+
# Copyright 2020 Google LLC
#

"""Tests for the src_scan module

This includes unit tests for scanning of the source code
"""

import copy
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
            None, True,
            [None, '', 'tools/dtoc/test/dtoc_test_scan_drivers.cxx'])
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
            add_file('.git/ignoreme.c')
            add_file('dir/fname2.c')
            add_file('build-sandbox/ignoreme2.c')

            # Mock out scan_driver and check that it is called with the
            # expected files
            with mock.patch.object(src_scan.Scanner, "scan_driver")  as mocked:
                scan = src_scan.Scanner(indir, True, None)
                scan.scan_drivers()
            self.assertEqual(2, len(mocked.mock_calls))
            self.assertEqual(mock.call(fname_list[0]),
                             mocked.mock_calls[0])
            # .git file should be ignored
            self.assertEqual(mock.call(fname_list[2]),
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

    def test_priv(self):
        """Test collection of struct info from drivers"""
        buff = '''
static const struct udevice_id test_ids[] = {
	{ .compatible = "nvidia,tegra114-i2c", .data = TYPE_114 },
	{ }
};

U_BOOT_DRIVER(testing) = {
	.name	= "testing",
	.id	= UCLASS_I2C,
	.of_match = test_ids,
	.priv_auto	= sizeof(struct some_priv),
	.plat_auto = sizeof(struct some_plat),
	.per_child_auto	= sizeof(struct some_cpriv),
	.per_child_plat_auto = sizeof(struct some_cplat),
};
'''
        scan = src_scan.Scanner(None, False, None)
        scan._parse_driver('file.c', buff)
        self.assertIn('testing', scan._drivers)
        drv = scan._drivers['testing']
        self.assertEqual('testing', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual(
            {'nvidia,tegra114-i2c': 'TYPE_114'}, drv.compat)
        self.assertEqual('some_priv', drv.priv)
        self.assertEqual('some_plat', drv.plat)
        self.assertEqual('some_cpriv', drv.child_priv)
        self.assertEqual('some_cplat', drv.child_plat)
        self.assertEqual(1, len(scan._drivers))

    def test_uclass_scan(self):
        """Test collection of uclass-driver info"""
        buff = '''
UCLASS_DRIVER(i2c) = {
	.id		= UCLASS_I2C,
	.name		= "i2c",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.priv_auto	= sizeof(struct some_priv),
	.per_device_auto	= sizeof(struct per_dev_priv),
	.per_device_plat_auto	= sizeof(struct per_dev_plat),
	.per_child_auto	= sizeof(struct per_child_priv),
	.per_child_plat_auto	= sizeof(struct per_child_plat),
	.child_post_bind = i2c_child_post_bind,
};

'''
        scan = src_scan.Scanner(None, False, None)
        scan._parse_uclass_driver('file.c', buff)
        self.assertIn('UCLASS_I2C', scan._uclass)
        drv = scan._uclass['UCLASS_I2C']
        self.assertEqual('i2c', drv.name)
        self.assertEqual('UCLASS_I2C', drv.uclass_id)
        self.assertEqual('some_priv', drv.priv)
        self.assertEqual('per_dev_priv', drv.per_dev_priv)
        self.assertEqual('per_dev_plat', drv.per_dev_plat)
        self.assertEqual('per_child_priv', drv.per_child_priv)
        self.assertEqual('per_child_plat', drv.per_child_plat)
        self.assertEqual(1, len(scan._uclass))

        drv2 = copy.deepcopy(drv)
        self.assertEqual(drv, drv2)
        drv2.priv = 'other_priv'
        self.assertNotEqual(drv, drv2)

        # The hashes only depend on the uclass ID, so should be equal
        self.assertEqual(drv.__hash__(), drv2.__hash__())

        self.assertEqual("UclassDriver(name='i2c', uclass_id='UCLASS_I2C')",
                         str(drv))

    def test_uclass_scan_errors(self):
        """Test detection of uclass scanning errors"""
        buff = '''
UCLASS_DRIVER(i2c) = {
	.name		= "i2c",
};

'''
        scan = src_scan.Scanner(None, False, None)
        with self.assertRaises(ValueError) as exc:
            scan._parse_uclass_driver('file.c', buff)
        self.assertIn("file.c: Cannot parse uclass ID in driver 'i2c'",
                      str(exc.exception))

    def test_struct_scan(self):
        """Test collection of struct info"""
        buff = '''
/* some comment */
struct some_struct1 {
	struct i2c_msg *msgs;
	uint nmsgs;
};
'''
        scan = src_scan.Scanner(None, False, None)
        scan._basedir = os.path.join(OUR_PATH, '..', '..')
        scan._parse_structs('arch/arm/include/asm/file.h', buff)
        self.assertIn('some_struct1', scan._structs)
        struc = scan._structs['some_struct1']
        self.assertEqual('some_struct1', struc.name)
        self.assertEqual('asm/file.h', struc.fname)

        buff = '''
/* another comment */
struct another_struct {
	int speed_hz;
	int max_transaction_bytes;
};
'''
        scan._parse_structs('include/file2.h', buff)
        self.assertIn('another_struct', scan._structs)
        struc = scan._structs['another_struct']
        self.assertEqual('another_struct', struc.name)
        self.assertEqual('file2.h', struc.fname)

        self.assertEqual(2, len(scan._structs))

        self.assertEqual("Struct(name='another_struct', fname='file2.h')",
                         str(struc))

    def test_struct_scan_errors(self):
        """Test scanning a header file with an invalid unicode file"""
        output = tools.GetOutputFilename('output.h')
        tools.WriteFile(output, b'struct this is a test \x81 of bad unicode')

        scan = src_scan.Scanner(None, False, None)
        with test_util.capture_sys_output() as (stdout, _):
            scan.scan_header(output)
        self.assertIn('due to unicode error', stdout.getvalue())

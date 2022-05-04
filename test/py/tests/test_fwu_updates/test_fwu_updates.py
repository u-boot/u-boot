# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2022, Linaro Limited
#
# FWU Multi Bank Firmware Update Test

"""
This test verifies FWU Multi Bank firmware update for raw images
"""

from subprocess import check_call, check_output, CalledProcessError
import pytest
from capsule_defs import *


@pytest.mark.boardspec('sandbox64')
@pytest.mark.buildconfigspec('efi_capsule_firmware_raw')
@pytest.mark.buildconfigspec('efi_capsule_on_disk')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('fwu_multi_bank_update')
@pytest.mark.buildconfigspec('dm_fwu_mdata')
@pytest.mark.buildconfigspec('fwu_mdata_mtd')
@pytest.mark.slow
class TestEfiCapsuleFirmwareRaw(object):
    def test_fwu_updates_fw1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 1 - Update U-Boot Bank 1 binary on SPI Flash
                      0x100000-0x150000: U-Boot binary Bank 0 (but dummy)
                      0x150000-0x200000: U-Boot binary Bank 1 (but dummy)
        """

        # other tests might have run and the
        # system might not be in a clean state.
        # Restart before starting the tests.
        u_boot_console.restart_uboot()

        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env save'])

            # initialize contents
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot-a.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'fatload host 0:1 4000000 %s/u-boot-b.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 140000 10'])

            output = u_boot_console.run_command_list([
                'sf read 5000000 100000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5100000 140000 10',
                'md.b 5100000 10'
                ])
            assert 'u-boot-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/metadata_bank0.bin' % CAPSULE_DATA_DIR,
                'sf write 4000000 0 $filesize',
                'sf write 4000000 10000 $filesize'])

            output = u_boot_console.run_command(
                'fwu_mdata_read')
            assert 'active_index: 0x0' in ''.join(output)
            assert 'previous_active_index: 0x1' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test01' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test01 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            # make sure that dfu_alt_info exists even persistent variables
            # are not available.
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)

            # need to run uefi command to initiate capsule handling
            output = u_boot_console.run_command(
                'efidebug capsule disk-update', wait_for_reboot = True)

            output = u_boot_console.run_command(
                'fwu_mdata_read')
            assert 'active_index: 0x1' in ''.join(output)
            assert 'previous_active_index: 0x0' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 140000 10',
                'md.b 4000000 10'])
            assert 'u-boot-b:New' in ''.join(output)

    def test_fwu_updates_fw2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 2 - Update U-Boot and U-Boot Env Bank 1 binary on SPI Flash
                      0x100000-0x110000: U-Boot binary Bank 0 (but dummy)
                      0x120000-0x130000: U-Boot Env binary Bank 0 (but dummy)
                      0x140000-0x150000: U-Boot binary Bank 1 (but dummy)
                      0x160000-0x170000: U-Boot Env binary Bank 1 (but dummy)
        """

        # other tests might have run and the
        # system might not be in a clean state.
        # Restart before starting the tests.
        u_boot_console.restart_uboot()

        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env save'])

            # initialize contents
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot-a.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'fatload host 0:1 4000000 %s/u-boot-env-a.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 120000 10',
                'fatload host 0:1 4000000 %s/u-boot-b.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 140000 10',
                'fatload host 0:1 4000000 %s/u-boot-env-b.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 160000 10'])

            output = u_boot_console.run_command_list([
                'sf read 5000000 100000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5000000 120000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-env-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5100000 140000 10',
                'md.b 5100000 10'
                ])
            assert 'u-boot-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5000000 160000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-env-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/metadata_bank0.bin' % CAPSULE_DATA_DIR,
                'sf write 4000000 0 100',
                'sf write 4000000 10000 100'])

            output = u_boot_console.run_command(
               'fwu_mdata_read')
            assert 'active_index: 0x0' in ''.join(output)
            assert 'previous_active_index: 0x1' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test01' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test01 $filesize' % CAPSULE_INSTALL_DIR,
                'fatload host 0:1 4000000 %s/Test02' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test02 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)
            assert 'Test02' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            # make sure that dfu_alt_info exists even persistent variables
            # are not available.
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)
            assert 'Test02' in ''.join(output)

            # need to run uefi command to initiate capsule handling
            output = u_boot_console.run_command(
                'efidebug capsule disk-update', wait_for_reboot = True)

            output = u_boot_console.run_command(
                'fwu_mdata_read')
            assert 'active_index: 0x1' in ''.join(output)
            assert 'previous_active_index: 0x0' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 120000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 140000 10',
                'md.b 4000000 10'])
            assert 'u-boot-b:New' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 160000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env-b:New' in ''.join(output)

    def test_fwu_updates_fw3(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 3 - Update U-Boot and U-Boot Env Bank 0 binary on SPI Flash
                      0x100000-0x110000: U-Boot binary Bank 0 (but dummy)
                      0x120000-0x130000: U-Boot Env binary Bank 0 (but dummy)
                      0x140000-0x150000: U-Boot binary Bank 1 (but dummy)
                      0x160000-0x170000: U-Boot Env binary Bank 1 (but dummy)
        """

        # other tests might have run and the
        # system might not be in a clean state.
        # Restart before starting the tests.
        u_boot_console.restart_uboot()

        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 3-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env save'])

            # initialize contents
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot-a.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'fatload host 0:1 4000000 %s/u-boot-env-a.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 120000 10',
                'fatload host 0:1 4000000 %s/u-boot-b.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 140000 10',
                'fatload host 0:1 4000000 %s/u-boot-env-b.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 160000 10'])

            output = u_boot_console.run_command_list([
                'sf read 5000000 100000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5000000 120000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-env-a:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5100000 140000 10',
                'md.b 5100000 10'
                ])
            assert 'u-boot-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 5000000 160000 10',
                'md.b 5000000 10'
                ])
            assert 'u-boot-env-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/metadata_bank1.bin' % CAPSULE_DATA_DIR,
                'sf write 4000000 0 100',
                'sf write 4000000 10000 100'])

            output = u_boot_console.run_command(
                'fwu_mdata_read')
            assert 'active_index: 0x1' in ''.join(output)
            assert 'previous_active_index: 0x0' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test03' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test03 $filesize' % CAPSULE_INSTALL_DIR,
                'fatload host 0:1 4000000 %s/Test04' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test04 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test03' in ''.join(output)
            assert 'Test04' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        with u_boot_console.log.section('Test Case 3-b, after reboot'):
            # make sure that dfu_alt_info exists even persistent variables
            # are not available.
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test03' in ''.join(output)
            assert 'Test04' in ''.join(output)

            # need to run uefi command to initiate capsule handling
            output = u_boot_console.run_command(
                'efidebug capsule disk-update', wait_for_reboot = True)

            output = u_boot_console.run_command(
                'fwu_mdata_read')
            assert 'active_index: 0x0' in ''.join(output)
            assert 'previous_active_index: 0x1' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot-a:New' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 120000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env-a:New' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 140000 10',
                'md.b 4000000 10'])
            assert 'u-boot-b:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 160000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env-b:Old' in ''.join(output)

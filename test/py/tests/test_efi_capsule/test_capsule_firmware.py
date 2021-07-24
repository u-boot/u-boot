# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Firmware Update Test

"""
This test verifies capsule-on-disk firmware update
"""

from subprocess import check_call, check_output, CalledProcessError
import pytest
from capsule_defs import *


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_capsule_firmware_fit')
@pytest.mark.buildconfigspec('efi_capsule_firmware_raw')
@pytest.mark.buildconfigspec('efi_capsule_on_disk')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.slow
class TestEfiCapsuleFirmwareFit(object):
    def test_efi_capsule_fw1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 1 - Update U-Boot and U-Boot environment on SPI Flash
                      but with OsIndications unset
                      No update should happen
                      0x100000-0x150000: U-Boot binary (but dummy)
                      0x150000-0x200000: U-Boot environment (but dummy)
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e OsIndications',
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize contents
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.env.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 150000 10',
                'sf read 5000000 150000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test01' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test01 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test01' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000')

            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:Old' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 150000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env:Old' in ''.join(output)

    def test_efi_capsule_fw2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 2 - Update U-Boot and U-Boot environment on SPI Flash
                      0x100000-0x150000: U-Boot binary (but dummy)
                      0x150000-0x200000: U-Boot environment (but dummy)
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize contents
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.env.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 150000 10',
                'sf read 5000000 150000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test01' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test01 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test01' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000')

            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test01' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:New' in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf read 4000000 150000 10',
                'md.b 4000000 10'])
            assert 'u-boot-env:New' in ''.join(output)

    def test_efi_capsule_fw3(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 3 - Update U-Boot on SPI Flash, raw image format
                      0x100000-0x150000: U-Boot binary (but dummy)
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 3-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize content
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old' % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test02' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test02 $filesize' % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test02' in ''.join(output)

        # reboot
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 3-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test02' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000')

            output = u_boot_console.run_command_list(['efidebug capsule esrt'])

            # ensure that EFI_FIRMWARE_IMAGE_TYPE_UBOOT_FIT_GUID is in the ESRT.
            assert 'AE13FF2D-9AD4-4E25-9AC8-6D80B3B22147' in ''.join(output)

            # ensure that  EFI_FIRMWARE_IMAGE_TYPE_UBOOT_RAW_GUID is in the ESRT.
            assert 'E2BB9C06-70E9-4B14-97A3-5A7913176E3F' in ''.join(output)

            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test02' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:New' in ''.join(output)

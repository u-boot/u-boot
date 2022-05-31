# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2021, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Firmware Update (Signed capsule with raw images) Test

"""
This test verifies capsule-on-disk firmware update
with signed capsule files containing raw images
"""

import pytest
from capsule_defs import CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_capsule_firmware_raw')
@pytest.mark.buildconfigspec('efi_capsule_authenticate')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.slow
class TestEfiCapsuleFirmwareSignedRaw(object):
    def test_efi_capsule_auth1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 1 - Update U-Boot on SPI Flash, raw image format
                      0x100000-0x150000: U-Boot binary (but dummy)

                      If the capsule is properly signed, the authentication
                      should pass and the firmware be updated.
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env set dfu_alt_info '
                        '"sf 0:0=u-boot-bin raw 0x100000 '
                        '0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize content
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old'
                        % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test11' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test11 $filesize'
                        % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test11' in ''.join(output)

        # reboot
        mnt_point = u_boot_config.persistent_data_dir + '/test_efi_capsule'
        u_boot_console.config.dtb = mnt_point + CAPSULE_DATA_DIR \
                                    + '/test_sig.dtb'
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info '
                            '"sf 0:0=u-boot-bin raw 0x100000 '
                            '0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test11' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000', wait_for_reboot = True)

            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test11' not in ''.join(output)

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:New' in ''.join(output)

    def test_efi_capsule_auth2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 2 - Update U-Boot on SPI Flash, raw image format
                      0x100000-0x150000: U-Boot binary (but dummy)

                      If the capsule is signed but with an invalid key,
                      the authentication should fail and the firmware
                      not be updated.
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env set dfu_alt_info '
                        '"sf 0:0=u-boot-bin raw 0x100000 '
                        '0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize content
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old'
                        % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test12' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test12 $filesize'
                                % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test12' in ''.join(output)

        # reboot
        mnt_point = u_boot_config.persistent_data_dir + '/test_efi_capsule'
        u_boot_console.config.dtb = mnt_point + CAPSULE_DATA_DIR \
                                    + '/test_sig.dtb'
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info '
                        '"sf 0:0=u-boot-bin raw 0x100000 '
                        '0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test12' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000', wait_for_reboot = True)

            # deleted any way
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test12' not in ''.join(output)

            # TODO: check CapsuleStatus in CapsuleXXXX

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:Old' in ''.join(output)

    def test_efi_capsule_auth3(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """
        Test Case 3 - Update U-Boot on SPI Flash, raw image format
                      0x100000-0x150000: U-Boot binary (but dummy)

                      If the capsule is not signed, the authentication
                      should fail and the firmware not be updated.
        """
        disk_img = efi_capsule_data
        with u_boot_console.log.section('Test Case 3-a, before reboot'):
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add -b 1 TEST host 0:1 /helloworld.efi',
                'efidebug boot order 1',
                'env set -e -nv -bs -rt OsIndications =0x0000000000000004',
                'env set dfu_alt_info '
                        '"sf 0:0=u-boot-bin raw 0x100000 '
                        '0x50000;u-boot-env raw 0x150000 0x200000"',
                'env save'])

            # initialize content
            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'fatload host 0:1 4000000 %s/u-boot.bin.old'
                        % CAPSULE_DATA_DIR,
                'sf write 4000000 100000 10',
                'sf read 5000000 100000 10',
                'md.b 5000000 10'])
            assert 'Old' in ''.join(output)

            # place a capsule file
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 %s/Test02' % CAPSULE_DATA_DIR,
                'fatwrite host 0:1 4000000 %s/Test02 $filesize'
                            % CAPSULE_INSTALL_DIR,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test02' in ''.join(output)

        # reboot
        mnt_point = u_boot_config.persistent_data_dir + '/test_efi_capsule'
        u_boot_console.config.dtb = mnt_point + CAPSULE_DATA_DIR \
                                    + '/test_sig.dtb'
        u_boot_console.restart_uboot()

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 3-b, after reboot'):
            if not capsule_early:
                # make sure that dfu_alt_info exists even persistent variables
                # are not available.
                output = u_boot_console.run_command_list([
                    'env set dfu_alt_info '
                            '"sf 0:0=u-boot-bin raw 0x100000 '
                            '0x50000;u-boot-env raw 0x150000 0x200000"',
                    'host bind 0 %s' % disk_img,
                    'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
                assert 'Test02' in ''.join(output)

                # need to run uefi command to initiate capsule handling
                output = u_boot_console.run_command(
                    'env print -e Capsule0000', wait_for_reboot = True)

            # deleted anyway
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatls host 0:1 %s' % CAPSULE_INSTALL_DIR])
            assert 'Test02' not in ''.join(output)

            # TODO: check CapsuleStatus in CapsuleXXXX

            output = u_boot_console.run_command_list([
                'sf probe 0:0',
                'sf read 4000000 100000 10',
                'md.b 4000000 10'])
            assert 'u-boot:Old' in ''.join(output)

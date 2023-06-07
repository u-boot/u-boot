# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2021, Linaro Limited
# Copyright (c) 2022, Arm Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>,
#         adapted to FIT images by Vincent Stehlé <vincent.stehle@arm.com>

"""U-Boot UEFI: Firmware Update (Signed capsule with FIT images) Test
This test verifies capsule-on-disk firmware update
with signed capsule files containing FIT images
"""

import pytest
from capsule_common import (
    setup,
    init_content,
    place_capsule_file,
    exec_manual_update,
    check_file_removed,
    verify_content,
    do_reboot_dtb_specified
)

@pytest.mark.boardspec('sandbox_flattree')
@pytest.mark.buildconfigspec('efi_capsule_firmware_fit')
@pytest.mark.buildconfigspec('efi_capsule_authenticate')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.slow
class TestEfiCapsuleFirmwareSignedFit():
    """Capsule-on-disk firmware update test
    """

    def test_efi_capsule_auth1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """Test Case 1
        Update U-Boot on SPI Flash, FIT image format
        x150000: U-Boot binary (but dummy)

        If the capsule is properly signed, the authentication
        should pass and the firmware be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test13']
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        do_reboot_dtb_specified(u_boot_config, u_boot_console, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            check_file_removed(u_boot_console, disk_img, capsule_files)

            verify_content(u_boot_console, '100000', 'u-boot:New')

    def test_efi_capsule_auth2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """Test Case 2
        Update U-Boot on SPI Flash, FIT image format
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is signed but with an invalid key,
        the authentication should fail and the firmware
        not be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test14']
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        do_reboot_dtb_specified(u_boot_config, u_boot_console, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # deleted any way
            check_file_removed(u_boot_console, disk_img, capsule_files)

            # TODO: check CapsuleStatus in CapsuleXXXX

            verify_content(u_boot_console, '100000', 'u-boot:Old')

    def test_efi_capsule_auth3(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """Test Case 3
        Update U-Boot on SPI Flash, FIT image format
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is not signed, the authentication
        should fail and the firmware not be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test02']
        with u_boot_console.log.section('Test Case 3-a, before reboot'):
            setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        do_reboot_dtb_specified(u_boot_config, u_boot_console, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with u_boot_console.log.section('Test Case 3-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # deleted any way
            check_file_removed(u_boot_console, disk_img, capsule_files)

            # TODO: check CapsuleStatus in CapsuleXXXX

            verify_content(u_boot_console, '100000', 'u-boot:Old')

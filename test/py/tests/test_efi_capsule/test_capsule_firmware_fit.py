# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

"""U-Boot UEFI: Firmware Update Test
This test verifies capsule-on-disk firmware update for FIT images
"""

import pytest
from capsule_common import (
    setup,
    init_content,
    place_capsule_file,
    exec_manual_update,
    check_file_removed,
    verify_content
)

@pytest.mark.boardspec('sandbox_flattree')
@pytest.mark.buildconfigspec('efi_capsule_firmware_fit')
@pytest.mark.buildconfigspec('efi_capsule_on_disk')
@pytest.mark.buildconfigspec('dfu')
@pytest.mark.buildconfigspec('dfu_sf')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.slow
class TestEfiCapsuleFirmwareFit():
    """Test capsule-on-disk firmware update for FIT images
    """

    def test_efi_capsule_fw1(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """Test Case 1
        Update U-Boot and U-Boot environment on SPI Flash
        but with an incorrect GUID value in the capsule
        No update should happen
        0x100000-0x150000: U-Boot binary (but dummy)
        0x150000-0x200000: U-Boot environment (but dummy)
        """
        # other tests might have run and the
        # system might not be in a clean state.
        # Restart before starting the tests.
        u_boot_console.restart_uboot()

        disk_img = efi_capsule_data
        capsule_files = ['Test05']
        with u_boot_console.log.section('Test Case 1-a, before reboot'):
            setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')

        # reboot
        u_boot_console.restart_uboot(expect_reset = capsule_early)

        with u_boot_console.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            # deleted anyway
            check_file_removed(u_boot_console, disk_img, capsule_files)

            verify_content(u_boot_console, '100000', 'u-boot:Old')
            verify_content(u_boot_console, '150000', 'u-boot-env:Old')

    def test_efi_capsule_fw2(
            self, u_boot_config, u_boot_console, efi_capsule_data):
        """Test Case 2
        Update U-Boot and U-Boot environment on SPI Flash
        0x100000-0x150000: U-Boot binary (but dummy)
        0x150000-0x200000: U-Boot environment (but dummy)
        """

        disk_img = efi_capsule_data
        capsule_files = ['Test04']
        with u_boot_console.log.section('Test Case 2-a, before reboot'):
            setup(u_boot_console, disk_img, '0x0000000000000004')
            init_content(u_boot_console, '100000', 'u-boot.bin.old', 'Old')
            init_content(u_boot_console, '150000', 'u-boot.env.old', 'Old')
            place_capsule_file(u_boot_console, capsule_files)

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        capsule_auth = u_boot_config.buildconfig.get(
            'config_efi_capsule_authenticate')

        # reboot
        u_boot_console.restart_uboot(expect_reset = capsule_early)

        with u_boot_console.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                exec_manual_update(u_boot_console, disk_img, capsule_files)

            check_file_removed(u_boot_console, disk_img, capsule_files)

            expected = 'u-boot:Old' if capsule_auth else 'u-boot:New'
            verify_content(u_boot_console, '100000', expected)

            expected = 'u-boot-env:Old' if capsule_auth else 'u-boot-env:New'
            verify_content(u_boot_console, '150000', expected)

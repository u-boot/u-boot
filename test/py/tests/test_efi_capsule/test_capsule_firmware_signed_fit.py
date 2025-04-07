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
    capsule_setup,
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
            self, u_boot_config, ubman, efi_capsule_data):
        """Test Case 1
        Update U-Boot on SPI Flash, FIT image format
        x150000: U-Boot binary (but dummy)

        If the capsule is properly signed, the authentication
        should pass and the firmware be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test13']
        with ubman.log.section('Test Case 1-a, before reboot'):
            capsule_setup(ubman, disk_img, '0x0000000000000004')
            init_content(ubman, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(ubman, capsule_files)

        do_reboot_dtb_specified(u_boot_config, ubman, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with ubman.log.section('Test Case 1-b, after reboot'):
            if not capsule_early:
                exec_manual_update(ubman, disk_img, capsule_files)

            check_file_removed(ubman, disk_img, capsule_files)

            verify_content(ubman, '100000', 'u-boot:New')

    def test_efi_capsule_auth2(
            self, u_boot_config, ubman, efi_capsule_data):
        """Test Case 2
        Update U-Boot on SPI Flash, FIT image format
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is signed but with an invalid key,
        the authentication should fail and the firmware
        not be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test14']
        with ubman.log.section('Test Case 2-a, before reboot'):
            capsule_setup(ubman, disk_img, '0x0000000000000004')
            init_content(ubman, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(ubman, capsule_files)

        do_reboot_dtb_specified(u_boot_config, ubman, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with ubman.log.section('Test Case 2-b, after reboot'):
            if not capsule_early:
                exec_manual_update(ubman, disk_img, capsule_files)

            # deleted any way
            check_file_removed(ubman, disk_img, capsule_files)

            # TODO: check CapsuleStatus in CapsuleXXXX

            verify_content(ubman, '100000', 'u-boot:Old')

    def test_efi_capsule_auth3(
            self, u_boot_config, ubman, efi_capsule_data):
        """Test Case 3
        Update U-Boot on SPI Flash, FIT image format
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is not signed, the authentication
        should fail and the firmware not be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test02']
        with ubman.log.section('Test Case 3-a, before reboot'):
            capsule_setup(ubman, disk_img, '0x0000000000000004')
            init_content(ubman, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(ubman, capsule_files)

        do_reboot_dtb_specified(u_boot_config, ubman, 'test_sig.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with ubman.log.section('Test Case 3-b, after reboot'):
            if not capsule_early:
                exec_manual_update(ubman, disk_img, capsule_files)

            # deleted any way
            check_file_removed(ubman, disk_img, capsule_files)

            # TODO: check CapsuleStatus in CapsuleXXXX

            verify_content(ubman, '100000', 'u-boot:Old')

    def test_efi_capsule_auth4(
            self, u_boot_config, ubman, efi_capsule_data):
        """Test Case 4 - Update U-Boot on SPI Flash, raw image format with version information
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is properly signed, the authentication
        should pass and the firmware be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test114']
        with ubman.log.section('Test Case 4-a, before reboot'):
            capsule_setup(ubman, disk_img, '0x0000000000000004')
            init_content(ubman, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(ubman, capsule_files)

        do_reboot_dtb_specified(u_boot_config, ubman, 'test_ver.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with ubman.log.section('Test Case 4-b, after reboot'):
            if not capsule_early:
                exec_manual_update(ubman, disk_img, capsule_files)

            check_file_removed(ubman, disk_img, capsule_files)

            output = ubman.run_command_list([
                'env set dfu_alt_info "sf 0:0=u-boot-bin raw 0x100000 0x50000;'
                'u-boot-env raw 0x150000 0x200000"',
                'efidebug capsule esrt'])

            # ensure that SANDBOX_UBOOT_IMAGE_GUID is in the ESRT.
            assert '46610520-469E-59DC-A8DD-C11832B877EA' in ''.join(output)
            assert 'ESRT: fw_version=5' in ''.join(output)
            assert 'ESRT: lowest_supported_fw_version=3' in ''.join(output)

            verify_content(ubman, '100000', 'u-boot:New')
            verify_content(ubman, '150000', 'u-boot-env:New')

    def test_efi_capsule_auth5(
            self, u_boot_config, ubman, efi_capsule_data):
        """Test Case 5 - Update U-Boot on SPI Flash, raw image format with version information
        0x100000-0x150000: U-Boot binary (but dummy)

        If the capsule is signed but fw_version is lower than lowest
        supported version, the authentication should fail and the firmware
        not be updated.
        """
        disk_img = efi_capsule_data
        capsule_files = ['Test115']
        with ubman.log.section('Test Case 5-a, before reboot'):
            capsule_setup(ubman, disk_img, '0x0000000000000004')
            init_content(ubman, '100000', 'u-boot.bin.old', 'Old')
            place_capsule_file(ubman, capsule_files)

        do_reboot_dtb_specified(u_boot_config, ubman, 'test_ver.dtb')

        capsule_early = u_boot_config.buildconfig.get(
            'config_efi_capsule_on_disk_early')
        with ubman.log.section('Test Case 5-b, after reboot'):
            if not capsule_early:
                exec_manual_update(ubman, disk_img, capsule_files)

            check_file_removed(ubman, disk_img, capsule_files)

            verify_content(ubman, '100000', 'u-boot:Old')

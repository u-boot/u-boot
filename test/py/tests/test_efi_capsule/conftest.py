# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

"""Fixture for UEFI capsule test."""

from subprocess import call, check_call, CalledProcessError
import pytest
from capsule_defs import CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR, EFITOOLS_PATH

@pytest.fixture(scope='session')
def efi_capsule_data(request, u_boot_config):
    """Set up a file system and return path to image.

    The function sets up a file system to be used in UEFI capsule and
    authentication test and returns a path to disk image to be used
    for testing.

    request -- Pytest request object.
    u_boot_config -- U-Boot configuration.
    """
    mnt_point = u_boot_config.persistent_data_dir + '/test_efi_capsule'
    data_dir = mnt_point + CAPSULE_DATA_DIR
    install_dir = mnt_point + CAPSULE_INSTALL_DIR
    image_path = u_boot_config.persistent_data_dir + '/test_efi_capsule.img'

    try:
        # Create a target device
        check_call('dd if=/dev/zero of=./spi.bin bs=1MiB count=16', shell=True)

        check_call('rm -rf %s' % mnt_point, shell=True)
        check_call('mkdir -p %s' % data_dir, shell=True)
        check_call('mkdir -p %s' % install_dir, shell=True)

        check_call('cp %s/u-boot.bin.* %s'
                   % (u_boot_config.build_dir, data_dir), shell=True)
        check_call('cp %s/u-boot.env.* %s'
                   % (u_boot_config.build_dir, data_dir), shell=True)
        check_call('cp %s/u-boot_bin_env.itb %s ' % (u_boot_config.build_dir, data_dir), shell=True)
        check_call('cp %s/Test* %s ' % (u_boot_config.build_dir, data_dir), shell=True)
        # Update dtb to add the version information
        check_call('cd %s; '
                   'cp %s/test/py/tests/test_efi_capsule/version.dts .'
                   % (data_dir, u_boot_config.source_dir), shell=True)

        capsule_auth_enabled = u_boot_config.buildconfig.get(
                    'config_efi_capsule_authenticate')
        if capsule_auth_enabled:
            check_call('cp %s/arch/sandbox/dts/test.dtb %s/test_sig.dtb' %
                       (u_boot_config.build_dir, data_dir), shell=True)
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dts; '
                       'fdtoverlay -i test_sig.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir), shell=True)
        else:
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dts; '
                       'fdtoverlay -i %s/arch/sandbox/dts/test.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir, u_boot_config.build_dir), shell=True)

        # Create a disk image with EFI system partition
        check_call('virt-make-fs --partition=gpt --size=+1M --type=vfat %s %s' %
                   (mnt_point, image_path), shell=True)
        check_call('sgdisk %s -A 1:set:0 -t 1:C12A7328-F81F-11D2-BA4B-00A0C93EC93B' %
                   image_path, shell=True)

    except CalledProcessError as exception:
        pytest.skip('Setup failed: %s' % exception.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -rf %s' % mnt_point, shell=True)
        call('rm -f %s' % image_path, shell=True)
        call('rm -f ./spi.bin', shell=True)

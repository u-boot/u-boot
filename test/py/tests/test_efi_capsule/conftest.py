# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

import os
import os.path
import re
from subprocess import call, check_call, check_output, CalledProcessError
import pytest
from capsule_defs import *

#
# Fixture for UEFI secure boot test
#


@pytest.fixture(scope='session')
def efi_capsule_data(request, u_boot_config):
    """Set up a file system to be used in UEFI capsule test.

    Args:
        request: Pytest request object.
        u_boot_config: U-boot configuration.

    Return:
        A path to disk image to be used for testing
    """
    global CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR

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

        # Create capsule files
        # two regions: one for u-boot.bin and the other for u-boot.env
        check_call('cd %s; echo -n u-boot:Old > u-boot.bin.old; echo -n u-boot:New > u-boot.bin.new; echo -n u-boot-env:Old -> u-boot.env.old; echo -n u-boot-env:New > u-boot.env.new' % data_dir,
                   shell=True)
        check_call('sed -e \"s?BINFILE1?u-boot.bin.new?\" -e \"s?BINFILE2?u-boot.env.new?\" %s/test/py/tests/test_efi_capsule/uboot_bin_env.its > %s/uboot_bin_env.its' %
                   (u_boot_config.source_dir, data_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkimage -f uboot_bin_env.its uboot_bin_env.itb' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --fit uboot_bin_env.itb --index 1 Test01' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --raw u-boot.bin.new --index 1 Test02' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)

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

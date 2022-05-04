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
# Fixture for UEFI capsule test
#

@pytest.fixture(scope='session')
def efi_capsule_data(request, u_boot_config):
    """Set up a file system to be used in UEFI capsule and
       authentication test.

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
        check_call('cd %s; echo -n u-boot-a:Old > u-boot-a.bin.old; echo -n u-boot-env-a:Old > u-boot-env-a.old; echo -n u-boot-a:New > u-boot-a.bin.new; echo -n u-boot-env-a:New > u-boot-env-a.new; echo -n u-boot-b:Old > u-boot-b.bin.old; echo -n u-boot-env-b:Old > u-boot-env-b.old; echo -n u-boot-b:New > u-boot-b.bin.new; echo -n u-boot-env-b:New > u-boot-env-b.new;' % data_dir,
                   shell=True)
        check_call('cd %s; %s/tools/mkfwumdata -i 2 -b 2 -a 0 -g af9e8c96-bec5-48be-9dab-3491c04b1366,09D7CF52-0720-4710-91D1-08469B7FE9C8,a8f61787-5d68-4c9d-9e4a-37bb0df99da7,52377abf-c4e4-4d0b-aafd-ba081a500847 af9e8c96-bec5-48be-9dab-3491c04b1366,5A7021F5-FEF2-48B4-AABA-832E777418C0,ea9d59af-e0e8-4ef5-9b16-4c80ff67524c,4e01d1fa-eebb-437e-9cfe-e7dfbcd04bb3 metadata_bank0.bin' %(data_dir, u_boot_config.build_dir), shell=True)
        check_call('cd %s; %s/tools/mkfwumdata -i 2 -b 2 -a 1 -g af9e8c96-bec5-48be-9dab-3491c04b1366,09D7CF52-0720-4710-91D1-08469B7FE9C8,a8f61787-5d68-4c9d-9e4a-37bb0df99da7,52377abf-c4e4-4d0b-aafd-ba081a500847 af9e8c96-bec5-48be-9dab-3491c04b1366,5A7021F5-FEF2-48B4-AABA-832E777418C0,ea9d59af-e0e8-4ef5-9b16-4c80ff67524c,4e01d1fa-eebb-437e-9cfe-e7dfbcd04bb3 metadata_bank1.bin' %(data_dir, u_boot_config.build_dir), shell=True)

        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 09D7CF52-0720-4710-91D1-08469B7FE9C8 u-boot-b.bin.new Test01' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 5A7021F5-FEF2-48B4-AABA-832E777418C0 u-boot-env-b.new Test02' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)

        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 09D7CF52-0720-4710-91D1-08469B7FE9C8 u-boot-a.bin.new Test03' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 5A7021F5-FEF2-48B4-AABA-832E777418C0 u-boot-env-a.new Test04' %
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

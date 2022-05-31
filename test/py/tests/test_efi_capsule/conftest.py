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

        capsule_auth_enabled = u_boot_config.buildconfig.get(
                    'config_efi_capsule_authenticate')
        if capsule_auth_enabled:
            # Create private key (SIGNER.key) and certificate (SIGNER.crt)
            check_call('cd %s; '
                       'openssl req -x509 -sha256 -newkey rsa:2048 '
                            '-subj /CN=TEST_SIGNER/ -keyout SIGNER.key '
                            '-out SIGNER.crt -nodes -days 365'
                       % data_dir, shell=True)
            check_call('cd %s; %scert-to-efi-sig-list SIGNER.crt SIGNER.esl'
                       % (data_dir, EFITOOLS_PATH), shell=True)

            # Update dtb adding capsule certificate
            check_call('cd %s; '
                       'cp %s/test/py/tests/test_efi_capsule/signature.dts .'
                       % (data_dir, u_boot_config.source_dir), shell=True)
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o signature.dtbo signature.dts; '
                       'fdtoverlay -i %s/arch/sandbox/dts/test.dtb '
                            '-o test_sig.dtb signature.dtbo'
                       % (data_dir, u_boot_config.build_dir), shell=True)

            # Create *malicious* private key (SIGNER2.key) and certificate
            # (SIGNER2.crt)
            check_call('cd %s; '
                       'openssl req -x509 -sha256 -newkey rsa:2048 '
                            '-subj /CN=TEST_SIGNER/ -keyout SIGNER2.key '
                            '-out SIGNER2.crt -nodes -days 365'
                       % data_dir, shell=True)

        # Create capsule files
        # two regions: one for u-boot.bin and the other for u-boot.env
        check_call('cd %s; echo -n u-boot:Old > u-boot.bin.old; echo -n u-boot:New > u-boot.bin.new; echo -n u-boot-env:Old > u-boot.env.old; echo -n u-boot-env:New > u-boot.env.new' % data_dir,
                   shell=True)
        check_call('sed -e \"s?BINFILE1?u-boot.bin.new?\" -e \"s?BINFILE2?u-boot.env.new?\" %s/test/py/tests/test_efi_capsule/uboot_bin_env.its > %s/uboot_bin_env.its' %
                   (u_boot_config.source_dir, data_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkimage -f uboot_bin_env.its uboot_bin_env.itb' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 09D7CF52-0720-4710-91D1-08469B7FE9C8 u-boot.bin.new Test01' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 2 --guid 5A7021F5-FEF2-48B4-AABA-832E777418C0 u-boot.env.new Test02' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 058B7D83-50D5-4C47-A195-60D86AD341C4 u-boot.bin.new Test03' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid 3673B45D-6A7C-46F3-9E60-ADABB03F7937 uboot_bin_env.itb Test04' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)
        check_call('cd %s; %s/tools/mkeficapsule --index 1 --guid  058B7D83-50D5-4C47-A195-60D86AD341C4 uboot_bin_env.itb Test05' %
                   (data_dir, u_boot_config.build_dir),
                   shell=True)

        if capsule_auth_enabled:
            # raw firmware signed with proper key
            check_call('cd %s; '
                       '%s/tools/mkeficapsule --index 1 --monotonic-count 1 '
                            '--private-key SIGNER.key --certificate SIGNER.crt '
                            '--guid 09D7CF52-0720-4710-91D1-08469B7FE9C8 '
                            'u-boot.bin.new Test11'
                       % (data_dir, u_boot_config.build_dir),
                       shell=True)
            # raw firmware signed with *mal* key
            check_call('cd %s; '
                       '%s/tools/mkeficapsule --index 1 --monotonic-count 1 '
                            '--private-key SIGNER2.key '
                            '--certificate SIGNER2.crt '
                            '--guid 09D7CF52-0720-4710-91D1-08469B7FE9C8 '
                            'u-boot.bin.new Test12'
                       % (data_dir, u_boot_config.build_dir),
                       shell=True)
            # FIT firmware signed with proper key
            check_call('cd %s; '
                       '%s/tools/mkeficapsule --index 1 --monotonic-count 1 '
                            '--private-key SIGNER.key --certificate SIGNER.crt '
                            '--guid 3673B45D-6A7C-46F3-9E60-ADABB03F7937 '
                            'uboot_bin_env.itb Test13'
                       % (data_dir, u_boot_config.build_dir),
                       shell=True)
            # FIT firmware signed with *mal* key
            check_call('cd %s; '
                       '%s/tools/mkeficapsule --index 1 --monotonic-count 1 '
                            '--private-key SIGNER2.key '
                            '--certificate SIGNER2.crt '
                            '--guid 3673B45D-6A7C-46F3-9E60-ADABB03F7937 '
                            'uboot_bin_env.itb Test14'
                       % (data_dir, u_boot_config.build_dir),
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

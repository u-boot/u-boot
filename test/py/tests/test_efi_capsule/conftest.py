# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2020, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

"""Fixture for UEFI capsule test."""

import os

from subprocess import call, check_call, CalledProcessError
from tests import fs_helper
import pytest
from capsule_defs import CAPSULE_DATA_DIR, CAPSULE_INSTALL_DIR, EFITOOLS_PATH

@pytest.fixture(scope='function')
def efi_capsule_data(request, ubman):
    """Set up a file system and return path to image.

    The function sets up a file system to be used in UEFI capsule and
    authentication test and returns a path to disk image to be used
    for testing.

    request -- Pytest request object.
    ubman -- U-Boot configuration.
    """
    try:
        image_path, mnt_point = fs_helper.setup_image(ubman, 0, 0xc,
                                                      basename='test_efi_capsule')
        data_dir = mnt_point + CAPSULE_DATA_DIR
        install_dir = mnt_point + CAPSULE_INSTALL_DIR

        # Create a target device
        check_call('dd if=/dev/zero of=./spi.bin bs=1MiB count=16', shell=True)

        check_call('rm -rf %s' % mnt_point, shell=True)
        check_call('mkdir -p %s' % data_dir, shell=True)
        check_call('mkdir -p %s' % install_dir, shell=True)

        capsule_auth_enabled = ubman.config.buildconfig.get(
                    'config_efi_capsule_authenticate')
        key_dir = ubman.config.source_dir + '/board/sandbox'
        if capsule_auth_enabled:
            # Get the keys from the board directory
            check_call('cp %s/capsule_priv_key_good.key %s/SIGNER.key'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_key_good.crt %s/SIGNER.crt'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_esl_good.esl %s/SIGNER.esl'
                       % (key_dir, data_dir), shell=True)

            check_call('cp %s/capsule_priv_key_bad.key %s/SIGNER2.key'
                       % (key_dir, data_dir), shell=True)
            check_call('cp %s/capsule_pub_key_bad.crt %s/SIGNER2.crt'
                       % (key_dir, data_dir), shell=True)

        # Update dtb to add the version information
        check_call('cd %s; '
                   'cp %s/test/py/tests/test_efi_capsule/version.dtso .'
                   % (data_dir, ubman.config.source_dir), shell=True)

        if capsule_auth_enabled:
            check_call('cd %s; '
                       'cp %s/arch/sandbox/dts/test.dtb test_sig.dtb'
                       % (data_dir, ubman.config.build_dir), shell=True)
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dtso; '
                       'fdtoverlay -i test_sig.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir), shell=True)
        else:
            check_call('cd %s; '
                       'dtc -@ -I dts -O dtb -o version.dtbo version.dtso; '
                       'fdtoverlay -i %s/arch/sandbox/dts/test.dtb '
                            '-o test_ver.dtb version.dtbo'
                       % (data_dir, ubman.config.build_dir), shell=True)

        # two regions: one for u-boot.bin and the other for u-boot.env
        check_call('cd %s; echo -n u-boot:Old > u-boot.bin.old; echo -n u-boot:New > u-boot.bin.new; echo -n u-boot-env:Old > u-boot.env.old; echo -n u-boot-env:New > u-boot.env.new' % data_dir,
                   shell=True)

        pythonpath = os.environ.get('PYTHONPATH', '')
        os.environ['PYTHONPATH'] = pythonpath + ':' + '%s/scripts/dtc/pylibfdt' % ubman.config.build_dir
        check_call('cd %s; '
                   'cc -E -I %s/include -x assembler-with-cpp -o capsule_gen_tmp.dts %s/test/py/tests/test_efi_capsule/capsule_gen_binman.dts; '
                   'dtc -I dts -O dtb capsule_gen_tmp.dts -o capsule_binman.dtb;'
                   % (data_dir, ubman.config.source_dir, ubman.config.source_dir), shell=True)
        check_call('cd %s; '
                   './tools/binman/binman --toolpath %s/tools build -u -d %s/capsule_binman.dtb -O %s -m --allow-missing -I %s -I ./board/sandbox -I ./arch/sandbox/dts'
                   % (ubman.config.source_dir, ubman.config.build_dir, data_dir, data_dir, data_dir), shell=True)
        check_call('cp %s/Test* %s' % (ubman.config.build_dir, data_dir), shell=True)
        os.environ['PYTHONPATH'] = pythonpath

        # Create a 16MiB partition as the EFI system partition in the disk
        # image
        fsfile = fs_helper.mk_fs(ubman.config, 'vfat', 0x1000000,
                                 'test_efi_capsule', mnt_point)
        check_call(f'dd conv=notrunc if={fsfile} of={image_path} bs=1M seek=1', shell=True)
        check_call('sgdisk --mbrtogpt %s' % image_path, shell=True)
        check_call('sgdisk %s -A 1:set:0 -t 1:C12A7328-F81F-11D2-BA4B-00A0C93EC93B' %
                   image_path, shell=True)
        call('rm -f %s' % fsfile, shell=True)

    except CalledProcessError as exception:
        pytest.skip('Setup failed: %s' % exception.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -rf %s' % mnt_point, shell=True)
        call('rm -f %s' % image_path, shell=True)
        call('rm -f ./spi.bin', shell=True)

# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

import os
import os.path
import pytest
import re
from subprocess import call, check_call, check_output, CalledProcessError
from defs import *

# from test/py/conftest.py
def tool_is_in_path(tool):
    for path in os.environ["PATH"].split(os.pathsep):
        fn = os.path.join(path, tool)
        if os.path.isfile(fn) and os.access(fn, os.X_OK):
            return True
    return False

#
# Fixture for UEFI secure boot test
#
@pytest.fixture(scope='session')
def efi_boot_env(request, u_boot_config):
    """Set up a file system to be used in UEFI secure boot test.

    Args:
        request: Pytest request object.
	u_boot_config: U-boot configuration.

    Return:
        A path to disk image to be used for testing
    """
    global HELLO_PATH

    image_path = u_boot_config.persistent_data_dir
    image_path = image_path + '/' + EFI_SECBOOT_IMAGE_NAME
    image_size = EFI_SECBOOT_IMAGE_SIZE
    part_size = EFI_SECBOOT_PART_SIZE
    fs_type = EFI_SECBOOT_FS_TYPE

    if HELLO_PATH == '':
        HELLO_PATH = u_boot_config.build_dir + '/lib/efi_loader/helloworld.efi'

    try:
        mnt_point = u_boot_config.persistent_data_dir + '/mnt_efisecure'
        check_call('mkdir -p {}'.format(mnt_point), shell=True)

        # create a disk/partition
        check_call('dd if=/dev/zero of=%s bs=1MiB count=%d'
                            % (image_path, image_size), shell=True)
        check_call('sgdisk %s -n 1:0:+%dMiB'
                            % (image_path, part_size), shell=True)
        # create a file system
        check_call('dd if=/dev/zero of=%s.tmp bs=1MiB count=%d'
                            % (image_path, part_size), shell=True)
        check_call('mkfs -t %s %s.tmp' % (fs_type, image_path), shell=True)
        check_call('dd if=%s.tmp of=%s bs=1MiB seek=1 count=%d conv=notrunc'
                            % (image_path, image_path, 1), shell=True)
        check_call('rm %s.tmp' % image_path, shell=True)
        loop_dev = check_output('sudo losetup -o 1MiB --sizelimit %dMiB --show -f %s | tr -d "\n"'
                                % (part_size, image_path), shell=True).decode()
        check_output('sudo mount -t %s -o umask=000 %s %s'
                                % (fs_type, loop_dev, mnt_point), shell=True)

        # suffix
        # *.key: RSA private key in PEM
        # *.crt: X509 certificate (self-signed) in PEM
        # *.esl: signature list
        # *.hash: message digest of image as signature list
        # *.auth: signed signature list in signature database format
        # *.efi: UEFI image
        # *.efi.signed: signed UEFI image

        # Create signature database
        ## PK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_PK/ -keyout PK.key -out PK.crt -nodes -days 365'
                            % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s PK.crt PK.esl; %ssign-efi-sig-list -c PK.crt -k PK.key PK PK.esl PK.auth'
                            % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                            shell=True)
        ## PK_null for deletion
        check_call('cd %s; sleep 2; touch PK_null.esl; %ssign-efi-sig-list -c PK.crt -k PK.key PK PK_null.esl PK_null.auth'
                            % (mnt_point, EFITOOLS_PATH), shell=True)
        ## KEK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_KEK/ -keyout KEK.key -out KEK.crt -nodes -days 365'
                            % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s KEK.crt KEK.esl; %ssign-efi-sig-list -c PK.crt -k PK.key KEK KEK.esl KEK.auth'
                            % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                            shell=True)
        ## db
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db/ -keyout db.key -out db.crt -nodes -days 365'
                            % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s db.crt db.esl; %ssign-efi-sig-list -c KEK.crt -k KEK.key db db.esl db.auth'
                            % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                            shell=True)
        ## db1
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db1/ -keyout db1.key -out db1.crt -nodes -days 365'
                            % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s db1.crt db1.esl; %ssign-efi-sig-list -c KEK.crt -k KEK.key db db1.esl db1.auth'
                            % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                            shell=True)
        ## db1-update
        check_call('cd %s; %ssign-efi-sig-list -a -c KEK.crt -k KEK.key db db1.esl db1-update.auth'
                            % (mnt_point, EFITOOLS_PATH), shell=True)
        ## dbx
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_dbx/ -keyout dbx.key -out dbx.crt -nodes -days 365'
                            % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s dbx.crt dbx.esl; %ssign-efi-sig-list -c KEK.crt -k KEK.key dbx dbx.esl dbx.auth'
                            % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                            shell=True)

        # Copy image
        check_call('cp %s %s' % (HELLO_PATH, mnt_point), shell=True)

        ## Sign image
        check_call('cd %s; sbsign --key db.key --cert db.crt helloworld.efi'
                            % mnt_point, shell=True)
        ## Digest image
        check_call('cd %s; %shash-to-efi-sig-list helloworld.efi db_hello.hash; %ssign-efi-sig-list -c KEK.crt -k KEK.key db db_hello.hash db_hello.auth'
                            % (mnt_point, EFITOOLS_PATH, EFITOOLS_PATH),
                            shell=True)

        check_call('sudo umount %s' % loop_dev, shell=True)
        check_call('sudo losetup -d %s' % loop_dev, shell=True)

    except CalledProcessError as e:
        pytest.skip('Setup failed: %s' % e.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -f %s' % image_path, shell=True)

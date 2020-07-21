# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

import os
import os.path
from subprocess import call, check_call, check_output, CalledProcessError
import pytest
from defs import *


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
    image_path = u_boot_config.persistent_data_dir
    image_path = image_path + '/test_efi_secboot.img'

    try:
        mnt_point = u_boot_config.build_dir + '/mnt_efisecure'
        check_call('rm -rf {}'.format(mnt_point), shell=True)
        check_call('mkdir -p {}'.format(mnt_point), shell=True)

        # suffix
        # *.key: RSA private key in PEM
        # *.crt: X509 certificate (self-signed) in PEM
        # *.esl: signature list
        # *.hash: message digest of image as signature list
        # *.auth: signed signature list in signature database format
        # *.efi: UEFI image
        # *.efi.signed: signed UEFI image

        # Create signature database
        # PK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_PK/ -keyout PK.key -out PK.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s PK.crt PK.esl; %ssign-efi-sig-list -t "2020-04-01" -c PK.crt -k PK.key PK PK.esl PK.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # PK_null for deletion
        check_call('cd %s; touch PK_null.esl; %ssign-efi-sig-list -t "2020-04-02" -c PK.crt -k PK.key PK PK_null.esl PK_null.auth'
                   % (mnt_point, EFITOOLS_PATH), shell=True)
        # KEK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_KEK/ -keyout KEK.key -out KEK.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s KEK.crt KEK.esl; %ssign-efi-sig-list -t "2020-04-03" -c PK.crt -k PK.key KEK KEK.esl KEK.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # db
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db/ -keyout db.key -out db.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s db.crt db.esl; %ssign-efi-sig-list -t "2020-04-04" -c KEK.crt -k KEK.key db db.esl db.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # db1
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db1/ -keyout db1.key -out db1.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s db1.crt db1.esl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key db db1.esl db1.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # db1-update
        check_call('cd %s; %ssign-efi-sig-list -t "2020-04-06" -a -c KEK.crt -k KEK.key db db1.esl db1-update.auth'
                   % (mnt_point, EFITOOLS_PATH), shell=True)
        # dbx (TEST_dbx certificate)
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_dbx/ -keyout dbx.key -out dbx.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s dbx.crt dbx.esl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx.esl dbx.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_hash (digest of TEST_db certificate)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -t 0 -s 256 db.crt dbx_hash.crl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx_hash.crl dbx_hash.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_hash1 (digest of TEST_db1 certificate)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -t 0 -s 256 db1.crt dbx_hash1.crl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx_hash1.crl dbx_hash1.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_db (with TEST_db certificate)
        check_call('cd %s; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx db.esl dbx_db.auth'
                   % (mnt_point, EFITOOLS_PATH),
                   shell=True)

        # Copy image
        check_call('cp %s/lib/efi_loader/helloworld.efi %s' %
                   (u_boot_config.build_dir, mnt_point), shell=True)

        # Sign image
        check_call('cd %s; sbsign --key db.key --cert db.crt helloworld.efi'
                   % mnt_point, shell=True)
        # Sign already-signed image with another key
        check_call('cd %s; sbsign --key db1.key --cert db1.crt --output helloworld.efi.signed_2sigs helloworld.efi.signed'
                   % mnt_point, shell=True)
        # Digest image
        check_call('cd %s; %shash-to-efi-sig-list helloworld.efi db_hello.hash; %ssign-efi-sig-list -t "2020-04-07" -c KEK.crt -k KEK.key db db_hello.hash db_hello.auth'
                   % (mnt_point, EFITOOLS_PATH, EFITOOLS_PATH),
                   shell=True)
        check_call('cd %s; %shash-to-efi-sig-list helloworld.efi.signed db_hello_signed.hash; %ssign-efi-sig-list -t "2020-04-03" -c KEK.crt -k KEK.key db db_hello_signed.hash db_hello_signed.auth'
                   % (mnt_point, EFITOOLS_PATH, EFITOOLS_PATH),
                   shell=True)
        check_call('cd %s; %ssign-efi-sig-list -t "2020-04-07" -c KEK.crt -k KEK.key dbx db_hello_signed.hash dbx_hello_signed.auth'
                   % (mnt_point, EFITOOLS_PATH),
                   shell=True)

        check_call('virt-make-fs --partition=gpt --size=+1M --type=vfat {} {}'.format(
            mnt_point, image_path), shell=True)
        check_call('rm -rf {}'.format(mnt_point), shell=True)

    except CalledProcessError as exception:
        pytest.skip('Setup failed: %s' % exception.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -f %s' % image_path, shell=True)

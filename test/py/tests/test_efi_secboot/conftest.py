# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>

"""Fixture for UEFI secure boot test."""

from subprocess import call, check_call, CalledProcessError
from tests import fs_helper
import pytest
from defs import *

@pytest.fixture(scope='function')
def efi_boot_env(request, ubman):
    """Set up a file system to be used in UEFI secure boot test.

    Args:
        request: Pytest request object.
        ubman: U-Boot configuration.

    Return:
        A path to disk image to be used for testing
    """

    try:
        image_path, mnt_point = fs_helper.setup_image(ubman, 0, 0xc,
                                                      basename='test_efi_secboot')

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
        # db2 (APPEND_WRITE)
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_db2/ -keyout db2.key -out db2.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s db2.crt db2.esl; %ssign-efi-sig-list -a -c KEK.crt -k KEK.key db db2.esl db2.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx (TEST_dbx certificate)
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_dbx/ -keyout dbx.key -out dbx.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s dbx.crt dbx.esl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx.esl dbx.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_hash (digest of TEST_db certificate)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -s 256 db.crt dbx_hash.crl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx_hash.crl dbx_hash.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -s 384 db.crt dbx_hash384.crl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx_hash384.crl dbx_hash384.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -s 512 db.crt dbx_hash512.crl; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx dbx_hash512.crl dbx_hash512.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_hash1 (digest of TEST_db1 certificate)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -s 256 db1.crt dbx_hash1.crl; %ssign-efi-sig-list -t "2020-04-06" -c KEK.crt -k KEK.key dbx dbx_hash1.crl dbx_hash1.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_hash2 (digest of TEST_db2 certificate, with APPEND_WRITE)
        check_call('cd %s; %scert-to-efi-hash-list -g %s -s 256 db2.crt dbx_hash2.crl; %ssign-efi-sig-list -a -c KEK.crt -k KEK.key dbx dbx_hash2.crl dbx_hash2.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # dbx_db (with TEST_db certificate)
        check_call('cd %s; %ssign-efi-sig-list -t "2020-04-05" -c KEK.crt -k KEK.key dbx db.esl dbx_db.auth'
                   % (mnt_point, EFITOOLS_PATH),
                   shell=True)

        # Copy image
        check_call('cp %s/lib/efi_loader/helloworld.efi %s' %
                   (ubman.config.build_dir, mnt_point), shell=True)

        # Sign image
        check_call('cd %s; sbsign --key db.key --cert db.crt helloworld.efi'
                   % mnt_point, shell=True)
        # Sign already-signed image with another key
        check_call('cd %s; sbsign --key db1.key --cert db1.crt --output helloworld.efi.signed_2sigs helloworld.efi.signed'
                   % mnt_point, shell=True)
        # Create a corrupted signed image
        check_call('cd %s; sh %s/test/py/tests/test_efi_secboot/forge_image.sh helloworld.efi.signed helloworld_forged.efi.signed'
                   % (mnt_point, ubman.config.source_dir), shell=True)
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

        fsfile = fs_helper.mk_fs(ubman.config, 'vfat', 0x1000000,
                                 'test_efi_secboot', mnt_point)
        check_call(f'dd if={fsfile} of={image_path} bs=1M seek=1', shell=True)

    except CalledProcessError as exception:
        pytest.skip('Setup failed: %s' % exception.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -rf %s' % mnt_point, shell=True)
        call('rm -f %s %s' % (image_path, fsfile), shell=True)

#
# Fixture for UEFI secure boot test of intermediate certificates
#


@pytest.fixture(scope='function')
def efi_boot_env_intca(request, ubman):
    """Set up file system for secure boot test.

    Set up a file system to be used in UEFI secure boot test
    of intermediate certificates.

    Args:
        request: Pytest request object.
        ubman: U-Boot configuration.

    Return:
        A path to disk image to be used for testing
    """

    try:
        image_path, mnt_point = fs_helper.setup_image(ubman, 0, 0xc,
                                                      basename='test_efi_secboot_intca')

        # Create signature database
        # PK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_PK/ -keyout PK.key -out PK.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s PK.crt PK.esl; %ssign-efi-sig-list -c PK.crt -k PK.key PK PK.esl PK.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        # KEK
        check_call('cd %s; openssl req -x509 -sha256 -newkey rsa:2048 -subj /CN=TEST_KEK/ -keyout KEK.key -out KEK.crt -nodes -days 365'
                   % mnt_point, shell=True)
        check_call('cd %s; %scert-to-efi-sig-list -g %s KEK.crt KEK.esl; %ssign-efi-sig-list -c PK.crt -k PK.key KEK KEK.esl KEK.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)

        # We will have three-tier hierarchy of certificates:
        #   TestRoot: Root CA (self-signed)
        #   TestSub: Intermediate CA (signed by Root CA)
        #   TestCert: User certificate (signed by Intermediate CA, and used
        #             for signing an image)
        #
        # NOTE:
        # I consulted the following EDK2 document for certificate options:
        #     BaseTools/Source/Python/Pkcs7Sign/Readme.md
        # Please not use them as they are in product system. They are
        # for test purpose only.

        # TestRoot
        check_call('cp %s/test/py/tests/test_efi_secboot/openssl.cnf %s'
                   % (ubman.config.source_dir, mnt_point), shell=True)
        check_call('cd %s; export OPENSSL_CONF=./openssl.cnf; openssl genrsa -out TestRoot.key 2048; openssl req -extensions v3_ca -new -x509 -days 365 -key TestRoot.key -out TestRoot.crt -subj "/CN=TEST_root/"; touch index.txt; touch index.txt.attr'
                   % mnt_point, shell=True)
        # TestSub
        check_call('cd %s; touch serial.new; export OPENSSL_CONF=./openssl.cnf; openssl genrsa -out TestSub.key 2048; openssl req -new -key TestSub.key -out TestSub.csr -subj "/CN=TEST_sub/"; openssl ca -in TestSub.csr -out TestSub.crt -extensions v3_int_ca -days 365 -batch -rand_serial -cert TestRoot.crt -keyfile TestRoot.key'
                   % mnt_point, shell=True)
        # TestCert
        check_call('cd %s; touch serial.new; export OPENSSL_CONF=./openssl.cnf; openssl genrsa -out TestCert.key 2048; openssl req -new -key TestCert.key -out TestCert.csr -subj "/CN=TEST_cert/"; openssl ca -in TestCert.csr -out TestCert.crt -extensions usr_cert -days 365 -batch -rand_serial -cert TestSub.crt -keyfile TestSub.key'
                   % mnt_point, shell=True)
        # db
        #  for TestCert
        check_call('cd %s; %scert-to-efi-sig-list -g %s TestCert.crt TestCert.esl; %ssign-efi-sig-list -c KEK.crt -k KEK.key db TestCert.esl db_a.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        #  for TestSub
        check_call('cd %s; %scert-to-efi-sig-list -g %s TestSub.crt TestSub.esl; %ssign-efi-sig-list -t "2020-07-16" -c KEK.crt -k KEK.key db TestSub.esl db_b.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        #  for TestRoot
        check_call('cd %s; %scert-to-efi-sig-list -g %s TestRoot.crt TestRoot.esl; %ssign-efi-sig-list -t "2020-07-17" -c KEK.crt -k KEK.key db TestRoot.esl db_c.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        ## dbx (hash of certificate with revocation time)
        #  for TestCert
        check_call('cd %s; %scert-to-efi-hash-list -g %s -t "2020-07-20" -s 256 TestCert.crt TestCert.crl; %ssign-efi-sig-list -c KEK.crt -k KEK.key dbx TestCert.crl dbx_a.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        #  for TestSub
        check_call('cd %s; %scert-to-efi-hash-list -g %s -t "2020-07-21" -s 256 TestSub.crt TestSub.crl; %ssign-efi-sig-list -t "2020-07-18" -c KEK.crt -k KEK.key dbx TestSub.crl dbx_b.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)
        #  for TestRoot
        check_call('cd %s; %scert-to-efi-hash-list -g %s -t "2020-07-22" -s 256 TestRoot.crt TestRoot.crl; %ssign-efi-sig-list -t "2020-07-19" -c KEK.crt -k KEK.key dbx TestRoot.crl dbx_c.auth'
                   % (mnt_point, EFITOOLS_PATH, GUID, EFITOOLS_PATH),
                   shell=True)

        # Sign image
        # additional intermediate certificates may be included
        # in SignedData

        check_call('cp %s/lib/efi_loader/helloworld.efi %s' %
                   (ubman.config.build_dir, mnt_point), shell=True)
        # signed by TestCert
        check_call('cd %s; %ssbsign --key TestCert.key --cert TestCert.crt --out helloworld.efi.signed_a helloworld.efi'
                   % (mnt_point, SBSIGN_PATH), shell=True)
        # signed by TestCert with TestSub in signature
        check_call('cd %s; %ssbsign --key TestCert.key --cert TestCert.crt --addcert TestSub.crt --out helloworld.efi.signed_ab helloworld.efi'
                   % (mnt_point, SBSIGN_PATH), shell=True)
        # signed by TestCert with TestSub and TestRoot in signature
        check_call('cd %s; cat TestSub.crt TestRoot.crt > TestSubRoot.crt; %ssbsign --key TestCert.key --cert TestCert.crt --addcert TestSubRoot.crt --out helloworld.efi.signed_abc helloworld.efi'
                   % (mnt_point, SBSIGN_PATH), shell=True)

        fsfile = fs_helper.mk_fs(ubman.config, 'vfat', 0x1000000,
                                 'test_efi_secboot_intca', mnt_point)
        check_call(f'dd if={fsfile} of={image_path} bs=1M seek=1', shell=True)

    except CalledProcessError as e:
        pytest.skip('Setup failed: %s' % e.cmd)
        return
    else:
        yield image_path
    finally:
        call('rm -rf %s' % mnt_point, shell=True)
        call('rm -f %s %s' % (image_path, fsfile), shell=True)

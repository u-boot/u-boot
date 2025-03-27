# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Signed Image Authentication Test

"""
This test verifies image authentication for unsigned images.
"""

import pytest


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiUnsignedImage(object):
    def test_efi_unsigned_image_auth1(self, ubman, efi_boot_env):
        """
        Test Case 1 - rejected when not digest in db or dbx
        """
        ubman.restart_uboot()
        disk_img = efi_boot_env
        with ubman.log.section('Test Case 1'):
            # Test Case 1
            output = ubman.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = ubman.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            output = ubman.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'efi_bootmgr_load() returned: 26' in ''.join(output)
            assert 'Hello, world!' not in ''.join(output)

    def test_efi_unsigned_image_auth2(self, ubman, efi_boot_env):
        """
        Test Case 2 - authenticated by digest in db
        """
        ubman.restart_uboot()
        disk_img = efi_boot_env
        with ubman.log.section('Test Case 2'):
            # Test Case 2
            output = ubman.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db_hello.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = ubman.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert 'Hello, world!' in ''.join(output)

    def test_efi_unsigned_image_auth3(self, ubman, efi_boot_env):
        """
        Test Case 3 - rejected by digest in dbx
        """
        ubman.restart_uboot()
        disk_img = efi_boot_env
        with ubman.log.section('Test Case 3a'):
            # Test Case 3a, rejected by dbx
            output = ubman.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db_hello.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize dbx',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize PK'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = ubman.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            output = ubman.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'efi_bootmgr_load() returned: 26' in ''.join(output)
            assert 'Hello, world!' not in ''.join(output)

        with ubman.log.section('Test Case 3b'):
            # Test Case 3b, rejected by dbx even if db allows
            output = ubman.run_command_list([
                'fatload host 0:1 4000000 db_hello.auth',
                'setenv -e -nv -bs -rt -at -i 4000000:$filesize db'])
            assert 'Failed to set EFI variable' not in ''.join(output)

            output = ubman.run_command_list([
                'efidebug boot add -b 1 HELLO host 0:1 /helloworld.efi -s ""',
                'efidebug boot order 1',
                'bootefi bootmgr'])
            assert '\'HELLO\' failed' in ''.join(output)
            output = ubman.run_command_list([
                'efidebug boot order 1',
                'efidebug test bootmgr'])
            assert 'efi_bootmgr_load() returned: 26' in ''.join(output)
            assert 'Hello, world!' not in ''.join(output)

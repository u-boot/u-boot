# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Signed Image Authentication Test

"""
This test verifies image authentication for unsigned images.
"""

import pytest
import re
from defs import *

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiUnsignedImage(object):
    def test_efi_unsigned_image_auth1(self, u_boot_console, efi_boot_env):
        """
        Test Case 1 - rejected when not digest in db or dbx
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 1'):
            # Test Case 1
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
		'fatload host 0:1 4000000 KEK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize KEK; echo',
		'fatload host 0:1 4000000 PK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize PK'])
            assert(not re.search('Failed to set EFI variable', ''.join(output)))

            output = u_boot_console.run_command_list([
                'efidebug boot add 1 HELLO host 0:1 /helloworld.efi ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert(re.search('\'HELLO\' failed', ''.join(output)))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'efidebug test bootmgr'])
            assert(re.search('efi_start_image[(][)] returned: 26',
                ''.join(output)))
            assert(not re.search('Hello, world!', ''.join(output)))

    def test_efi_unsigned_image_auth2(self, u_boot_console, efi_boot_env):
        """
        Test Case 2 - authenticated by digest in db
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 2'):
            # Test Case 2
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
		'fatload host 0:1 4000000 db_hello.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize db; echo',
		'fatload host 0:1 4000000 KEK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize KEK',
		'fatload host 0:1 4000000 PK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize PK'])
            assert(not re.search('Failed to set EFI variable', ''.join(output)))

            output = u_boot_console.run_command_list([
                'efidebug boot add 1 HELLO host 0:1 /helloworld.efi ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert(re.search('Hello, world!', ''.join(output)))

    def test_efi_unsigned_image_auth3(self, u_boot_console, efi_boot_env):
        """
        Test Case 3 - rejected by digest in dbx
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 3a'):
            # Test Case 3a, rejected by dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
		'fatload host 0:1 4000000 db_hello.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize dbx; echo',
		'fatload host 0:1 4000000 KEK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize KEK',
		'fatload host 0:1 4000000 PK.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize PK'])
            assert(not re.search('Failed to set EFI variable', ''.join(output)))

            output = u_boot_console.run_command_list([
                'efidebug boot add 1 HELLO host 0:1 /helloworld.efi ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert(re.search('\'HELLO\' failed', ''.join(output)))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'efidebug test bootmgr'])
            assert(re.search('efi_start_image[(][)] returned: 26',
                ''.join(output)))
            assert(not re.search('Hello, world!', ''.join(output)))

        with u_boot_console.log.section('Test Case 3b'):
            # Test Case 3b, rejected by dbx even if db allows
            output = u_boot_console.run_command_list([
		'fatload host 0:1 4000000 db_hello.auth',
		'setenv -e -nv -bs -rt -at -i 4000000,$filesize db'])
            assert(not re.search('Failed to set EFI variable', ''.join(output)))

            output = u_boot_console.run_command_list([
                'efidebug boot add 1 HELLO host 0:1 /helloworld.efi ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert(re.search('\'HELLO\' failed', ''.join(output)))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'efidebug test bootmgr'])
            assert(re.search('efi_start_image[(][)] returned: 26',
                ''.join(output)))
            assert(not re.search('Hello, world!', ''.join(output)))

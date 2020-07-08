# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Linaro Limited
# Author: AKASHI Takahiro <takahiro.akashi@linaro.org>
#
# U-Boot UEFI: Signed Image Authentication Test

"""
This test verifies image authentication for signed images.
"""

import pytest
from defs import *


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('efi_secure_boot')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_nvedit_efi')
@pytest.mark.slow
class TestEfiSignedImage(object):
    def test_efi_signed_image_auth1(self, u_boot_console, efi_boot_env):
        """
        Test Case 1 - authenticated by db
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 1a'):
            # Test Case 1a, run signed image if no db/dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'efidebug boot add 1 HELLO1 host 0:1 /helloworld.efi.signed ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert('Hello, world!' in ''.join(output))

        with u_boot_console.log.section('Test Case 1b'):
            # Test Case 1b, run unsigned image if no db/dbx
            output = u_boot_console.run_command_list([
                'efidebug boot add 2 HELLO2 host 0:1 /helloworld.efi ""',
                'efidebug boot next 2',
                'bootefi bootmgr'])
            assert('Hello, world!' in ''.join(output))

        with u_boot_console.log.section('Test Case 1c'):
            # Test Case 1c, not authenticated by db
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize db',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize PK'])
            assert(not 'Failed to set EFI variable' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot next 2',
                'bootefi bootmgr'])
            assert('\'HELLO2\' failed' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot next 2',
                'efidebug test bootmgr'])
            assert('efi_start_image() returned: 26' in ''.join(output))
            assert(not 'Hello, world!' in ''.join(output))

        with u_boot_console.log.section('Test Case 1d'):
            # Test Case 1d, authenticated by db
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert('Hello, world!' in ''.join(output))

    def test_efi_signed_image_auth2(self, u_boot_console, efi_boot_env):
        """
        Test Case 2 - rejected by dbx
        """
        u_boot_console.restart_uboot()
        disk_img = efi_boot_env
        with u_boot_console.log.section('Test Case 2a'):
            # Test Case 2a, rejected by dbx
            output = u_boot_console.run_command_list([
                'host bind 0 %s' % disk_img,
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize dbx',
                'fatload host 0:1 4000000 KEK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize KEK',
                'fatload host 0:1 4000000 PK.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize PK'])
            assert(not 'Failed to set EFI variable' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot add 1 HELLO host 0:1 /helloworld.efi.signed ""',
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert('\'HELLO\' failed' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'efidebug test bootmgr'])
            assert('efi_start_image() returned: 26' in ''.join(output))
            assert(not 'Hello, world!' in ''.join(output))

        with u_boot_console.log.section('Test Case 2b'):
            # Test Case 2b, rejected by dbx even if db allows
            output = u_boot_console.run_command_list([
                'fatload host 0:1 4000000 db.auth',
                'setenv -e -nv -bs -rt -at -i 4000000,$filesize db'])
            assert(not 'Failed to set EFI variable' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'bootefi bootmgr'])
            assert('\'HELLO\' failed' in ''.join(output))
            output = u_boot_console.run_command_list([
                'efidebug boot next 1',
                'efidebug test bootmgr'])
            assert('efi_start_image() returned: 26' in ''.join(output))
            assert(not 'Hello, world!' in ''.join(output))

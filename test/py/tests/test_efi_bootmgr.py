# SPDX-License-Identifier:      GPL-2.0+
""" Unit test for UEFI bootmanager
"""

import shutil
import pytest
from subprocess import call, check_call, CalledProcessError
from tests.fs_helper import DiskHelper, FsHelper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_bootefi_bootmgr')
@pytest.mark.singlethread
def test_efi_bootmgr(ubman):
    """ Unit test for UEFI bootmanager
    The efidebug command is used to set up UEFI load options.
    The bootefi bootmgr loads initrddump.efi as a payload.
    The crc32 of the loaded initrd.img is checked

    Args:
        ubman -- U-Boot console
    """
    with DiskHelper(ubman.config, 0, 'test_efi_bootmgr') as img, \
            FsHelper(ubman.config, 'vfat', 1, 'test_efi_bootmgr') as fsh:
        with open(f'{fsh.srcdir}/initrd-1.img', 'w', encoding = 'ascii') as outf:
            outf.write("initrd 1")
        with open(f'{fsh.srcdir}/initrd-2.img', 'w', encoding = 'ascii') as outf:
            outf.write("initrd 2")
        shutil.copyfile(
            ubman.config.build_dir + '/lib/efi_loader/initrddump.efi',
            f'{fsh.srcdir}/initrddump.efi')
        fsh.mk_fs()

        img.add_fs(fsh, DiskHelper.VFAT)
        efi_bootmgr_data = img.create()

        ubman.run_command(cmd = f'host bind 0 {efi_bootmgr_data}')

        ubman.run_command(cmd = 'efidebug boot add ' \
            '-b 0001 label-1 host 0:1 initrddump.efi ' \
            '-i host 0:1 initrd-1.img -s nocolor')
        ubman.run_command(cmd = 'efidebug boot dump')
        ubman.run_command(cmd = 'efidebug boot order 0001')
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x181464af' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        ubman.run_command(cmd = 'efidebug boot add ' \
            '-B 0002 label-2 host 0:1 initrddump.efi ' \
            '-I host 0:1 initrd-2.img -s nocolor')
        ubman.run_command(cmd = 'efidebug boot dump')
        ubman.run_command(cmd = 'efidebug boot order 0002')
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x811d3515' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        ubman.run_command(cmd = 'efidebug boot rm 0001')
        ubman.run_command(cmd = 'efidebug boot rm 0002')

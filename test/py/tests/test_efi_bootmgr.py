# SPDX-License-Identifier:      GPL-2.0+
""" Unit test for UEFI bootmanager
"""

import shutil
import pytest
from subprocess import call, check_call, CalledProcessError
from tests import fs_helper

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
    try:
        efi_bootmgr_data, mnt = fs_helper.setup_image(ubman, 0, 0xc,
                                                      basename='test_efi_bootmgr')

        with open(mnt + '/initrd-1.img', 'w', encoding = 'ascii') as file:
            file.write("initrd 1")

        with open(mnt + '/initrd-2.img', 'w', encoding = 'ascii') as file:
            file.write("initrd 2")

        shutil.copyfile(ubman.config.build_dir + '/lib/efi_loader/initrddump.efi',
                        mnt + '/initrddump.efi')

        fsfile = fs_helper.mk_fs(ubman.config, 'vfat', 0x100000,
                                 'test_efi_bootmgr', mnt)
        check_call(f'dd if={fsfile} of={efi_bootmgr_data} bs=1M seek=1', shell=True)

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
    except CalledProcessError as err:
        pytest.skip('Preparing test_efi_bootmgr image failed')
        call('rm -f %s' % efi_bootmgr_data, shell=True)
        return
    finally:
        call('rm -rf %s' % mnt, shell=True)
        call('rm -f %s' % efi_bootmgr_data, shell=True)

# SPDX-License-Identifier:      GPL-2.0+

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('cmd_bootefi_bootmgr')
def test_efi_bootmgr(u_boot_console, efi_bootmgr_data):
    u_boot_console.run_command(cmd = 'host bind 0 {}'.format(efi_bootmgr_data))

    u_boot_console.run_command(cmd = 'efidebug boot add ' \
        '-b 0001 label-1 host 0:1 initrddump.efi ' \
        '-i host 0:1 initrd-1.img -s nocolor')
    u_boot_console.run_command(cmd = 'efidebug boot dump')
    u_boot_console.run_command(cmd = 'efidebug boot order 0001')
    u_boot_console.run_command(cmd = 'bootefi bootmgr')
    response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
    assert 'crc32: 0x181464af' in response
    u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

    u_boot_console.run_command(cmd = 'efidebug boot add ' \
        '-B 0002 label-2 host 0:1 initrddump.efi ' \
        '-I host 0:1 initrd-2.img -s nocolor')
    u_boot_console.run_command(cmd = 'efidebug boot dump')
    u_boot_console.run_command(cmd = 'efidebug boot order 0002')
    u_boot_console.run_command(cmd = 'bootefi bootmgr')
    response = u_boot_console.run_command(cmd = 'load', wait_for_echo=False)
    assert 'crc32: 0x811d3515' in response
    u_boot_console.run_command(cmd = 'exit', wait_for_echo=False)

    u_boot_console.run_command(cmd = 'efidebug boot rm 0001')
    u_boot_console.run_command(cmd = 'efidebug boot rm 0002')

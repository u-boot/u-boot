# SPDX-License-Identifier: GPL-2.0+

"""Test bootmenu"""

import pytest

@pytest.mark.buildconfigspec('cmd_bootmenu')
def test_bootmenu(ubman):
    """Test bootmenu

    Args:
        ubman: U-Boot console
    """
    with ubman.temporary_timeout(500):
        ubman.run_command('setenv bootmenu_default 1')
        ubman.run_command('setenv bootmenu_0 test 1=echo ok 1')
        ubman.run_command('setenv bootmenu_1 test 2=echo ok 2')
        ubman.run_command('setenv bootmenu_2 test 3=echo ok 3')
        ubman.run_command('bootmenu 2', wait_for_prompt=False)
        for i in ('U-Boot Boot Menu', 'test 1', 'test 2', 'test 3', 'autoboot'):
            ubman.p.expect([i])
        # Press enter key to execute default entry
        response = ubman.run_command(cmd='\x0d', wait_for_echo=False, send_nl=False)
        assert 'ok 2' in response
        ubman.run_command('bootmenu 2', wait_for_prompt=False)
        ubman.p.expect(['autoboot'])
        # Press up key to select prior entry followed by the enter key
        response = ubman.run_command(cmd='\x1b\x5b\x41\x0d', wait_for_echo=False,
                                              send_nl=False)
        assert 'ok 1' in response
        ubman.run_command('bootmenu 2', wait_for_prompt=False)
        ubman.p.expect(['autoboot'])
        # Press down key to select next entry followed by the enter key
        response = ubman.run_command(cmd='\x1b\x5b\x42\x0d', wait_for_echo=False,
                                              send_nl=False)
        assert 'ok 3' in response
        ubman.run_command('bootmenu 2; echo rc:$?', wait_for_prompt=False)
        ubman.p.expect(['autoboot'])
        # Press the escape key
        response = ubman.run_command(cmd='\x1b', wait_for_echo=False, send_nl=False)
        assert 'ok' not in response
        assert 'rc:0' in response
        ubman.run_command('setenv bootmenu_default')
        ubman.run_command('setenv bootmenu_0')
        ubman.run_command('setenv bootmenu_1')
        ubman.run_command('setenv bootmenu_2')

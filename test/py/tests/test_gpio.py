# SPDX-License-Identifier: GPL-2.0+

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_input(u_boot_console):
    """Test that gpio input correctly returns the value of a gpio pin."""

    response = u_boot_console.run_command('gpio input 0; echo rc:$?')
    expected_response = 'rc:0'
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio toggle 0; gpio input 0; echo rc:$?')
    expected_response = 'rc:1'
    assert(expected_response in response)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_gpio_exit_statuses(u_boot_console):
    """Test that non-input gpio commands correctly return the command
    success/failure status."""

    expected_response = 'rc:0'
    response = u_boot_console.run_command('gpio clear 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio set 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio toggle 0; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio status -a; echo rc:$?')
    assert(expected_response in response)

    expected_response = 'rc:1'
    response = u_boot_console.run_command('gpio nonexistent-command; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('gpio input 200; echo rc:$?')
    assert(expected_response in response)

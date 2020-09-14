# SPDX-License-Identifier: GPL-2.0+

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_button')
def test_button_exit_statuses(u_boot_console):
    """Test that non-input button commands correctly return the command
    success/failure status."""

    expected_response = 'rc:0'
    response = u_boot_console.run_command('button list; echo rc:$?')
    assert(expected_response in response)
    response = u_boot_console.run_command('button button1; echo rc:$?')
    assert(expected_response in response)

    expected_response = 'rc:1'
    response = u_boot_console.run_command('button nonexistent-button; echo rc:$?')
    assert(expected_response in response)

# SPDX-License-Identifier: GPL-2.0+

"""Tests for the button command"""

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_button')
def test_button_list(ubman):
    """Test listing buttons"""

    response = ubman.run_command('button list; echo rc:$?')
    assert('button1' in response)
    assert('button2' in response)
    assert('rc:0' in response)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_button')
@pytest.mark.buildconfigspec('cmd_gpio')
def test_button_return_code(ubman):
    """Test correct reporting of the button status

    The sandbox gpio driver reports the last output value as input value.
    We can use this in our test to emulate different input statuses.
    """

    ubman.run_command('gpio set a3; gpio input a3');
    response = ubman.run_command('button button1; echo rc:$?')
    assert('on' in response)
    assert('rc:0' in response)

    ubman.run_command('gpio clear a3; gpio input a3');
    response = ubman.run_command('button button1; echo rc:$?')
    assert('off' in response)
    assert('rc:1' in response)

    response = ubman.run_command('button nonexistent-button; echo rc:$?')
    assert('not found' in response)
    assert('rc:1' in response)

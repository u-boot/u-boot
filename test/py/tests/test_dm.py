# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Sean Anderson

import pytest

@pytest.mark.buildconfigspec('cmd_dm')
def test_dm_drivers(u_boot_console):
    """Test that each driver in `dm tree` is also listed in `dm drivers`."""
    response = u_boot_console.run_command('dm tree')
    driver_index = response.find('Driver')
    assert driver_index != -1
    drivers = (line[driver_index:].split()[0]
               for line in response[:-1].split('\n')[2:])

    response = u_boot_console.run_command('dm drivers')
    for driver in drivers:
        assert driver in response

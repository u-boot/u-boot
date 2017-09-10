# Copyright (c) 2017 Alison Chaiken
#
# SPDX-License-Identifier: GPL-2.0

# Test GPT manipulation commands.

import os
import pytest
import u_boot_utils
import make_test_disk

"""
These tests rely on a 4 MB block device called testdisk.raw
which is automatically removed at the end of the tests.
"""

@pytest.mark.buildconfigspec('cmd_gpt')
def test_gpt_guid(u_boot_console):
    """Test the gpt guid command."""

    if u_boot_console.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    make_test_disk.makeDisk()
    u_boot_console.run_command('host bind 0 testdisk.raw')
    output = u_boot_console.run_command('gpt guid host 0')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.buildconfigspec('cmd_gpt')
def test_gpt_save_guid(u_boot_console):
    """Test the gpt guid command to save GUID into a string."""

    if u_boot_console.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    u_boot_console.run_command('host bind 0 testdisk.raw')
    output = u_boot_console.run_command('gpt guid host 0 newguid')
    output = u_boot_console.run_command('printenv newguid')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output
    os.remove('testdisk.raw')

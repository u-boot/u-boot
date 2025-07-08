# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

import pytest

@pytest.mark.buildconfigspec('cmd_help')
def test_help(ubman):
    """Test that the "help" command can be executed."""

    lines = ubman.run_command('help')
    if ubman.config.buildconfig.get('config_cmd_2048', 'n') == 'y':
        assert lines.splitlines()[0] == "2048      - The 2048 game"
    else:
        assert lines.splitlines()[0] == "?         - alias for 'help'"

@pytest.mark.boardspec('sandbox')
def test_help_no_devicetree(ubman):
    try:
        ubman.restart_uboot_with_flags([], use_dtb=False)
        ubman.run_command('help')
        output = ubman.get_spawn_output().replace('\r', '')
        assert 'print command description/usage' in output
    finally:
        # Restart afterward to get the normal device tree back
        ubman.restart_uboot()

@pytest.mark.boardspec('sandbox_vpl')
def test_vpl_help(ubman):
    try:
        ubman.restart_uboot()
        ubman.run_command('help')
        output = ubman.get_spawn_output().replace('\r', '')
        assert 'print command description/usage' in output
    finally:
        # Restart afterward to get the normal device tree back
        ubman.restart_uboot()

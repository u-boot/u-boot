# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

def test_help(u_boot_console):
    """Test that the "help" command can be executed."""

    u_boot_console.run_command('help')

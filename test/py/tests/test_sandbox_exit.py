# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

import pytest
import signal

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('sysreset_cmd_poweroff')
def test_poweroff(ubman):
    """Test that the "poweroff" command exits sandbox process."""

    ubman.run_command('poweroff', wait_for_prompt=False)
    assert(ubman.validate_exited())

@pytest.mark.boardspec('sandbox')
def test_ctrl_c(ubman):
    """Test that sending SIGINT to sandbox causes it to exit."""

    ubman.kill(signal.SIGINT)
    assert(ubman.validate_exited())

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_exception')
@pytest.mark.buildconfigspec('sandbox_crash_reset')
def test_exception_reset(ubman):
    """Test that SIGILL causes a reset."""

    ubman.run_command('exception undefined', wait_for_prompt=False)
    m = ubman.p.expect(['resetting ...', 'U-Boot'])
    if m != 0:
        raise Exception('SIGILL did not lead to reset')
    m = ubman.p.expect(['U-Boot', '=>'])
    if m != 0:
        raise Exception('SIGILL did not lead to reset')
    ubman.restart_uboot()

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_exception')
@pytest.mark.notbuildconfigspec('sandbox_crash_reset')
def test_exception_exit(ubman):
    """Test that SIGILL causes a reset."""

    ubman.run_command('exception undefined', wait_for_prompt=False)
    assert(ubman.validate_exited())

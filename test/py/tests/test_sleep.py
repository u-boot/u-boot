# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import time

def test_sleep(u_boot_console):
    '''Test the sleep command, and validate that it sleeps for approximately
    the correct amount of time.'''

    # Do this before we time anything, to make sure U-Boot is already running.
    # Otherwise, the system boot time is included in the time measurement.
    u_boot_console.ensure_spawned()

    # 3s isn't too long, but is enough to cross a few second boundaries.
    sleep_time = 3
    tstart = time.time()
    u_boot_console.run_command('sleep %d' % sleep_time)
    tend = time.time()
    elapsed = tend - tstart
    delta_to_expected = abs(elapsed - sleep_time)
    # 0.25s margin is hopefully enough to account for any system overhead.
    assert delta_to_expected < 0.25

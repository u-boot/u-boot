# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

"""
Note: This test doesn't rely on boardenv_* configuration value but they can
change test behavior.

For example:

# Setup env__reset_test_skip to True if reset test is not possible or desired
# and should be skipped.
env__reset_test_skip = True

# Setup env__reset_test to set the bootmode if 'modeboot' u-boot environment
# variable is not set. Test will be skipped if bootmode is not set in both
# places i.e, boardenv and modeboot u-boot environment variable
env__reset_test = {
    'bootmode': 'qspiboot',
}

# This test will be also skipped if the bootmode is detected to JTAG.
"""

import pytest
import test_000_version

def setup_reset_env(ubman):
    if ubman.config.env.get('env__reset_test_skip', False):
        pytest.skip('reset test is not enabled')

    output = ubman.run_command('echo $modeboot')
    if output:
        bootmode = output
    else:
        f = ubman.config.env.get('env__reset_test', None)
        if not f:
            pytest.skip('bootmode cannot be determined')
        bootmode = f.get('bootmode', 'jtagboot')

    if 'jtag' in bootmode:
        pytest.skip('skipping reset test due to jtag bootmode')

@pytest.mark.buildconfigspec('hush_parser')
def test_reset(ubman):
    """Test the reset command in non-JTAG bootmode.
    It does COLD reset, which resets CPU, DDR and peripherals
    """
    setup_reset_env(ubman)
    ubman.run_command('reset', wait_for_reboot=True)

    # Checks the u-boot command prompt's functionality after reset
    test_000_version.test_version(ubman)

@pytest.mark.buildconfigspec('hush_parser')
def test_reset_w(ubman):
    """Test the reset -w command in non-JTAG bootmode.
    It does WARM reset, which resets CPU but keep DDR/peripherals active.
    """
    setup_reset_env(ubman)
    ubman.run_command('reset -w', wait_for_reboot=True)

    # Checks the u-boot command prompt's functionality after reset
    test_000_version.test_version(ubman)

# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest
import re
import u_boot_utils as util

# This is only a partial test - coverting 64-bit sandbox. It does not test
# big-endian images, nor 32-bit images
@pytest.mark.boardspec('sandbox')
def test_event_dump(u_boot_console):
    """Test that the "help" command can be executed."""
    cons = u_boot_console
    sandbox = cons.config.build_dir + '/u-boot'
    out = util.run_and_log(cons, ['scripts/event_dump.py', sandbox])
    expect = '''.*Event type            Id                              Source location
--------------------  ------------------------------  ------------------------------
EVT_MISC_INIT_F       sandbox_misc_init_f             .*arch/sandbox/cpu/start.c:'''
    assert re.match(expect, out, re.MULTILINE) is not None

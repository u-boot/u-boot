# SPDX-License-Identifier: GPL-2.0+
# Copyright 2024 Linaro Ltd
#
# Run the LMB tests

import pytest

base_script = '''
ut lmb -f
'''

@pytest.mark.boardspec('sandbox')
def test_lmb(u_boot_console):
    cons = u_boot_console
    cmd = base_script

    with cons.log.section('LMB Unit Test'):
        output = cons.run_command_list(cmd.splitlines())

    assert 'Failures: 0' in output[-1]

    # Restart so that the LMB memory map starts with
    # a clean slate for the next set of tests.
    u_boot_console.restart_uboot()

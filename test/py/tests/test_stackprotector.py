# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2021 Broadcom

import pytest
import signal

@pytest.mark.buildconfigspec('cmd_stackprotector_test')
@pytest.mark.notbuildconfigspec('asan')
def test_stackprotector(ubman):
    """Test that the stackprotector function works."""

    ubman.run_command('stackprot_test',wait_for_prompt=False)
    expected_response = 'Stack smashing detected'
    ubman.wait_for(expected_response)
    ubman.restart_uboot()

# Copyright (c) 2017 Xilinx, Inc. Michal Simek
#
# SPDX-License-Identifier: GPL-2.0

import pytest
pytestmark = pytest.mark.buildconfigspec('cmd_memtest')


def test_memtest_failed(u_boot_console):
    """Failed testcase where end address is smaller than starting address"""
    expected_response = 'Refusing to do empty test'
    response = u_boot_console.run_command('mtest 20 10 0 2')
    assert(expected_response in response)

# CR-992632
@pytest.mark.xfail
def test_memtest_ddr(u_boot_console):
    """Test that md reads memory as expected, and that memory can be modified
    using the mw command."""
    start = 0
    addr = '%08x' % start
    size = 10000
    end = '%08x' % size
    interactions = 16
    int_hex = '%08x' % interactions
    expected_response = 'Tested ' + str(interactions) + ' iteration(s) with 0 errors.'
    response = u_boot_console.run_command('mtest ' + addr + ' ' + end +' 0 ' + int_hex)
    assert(expected_response in response)

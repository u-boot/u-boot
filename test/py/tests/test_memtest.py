# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest

"""
Note: This test relies on boardenv_* containing configuration values to define
the memory test parameters such as start address, memory size, pattern,
iterations and timeout. This test will be automatically skipped without this.

For example:

# Setup env__memtest to set the start address of the memory range, size of the
# memory range to test from starting address, pattern to be written to memory,
# number of test iterations, and expected time to complete the test of mtest
# command. start address, size, and pattern parameters value should be in hex
# and rest of the params value should be integer.
env__memtest = {
    'start_addr': 0x0,
    'size': 0x1000,
    'pattern': 0x0,
    'iteration': 16,
    'timeout': 50000,
}
"""

def get_memtest_env(ubman):
    f = ubman.config.env.get("env__memtest", None)
    if not f:
        pytest.skip("memtest is not enabled!")
    else:
        start = hex(f.get("start_addr", 0x0))
        size = hex(f.get("size", 0x1000))
        pattern = hex(f.get("pattern", 0x0))
        iteration = f.get("iteration", 2)
        timeout = f.get("timeout", 50000)
        end = hex(int(start, 16) + int(size, 16))
        return start, end, pattern, iteration, timeout

@pytest.mark.buildconfigspec("cmd_memtest")
def test_memtest_negative(ubman):
    """Negative testcase where end address is smaller than starting address and
    pattern is invalid."""
    start, end, pattern, iteration, timeout = get_memtest_env(ubman)
    expected_response = "Refusing to do empty test"
    response = ubman.run_command(
        f"mtest 2000 1000 {pattern} {hex(iteration)}"
    )
    assert expected_response in response
    output = ubman.run_command("echo $?")
    assert not output.endswith("0")
    ubman.run_command(f"mtest {start} {end} 'xyz' {hex(iteration)}")
    output = ubman.run_command("echo $?")
    assert not output.endswith("0")

@pytest.mark.buildconfigspec("cmd_memtest")
def test_memtest_ddr(ubman):
    """Test that md reads memory as expected, and that memory can be modified
    using the mw command."""
    start, end, pattern, iteration, timeout = get_memtest_env(ubman)
    expected_response = f"Tested {str(iteration)} iteration(s) with 0 errors."
    with ubman.temporary_timeout(timeout):
        response = ubman.run_command(
            f"mtest {start} {end} {pattern} {hex(iteration)}"
        )
        assert expected_response in response
    output = ubman.run_command("echo $?")
    assert output.endswith("0")

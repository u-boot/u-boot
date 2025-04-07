# SPDX-License-Identifier: GPL-2.0
# Copyright 2020 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import os.path
import pytest

@pytest.mark.buildconfigspec('spl_unit_test')
def test_ut_spl_init(ubman):
    """Initialize data for ut spl tests."""

    fn = ubman.config.source_dir + '/spi.bin'
    if not os.path.exists(fn):
        data = b'\x00' * (2 * 1024 * 1024)
        with open(fn, 'wb') as fh:
            fh.write(data)

def test_spl(ubman, ut_spl_subtest):
    """Execute a "ut" subtest.

    The subtests are collected in function generate_ut_subtest() from linker
    generated lists by applying a regular expression to the lines of file
    spl/u-boot-spl.sym. The list entries are created using the C macro
    UNIT_TEST().

    Strict naming conventions have to be followed to match the regular
    expression. Use UNIT_TEST(foo_test_bar, _flags, foo_test) for a test bar in
    test suite foo that can be executed via command 'ut foo bar' and is
    implemented in C function foo_test_bar().

    Args:
        ubman (ConsoleBase): U-Boot console
        ut_subtest (str): SPL test to be executed (e.g. 'dm platdata_phandle')
    """
    try:
        ubman.restart_uboot_with_flags(['-u', '-k', ut_spl_subtest.split()[1]])
        output = ubman.get_spawn_output().replace('\r', '')
        assert 'failures: 0' in output
    finally:
        # Restart afterward in case a non-SPL test is run next. This should not
        # happen since SPL tests are run in their own invocation of test.py, but
        # the cost of doing this is not too great at present.
        ubman.restart_uboot()

# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Test operation of the "if" shell command.

import os
import os.path
import pytest

# TODO: These tests should be converted to a C test.
# For more information please take a look at the thread
# https://lists.denx.de/pipermail/u-boot/2019-October/388732.html

pytestmark = pytest.mark.buildconfigspec('hush_parser')

# The list of "if test" conditions to test.
subtests = (
    # Base if functionality.

    ('true', True),
    ('false', False),

    # Basic operators.

    ('test aaa = aaa', True),
    ('test aaa = bbb', False),

    ('test aaa != bbb', True),
    ('test aaa != aaa', False),

    ('test aaa < bbb', True),
    ('test bbb < aaa', False),

    ('test bbb > aaa', True),
    ('test aaa > bbb', False),

    ('test 123 -eq 123', True),
    ('test 123 -eq 456', False),

    ('test 123 -ne 456', True),
    ('test 123 -ne 123', False),

    ('test 123 -lt 456', True),
    ('test 123 -lt 123', False),
    ('test 456 -lt 123', False),

    ('test 123 -le 456', True),
    ('test 123 -le 123', True),
    ('test 456 -le 123', False),

    ('test 456 -gt 123', True),
    ('test 123 -gt 123', False),
    ('test 123 -gt 456', False),

    ('test 456 -ge 123', True),
    ('test 123 -ge 123', True),
    ('test 123 -ge 456', False),

    # Octal tests

    ('test 010 -eq 010', True),
    ('test 010 -eq 011', False),

    ('test 010 -ne 011', True),
    ('test 010 -ne 010', False),

    # Hexadecimal tests

    ('test 0x2000000 -gt 0x2000001', False),
    ('test 0x2000000 -gt 0x2000000', False),
    ('test 0x2000000 -gt 0x1ffffff', True),

    # Mixed tests

    ('test 010 -eq 10', False),
    ('test 010 -ne 10', True),
    ('test 0xa -eq 10', True),
    ('test 0xa -eq 012', True),

    ('test 2000000 -gt 0x1ffffff', False),
    ('test 0x2000000 -gt 1ffffff', True),
    ('test 0x2000000 -lt 1ffffff', False),
    ('test 0x2000000 -eq 2000000', False),
    ('test 0x2000000 -ne 2000000', True),

    ('test -z ""', True),
    ('test -z "aaa"', False),

    ('test -n "aaa"', True),
    ('test -n ""', False),

    # Inversion of simple tests.

    ('test ! aaa = aaa', False),
    ('test ! aaa = bbb', True),
    ('test ! ! aaa = aaa', True),
    ('test ! ! aaa = bbb', False),

    # Binary operators.

    ('test aaa != aaa -o bbb != bbb', False),
    ('test aaa != aaa -o bbb = bbb', True),
    ('test aaa = aaa -o bbb != bbb', True),
    ('test aaa = aaa -o bbb = bbb', True),

    ('test aaa != aaa -a bbb != bbb', False),
    ('test aaa != aaa -a bbb = bbb', False),
    ('test aaa = aaa -a bbb != bbb', False),
    ('test aaa = aaa -a bbb = bbb', True),

    # Inversion within binary operators.

    ('test ! aaa != aaa -o ! bbb != bbb', True),
    ('test ! aaa != aaa -o ! bbb = bbb', True),
    ('test ! aaa = aaa -o ! bbb != bbb', True),
    ('test ! aaa = aaa -o ! bbb = bbb', False),

    ('test ! ! aaa != aaa -o ! ! bbb != bbb', False),
    ('test ! ! aaa != aaa -o ! ! bbb = bbb', True),
    ('test ! ! aaa = aaa -o ! ! bbb != bbb', True),
    ('test ! ! aaa = aaa -o ! ! bbb = bbb', True),
)

def exec_hush_if(u_boot_console, expr, result):
    """Execute a shell "if" command, and validate its result."""

    config = u_boot_console.config.buildconfig
    maxargs = int(config.get('config_sys_maxargs', '0'))
    args = len(expr.split(' ')) - 1
    if args > maxargs:
        u_boot_console.log.warning('CONFIG_SYS_MAXARGS too low; need ' +
            str(args))
        pytest.skip()

    cmd = 'if ' + expr + '; then echo true; else echo false; fi'
    response = u_boot_console.run_command(cmd)
    assert response.strip() == str(result).lower()

@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.parametrize('expr,result', subtests)
def test_hush_if_test(u_boot_console, expr, result):
    """Test a single "if test" condition."""

    exec_hush_if(u_boot_console, expr, result)

def test_hush_z(u_boot_console):
    """Test the -z operator"""
    u_boot_console.run_command('setenv ut_var_nonexistent')
    u_boot_console.run_command('setenv ut_var_exists 1')
    exec_hush_if(u_boot_console, 'test -z "$ut_var_nonexistent"', True)
    exec_hush_if(u_boot_console, 'test -z "$ut_var_exists"', False)
    u_boot_console.run_command('setenv ut_var_exists')

# We might test this on real filesystems via UMS, DFU, 'save', etc.
# Of those, only UMS currently allows file removal though.
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.boardspec('sandbox')
def test_hush_if_test_host_file_exists(u_boot_console):
    """Test the "if test -e" shell command."""

    test_file = u_boot_console.config.result_dir + \
        '/creating_this_file_breaks_u_boot_tests'

    try:
        os.unlink(test_file)
    except:
        pass
    assert not os.path.exists(test_file)

    expr = 'test -e hostfs - ' + test_file
    exec_hush_if(u_boot_console, expr, False)

    try:
        with open(test_file, 'wb'):
            pass
        assert os.path.exists(test_file)

        expr = 'test -e hostfs - ' + test_file
        exec_hush_if(u_boot_console, expr, True)
    finally:
        os.unlink(test_file)

    expr = 'test -e hostfs - ' + test_file
    exec_hush_if(u_boot_console, expr, False)

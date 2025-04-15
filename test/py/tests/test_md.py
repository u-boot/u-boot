# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

import pytest
import utils

@pytest.mark.buildconfigspec('cmd_memory')
def test_md(ubman):
    """Test that md reads memory as expected, and that memory can be modified
    using the mw command."""

    ram_base = utils.find_ram_base(ubman)
    addr = '%08x' % ram_base
    val = 'a5f09876'
    expected_response = addr + ': ' + val
    ubman.run_command('mw ' + addr + ' 0 10')
    response = ubman.run_command('md ' + addr + ' 10')
    assert(not (expected_response in response))
    ubman.run_command('mw ' + addr + ' ' + val)
    response = ubman.run_command('md ' + addr + ' 10')
    assert(expected_response in response)

@pytest.mark.buildconfigspec('cmd_memory')
def test_md_repeat(ubman):
    """Test command repeat (via executing an empty command) operates correctly
    for "md"; the command must repeat and dump an incrementing address."""

    ram_base = utils.find_ram_base(ubman)
    addr_base = '%08x' % ram_base
    words = 0x10
    addr_repeat = '%08x' % (ram_base + (words * 4))
    ubman.run_command('md %s %x' % (addr_base, words))
    response = ubman.run_command('')
    expected_response = addr_repeat + ': '
    assert(expected_response in response)

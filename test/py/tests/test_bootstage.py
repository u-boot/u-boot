# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

"""
Test the bootstage command.

It is used for checking the boot progress and timing by printing the bootstage
report, stashes the data into memory and unstashes the data from memory.

Note: This test relies on boardenv_* containing configuration values to define
the data size, memory address, and bootstage magic address (defined in
common/bootstage.c). Without this, bootstage stash and unstash tests will be
automatically skipped.

For example:

.. code-block:: python

    env__bootstage_cmd_file = {
        'addr': 0x200000,
        'size': 0x1000,
        'bootstage_magic_addr': 0xb00757a3,
    }
"""

import pytest

@pytest.mark.buildconfigspec('bootstage')
@pytest.mark.buildconfigspec('cmd_bootstage')
def test_bootstage_report(ubman):
    """Test the bootstage report subcommand

    This will run the 'bootstage report' subcommand and ensure that we are
    reporting:

    - A timer summary in microseconds
    - The accumulated time
    - That at least the phrase 'dm_r' is in the output

    Note that the time values are not checked.
    """
    output = ubman.run_command('bootstage report')
    assert 'Timer summary in microseconds' in output
    assert 'Accumulated time:' in output
    assert 'dm_r' in output

@pytest.mark.buildconfigspec('bootstage')
@pytest.mark.buildconfigspec('cmd_bootstage')
@pytest.mark.buildconfigspec('bootstage_stash')
def test_bootstage_stash_and_unstash(ubman):
    """Test the bootstage stash and unstash subcommands

    After checking that we have configured an environment file to use, we will
    use the stash subcommand to save information. Then we will use the md
    command to verify the contents in memory. Finally we confirm the unstash
    subcommand runs successfully.
    """
    f = ubman.config.env.get('env__bootstage_cmd_file', None)
    if not f:
        pytest.skip('No bootstage environment file is defined')

    addr = f.get('addr')
    size = f.get('size')
    bootstage_magic = f.get('bootstage_magic_addr')
    expected_text = 'dm_r'

    ubman.run_command('bootstage stash %x %x' % (addr, size))
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    output = ubman.run_command('md %x 100' % addr)

    # Check BOOTSTAGE_MAGIC address at 4th byte address
    assert '0x' + output.split('\n')[0].split()[4] == hex(bootstage_magic)

    # Check expected string in last column of output
    output_last_col = ''.join([i.split()[-1] for i in output.split('\n')])
    assert expected_text in output_last_col

    # Check that unstash works as expected
    ubman.run_command('bootstage unstash %x %x' % (addr, size))
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

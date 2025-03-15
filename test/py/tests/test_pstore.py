# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2020, Collabora
# Author: Frédéric Danis <frederic.danis@collabora.com>

import pytest
import utils
import os
import tempfile
import shutil

PSTORE_ADDR=0x3000000
PSTORE_LENGTH=0x100000
PSTORE_PANIC1='test/py/tests/test_pstore_data_panic1.hex'
PSTORE_PANIC2='test/py/tests/test_pstore_data_panic2.hex'
PSTORE_CONSOLE='test/py/tests/test_pstore_data_console.hex'
ADDR=0x01000008

def load_pstore(ubman):
    """Load PStore records from sample files"""

    output = ubman.run_command_list([
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR,
            os.path.join(ubman.config.source_dir, PSTORE_PANIC1)),
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR + 4096,
            os.path.join(ubman.config.source_dir, PSTORE_PANIC2)),
        'host load hostfs - 0x%x %s' % (PSTORE_ADDR + 253 * 4096,
            os.path.join(ubman.config.source_dir, PSTORE_CONSOLE)),
        'pstore set 0x%x 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH)])

def checkfile(ubman, path, filesize, checksum):
    """Check file against MD5 checksum"""

    output = ubman.run_command_list([
        'load hostfs - %x %s' % (ADDR, path),
        'printenv filesize'])
    assert('filesize=%x' % (filesize) in ''.join(output))

    output = ubman.run_command_list([
        'md5sum %x $filesize' % ADDR,
        'setenv filesize'])
    assert(checksum in ''.join(output))

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_display_all_records(ubman):
    """Test that pstore displays all records."""

    ubman.run_command('')
    load_pstore(ubman)
    response = ubman.run_command('pstore display')
    assert('**** Dump' in response)
    assert('**** Console' in response)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_display_one_record(ubman):
    """Test that pstore displays only one record."""

    ubman.run_command('')
    load_pstore(ubman)
    response = ubman.run_command('pstore display dump 1')
    assert('Panic#2 Part1' in response)
    assert('**** Console' not in response)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_save_records(ubman):
    """Test that pstore saves all records."""

    outdir = tempfile.mkdtemp()

    ubman.run_command('')
    load_pstore(ubman)
    ubman.run_command('pstore save hostfs - %s' % (outdir))

    checkfile(ubman, '%s/dmesg-ramoops-0' % (outdir), 3798, '8059335ab4cfa62c77324c491659c503')
    checkfile(ubman, '%s/dmesg-ramoops-1' % (outdir), 4035, '3ff30df3429d81939c75d0070b5187b9')
    checkfile(ubman, '%s/console-ramoops-0' % (outdir), 4084, 'bb44de4a9b8ebd9b17ae98003287325b')

    shutil.rmtree(outdir)

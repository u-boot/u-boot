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
PSTORE_SIG=0x43474244
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

def write_header(ubman, addr, sig, start, size):
    """Write a record header at addr"""

    ubman.run_command_list([
        'mw.l 0x%x 0x%x' % (addr, sig),
        'mw.l 0x%x 0x%x' % (addr + 4, start),
        'mw.l 0x%x 0x%x' % (addr + 8, size)])

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

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_invalid_records(ubman):
    """Test that pstore skips records with an invalid header."""

    console = PSTORE_ADDR + PSTORE_LENGTH - 3 * 4096
    ftrace = console + 4096
    pmsg = ftrace + 4096
    outdir = tempfile.mkdtemp()

    ubman.run_command('')
    ubman.run_command_list([
        'mw.b 0x%x 0 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH),
        'pstore set 0x%x 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH)])
    # Start beyond the size
    write_header(ubman, console, PSTORE_SIG, 0x10, 0x8)
    # The same in the ftrace zone, which is read without checking the signature
    write_header(ubman, ftrace, 0, 0x10, 0x8)
    # Size larger than the zone minus the header
    write_header(ubman, pmsg, PSTORE_SIG, 0, 4096)

    response = ubman.run_command('pstore display')
    assert('****' not in response)

    ubman.run_command('pstore save hostfs - %s' % (outdir))
    assert(not os.listdir(outdir))

    shutil.rmtree(outdir)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_set_larger_record(ubman):
    """Test that pstore save works after pstore set makes a record larger."""

    size = 0x2000
    console = PSTORE_ADDR + PSTORE_LENGTH - 2 * 4096 - size
    outdir = tempfile.mkdtemp()

    ubman.run_command('')
    try:
        # Size the buffer for the default record sizes
        ubman.run_command_list([
            'mw.b 0x%x 0 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH),
            'pstore set 0x%x 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH),
            'pstore display'])
        # Make the console record larger and save a full one. With 0xaa
        # data, an overflow of the buffer fails the sandbox malloc checks.
        ubman.run_command_list([
            'pstore set 0x%x 0x%x 0x1000 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH,
                                                  size),
            'mw.b 0x%x 0xaa 0x%x' % (console + 12, size - 12)])
        write_header(ubman, console, PSTORE_SIG, size - 12, size - 12)
        ubman.run_command('pstore save hostfs - %s' % (outdir))
        with open('%s/console-ramoops-0' % (outdir), 'rb') as f:
            assert(f.read() == b'\xaa' * (size - 12))
    finally:
        # Restore the default record sizes
        ubman.restart_uboot()

    shutil.rmtree(outdir)

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_cut_dump_header(ubman):
    """Test that pstore skips a dump record whose header line is cut."""

    outdir = tempfile.mkdtemp()

    ubman.run_command('')
    ubman.run_command('mw.b 0x%x 0 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH))
    load_pstore(ubman)
    # Cut the second dump record inside its header line
    write_header(ubman, PSTORE_ADDR + 4096, PSTORE_SIG, 8, 8)

    response = ubman.run_command('pstore display dump')
    assert(response.count('**** Dump') == 1)

    ubman.run_command('pstore save hostfs - %s' % (outdir))
    assert(not os.path.exists('%s/dmesg-ramoops-1' % (outdir)))

    shutil.rmtree(outdir)

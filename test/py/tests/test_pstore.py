# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2020, Collabora
# Author: Frédéric Danis <frederic.danis@collabora.com>

import pytest
import utils
import os
import tempfile
import shutil
import struct

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

def write_record(ubman, addr, sig, data):
    """Write a record holding data at addr"""

    with tempfile.NamedTemporaryFile() as f:
        f.write(struct.pack('<III', sig, 0, len(data)) + data)
        f.flush()
        ubman.run_command('host load hostfs - 0x%x %s' % (addr, f.name))

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

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_buffer_too_large(ubman):
    """Test that pstore reports a buffer that it cannot allocate."""

    # A record size larger than the malloc pool
    malloc_len = int(ubman.config.buildconfig['config_sys_malloc_len'], 0)
    size = 1 << malloc_len.bit_length()

    ubman.run_command('')
    try:
        ubman.run_command('pstore set 0x%x 0x%x 0x%x 0 0 0' %
                          (PSTORE_ADDR, size, size))
        write_record(ubman, PSTORE_ADDR, PSTORE_SIG, b'====1.0-D\ndump\n')

        response = ubman.run_command('pstore display')
        assert('Cannot allocate' in response)

        with tempfile.TemporaryDirectory() as outdir:
            response = ubman.run_command('pstore save hostfs - %s' % outdir)
            assert('Cannot allocate' in response)
            assert(not os.listdir(outdir))
    finally:
        # Restore the default record sizes
        ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_zone_layout(ubman):
    """Test that pstore finds the zones where Linux puts them."""

    # Linux rounds the console size down to 0x1000, which leaves 0xf000 for
    # the dump records: 3 zones of 0x5000 for a record size of 0x4000
    dump1 = PSTORE_ADDR + 0x5000
    console = PSTORE_ADDR + 0xf000
    # Larger than the record size, but fits in the zone
    dump = b'x' * 0x4800 + b'\nend of dump 1\n'
    # Looks like a dump record, to catch a read past the dump zones
    text = b'====1.0-D\nconsole text\n'

    ubman.run_command('')
    try:
        ubman.run_command_list([
            'mw.b 0x%x 0 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH),
            'pstore set 0x%x 0x12000 0x4000 0x1800 0x1000 0x1000' %
            (PSTORE_ADDR)])
        write_record(ubman, dump1, PSTORE_SIG, b'====1.0-D\n' + dump)
        write_record(ubman, console, PSTORE_SIG, text)

        response = ubman.run_command('pstore display')
        assert('end of dump 1' in response)
        assert('console text' in response)

        # There is no dump zone 3
        response = ubman.run_command('pstore display dump 3')
        assert('****' not in response)
        if ubman.config.buildconfig.get('config_64bit') == 'y':
            # Nor zone 2^52 + 1, whose offset wraps around to zone 1
            response = ubman.run_command('pstore display dump %d' %
                                         ((1 << 52) + 1))
            assert('****' not in response)

        with tempfile.TemporaryDirectory() as outdir:
            ubman.run_command('pstore save hostfs - %s' % (outdir))
            with open('%s/dmesg-ramoops-0' % (outdir), 'rb') as f:
                assert(f.read() == dump)
            with open('%s/console-ramoops-0' % (outdir), 'rb') as f:
                assert(f.read() == text)
    finally:
        # Restore the default record sizes
        ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_no_dump_zones(ubman):
    """Test that pstore finds the console zone with a record size of 0."""

    ubman.run_command('')
    try:
        ubman.run_command_list([
            'mw.b 0x%x 0 0x%x' % (PSTORE_ADDR, PSTORE_LENGTH),
            'pstore set 0x%x 0x%x 0 0x1000 0x1000 0x1000' %
            (PSTORE_ADDR, PSTORE_LENGTH)])
        # Linux makes no dump zones, the console zone comes first
        write_record(ubman, PSTORE_ADDR, PSTORE_SIG, b'console text\n')

        response = ubman.run_command('pstore display')
        assert('console text' in response)
        response = ubman.run_command('pstore display dump 0')
        assert('****' not in response)

        with tempfile.TemporaryDirectory() as outdir:
            ubman.run_command('pstore save hostfs - %s' % (outdir))
            with open('%s/console-ramoops-0' % (outdir), 'rb') as f:
                assert(f.read() == b'console text\n')
    finally:
        # Restore the default record sizes
        ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_pstore')
def test_pstore_rounded_sizes(ubman):
    """Test that pstore set checks the sizes as Linux rounds them."""

    ubman.run_command('')
    try:
        # Rounded down to 0x1000, a dump and a console zone fit in 0x2000
        ubman.run_command('mw.b 0x%x 0 0x2000' % (PSTORE_ADDR))
        response = ubman.run_command('pstore set 0x%x 0x2000 0x1800 0x1800 0 0'
                                     % (PSTORE_ADDR))
        assert('should be larger' not in response)
        write_record(ubman, PSTORE_ADDR + 0x1000, PSTORE_SIG,
                     b'console text\n')
        response = ubman.run_command('pstore display console')
        assert('console text' in response)

        # Sizes whose sum wraps around in 32 bits
        response = ubman.run_command(
            'pstore set 0x%x 0x%x 0x80000000 0x80000000 0x80000000 0x80000000'
            % (PSTORE_ADDR, PSTORE_LENGTH))
        assert('should be larger' in response)
        response = ubman.run_command('pstore display')
        assert('Please set PStore configuration' in response)
    finally:
        # Restore the default record sizes
        ubman.restart_uboot()

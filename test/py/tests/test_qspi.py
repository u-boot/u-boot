# Copyright (c) 2016, Xilinx Inc. Michal Simek
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import re
import random
import u_boot_utils

import test_net

"""
Note: This test relies on boardenv_* containing configuration values to define
qspi minimum and maximum frequnecies at which the flash part can operate on and
these tests run at 5 different qspi frequnecy randomised values in the range.
with out this, this test runs with only one frequnecy value that is 0.

Example:
env__qspi_freq = {
    "min_freq": 10000000,
    "max_freq": 100000000,
}

"""
page_size = 0
erase_size = 0
total_size = 0

# Find out qspi memory parameters
def qspi_pre_commands(u_boot_console, freq):

    output = u_boot_console.run_command('sf probe 0 %d 0' % (freq))
    if not "SF: Detected" in output:
        pytest.skip('No QSPI device available')

    m = re.search('page size (.+?) Bytes', output)
    if m:
        try:
            global page_size
            page_size = int(m.group(1))
        except ValueError:
            pytest.fail("QSPI page size not recognized")

        print 'Page size is: ' + str(page_size) + " B"

    m = re.search('erase size (.+?) KiB', output)
    if m:
        try:
           global erase_size
           erase_size = int(m.group(1))
        except ValueError:
           pytest.fail("QSPI erase size not recognized")

        erase_size *= 1024
        print 'Erase size is: ' + str(erase_size) + " B"

    m = re.search('total (.+?) MiB', output)
    if m:
        try:
            global total_size
            total_size = int(m.group(1))
        except ValueError:
            pytest.fail("QSPI total size not recognized")

        total_size *= 1024 * 1024
        print 'Total size is: ' + str(total_size) + " B"

# Find out minimum and maximum frequnecies that device can operate
def qspi_find_freq_range(u_boot_console):
    f = u_boot_console.config.env.get('env__qspi_freq', None)
    global min_f
    global max_f
    global loop

    if not f:
        min_f = 0
        max_f = 0
        loop = 1
        return

    min_f = f.get('min_freq', None)
    if not min_f:
        min_f = 0
    max_f = f.get('max_freq', None)
    if not max_f:
        max_f = 0

    if max_f < min_f:
        max_f = min_f

    if min_f == 0 and max_f == 0:
        loop = 1
        return

    loop = 5

# Read the whole QSPI flash twice, random_size till full flash size, random till page size
def qspi_read_twice(u_boot_console):

    expected_read = "Read: OK"

    # TODO maybe add alignment and different start for pages
    for size in random.randint(4, page_size), random.randint(4, total_size), total_size:
        addr = u_boot_utils.find_ram_base(u_boot_console)
        # FIXME using 0 is failing for me
        output = u_boot_console.run_command('sf read %x 0 %x' % (addr + total_size, size))
        assert expected_read in output
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size, size))
        m = re.search('==> (.+?)', output)
        if not m:
            pytest.fail("CRC32 failed")
        expected_crc32 = m.group(1)
        output = u_boot_console.run_command('sf read %x 0 %x' % (addr + total_size + 10, size))
        assert expected_read in output
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size + 10, size))
        assert expected_crc32 in output

@pytest.mark.buildconfigspec("cmd_sf")
@pytest.mark.buildconfigspec("cmd_bdi")
@pytest.mark.buildconfigspec("cmd_memory")
def test_qspi_read_twice(u_boot_console):
    qspi_find_freq_range(u_boot_console)
    i = 0
    while i < loop:
        qspi_pre_commands(u_boot_console, random.randint(min_f, max_f))
        qspi_read_twice(u_boot_console)
        i = i + 1

# This test check crossing boundary for dual/parralel configurations
def qspi_erase_block(u_boot_console):

    expected_erase = "Erased: OK"
    for start in range(0, total_size, erase_size):
        output = u_boot_console.run_command('sf erase %x %x' % (start, erase_size))
        assert expected_erase in output

@pytest.mark.buildconfigspec('cmd_sf')
def test_qspi_erase_block(u_boot_console):
    qspi_find_freq_range(u_boot_console)
    i = 0
    while i < loop:
        qspi_pre_commands(u_boot_console, random.randint(min_f, max_f))
        qspi_erase_block(u_boot_console)
        i = i + 1

# Random write till page size, random till size and full size
def qspi_write_twice(u_boot_console):

    expected_write = "Written: OK"
    expected_read = "Read: OK"

    old_size = 0
    # TODO maybe add alignment and different start for pages
    for size in random.randint(4, page_size), random.randint(page_size, total_size), total_size:
        addr = u_boot_utils.find_ram_base(u_boot_console)
        size = size - old_size
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size, size))
        m = re.search('==> (.+?)', output)
        if not m:
            pytest.fail("CRC32 failed")

        expected_crc32 = m.group(1)
        # print expected_crc32
        output = u_boot_console.run_command('sf write %x %x %x' % (addr + total_size, old_size, size))
        assert expected_write in output
        output = u_boot_console.run_command('sf read %x %x %x' % (addr + total_size + 10, old_size, size))
        assert expected_read in output
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size + 10, size))
        assert expected_crc32 in output
        old_size = size

@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_memory')
def test_qspi_write_twice(u_boot_console):
    qspi_find_freq_range(u_boot_console)
    i = 0
    while i < loop:
        qspi_pre_commands(u_boot_console, random.randint(min_f, max_f))
        qspi_write_twice(u_boot_console)
        i = i + 1

def qspi_erase_all(u_boot_console):
    timeout = 10000000

    expected_erase = "Erased: OK"
    start = 0
    with u_boot_console.temporary_timeout(timeout):
        output = u_boot_console.run_command('sf erase 0 ' + str(hex(total_size)))
        assert expected_erase in output

@pytest.mark.buildconfigspec("cmd_sf")
def test_qspi_erase_all(u_boot_console):
    qspi_find_freq_range(u_boot_console)
    i = 0
    while i < loop:
        qspi_pre_commands(u_boot_console, random.randint(min_f, max_f))
        qspi_erase_all(u_boot_console)
        i = i + 1

# Load FIT image and write boot.bin to start of qspi to be ready for qspi boot
def qspi_boot_images(u_boot_console):

    if not test_net.net_set_up:
        pytest.skip('Network not initialized')

    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)
    test_net.test_net_tftpboot(u_boot_console)

    f = u_boot_console.config.env.get('env__net_tftp_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)
    if not addr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    map = 0x0
    temp = 0x50000
    expected_write = "OK"
    output = u_boot_console.run_command('imxtract %x boot@1 %x' % (addr, temp))
    assert expected_write in output

    expected_erase = "Erased: OK"
    output = u_boot_console.run_command('sf erase %x +$filesize ' % map)
    assert expected_erase in output

    expected_write = "Written: OK"
    output = u_boot_console.run_command('sf write %x %x $filesize' % (temp, map))
    assert expected_write in output

    map = u_boot_console.config.buildconfig.get('config_sys_spi_u_boot_offs', "0x1000")
    map = int(map, 16)
    expected_write = "OK"
    output = u_boot_console.run_command('imxtract %x boot@2 %x' % (addr, temp))
    assert expected_write in output

    expected_erase = "Erased: OK"
    output = u_boot_console.run_command('sf erase %x +$filesize ' % map)
    assert expected_erase in output

    expected_write = "Written: OK"
    output = u_boot_console.run_command('sf write %x %x $filesize' % (temp, map))
    assert expected_write in output

@pytest.mark.buildconfigspec("cmd_bdi")
@pytest.mark.buildconfigspec("cmd_sf")
@pytest.mark.buildconfigspec("cmd_ximg")
def test_qspi_boot_images(u_boot_console):
    qspi_find_freq_range(u_boot_console)
    i = 0
    while i < loop:
        qspi_pre_commands(u_boot_console, random.randint(min_f, max_f))
        qspi_boot_images(u_boot_console)
        i = i + 1

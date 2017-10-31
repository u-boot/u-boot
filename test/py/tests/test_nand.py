# Copyright (c) 2017, Xilinx Inc.
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import re
import random
import u_boot_utils

"""
Note: This test relies on boardenv_* containing configuration value to define
the nand device total size available for testing(env__nand_size). Without this,
the testwill be failed.

TODO:
- Take flash start and end addresses from boardenv_* so that the test will work
  on specified range and will not touch any reserved areas in flash.
- Handle multiple controller cases, as of now it assumes one.

"""
pytestmark = [pytest.mark.buildconfigspec("cmd_bdi"),
	      pytest.mark.buildconfigspec("cmd_nand"),
	      pytest.mark.buildconfigspec("cmd_memory")]

nand_detected = False
page_size = 0
erase_size = 0
total_size = 0
sector_size = 0

# Find out nand memory parameters
def nand_pre_commands(u_boot_console):
    global nand_detected
    if nand_detected:
        return

    output = u_boot_console.run_command('nand info')
    if not "Device 0: nand0" in output:
        pytest.skip('No NAND device available')

    m = re.search('Page size (.+?) b', output)
    if m:
        try:
            global page_size
            page_size = int(m.group(1))
        except ValueError:
            pytest.fail("NAND page size not recognized")

        print 'Page size is: ' + str(page_size) + " B"

    m = re.search('sector size (.+?) KiB', output)
    if m:
        try:
           global erase_size
           global sector_size
           erase_size = int(m.group(1))
           sector_size = erase_size
        except ValueError:
           pytest.fail("NAND erase size not recognized")

        erase_size *= 1024
        print 'Erase size is: ' + str(erase_size) + " B"

    output = u_boot_console.run_command('nand bad')
    if not "bad blocks:" in output:
        pytest.skip('No NAND device available')

    count = 0
    m = re.search('bad blocks:([\n\s\s\d\w]*)', output)
    if m:
        print m.group(1)
        var = m.group(1).split()
        count = len(var)
    print 'bad blocks count= ' + str(count)

    global total_size
    total_size = u_boot_console.config.env.get('env__nand_size', False)
    print total_size
    m = re.search('(.+?)MiB', total_size)
    if m:
        try:
            total_size = int(m.group(1))
            total_size *= 1024 * 1024
            print 'Total size is: ' + str(total_size) + " B"
            total_size -= count * sector_size * 1024
            print 'New Total size is: ' + str(total_size) + " B"
        except ValueError:
            pytest.fail("NAND size not recognized")

    nand_detected = True

# Read the whole NAND flash twice, random_size till full flash size, random till page size
def test_nand_read_twice(u_boot_console):
    nand_pre_commands(u_boot_console)

    expected_read = "read: OK"

    for size in random.randint(4, page_size), random.randint(4, total_size), total_size:
        addr = u_boot_utils.find_ram_base(u_boot_console)

        output = u_boot_console.run_command('nand read %x 0 %x' % (addr + total_size, size))
        assert expected_read in output

        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size, size))
        m = re.search('==> (.+?)', output)
        if not m:
            pytest.fail("CRC32 failed")
        expected_crc32 = m.group(1)

        output = u_boot_console.run_command('nand read %x 0 %x' % (addr + total_size + 10, size))
        assert expected_read in output

        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size + 10, size))
        assert expected_crc32 in output

# Random write till page size, random till size and full size
def test_nand_write_twice(u_boot_console):
    nand_pre_commands(u_boot_console)

    expected_write = "written: OK"
    expected_read = "read: OK"
    expected_erase = "100% complete."
    old_size = 0

    for size in random.randint(4, page_size), random.randint(page_size, total_size), total_size:
        addr = u_boot_utils.find_ram_base(u_boot_console)
        size = size - old_size
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size, size))
        m = re.search('==> (.+?)', output)
        if not m:
            pytest.fail("CRC32 failed")

        expected_crc32 = m.group(1)
        if old_size % page_size:
            old_size /= page_size
            old_size *= page_size

        if old_size+size > total_size:
            size = total_size - old_size

        erasesize = old_size/erase_size
        erasesize *= erase_size

        output = u_boot_console.run_command('nand erase.spread %x %x' % (erasesize, size))
        assert expected_erase in output

        # print expected_crc32
        output = u_boot_console.run_command('nand write %x %x %x' % (addr + total_size, old_size, size))
        assert expected_write in output
        output = u_boot_console.run_command('nand read %x %x %x' % (addr + total_size + 10, old_size, size))
        assert expected_read in output
        output = u_boot_console.run_command('crc32 %x %x' % (addr + total_size + 10, size))
        assert expected_crc32 in output
        old_size = size

def test_nand_erase_block(u_boot_console):
    nand_pre_commands(u_boot_console)

    expected_erase = "100% complete."
    for start in range(0, total_size, erase_size):
        output = u_boot_console.run_command('nand erase.spread %x %x' % (start, erase_size))
        assert expected_erase in output

def test_nand_erase_all(u_boot_console):
    nand_pre_commands(u_boot_console)

    timeout = 100000

    expected_erase = "100% complete."
    start = 0
    with u_boot_console.temporary_timeout(timeout):
        output = u_boot_console.run_command('nand erase.spread 0 ' + str(hex(total_size)))
        assert expected_erase in output

# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

"""
Note: This test relies on boardenv_* containing configuration values to define
the nand device total size and timeout available for testing. Without this, the
test will be automatically skipped. This test will be also skipped if the NAND
flash device is not detected.

For example:

.. code-block:: python

   # Setup env__nand_device_test to set the NAND flash total size and timeout.
   env__nand_device_test = {
       'size': '8192 MB',
       'timeout': 100000,
   }
"""

import pytest
import random
import re
import utils

def nand_pre_commands(ubman):
    """Probe the NAND flash device and gather geometry from `nand info`.

    Args:
        ubman: A U-Boot console connection.

    Returns:
        A dictionary with the following keys:
            page_size: NAND page size in bytes.
            erase_size: NAND erase (sector) size in bytes.
            total_size: Usable NAND size in bytes (bad blocks subtracted).
            timeout: Timeout in milliseconds for long-running operations.
    """

    f = ubman.config.env.get('env__nand_device_test', None)
    if not f:
        pytest.skip('No env file to read for NAND device test')

    total_size = f.get('size', None)
    timeout = f.get('timeout')

    if not total_size:
        pytest.skip('NAND device size not recognized')

    output = ubman.run_command('nand info')
    if not 'Device 0: nand0' in output:
        pytest.skip('No NAND device available')

    m = re.search(r'Page size\s+(\d+)\s*b', output)
    if not m:
        pytest.fail('NAND page size not recognized')
    page_size = int(m.group(1))

    m = re.search(r'sector size\s+(\d+)\s*KiB', output)
    if not m:
        pytest.fail('NAND erase size not recognized')
    sector_size_kib = int(m.group(1))
    erase_size = sector_size_kib * 1024

    output = ubman.run_command('nand bad')
    if not 'bad blocks:' in output:
        pytest.fail('NAND bad blocks output not recognized')

    count = 0
    m = re.search(r'bad blocks:([\s\d\w]*)', output)
    if m:
        count = len(m.group(1).split())

    m = re.search(r'(\d+)\s*MB', total_size)
    if not m:
        pytest.fail('NAND size not recognized')
    total_size = int(m.group(1)) * 1024 * 1024
    total_size -= count * sector_size_kib * 1024

    return {
        'page_size': page_size,
        'erase_size': erase_size,
        'total_size': total_size,
        'timeout': timeout,
    }

@pytest.mark.buildconfigspec('cmd_nand')
@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_memory')
def test_nand_read_twice(ubman):
    """This test reads the whole NAND flash twice, random_size till full flash
    size, random till page size.
    """

    nand_params = nand_pre_commands(ubman)
    page_size = nand_params['page_size']
    total_size = nand_params['total_size']
    expected_read = 'read: OK'

    for size in (random.randint(4, page_size),
                 random.randint(4, total_size),
                 total_size):
        addr = utils.find_ram_base(ubman)

        output = ubman.run_command(
            'nand read %x 0 %x' % (addr + total_size, size)
        )
        assert expected_read in output

        expected_crc32 = utils.crc32(ubman, addr + total_size, size)

        output = ubman.run_command(
            'nand read %x 0 %x' % (addr + total_size + 10, size)
        )
        assert expected_read in output

        crc32_readback = utils.crc32(ubman, addr + total_size + 10, size)
        assert expected_crc32 == crc32_readback

@pytest.mark.buildconfigspec('cmd_nand')
@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_memory')
def test_nand_write_twice(ubman):
    """This test does the random writes till page size, size and full size"""

    nand_params = nand_pre_commands(ubman)
    page_size = nand_params['page_size']
    erase_size = nand_params['erase_size']
    total_size = nand_params['total_size']
    expected_write = 'written: OK'
    expected_read = 'read: OK'
    expected_erase = '100% complete.'
    old_size = 0

    for size in (
        random.randint(4, page_size),
        random.randint(page_size, total_size),
        total_size,
    ):
        offset = page_size
        addr = utils.find_ram_base(ubman)
        size = size - old_size
        expected_crc32 = utils.crc32(ubman, addr + total_size, size)

        if old_size % page_size:
            old_size = int(old_size / page_size + 1)
            old_size *= page_size

        if old_size + size > total_size:
            size = total_size - old_size

        eraseoffset = int(old_size / erase_size)
        eraseoffset *= erase_size

        erasesize = int(size / erase_size + 1)
        erasesize *= erase_size

        output = ubman.run_command(
            'nand erase.spread %x %x' % (eraseoffset, erasesize)
        )
        assert expected_erase in output

        output = ubman.run_command(
            'nand write %x %x %x' % (addr + total_size, old_size, size)
        )
        assert expected_write in output
        output = ubman.run_command(
            'nand read %x %x %x' % (addr + total_size + offset, old_size, size)
        )
        assert expected_read in output
        crc32_readback = utils.crc32(ubman, addr + total_size + offset, size)
        assert expected_crc32 == crc32_readback
        old_size = size

@pytest.mark.buildconfigspec('cmd_nand')
def test_nand_erase_block(ubman):
    """Erase the NAND flash one erase block at a time."""

    nand_params = nand_pre_commands(ubman)
    erase_size = nand_params['erase_size']
    total_size = nand_params['total_size']

    expected_erase = '100% complete.'
    for start in range(0, total_size, erase_size):
        output = ubman.run_command(
            'nand erase.spread %x %x' % (start, erase_size)
        )
        assert expected_erase in output

@pytest.mark.buildconfigspec('cmd_nand')
def test_nand_erase_all(ubman):
    """Erase the entire NAND flash in a single operation."""

    nand_params = nand_pre_commands(ubman)
    total_size = nand_params['total_size']
    timeout = nand_params['timeout']

    expected_erase = '100% complete.'
    with ubman.temporary_timeout(timeout):
        output = ubman.run_command('nand erase.spread 0 %x' % total_size)
        assert expected_erase in output

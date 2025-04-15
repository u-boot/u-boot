# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2024, Advanced Micro Devices, Inc.

"""
Note: This test relies on boardenv_* containing configuration values to define
spi minimum and maximum frequencies at which the flash part can operate on and
these tests run at different spi frequency randomised values in the range
multiple times based on the user defined iteration value.
It also defines the SPI bus number containing the SPI-flash chip, SPI
chip-select, SPI mode, SPI flash part name and timeout parameters. If minimum
and maximum frequency is not defined, it will run on freq 0 by default.

Without the boardenv_* configuration, this test will be automatically skipped.

It also relies on configuration values for supported flashes for lock and
unlock cases for SPI family flash. It will run lock-unlock cases only for the
supported flash parts.

For Example:

# Details of SPI device test parameters required for SPI device testing:

# bus - SPI bus number to init the flash device
# chip_select - SPI chip select number to init the flash device
# min_freq - Minimum frequency in hz at which the flash part can operate, set 0
# or None for default frequency
# max_freq - Maximum frequency in hz at which the flash part can operate, set 0
# or None for default frequency
# mode - SPI mode to init the flash device
# part_name - SPI flash part name to be detected
# timeout - Default timeout to run the sf commands
# iteration - No of iteration to run SPI flash test

env__spi_device_test = {
    'bus': 0,
    'chip_select': 0,
    'min_freq': 10000000,
    'max_freq': 100000000,
    'mode': 0,
    'part_name': 'n25q00a',
    'timeout': 100000,
    'iteration': 5,
}

# supported_flash - Flash parts name which support lock-unlock functionality
env__spi_lock_unlock = {
    'supported_flash': 'mt25qu512a, n25q00a, n25q512ax3',
}
"""

import random
import re
import pytest
import utils

SPI_DATA = {}
EXPECTED_ERASE = 'Erased: OK'
EXPECTED_WRITE = 'Written: OK'
EXPECTED_READ = 'Read: OK'
EXPECTED_ERASE_ERRORS = [
    'Erase operation failed',
    'Attempted to modify a protected sector',
    'Erased: ERROR',
    'is protected and cannot be erased',
    'ERROR: flash area is locked',
]
EXPECTED_WRITE_ERRORS = [
    'ERROR: flash area is locked',
    'Program operation failed',
    'Attempted to modify a protected sector',
    'Written: ERROR',
]

def get_params_spi(ubman):
    ''' Get SPI device test parameters from boardenv file '''
    f = ubman.config.env.get('env__spi_device_test', None)
    if not f:
        pytest.skip('No SPI test device configured')

    bus = f.get('bus', 0)
    cs = f.get('chip_select', 0)
    mode = f.get('mode', 0)
    part_name = f.get('part_name', None)
    timeout = f.get('timeout', None)

    if not part_name:
        pytest.skip('No SPI test device configured')

    return bus, cs, mode, part_name, timeout

def spi_find_freq_range(ubman):
    '''Find out minimum and maximum frequnecies that SPI device can operate'''
    f = ubman.config.env.get('env__spi_device_test', None)
    if not f:
        pytest.skip('No SPI test device configured')

    min_f = f.get('min_freq', None)
    max_f = f.get('max_freq', None)
    iterations = f.get('iteration', 1)

    if not min_f:
        min_f = 0
    if not max_f:
        max_f = 0

    max_f = max(max_f, min_f)

    return min_f, max_f, iterations

def spi_pre_commands(ubman, freq):
    ''' Find out SPI family flash memory parameters '''
    bus, cs, mode, part_name, timeout = get_params_spi(ubman)

    output = ubman.run_command(f'sf probe {bus}:{cs} {freq} {mode}')
    if not 'SF: Detected' in output:
        pytest.fail('No SPI device available')

    if not part_name in output:
        pytest.fail('Not recognized the SPI flash part name')

    m = re.search('page size (.+?) Bytes', output)
    assert m
    try:
        page_size = int(m.group(1))
    except ValueError:
        pytest.fail('Not recognized the SPI page size')

    m = re.search('erase size (.+?) KiB', output)
    assert m
    try:
        erase_size = int(m.group(1))
        erase_size *= 1024
    except ValueError:
        pytest.fail('Not recognized the SPI erase size')

    m = re.search('total (.+?) MiB', output)
    assert m
    try:
        total_size = int(m.group(1))
        total_size *= 1024 * 1024
    except ValueError:
        pytest.fail('Not recognized the SPI total size')

    m = re.search('Detected (.+?) with', output)
    assert m
    try:
        flash_part = m.group(1)
        assert flash_part == part_name
    except ValueError:
        pytest.fail('Not recognized the SPI flash part')

    global SPI_DATA
    SPI_DATA = {
        'page_size': page_size,
        'erase_size': erase_size,
        'total_size': total_size,
        'flash_part': flash_part,
        'timeout': timeout,
    }

def get_page_size():
    ''' Get the SPI page size from spi data '''
    return SPI_DATA['page_size']

def get_erase_size():
    ''' Get the SPI erase size from spi data '''
    return SPI_DATA['erase_size']

def get_total_size():
    ''' Get the SPI total size from spi data '''
    return SPI_DATA['total_size']

def get_flash_part():
    ''' Get the SPI flash part name from spi data '''
    return SPI_DATA['flash_part']

def get_timeout():
    ''' Get the SPI timeout from spi data '''
    return SPI_DATA['timeout']

def spi_erase_block(ubman, erase_size, total_size):
    ''' Erase SPI flash memory block wise '''
    for start in range(0, total_size, erase_size):
        output = ubman.run_command(f'sf erase {hex(start)} {hex(erase_size)}')
        assert EXPECTED_ERASE in output

@pytest.mark.buildconfigspec('cmd_sf')
def test_spi_erase_block(ubman):
    ''' Test case to check SPI erase functionality by erasing memory regions
    block-wise '''

    min_f, max_f, loop = spi_find_freq_range(ubman)
    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        spi_erase_block(ubman, get_erase_size(), get_total_size())
        i = i + 1

def spi_write_twice(ubman, page_size, erase_size, total_size, timeout):
    ''' Random write till page size, random till size and full size '''
    addr = utils.find_ram_base(ubman)

    old_size = 0
    for size in (
        random.randint(4, page_size),
        random.randint(page_size, total_size),
        total_size,
    ):
        offset = random.randint(4, page_size)
        offset = offset & ~3
        size = size & ~3
        size = size - old_size
        output = ubman.run_command(f'crc32 {hex(addr + total_size)} {hex(size)}')
        m = re.search('==> (.+?)$', output)
        if not m:
            pytest.fail('CRC32 failed')

        expected_crc32 = m.group(1)
        if old_size % page_size:
            old_size = int(old_size / page_size)
            old_size *= page_size

        if size % erase_size:
            erasesize = int(size / erase_size + 1)
            erasesize *= erase_size

        eraseoffset = int(old_size / erase_size)
        eraseoffset *= erase_size

        timeout = 100000000
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf erase {hex(eraseoffset)} {hex(erasesize)}'
            )
            assert EXPECTED_ERASE in output

        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf write {hex(addr + total_size)} {hex(old_size)} {hex(size)}'
            )
            assert EXPECTED_WRITE in output
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf read {hex(addr + total_size + offset)} {hex(old_size)} {hex(size)}'
            )
            assert EXPECTED_READ in output
        output = ubman.run_command(
            f'crc32 {hex(addr + total_size + offset)} {hex(size)}'
        )
        assert expected_crc32 in output
        old_size = size

@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_memory')
def test_spi_write_twice(ubman):
    ''' Test to write data with random size twice for SPI '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        spi_write_twice(
            ubman,
            get_page_size(),
            get_erase_size(),
            get_total_size(),
            get_timeout()
        )
        i = i + 1

def spi_write_continues(ubman, page_size, erase_size, total_size, timeout):
    ''' Write with random size of data to continue SPI write case '''
    spi_erase_block(ubman, erase_size, total_size)
    addr = utils.find_ram_base(ubman)

    output = ubman.run_command(f'crc32 {hex(addr + 0x10000)} {hex(total_size)}')
    m = re.search('==> (.+?)$', output)
    if not m:
        pytest.fail('CRC32 failed')
    expected_crc32 = m.group(1)

    old_size = 0
    for size in (
        random.randint(4, page_size),
        random.randint(page_size, total_size),
        total_size,
    ):
        size = size & ~3
        size = size - old_size
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf write {hex(addr + 0x10000 + old_size)} {hex(old_size)} {hex(size)}'
            )
            assert EXPECTED_WRITE in output
        old_size += size

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command(
            f'sf read {hex(addr + 0x10000 + total_size)} 0 {hex(total_size)}'
        )
        assert EXPECTED_READ in output

    output = ubman.run_command(
        f'crc32 {hex(addr + 0x10000 + total_size)} {hex(total_size)}'
    )
    assert expected_crc32 in output

@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_memory')
def test_spi_write_continues(ubman):
    ''' Test to write more random size data for SPI '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        spi_write_twice(
            ubman,
            get_page_size(),
            get_erase_size(),
            get_total_size(),
            get_timeout(),
        )
        i = i + 1

def spi_read_twice(ubman, page_size, total_size, timeout):
    ''' Read the whole SPI flash twice, random_size till full flash size,
    random till page size '''
    for size in random.randint(4, page_size), random.randint(4, total_size), total_size:
        addr = utils.find_ram_base(ubman)
        size = size & ~3
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf read {hex(addr + total_size)} 0 {hex(size)}'
            )
            assert EXPECTED_READ in output
        output = ubman.run_command(f'crc32 {hex(addr + total_size)} {hex(size)}')
        m = re.search('==> (.+?)$', output)
        if not m:
            pytest.fail('CRC32 failed')
        expected_crc32 = m.group(1)
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf read {hex(addr + total_size + 10)} 0 {hex(size)}'
            )
            assert EXPECTED_READ in output
        output = ubman.run_command(
            f'crc32 {hex(addr + total_size + 10)} {hex(size)}'
        )
        assert expected_crc32 in output

@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_memory')
def test_spi_read_twice(ubman):
    ''' Test to read random data twice from SPI '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        spi_read_twice(ubman, get_page_size(), get_total_size(), get_timeout())
        i = i + 1

def spi_erase_all(ubman, total_size, timeout):
    ''' Erase the full chip SPI '''
    start = 0
    with ubman.temporary_timeout(timeout):
        output = ubman.run_command(f'sf erase {start} {hex(total_size)}')
        assert EXPECTED_ERASE in output

@pytest.mark.buildconfigspec('cmd_sf')
def test_spi_erase_all(ubman):
    ''' Test to check full chip erase for SPI '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        spi_erase_all(ubman, get_total_size(), get_timeout())
        i = i + 1

def flash_ops(
    ubman, ops, start, size, offset=0, exp_ret=0, exp_str='', not_exp_str=''
):
    ''' Flash operations: erase, write and read '''

    f = ubman.config.env.get('env__spi_device_test', None)
    if not f:
        timeout = 1000000

    timeout = f.get('timeout', 1000000)

    if ops == 'erase':
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(f'sf erase {hex(start)} {hex(size)}')
    else:
        with ubman.temporary_timeout(timeout):
            output = ubman.run_command(
                f'sf {ops} {hex(offset)} {hex(start)} {hex(size)}'
            )

    if exp_str:
        assert exp_str in output
    if not_exp_str:
        assert not_exp_str not in output

    ret_code = ubman.run_command('echo $?')
    if exp_ret >= 0:
        assert ret_code.endswith(str(exp_ret))

    return output, ret_code

def spi_unlock_exit(ubman, addr, size):
    ''' Unlock the flash before making it fail '''
    ubman.run_command(f'sf protect unlock {hex(addr)} {hex(size)}')
    assert False, 'FAIL: Flash lock is unable to protect the data!'

def find_prot_region(lock_addr, lock_size):
    ''' Get the protected and un-protected region of flash '''
    total_size = get_total_size()
    erase_size = get_erase_size()

    if lock_addr < (total_size // 2):
        sect_num = (lock_addr + lock_size) // erase_size
        x = 1
        while x < sect_num:
            x *= 2
        prot_start = 0
        prot_size = x * erase_size
        unprot_start = prot_start + prot_size
        unprot_size = total_size - unprot_start
    else:
        sect_num = (total_size - lock_addr) // erase_size
        x = 1
        while x < sect_num:
            x *= 2
        prot_start = total_size - (x * erase_size)
        prot_size = total_size - prot_start
        unprot_start = 0
        unprot_size = prot_start

    return prot_start, prot_size, unprot_start, unprot_size

def protect_ops(ubman, lock_addr, lock_size, ops="unlock"):
    ''' Run the command to lock or Unlock the flash '''
    ubman.run_command(f'sf protect {ops} {hex(lock_addr)} {hex(lock_size)}')
    output = ubman.run_command('echo $?')
    if ops == "lock" and not output.endswith('0'):
        ubman.run_command(f'sf protect unlock {hex(lock_addr)} {hex(lock_size)}')
        assert False, "sf protect lock command exits with non-zero return code"
    assert output.endswith('0')

def erase_write_ops(ubman, start, size):
    ''' Basic erase and write operation for flash '''
    addr = utils.find_ram_base(ubman)
    flash_ops(ubman, 'erase', start, size, 0, 0, EXPECTED_ERASE)
    flash_ops(ubman, 'write', start, size, addr, 0, EXPECTED_WRITE)

def spi_lock_unlock(ubman, lock_addr, lock_size):
    ''' Lock unlock operations for SPI family flash '''
    addr = utils.find_ram_base(ubman)
    erase_size = get_erase_size()

    # Find the protected/un-protected region
    prot_start, prot_size, unprot_start, unprot_size = find_prot_region(lock_addr, lock_size)

    # Check erase/write operation before locking
    erase_write_ops(ubman, prot_start, prot_size)

    # Locking the flash
    protect_ops(ubman, lock_addr, lock_size, 'lock')

    # Check erase/write operation after locking
    output, ret_code = flash_ops(ubman, 'erase', prot_start, prot_size, 0, -1)
    if not any(error in output for error in EXPECTED_ERASE_ERRORS) or ret_code.endswith(
        '0'
    ):
        spi_unlock_exit(ubman, lock_addr, lock_size)

    output, ret_code = flash_ops(
        ubman, 'write', prot_start, prot_size, addr, -1
    )
    if not any(error in output for error in EXPECTED_WRITE_ERRORS) or ret_code.endswith(
        '0'
    ):
        spi_unlock_exit(ubman, lock_addr, lock_size)

    # Check locked sectors
    sect_lock_start = random.randrange(prot_start, (prot_start + prot_size), erase_size)
    if prot_size > erase_size:
        sect_lock_size = random.randrange(
            erase_size, (prot_start + prot_size - sect_lock_start), erase_size
        )
    else:
        sect_lock_size = erase_size
    sect_write_size = random.randint(1, sect_lock_size)

    output, ret_code = flash_ops(
        ubman, 'erase', sect_lock_start, sect_lock_size, 0, -1
    )
    if not any(error in output for error in EXPECTED_ERASE_ERRORS) or ret_code.endswith(
        '0'
    ):
        spi_unlock_exit(ubman, lock_addr, lock_size)

    output, ret_code = flash_ops(
        ubman, 'write', sect_lock_start, sect_write_size, addr, -1
    )
    if not any(error in output for error in EXPECTED_WRITE_ERRORS) or ret_code.endswith(
        '0'
    ):
        spi_unlock_exit(ubman, lock_addr, lock_size)

    # Check unlocked sectors
    if unprot_size != 0:
        sect_unlock_start = random.randrange(
            unprot_start, (unprot_start + unprot_size), erase_size
        )
        if unprot_size > erase_size:
            sect_unlock_size = random.randrange(
                erase_size, (unprot_start + unprot_size - sect_unlock_start), erase_size
            )
        else:
            sect_unlock_size = erase_size
        sect_write_size = random.randint(1, sect_unlock_size)

        output, ret_code = flash_ops(
            ubman, 'erase', sect_unlock_start, sect_unlock_size, 0, -1
        )
        if EXPECTED_ERASE not in output or ret_code.endswith('1'):
            spi_unlock_exit(ubman, lock_addr, lock_size)

        output, ret_code = flash_ops(
            ubman, 'write', sect_unlock_start, sect_write_size, addr, -1
        )
        if EXPECTED_WRITE not in output or ret_code.endswith('1'):
            spi_unlock_exit(ubman, lock_addr, lock_size)

    # Unlocking the flash
    protect_ops(ubman, lock_addr, lock_size, 'unlock')

    # Check erase/write operation after un-locking
    erase_write_ops(ubman, prot_start, prot_size)

    # Check previous locked sectors
    sect_lock_start = random.randrange(prot_start, (prot_start + prot_size), erase_size)
    if prot_size > erase_size:
        sect_lock_size = random.randrange(
            erase_size, (prot_start + prot_size - sect_lock_start), erase_size
        )
    else:
        sect_lock_size = erase_size
    sect_write_size = random.randint(1, sect_lock_size)

    flash_ops(
        ubman, 'erase', sect_lock_start, sect_lock_size, 0, 0, EXPECTED_ERASE
    )
    flash_ops(
        ubman,
        'write',
        sect_lock_start,
        sect_write_size,
        addr,
        0,
        EXPECTED_WRITE,
    )

@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_memory')
def test_spi_lock_unlock(ubman):
    ''' Test to check the lock-unlock functionality for SPI family flash '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    flashes = ubman.config.env.get('env__spi_lock_unlock', False)
    if not flashes:
        pytest.skip('No SPI test device configured for lock/unlock')

    i = 0
    while i < loop:
        spi_pre_commands(ubman, random.randint(min_f, max_f))
        total_size = get_total_size()
        flash_part = get_flash_part()

        flashes_list = flashes.get('supported_flash', None).split(',')
        flashes_list = [x.strip() for x in flashes_list]
        if flash_part not in flashes_list:
            pytest.skip('Detected flash does not support lock/unlock')

        # For lower half of memory
        lock_addr = random.randint(0, (total_size // 2) - 1)
        lock_size = random.randint(1, ((total_size // 2) - lock_addr))
        spi_lock_unlock(ubman, lock_addr, lock_size)

        # For upper half of memory
        lock_addr = random.randint((total_size // 2), total_size - 1)
        lock_size = random.randint(1, (total_size - lock_addr))
        spi_lock_unlock(ubman, lock_addr, lock_size)

        # For entire flash
        lock_addr = random.randint(0, total_size - 1)
        lock_size = random.randint(1, (total_size - lock_addr))
        spi_lock_unlock(ubman, lock_addr, lock_size)

        i = i + 1

@pytest.mark.buildconfigspec('cmd_bdi')
@pytest.mark.buildconfigspec('cmd_sf')
@pytest.mark.buildconfigspec('cmd_memory')
def test_spi_negative(ubman):
    ''' Negative tests for SPI '''
    min_f, max_f, loop = spi_find_freq_range(ubman)
    spi_pre_commands(ubman, random.randint(min_f, max_f))
    total_size = get_total_size()
    erase_size = get_erase_size()
    page_size = get_page_size()
    addr = utils.find_ram_base(ubman)
    i = 0
    while i < loop:
        # Erase negative test
        start = random.randint(0, total_size)
        esize = erase_size

        # If erasesize is not multiple of flash's erase size
        while esize % erase_size == 0:
            esize = random.randint(0, total_size - start)

        error_msg = 'Erased: ERROR'
        flash_ops(
            ubman, 'erase', start, esize, 0, 1, error_msg, EXPECTED_ERASE
        )

        # If eraseoffset exceeds beyond flash size
        eoffset = random.randint(total_size, (total_size + int(0x1000000)))
        error_msg = 'Offset exceeds device limit'
        flash_ops(
            ubman, 'erase', eoffset, esize, 0, 1, error_msg, EXPECTED_ERASE
        )

        # If erasesize exceeds beyond flash size
        esize = random.randint((total_size - start), (total_size + int(0x1000000)))
        error_msg = 'ERROR: attempting erase past flash size'
        flash_ops(
            ubman, 'erase', start, esize, 0, 1, error_msg, EXPECTED_ERASE
        )

        # If erase size is 0
        esize = 0
        error_msg = None
        flash_ops(
            ubman, 'erase', start, esize, 0, 1, error_msg, EXPECTED_ERASE
        )

        # If erasesize is less than flash's page size
        esize = random.randint(0, page_size)
        start = random.randint(0, (total_size - page_size))
        error_msg = 'Erased: ERROR'
        flash_ops(
            ubman, 'erase', start, esize, 0, 1, error_msg, EXPECTED_ERASE
        )

        # Write/Read negative test
        # if Write/Read size exceeds beyond flash size
        offset = random.randint(0, total_size)
        size = random.randint((total_size - offset), (total_size + int(0x1000000)))
        error_msg = 'Size exceeds partition or device limit'
        flash_ops(
            ubman, 'write', offset, size, addr, 1, error_msg, EXPECTED_WRITE
        )
        flash_ops(
            ubman, 'read', offset, size, addr, 1, error_msg, EXPECTED_READ
        )

        # if Write/Read offset exceeds beyond flash size
        offset = random.randint(total_size, (total_size + int(0x1000000)))
        size = random.randint(0, total_size)
        error_msg = 'Offset exceeds device limit'
        flash_ops(
            ubman, 'write', offset, size, addr, 1, error_msg, EXPECTED_WRITE
        )
        flash_ops(
            ubman, 'read', offset, size, addr, 1, error_msg, EXPECTED_READ
        )

        # if Write/Read size is 0
        offset = random.randint(0, 2)
        size = 0
        error_msg = None
        flash_ops(
            ubman, 'write', offset, size, addr, 1, error_msg, EXPECTED_WRITE
        )
        flash_ops(
            ubman, 'read', offset, size, addr, 1, error_msg, EXPECTED_READ
        )

        # Read to relocation address
        output = ubman.run_command('bdinfo')
        m = re.search(r'relocaddr\s*= (.+)', output)
        res_area = int(m.group(1), 16)

        start = 0
        size = 0x2000
        error_msg = 'ERROR: trying to overwrite reserved memory'
        flash_ops(
            ubman, 'read', start, size, res_area, 1, error_msg, EXPECTED_READ
        )

        # Start reading from the reserved area
        m = re.search(r'reserved\[0\]\s*\[(0x.+)-(0x.+)\]', output)
        if not m or int(m.group(1), 16) == 0:
            ubman.log.info('No reserved area is defined or start addr is 0x0!')
        else:
            rstart_area = int(m.group(1), 16)
            rend_area = int(m.group(2), 16)

            # Case 1: Start reading from the middle of the reserved area
            r_size = rend_area - rstart_area
            r_area = rstart_area + r_size
            flash_ops(
                ubman, 'read', start, size, r_area, 1, error_msg, EXPECTED_READ
            )

            # Case 2: Start reading from before the reserved area to cross-over
            # the reserved area
            rstart_area = rstart_area - int(size/2)
            flash_ops(
                ubman, 'read', start, size, rstart_area, 1, error_msg, EXPECTED_READ
            )

            # Case 3: Start reading till after the reserved area to cross-over
            # the reserved area
            rend_area = rend_area - int(size/2)
            flash_ops(
                ubman, 'read', start, size, rend_area, 1, error_msg, EXPECTED_READ
            )

        i = i + 1

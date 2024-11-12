# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import re
import u_boot_utils

"""
Note: This test doesn't rely on boardenv_* configuration values but it can
change the test behavior. To test MMC file system cases (fat32, ext2, ext4),
MMC device should be formatted and valid partitions should be created for
different file system, otherwise it may leads to failure. This test will be
skipped if the MMC device is not detected.

For example:

# Setup env__mmc_device_test_skip to not skipping the test. By default, its
# value is set to True. Set it to False to run all tests for MMC device.
env__mmc_device_test_skip = False

# Setup env__mmc_device to set the supported mmc modes to be tested
env__mmc_device {
    'mmc_modes': ['MMC_LEGACY', 'SD_HS'],
}

"""

mmc_set_up = False
controllers = 0
devices = {}
mmc_modes_name = []
mmc_modes = []

def setup_mmc_modes(cons):
    global mmc_modes, mmc_modes_name
    f = cons.config.env.get('env__mmc_device', None)
    if f:
        mmc_modes_name = f.get('mmc_modes', None)

    # Set mmc mode to default mode (legacy), if speed mode config isn't enabled
    if cons.config.buildconfig.get('config_mmc_speed_mode_set', 'n') != 'y':
        mmc_modes = [0]
        return

    if mmc_modes_name:
        mmc_help = cons.run_command('mmc -help')
        m = re.search(r"\[MMC_LEGACY(.*\n.+])", mmc_help)
        modes = [
            x.strip()
            for x in m.group()
            .replace('\n', '')
            .replace('[', '')
            .replace(']', '')
            .split(',')
        ]

        for mode in mmc_modes_name:
            mmc_modes += [modes.index(mode)]
    else:
        # Set mmc mode to default mode (legacy), if it is not defined in env
        mmc_modes = [0]

def setup_mmc(u_boot_console):
    if u_boot_console.config.env.get('env__mmc_device_test_skip', True):
        pytest.skip('MMC device test is not enabled')

    setup_mmc_modes(u_boot_console)

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_list(u_boot_console):
    setup_mmc(u_boot_console)
    output = u_boot_console.run_command('mmc list')
    if 'No MMC device available' in output:
        pytest.skip('No SD/MMC/eMMC controller available')

    if 'Card did not respond to voltage select' in output:
        pytest.skip('No SD/MMC card present')

    array = output.split()
    global devices
    global controllers
    controllers = int(len(array) / 2)
    for x in range(0, controllers):
        y = x * 2
        devices[x] = {}
        devices[x]['name'] = array[y]

    global mmc_set_up
    mmc_set_up = True

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_dev(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    fail = 0
    for x in range(0, controllers):
        devices[x]['detected'] = 'yes'

        for y in mmc_modes:
            output = u_boot_console.run_command('mmc dev %d 0 %d' % x, y)

            if 'Card did not respond to voltage select' in output:
                fail = 1
                devices[x]['detected'] = 'no'

            if 'no mmc device at slot' in output:
                devices[x]['detected'] = 'no'

            if 'MMC: no card present' in output:
                devices[x]['detected'] = 'no'

        if fail:
            pytest.fail('Card not present')

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmcinfo(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for y in mmc_modes:
                u_boot_console.run_command('mmc dev %d 0 %d' % x, y)
                output = u_boot_console.run_command('mmcinfo')
                if 'busy timeout' in output:
                    pytest.skip('No SD/MMC/eMMC device present')

                assert mmc_modes_name[mmc_modes.index(y)] in output

                obj = re.search(r'Capacity: (\d+|\d+[\.]?\d)', output)
                try:
                    capacity = float(obj.groups()[0])
                    print(capacity)
                    devices[x]['capacity'] = capacity
                    print('Capacity of dev %d is: %g GiB' % (x, capacity))
                except ValueError:
                    pytest.fail('MMC capacity not recognized')

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_info(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for y in mmc_modes:
                u_boot_console.run_command('mmc dev %d 0 %d' % x, y)

                output = u_boot_console.run_command('mmc info')
                assert mmc_modes_name[mmc_modes.index(y)] in output

                obj = re.search(r'Capacity: (\d+|\d+[\.]?\d)', output)
                try:
                    capacity = float(obj.groups()[0])
                    print(capacity)
                    if devices[x]['capacity'] != capacity:
                        pytest.fail("MMC capacity doesn't match mmcinfo")

                except ValueError:
                    pytest.fail('MMC capacity not recognized')

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_rescan(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for y in mmc_modes:
                u_boot_console.run_command('mmc dev %d 0 %d' % x, y)
                output = u_boot_console.run_command('mmc rescan')
                if output:
                    pytest.fail('mmc rescan has something to check')
                output = u_boot_console.run_command('echo $?')
                assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_part(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            u_boot_console.run_command('mmc dev %d' % x)
            output = u_boot_console.run_command('mmc part')

            lines = output.split('\n')
            part_fat = []
            part_ext2 = []
            part_ext4 = []
            for line in lines:
                obj = re.search(
                        r'(\d)\s+\d+\s+\d+\s+\w+\d+\w+-\d+\s+(\d+\w+)', line)
                if obj:
                    part_id = int(obj.groups()[0])
                    part_type = obj.groups()[1]
                    print('part_id:%d, part_type:%s' % (part_id, part_type))

                    if part_type in ['0c', '0b', '0e']:
                        print('Fat detected')
                        part_fat.append(part_id)
                    elif part_type == '83':
                        print('ext(2/4) detected')
                        output = u_boot_console.run_command(
                            'fstype mmc %d:%d' % x, part_id
                        )
                        if 'ext2' in output:
                            part_ext2.append(part_id)
                        elif 'ext4' in output:
                            part_ext4.append(part_id)
                    else:
                        pytest.fail('Unsupported Filesystem on device %d' % x)
            devices[x]['ext4'] = part_ext4
            devices[x]['ext2'] = part_ext2
            devices[x]['fat'] = part_fat

            if not part_ext2 and not part_ext4 and not part_fat:
                pytest.fail('No partition detected on device %d' % x)

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fat')
def test_mmc_fatls_fatinfo(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'fat'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    output = u_boot_console.run_command(
                            'fatls mmc %d:%s' % (x, part))
                    if 'Unrecognized filesystem type' in output:
                        partitions.remove(part)
                        pytest.fail('Unrecognized filesystem')

                    if not re.search(r'\d file\(s\), \d dir\(s\)', output):
                        pytest.fail('%s read failed on device %d' % (fs.upper, x))
                    output = u_boot_console.run_command(
                            'fatinfo mmc %d:%s' % (x, part))
                    string = 'Filesystem: %s' % fs.upper
                    if re.search(string, output):
                        pytest.fail('%s FS failed on device %d' % (fs.upper(), x))
                    part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())


@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
def test_mmc_fatload_fatwrite(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'fat'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    part_detect = 1
                    addr = u_boot_utils.find_ram_base(u_boot_console)
                    devices[x]['addr_%d' % part] = addr
                    size = random.randint(4, 1 * 1024 * 1024)
                    devices[x]['size_%d' % part] = size
                    # count CRC32
                    output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                    m = re.search('==> (.+?)', output)
                    if not m:
                        pytest.fail('CRC32 failed')
                    expected_crc32 = m.group(1)
                    devices[x]['expected_crc32_%d' % part] = expected_crc32
                    # do write
                    file = '%s_%d' % ('uboot_test', size)
                    devices[x]['file_%d' % part] = file
                    output = u_boot_console.run_command(
                        '%swrite mmc %d:%s %x %s %x' % (fs, x, part, addr, file, size)
                    )
                    assert 'Unable to write' not in output
                    assert 'Error' not in output
                    assert 'overflow' not in output
                    expected_text = '%d bytes written' % size
                    assert expected_text in output

                    alignment = int(
                        u_boot_console.config.buildconfig.get(
                            'config_sys_cacheline_size', 128
                        )
                    )
                    offset = random.randrange(alignment, 1024, alignment)
                    output = u_boot_console.run_command(
                        '%sload mmc %d:%s %x %s' % (fs, x, part, addr + offset, file)
                    )
                    assert 'Invalid FAT entry' not in output
                    assert 'Unable to read file' not in output
                    assert 'Misaligned buffer address' not in output
                    expected_text = '%d bytes read' % size
                    assert expected_text in output

                    output = u_boot_console.run_command(
                        'crc32 %x $filesize' % (addr + offset)
                    )
                    assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_ext4')
def test_mmc_ext4ls(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext4'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    output = u_boot_console.run_command(
                        '%sls mmc %d:%s' % (fs, x, part)
                    )
                    if 'Unrecognized filesystem type' in output:
                        partitions.remove(part)
                        pytest.fail('Unrecognized filesystem')
                    part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.buildconfigspec('ext4_write')
@pytest.mark.buildconfigspec('cmd_memory')
def test_mmc_ext4load_ext4write(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext4'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    part_detect = 1
                    addr = u_boot_utils.find_ram_base(u_boot_console)
                    devices[x]['addr_%d' % part] = addr
                    size = random.randint(4, 1 * 1024 * 1024)
                    devices[x]['size_%d' % part] = size
                    # count CRC32
                    output = u_boot_console.run_command('crc32 %x %x' % (addr, size))
                    m = re.search('==> (.+?)', output)
                    if not m:
                        pytest.fail('CRC32 failed')
                    expected_crc32 = m.group(1)
                    devices[x]['expected_crc32_%d' % part] = expected_crc32

                    # do write
                    file = '%s_%d' % ('uboot_test', size)
                    devices[x]['file_%d' % part] = file
                    output = u_boot_console.run_command(
                        '%swrite mmc %d:%s %x /%s %x' % (fs, x, part, addr, file, size)
                    )
                    assert 'Unable to write' not in output
                    assert 'Error' not in output
                    assert 'overflow' not in output
                    expected_text = '%d bytes written' % size
                    assert expected_text in output

                    offset = random.randrange(128, 1024, 128)
                    output = u_boot_console.run_command(
                        '%sload mmc %d:%s %x /%s' % (fs, x, part, addr + offset, file)
                    )
                    expected_text = '%d bytes read' % size
                    assert expected_text in output

                    output = u_boot_console.run_command(
                        'crc32 %x $filesize' % (addr + offset)
                    )
                    assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_ext2')
def test_mmc_ext2ls(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext2'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    part_detect = 1
                    output = u_boot_console.run_command(
                        '%sls mmc %d:%s' % (fs, x, part)
                    )
                    if 'Unrecognized filesystem type' in output:
                        partitions.remove(part)
                        pytest.fail('Unrecognized filesystem')
                    part_detect = 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_ext2')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.buildconfigspec('ext4_write')
@pytest.mark.buildconfigspec('cmd_memory')
def test_mmc_ext2load(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'ext2'
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    part_detect = 1
                    addr = devices[x]['addr_%d' % part]
                    size = devices[x]['size_%d' % part]
                    expected_crc32 = devices[x]['expected_crc32_%d' % part]
                    file = devices[x]['file_%d' % part]

                    offset = random.randrange(128, 1024, 128)
                    output = u_boot_console.run_command(
                        '%sload mmc %d:%s %x /%s' % (fs, x, part, addr + offset, file)
                    )
                    expected_text = '%d bytes read' % size
                    assert expected_text in output

                    output = u_boot_console.run_command(
                        'crc32 %x $filesize' % (addr + offset)
                    )
                    assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_mmc_ls(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for fs in ['fat', 'ext4', 'ext2']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    for y in mmc_modes:
                        u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                        part_detect = 1
                        output = u_boot_console.run_command('ls mmc %d:%s' % (x, part))
                        if re.search(r'No \w+ table on this device', output):
                            pytest.fail(
                                '%s: Partition table not found %d' % (fs.upper(), x)
                            )

    if not part_detect:
        pytest.skip('No partition detected')

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_mmc_load(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for fs in ['fat', 'ext4', 'ext2']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    for y in mmc_modes:
                        u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                        part_detect = 1
                        addr = devices[x]['addr_%d' % part]
                        size = devices[x]['size_%d' % part]
                        expected_crc32 = devices[x]['expected_crc32_%d' % part]
                        file = devices[x]['file_%d' % part]

                        offset = random.randrange(128, 1024, 128)
                        output = u_boot_console.run_command(
                            'load mmc %d:%s %x /%s' % (x, part, addr + offset, file)
                        )
                        expected_text = '%d bytes read' % size
                        assert expected_text in output

                        output = u_boot_console.run_command(
                            'crc32 %x $filesize' % (addr + offset)
                        )
                        assert expected_crc32 in output

    if not part_detect:
        pytest.skip('No partition detected')

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fs_generic')
def test_mmc_save(u_boot_console):
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            for fs in ['fat', 'ext4', 'ext2']:
                try:
                    partitions = devices[x][fs]
                except:
                    print('No %s table on this device' % fs.upper())
                    continue

                for part in partitions:
                    for y in mmc_modes:
                        u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                        part_detect = 1
                        addr = devices[x]['addr_%d' % part]
                        size = 0
                        file = devices[x]['file_%d' % part]

                        offset = random.randrange(128, 1024, 128)
                        output = u_boot_console.run_command(
                            'save mmc %d:%s %x /%s %d'
                            % (x, part, addr + offset, file, size)
                        )
                        expected_text = '%d bytes written' % size
                        assert expected_text in output

    if not part_detect:
        pytest.skip('No partition detected')

@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.buildconfigspec('cmd_fat')
@pytest.mark.buildconfigspec('cmd_memory')
def test_mmc_fat_read_write_files(u_boot_console):
    test_mmc_list(u_boot_console)
    test_mmc_dev(u_boot_console)
    test_mmcinfo(u_boot_console)
    test_mmc_part(u_boot_console)
    if not mmc_set_up:
        pytest.skip('No SD/MMC/eMMC controller available')

    if not devices:
        pytest.skip('No devices detected')

    part_detect = 0
    fs = 'fat'

    # Number of files to be written/read in MMC card
    num_files = 100

    for x in range(0, controllers):
        if devices[x]['detected'] == 'yes':
            try:
                partitions = devices[x][fs]
            except:
                print('No %s table on this device' % fs.upper())
                continue

            for part in partitions:
                for y in mmc_modes:
                    u_boot_console.run_command('mmc dev %d %d %d' % x, part, y)
                    part_detect = 1
                    addr = u_boot_utils.find_ram_base(u_boot_console)
                    count_f = 0
                    addr_l = []
                    size_l = []
                    file_l = []
                    crc32_l = []
                    offset_l = []
                    addr_l.append(addr)

                    while count_f < num_files:
                        size_l.append(random.randint(4, 1 * 1024 * 1024))

                        # CRC32 count
                        output = u_boot_console.run_command(
                            'crc32 %x %x' % (addr_l[count_f], size_l[count_f])
                        )
                        m = re.search('==> (.+?)', output)
                        if not m:
                            pytest.fail('CRC32 failed')
                        crc32_l.append(m.group(1))

                        # Write operation
                        file_l.append(
                            '%s_%d_%d' % ('uboot_test', count_f, size_l[count_f])
                        )
                        output = u_boot_console.run_command(
                            '%swrite mmc %d:%s %x %s %x'
                            % (
                                fs,
                                x,
                                part,
                                addr_l[count_f],
                                file_l[count_f],
                                size_l[count_f],
                            )
                        )
                        assert 'Unable to write' not in output
                        assert 'Error' not in output
                        assert 'overflow' not in output
                        expected_text = '%d bytes written' % size_l[count_f]
                        assert expected_text in output

                        addr_l.append(addr_l[count_f] + size_l[count_f] + 1048576)
                        count_f += 1

                    count_f = 0
                    while count_f < num_files:
                        alignment = int(
                            u_boot_console.config.buildconfig.get(
                                'config_sys_cacheline_size', 128
                            )
                        )
                        offset_l.append(random.randrange(alignment, 1024, alignment))

                        # Read operation
                        output = u_boot_console.run_command(
                            '%sload mmc %d:%s %x %s'
                            % (
                                fs,
                                x,
                                part,
                                addr_l[count_f] + offset_l[count_f],
                                file_l[count_f],
                            )
                        )
                        assert 'Invalid FAT entry' not in output
                        assert 'Unable to read file' not in output
                        assert 'Misaligned buffer address' not in output
                        expected_text = '%d bytes read' % size_l[count_f]
                        assert expected_text in output

                        output = u_boot_console.run_command(
                            'crc32 %x $filesize' % (addr_l[count_f] + offset_l[count_f])
                        )
                        assert crc32_l[count_f] in output

                        count_f += 1

    if not part_detect:
        pytest.skip('No %s partition detected' % fs.upper())

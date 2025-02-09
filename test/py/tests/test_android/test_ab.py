# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2018 Texas Instruments, <www.ti.com>

# Test A/B update commands.

import os
import pytest
import utils

class ABTestDiskImage(object):
    """Disk Image used by the A/B tests."""

    def __init__(self, ubman):
        """Initialize a new ABTestDiskImage object.

        Args:
            ubman: A U-Boot console.

        Returns:
            Nothing.
        """

        filename = 'test_ab_disk_image.bin'

        persistent = ubman.config.persistent_data_dir + '/' + filename
        self.path = ubman.config.result_dir  + '/' + filename

        with utils.persistent_file_helper(ubman.log, persistent):
            if os.path.exists(persistent):
                ubman.log.action('Disk image file ' + persistent +
                    ' already exists')
            else:
                ubman.log.action('Generating ' + persistent)
                fd = os.open(persistent, os.O_RDWR | os.O_CREAT)
                os.ftruncate(fd, 524288)
                os.close(fd)
                cmd = ('sgdisk', persistent)
                utils.run_and_log(ubman, cmd)

                cmd = ('sgdisk', '--new=1:64:512', '--change-name=1:misc',
                    persistent)
                utils.run_and_log(ubman, cmd)
                cmd = ('sgdisk', '--load-backup=' + persistent)
                utils.run_and_log(ubman, cmd)

        cmd = ('cp', persistent, self.path)
        utils.run_and_log(ubman, cmd)

di = None
@pytest.fixture(scope='function')
def ab_disk_image(ubman):
    global di
    if not di:
        di = ABTestDiskImage(ubman)
    return di

def ab_dump(ubman, slot_num, crc):
    output = ubman.run_command('bcb ab_dump host 0#misc')
    header, slot0, slot1 = output.split('\r\r\n\r\r\n')
    slots = [slot0, slot1]
    slot_suffixes = ['_a', '_b']

    header = dict(map(lambda x: map(str.strip, x.split(':')), header.split('\r\r\n')))
    assert header['Bootloader Control'] == '[misc]'
    assert header['Active Slot'] == slot_suffixes[slot_num]
    assert header['Magic Number'] == '0x42414342'
    assert header['Version'] == '1'
    assert header['Number of Slots'] == '2'
    assert header['Recovery Tries Remaining'] == '0'
    assert header['CRC'] == '{} (Valid)'.format(crc)

    slot = dict(map(lambda x: map(str.strip, x.split(':')), slots[slot_num].split('\r\r\n\t- ')[1:]))
    assert slot['Priority'] == '15'
    assert slot['Tries Remaining'] == '6'
    assert slot['Successful Boot'] == '0'
    assert slot['Verity Corrupted'] == '0'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_ab')
@pytest.mark.buildconfigspec('cmd_bcb')
@pytest.mark.requiredtool('sgdisk')
def test_ab(ab_disk_image, ubman):
    """Test the 'bcb ab_select' command."""

    ubman.run_command('host bind 0 ' + ab_disk_image.path)

    output = ubman.run_command('bcb ab_select slot_name host 0#misc')
    assert 're-initializing A/B metadata' in output
    assert 'Attempting slot a, tries remaining 7' in output
    output = ubman.run_command('printenv slot_name')
    assert 'slot_name=a' in output
    ab_dump(ubman, 0, '0xd438d1b9')

    output = ubman.run_command('bcb ab_select slot_name host 0:1')
    assert 'Attempting slot b, tries remaining 7' in output
    output = ubman.run_command('printenv slot_name')
    assert 'slot_name=b' in output
    ab_dump(ubman, 1, '0x011ec016')

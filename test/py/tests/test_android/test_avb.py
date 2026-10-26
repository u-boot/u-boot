# SPDX-License-Identifier:  GPL-2.0+
# Copyright (c) 2018, Linaro Limited
#
# Android Verified Boot 2.0 Test

"""
This tests Android Verified Boot 2.0 support in U-Boot:

For additional details about how to build proper vbmeta partition
check doc/android/avb2.rst

For configuration verification:
- Corrupt boot partition and check for failure
- Corrupt vbmeta partition and check for failure
"""
import os

import pytest
import shutil
from android_image import setup_android_avb_image

temp_addr = 0x90000000
temp_addr2 = 0x90002000

@pytest.fixture(scope='function')
def avb_device_id(ubman):
    """Returns the device id number for AVB

    On sandbox, creates mmc12.img and binds the disabled mmc12 DT node.
    On other boards, defaults to device 1.
    """
    if ubman.config.buildconfig.get('config_sandbox'):
        if not ubman.config.buildconfig.get('config_cmd_bind'):
            pytest.skip('CMD_BIND required for AVB on sandbox')

        required_host_tools = ['qemu-img', 'cgpt']
        for tool in required_host_tools:
            if shutil.which(tool) is None:
                pytest.skip(f'tool "{tool}" not in $PATH')

        mmc_dev = 0xc
        fname = os.path.join(ubman.config.source_dir, f'mmc{mmc_dev}.img')
        if not os.path.exists(fname):
            setup_android_avb_image(ubman)

        response = ubman.run_command(f'bind /mmc{mmc_dev} mmc_sandbox')
        assert response == ''
        # Run mmc list to ensure that mmc_sandbox driver is probed
        ubman.run_command('mmc list')

        yield mmc_dev

        ubman.run_command(f'unbind /mmc{mmc_dev}')
        return

    # default mmc id
    yield 1

@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
def test_avb_verify(ubman, avb_device_id):
    """Run AVB 2.0 boot verification chain with avb subset of commands
    """

    success_str = "Verification passed successfully"

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''
    response = ubman.run_command('avb verify')
    assert success_str in response


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.notbuildconfigspec('sandbox')
def test_avb_mmc_uuid(ubman, avb_device_id):
    """Check if 'avb get_uuid' works, compare results with
    'part list mmc 1' output
    """

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''

    response = ubman.run_command('mmc rescan; mmc dev %d' % avb_device_id)
    assert 'is current device' in response

    part_lines = ubman.run_command('mmc part').splitlines()
    part_list = {}
    cur_partname = ''

    for line in part_lines:
        if '"' in line:
            start_pt = line.find('"')
            end_pt = line.find('"', start_pt + 1)
            cur_partname = line[start_pt + 1: end_pt]

        if 'guid:' in line:
            guid_to_check = line.split('guid:\t')
            part_list[cur_partname] = guid_to_check[1]

    # lets check all guids with avb get_guid
    for part, guid in part_list.items():
        avb_guid_resp = ubman.run_command('avb get_uuid %s' % part)
        assert guid == avb_guid_resp.split('UUID: ')[1]


@pytest.mark.buildconfigspec('cmd_avb')
def test_avb_read_rb(ubman, avb_device_id):
    """Test reading rollback indexes
    """

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''

    response = ubman.run_command('avb read_rb 1')
    assert response == 'Rollback index: 0'


@pytest.mark.buildconfigspec('cmd_avb')
def test_avb_is_unlocked(ubman, avb_device_id):
    """Test if device is in the unlocked state
    """

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''

    response = ubman.run_command('avb is_unlocked')
    assert response == 'Unlocked = 1'


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.notbuildconfigspec('sandbox')
def test_avb_mmc_read(ubman, avb_device_id):
    """Test mmc read operation
    """

    response = ubman.run_command('mmc rescan; mmc dev %d 0' % avb_device_id)
    assert 'is current device' in response

    response = ubman.run_command('mmc read 0x%x 0x100 0x1' % temp_addr)
    assert 'read: OK' in response

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''

    response = ubman.run_command('avb read_part xloader 0 100 0x%x' %
                                           temp_addr2)
    assert 'Read 512 bytes' in response

    # Now lets compare two buffers
    response = ubman.run_command('cmp 0x%x 0x%x 40' %
                                          (temp_addr, temp_addr2))
    assert '64 word' in response


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('optee_ta_avb')
def test_avb_persistent_values(ubman, avb_device_id):
    """Test reading/writing persistent storage to avb
    """

    response = ubman.run_command('avb init %x' % avb_device_id)
    assert response == ''

    response = ubman.run_command('avb write_pvalue test value_value')
    assert response == 'Wrote 12 bytes'

    response = ubman.run_command('avb read_pvalue test 12')
    assert response == 'Read 12 bytes, value = value_value'


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
def test_avb_init_interface(ubman):
    """Check that 'avb init' accepts an explicit interface

    'avb init' only records the interface and device number, so this does
    not need a device that exists.
    """

    response = ubman.run_command('avb init mmc 1')
    assert response == ''

    response = ubman.run_command('avb init 1')
    assert response == ''


@pytest.mark.buildconfigspec('cmd_avb')
def test_avb_init_bad_interface(ubman):
    """Check that 'avb init' rejects an interface that does not exist

    The device itself is only resolved on first access, so a device number
    that does not exist is not rejected here.
    """

    response = ubman.run_command('avb init nonsense 0')
    assert response.find("Unknown interface 'nonsense'") != -1

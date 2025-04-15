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

import pytest

# defauld mmc id
mmc_dev = 1
temp_addr = 0x90000000
temp_addr2 = 0x90002000

@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
def test_avb_verify(ubman):
    """Run AVB 2.0 boot verification chain with avb subset of commands
    """

    success_str = "Verification passed successfully"

    response = ubman.run_command('avb init %s' %str(mmc_dev))
    assert response == ''
    response = ubman.run_command('avb verify')
    assert response.find(success_str)


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.notbuildconfigspec('sandbox')
def test_avb_mmc_uuid(ubman):
    """Check if 'avb get_uuid' works, compare results with
    'part list mmc 1' output
    """

    response = ubman.run_command('avb init %s' % str(mmc_dev))
    assert response == ''

    response = ubman.run_command('mmc rescan; mmc dev %s' %
                                          str(mmc_dev))
    assert response.find('is current device')

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
def test_avb_read_rb(ubman):
    """Test reading rollback indexes
    """

    response = ubman.run_command('avb init %s' % str(mmc_dev))
    assert response == ''

    response = ubman.run_command('avb read_rb 1')
    assert response == 'Rollback index: 0'


@pytest.mark.buildconfigspec('cmd_avb')
def test_avb_is_unlocked(ubman):
    """Test if device is in the unlocked state
    """

    response = ubman.run_command('avb init %s' % str(mmc_dev))
    assert response == ''

    response = ubman.run_command('avb is_unlocked')
    assert response == 'Unlocked = 1'


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('cmd_mmc')
@pytest.mark.notbuildconfigspec('sandbox')
def test_avb_mmc_read(ubman):
    """Test mmc read operation
    """

    response = ubman.run_command('mmc rescan; mmc dev %s 0' %
                                          str(mmc_dev))
    assert response.find('is current device')

    response = ubman.run_command('mmc read 0x%x 0x100 0x1' % temp_addr)
    assert response.find('read: OK')

    response = ubman.run_command('avb init %s' % str(mmc_dev))
    assert response == ''

    response = ubman.run_command('avb read_part xloader 0 100 0x%x' %
                                           temp_addr2)
    assert response.find('Read 512 bytes')

    # Now lets compare two buffers
    response = ubman.run_command('cmp 0x%x 0x%x 40' %
                                          (temp_addr, temp_addr2))
    assert response.find('64 word')


@pytest.mark.buildconfigspec('cmd_avb')
@pytest.mark.buildconfigspec('optee_ta_avb')
def test_avb_persistent_values(ubman):
    """Test reading/writing persistent storage to avb
    """

    response = ubman.run_command('avb init %s' % str(mmc_dev))
    assert response == ''

    response = ubman.run_command('avb write_pvalue test value_value')
    assert response == 'Wrote 12 bytes'

    response = ubman.run_command('avb read_pvalue test 12')
    assert response == 'Read 12 bytes, value = value_value'

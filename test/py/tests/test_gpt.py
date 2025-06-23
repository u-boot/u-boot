# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017 Alison Chaiken
# Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.

# Test GPT manipulation commands.

import os
import pytest
import utils

"""
These tests rely on a 4 MB disk image, which is automatically created by
the test.
"""

# Mark all tests here as slow
pytestmark = pytest.mark.slow

def parse_gpt_parts(disk_str):
    """Parser a partition string into a list of partitions.

    Args:
        disk_str: The disk description string, as returned by `gpt read`

    Returns:
        A list of parsed partitions. Each partition is a dictionary with the
        string value from each specified key in the partition description, or a
        key with with the value True for a boolean flag
    """
    parts = []
    for part_str in disk_str.split(';'):
        part = {}
        for option in part_str.split(","):
            if not option:
                continue

            if "=" in option:
                key, value = option.split("=")
                part[key] = value
            else:
                part[option] = True

        if part:
            parts.append(part)

    return parts

class GptTestDiskImage(object):
    """Disk Image used by the GPT tests."""

    def __init__(self, ubman):
        """Initialize a new GptTestDiskImage object.

        Args:
            ubman: A U-Boot console.

        Returns:
            Nothing.
        """

        filename = 'test_gpt_disk_image.bin'

        persistent = ubman.config.persistent_data_dir + '/' + filename
        self.path = ubman.config.result_dir  + '/' + filename

        with utils.persistent_file_helper(ubman.log, persistent):
            if os.path.exists(persistent):
                ubman.log.action('Disk image file ' + persistent +
                    ' already exists')
            else:
                ubman.log.action('Generating ' + persistent)
                fd = os.open(persistent, os.O_RDWR | os.O_CREAT)
                os.ftruncate(fd, 4194304)
                os.close(fd)
                cmd = ('sgdisk',
                    '--disk-guid=375a56f7-d6c9-4e81-b5f0-09d41ca89efe',
                    persistent)
                utils.run_and_log(ubman, cmd)
                # part1 offset 1MB size 1MB
                cmd = ('sgdisk', '--new=1:2048:4095', '--change-name=1:part1',
                    '--partition-guid=1:33194895-67f6-4561-8457-6fdeed4f50a3',
                    '-A 1:set:2',
                    persistent)
                # part2 offset 2MB size 1.5MB
                utils.run_and_log(ubman, cmd)
                cmd = ('sgdisk', '--new=2:4096:7167', '--change-name=2:part2',
                    '--partition-guid=2:cc9c6e4a-6551-4cb5-87be-3210f96c86fb',
                    persistent)
                utils.run_and_log(ubman, cmd)
                cmd = ('sgdisk', '--load-backup=' + persistent)
                utils.run_and_log(ubman, cmd)

        cmd = ('cp', persistent, self.path)
        utils.run_and_log(ubman, cmd)

@pytest.fixture(scope='function')
def state_disk_image(ubman):
    """pytest fixture to provide a GptTestDiskImage object to tests.
    This is function-scoped because it uses ubman, which is also
    function-scoped. A new disk is returned each time to prevent tests from
    interfering with each other."""

    return GptTestDiskImage(ubman)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_read(state_disk_image, ubman):
    """Test the gpt read command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt read host 0')
    assert 'Start 1MiB, size 1MiB' in output
    assert 'Block size 512, name part1' in output
    assert 'Start 2MiB, size 1MiB' in output
    assert 'Block size 512, name part2' in output
    output = ubman.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part1"' in output
    assert '0x00001000	0x00001bff	"part2"' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('partition_type_guid')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_read_var(state_disk_image, ubman):
    """Test the gpt read command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt read host 0 gpt_parts')
    assert 'success!' in output

    output = ubman.run_command('echo ${gpt_parts}')
    parts = parse_gpt_parts(output.rstrip())

    assert parts == [
        {
            "uuid_disk": "375a56f7-d6c9-4e81-b5f0-09d41ca89efe",
        },
        {
            "name": "part1",
            "start": "0x100000",
            "size": "0x100000",
            "type": "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "uuid": "33194895-67f6-4561-8457-6fdeed4f50a3",
            "bootable": True,
        },
        {
            "name": "part2",
            "start": "0x200000",
            "size": "0x180000",
            "type": "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "uuid": "cc9c6e4a-6551-4cb5-87be-3210f96c86fb",
        },
    ]

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_verify(state_disk_image, ubman):
    """Test the gpt verify command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt verify host 0')
    assert 'Verify GPT: success!' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_repair(state_disk_image, ubman):
    """Test the gpt repair command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt repair host 0')
    assert 'Repairing GPT: success!' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_guid(state_disk_image, ubman):
    """Test the gpt guid command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt guid host 0')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_setenv(state_disk_image, ubman):
    """Test the gpt setenv command."""
    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt setenv host 0 part1')
    assert 'success!' in output
    output = ubman.run_command('echo ${gpt_partition_addr}')
    assert output.rstrip() == '800'
    output = ubman.run_command('echo ${gpt_partition_size}')
    assert output.rstrip() == '800'
    output = ubman.run_command('echo ${gpt_partition_name}')
    assert output.rstrip() == 'part1'
    output = ubman.run_command('echo ${gpt_partition_entry}')
    assert output.rstrip() == '1'
    output = ubman.run_command('echo ${gpt_partition_bootable}')
    assert output.rstrip() == '1'

    output = ubman.run_command('gpt setenv host 0 part2')
    assert 'success!' in output
    output = ubman.run_command('echo ${gpt_partition_addr}')
    assert output.rstrip() == '1000'
    output = ubman.run_command('echo ${gpt_partition_size}')
    assert output.rstrip() == 'c00'
    output = ubman.run_command('echo ${gpt_partition_name}')
    assert output.rstrip() == 'part2'
    output = ubman.run_command('echo ${gpt_partition_entry}')
    assert output.rstrip() == '2'
    output = ubman.run_command('echo ${gpt_partition_bootable}')
    assert output.rstrip() == '0'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_save_guid(state_disk_image, ubman):
    """Test the gpt guid command to save GUID into a string."""

    if ubman.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt guid host 0 newguid')
    output = ubman.run_command('printenv newguid')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_part_type_uuid(state_disk_image, ubman):
    """Test the gpt partittion type UUID command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('part type host 0:1')
    assert '0fc63daf-8483-4772-8e79-3d69d8477de4' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_part_type_save_uuid(state_disk_image, ubman):
    """Test the gpt partittion type to save UUID into a string."""

    if ubman.config.buildconfig.get('config_cmd_gpt', 'n') != 'y':
        pytest.skip('gpt command not supported')
    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('part type host 0:1 newguid')
    output = ubman.run_command('printenv newguid')
    assert '0fc63daf-8483-4772-8e79-3d69d8477de4' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_rename_partition(state_disk_image, ubman):
    """Test the gpt rename command to write partition names."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    ubman.run_command('gpt rename host 0 1 first')
    output = ubman.run_command('gpt read host 0')
    assert 'name first' in output
    ubman.run_command('gpt rename host 0 2 second')
    output = ubman.run_command('gpt read host 0')
    assert 'name second' in output
    output = ubman.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"first"' in output
    assert '0x00001000	0x00001bff	"second"' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_swap_partitions(state_disk_image, ubman):
    """Test the gpt swap command to exchange two partition names."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part1"' in output
    assert '0x00001000	0x00001bff	"part2"' in output
    ubman.run_command('gpt swap host 0 part1 part2')
    output = ubman.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"part2"' in output
    assert '0x00001000	0x00001bff	"part1"' in output

@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_set_bootable(state_disk_image, ubman):
    """Test the gpt set-bootable command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    parts = ('part2', 'part1')
    for bootable in parts:
        output = ubman.run_command(f'gpt set-bootable host 0 {bootable}')
        assert 'success!' in output

        for p in parts:
            output = ubman.run_command(f'gpt setenv host 0 {p}')
            assert 'success!' in output
            output = ubman.run_command('echo ${gpt_partition_bootable}')
            if p == bootable:
                assert output.rstrip() == '1'
            else:
                assert output.rstrip() == '0'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_write(state_disk_image, ubman):
    """Test the gpt write command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('gpt write host 0 "name=all,size=0"')
    assert 'Writing GPT: success!' in output
    output = ubman.run_command('part list host 0')
    assert '0x00000022	0x00001fde	"all"' in output
    output = ubman.run_command('gpt write host 0 "uuid_disk=375a56f7-d6c9-4e81-b5f0-09d41ca89efe;name=first,start=1M,size=1M;name=second,start=0x200000,size=0x180000;"')
    assert 'Writing GPT: success!' in output
    output = ubman.run_command('part list host 0')
    assert '0x00000800	0x00000fff	"first"' in output
    assert '0x00001000	0x00001bff	"second"' in output
    output = ubman.run_command('gpt guid host 0')
    assert '375a56f7-d6c9-4e81-b5f0-09d41ca89efe' in output

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.buildconfigspec('partition_type_guid')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_write_part_type(state_disk_image, ubman):
    """Test the gpt command with part type uuid."""

    output = ubman.run_command('gpt write host 0 "name=part1,type=data,size=1M;name=part2,size=512K,type=system;name=part3,size=65536,type=u-boot-env;name=part4,size=65536,type=375a56f7-d6c9-4e81-b5f0-09d41ca89efe;name=part5,size=-,type=linux"')
    assert 'Writing GPT: success!' in output
    output = ubman.run_command('part list host 0')
    assert '1\t0x00000022\t0x00000821\t"part1"' in output
    assert 'ebd0a0a2-b9e5-4433-87c0-68b6b72699c7' in output
    assert '(data)' in output
    assert '2\t0x00000822\t0x00000c21\t"part2"' in output
    assert 'c12a7328-f81f-11d2-ba4b-00a0c93ec93b' in output
    assert '(EFI System Partition)' in output
    assert '3\t0x00000c22\t0x00000ca1\t"part3"' in output
    assert '3de21764-95bd-54bd-a5c3-4abe786f38a8' in output
    assert '(u-boot-env)' in output
    assert '4\t0x00000ca2\t0x00000d21\t"part4"' in output
    assert 'ebd0a0a2-b9e5-4433-87c0-68b6b72699c7' in output
    assert '(375a56f7-d6c9-4e81-b5f0-09d41ca89efe)' in output
    assert '5\t0x00000d22\t0x00001fde\t"part5"' in output
    assert '0fc63daf-8483-4772-8e79-3d69d8477de4' in output
    assert '(linux)' in output

@pytest.mark.buildconfigspec('cmd_gpt')
@pytest.mark.buildconfigspec('cmd_gpt_rename')
@pytest.mark.buildconfigspec('cmd_part')
@pytest.mark.requiredtool('sgdisk')
def test_gpt_transpose(state_disk_image, ubman):
    """Test the gpt transpose command."""

    ubman.run_command('host bind 0 ' + state_disk_image.path)
    output = ubman.run_command('part list host 0')
    assert '1\t0x00000800\t0x00000fff\t"part1"' in output
    assert '2\t0x00001000\t0x00001bff\t"part2"' in output

    output = ubman.run_command('gpt transpose host 0 1 2')
    assert 'success!' in output

    output = ubman.run_command('part list host 0')
    assert '2\t0x00000800\t0x00000fff\t"part1"' in output
    assert '1\t0x00001000\t0x00001bff\t"part2"' in output

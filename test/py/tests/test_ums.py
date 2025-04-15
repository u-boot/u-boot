# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Test U-Boot's "ums" command. The test starts UMS in U-Boot, waits for USB
# device enumeration on the host, reads a small block of data from the UMS
# block device, optionally mounts a partition and performs filesystem-based
# read/write tests, and finally aborts the "ums" command in U-Boot.

import os
import os.path
import pytest
import re
import time
import utils

"""
Note: This test relies on:

a) boardenv_* to contain configuration values to define which USB ports are
available for testing. Without this, this test will be automatically skipped.
For example:

# Leave this list empty if you have no block_devs below with writable
# partitions defined.
env__mount_points = (
    '/mnt/ubtest-mnt-p2371-2180-na',
)

env__usb_dev_ports = (
    {
        'fixture_id': 'micro_b',
        'tgt_usb_ctlr': '0',
        'host_ums_dev_node': '/dev/disk/by-path/pci-0000:00:14.0-usb-0:13:1.0-scsi-0:0:0:0',
    },
)

env__block_devs = (
    # eMMC; always present
    {
        'fixture_id': 'emmc',
        'type': 'mmc',
        'id': '0',
        # The following two properties are optional.
        # If present, the partition will be mounted and a file written-to and
        # read-from it. If missing, only a simple block read test will be
        # performed.
        'writable_fs_partition': 1,
        'writable_fs_subdir': 'tmp/',
    },
    # SD card; present since I plugged one in
    {
        'fixture_id': 'sd',
        'type': 'mmc',
        'id': '1'
    },
)

b) udev rules to set permissions on devices nodes, so that sudo is not
required. For example:

ACTION=="add", SUBSYSTEM=="block", SUBSYSTEMS=="usb", KERNELS=="3-13", MODE:="666"

(You may wish to change the group ID instead of setting the permissions wide
open. All that matters is that the user ID running the test can access the
device.)

c) /etc/fstab entries to allow the block device to be mounted without requiring
root permissions. For example:

/dev/disk/by-path/pci-0000:00:14.0-usb-0:13:1.0-scsi-0:0:0:0-part1 /mnt/ubtest-mnt-p2371-2180-na ext4 noauto,user,nosuid,nodev

This entry is only needed if any block_devs above contain a
writable_fs_partition value.
"""

@pytest.mark.buildconfigspec('cmd_usb_mass_storage')
def test_ums(ubman, env__usb_dev_port, env__block_devs):
    """Test the "ums" command; the host system must be able to enumerate a UMS
    device when "ums" is running, block and optionally file I/O are tested,
    and this device must disappear when "ums" is aborted.

    Args:
        ubman: A U-Boot console connection.
        env__usb_dev_port: The single USB device-mode port specification on
            which to run the test. See the file-level comment above for
            details of the format.
        env__block_devs: The list of block devices that the target U-Boot
            device has attached. See the file-level comment above for details
            of the format.

    Returns:
        Nothing.
    """

    have_writable_fs_partition = 'writable_fs_partition' in env__block_devs[0]
    if not have_writable_fs_partition:
        # If 'writable_fs_subdir' is missing, we'll skip all parts of the
        # testing which mount filesystems.
        ubman.log.warning(
            'boardenv missing "writable_fs_partition"; ' +
            'UMS testing will be limited.')

    tgt_usb_ctlr = env__usb_dev_port['tgt_usb_ctlr']
    host_ums_dev_node = env__usb_dev_port['host_ums_dev_node']

    # We're interested in testing USB device mode on each port, not the cross-
    # product of that with each device. So, just pick the first entry in the
    # device list here. We'll test each block device somewhere else.
    tgt_dev_type = env__block_devs[0]['type']
    tgt_dev_id = env__block_devs[0]['id']
    if have_writable_fs_partition:
        mount_point = ubman.config.env['env__mount_points'][0]
        mount_subdir = env__block_devs[0]['writable_fs_subdir']
        part_num = env__block_devs[0]['writable_fs_partition']
        host_ums_part_node = '%s-part%d' % (host_ums_dev_node, part_num)
        test_f = utils.PersistentRandomFile(ubman, 'ums.bin', 1024 * 1024);
        mounted_test_fn = mount_point + '/' + mount_subdir + test_f.fn
    else:
        host_ums_part_node = host_ums_dev_node

    def start_ums():
        """Start U-Boot's ums shell command.

        This also waits for the host-side USB enumeration process to complete.

        Args:
            None.

        Returns:
            Nothing.
        """

        ubman.log.action(
            'Starting long-running U-Boot ums shell command')
        cmd = 'ums %s %s %s' % (tgt_usb_ctlr, tgt_dev_type, tgt_dev_id)
        ubman.run_command(cmd, wait_for_prompt=False)
        ubman.wait_for(re.compile('UMS: LUN.*[\r\n]'))
        fh = utils.wait_until_open_succeeds(host_ums_part_node)
        ubman.log.action('Reading raw data from UMS device')
        fh.read(4096)
        fh.close()

    def mount():
        """Mount the block device that U-Boot exports.

        Args:
            None.

        Returns:
            Nothing.
        """

        ubman.log.action('Mounting exported UMS device')
        cmd = ('/bin/mount', host_ums_part_node)
        utils.run_and_log(ubman, cmd)

    def umount(ignore_errors):
        """Unmount the block device that U-Boot exports.

        Args:
            ignore_errors: Ignore any errors. This is useful if an error has
                already been detected, and the code is performing best-effort
                cleanup. In this case, we do not want to mask the original
                error by "honoring" any new errors.

        Returns:
            Nothing.
        """

        ubman.log.action('Unmounting UMS device')
        cmd = ('/bin/umount', host_ums_part_node)
        utils.run_and_log(ubman, cmd, ignore_errors)

    def stop_ums(ignore_errors):
        """Stop U-Boot's ums shell command from executing.

        This also waits for the host-side USB de-enumeration process to
        complete.

        Args:
            ignore_errors: Ignore any errors. This is useful if an error has
                already been detected, and the code is performing best-effort
                cleanup. In this case, we do not want to mask the original
                error by "honoring" any new errors.

        Returns:
            Nothing.
        """

        ubman.log.action(
            'Stopping long-running U-Boot ums shell command')
        ubman.ctrlc()
        utils.wait_until_file_open_fails(host_ums_part_node,
            ignore_errors)

    ignore_cleanup_errors = True
    if have_writable_fs_partition:
        try:
            start_ums()
            try:
                mount()
                ubman.log.action('Writing test file via UMS')
                cmd = ('rm', '-f', mounted_test_fn)
                utils.run_and_log(ubman, cmd)
                if os.path.exists(mounted_test_fn):
                    raise Exception('Could not rm target UMS test file')
                cmd = ('cp', test_f.abs_fn, mounted_test_fn)
                utils.run_and_log(ubman, cmd)
                ignore_cleanup_errors = False
            finally:
                umount(ignore_errors=ignore_cleanup_errors)
        finally:
            stop_ums(ignore_errors=ignore_cleanup_errors)

    ignore_cleanup_errors = True
    try:
        start_ums()
        try:
            mount()
            ubman.log.action('Reading test file back via UMS')
            read_back_hash = utils.md5sum_file(mounted_test_fn)
            cmd = ('rm', '-f', mounted_test_fn)
            utils.run_and_log(ubman, cmd)
            ignore_cleanup_errors = False
        finally:
            umount(ignore_errors=ignore_cleanup_errors)
    finally:
        stop_ums(ignore_errors=ignore_cleanup_errors)

    written_hash = test_f.content_hash
    assert(written_hash == read_back_hash)

# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Test U-Boot's "ums" command. At present, this test only ensures that a UMS
# device can be enumerated by the host/test machine. In the future, this test
# should be enhanced to validate disk IO.

import os
import pytest
import time

'''
Note: This test relies on:

a) boardenv_* to contain configuration values to define which USB ports are
available for testing. Without this, this test will be automatically skipped.
For example:

env__usb_dev_ports = (
    {'tgt_usb_ctlr': '0', 'host_ums_dev_node': '/dev/disk/by-path/pci-0000:00:14.0-usb-0:13:1.0-scsi-0:0:0:0'},
)

env__block_devs = (
    {'type': 'mmc', 'id': '0'}, # eMMC; always present
    {'type': 'mmc', 'id': '1'}, # SD card; present since I plugged one in
)

b) udev rules to set permissions on devices nodes, so that sudo is not
required. For example:

ACTION=="add", SUBSYSTEM=="block", SUBSYSTEMS=="usb", KERNELS=="3-13", MODE:="666"

(You may wish to change the group ID instead of setting the permissions wide
open. All that matters is that the user ID running the test can access the
device.)
'''

def open_ums_device(host_ums_dev_node):
    '''Attempt to open a device node, returning either the opened file handle,
    or None on any error.'''

    try:
        return open(host_ums_dev_node, 'rb')
    except:
        return None

def wait_for_ums_device(host_ums_dev_node):
    '''Continually attempt to open the device node exported by the "ums"
    command, and either return the opened file handle, or raise an exception
    after a timeout.'''

    for i in xrange(100):
        fh = open_ums_device(host_ums_dev_node)
        if fh:
            return fh
        time.sleep(0.1)
    raise Exception('UMS device did not appear')

def wait_for_ums_device_gone(host_ums_dev_node):
    '''Continually attempt to open the device node exported by the "ums"
    command, and either return once the device has disappeared, or raise an
    exception if it does not before a timeout occurs.'''

    for i in xrange(100):
        fh = open_ums_device(host_ums_dev_node)
        if not fh:
            return
        fh.close()
        time.sleep(0.1)
    raise Exception('UMS device did not disappear')

@pytest.mark.buildconfigspec('cmd_usb_mass_storage')
def test_ums(u_boot_console, env__usb_dev_port, env__block_devs):
    '''Test the "ums" command; the host system must be able to enumerate a UMS
    device when "ums" is running, and this device must disappear when "ums" is
    aborted.'''

    tgt_usb_ctlr = env__usb_dev_port['tgt_usb_ctlr']
    host_ums_dev_node = env__usb_dev_port['host_ums_dev_node']

    # We're interested in testing USB device mode on each port, not the cross-
    # product of that with each device. So, just pick the first entry in the
    # device list here. We'll test each block device somewhere else.
    tgt_dev_type = env__block_devs[0]['type']
    tgt_dev_id = env__block_devs[0]['id']

    cmd = 'ums %s %s %s' % (tgt_usb_ctlr, tgt_dev_type, tgt_dev_id)
    u_boot_console.run_command('ums 0 mmc 0', wait_for_prompt=False)
    fh = wait_for_ums_device(host_ums_dev_node)
    fh.read(4096)
    fh.close()
    u_boot_console.ctrlc()
    wait_for_ums_device_gone(host_ums_dev_node)

# Copyright (c) 2018, Xilinx Inc.
#
# Siva Durga Prasad Paladugu
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import re
import random
import u_boot_utils

"""
Note: This test relies on boardenv_* containing configuration values to define
the network available and files to be used for testing. Without this, this test
will be automatically skipped.

For example:

# True if a DHCP server is attached to the network, and should be tested.
env__net_dhcp_server = True

# A list of environment variables that should be set in order to configure a
# static IP. In this test case we atleast need serverip for performing tftpb
# to get required files.
env__net_static_env_vars = [
    ("ipaddr", "10.0.0.100"),
    ("netmask", "255.255.255.0"),
    ("serverip", "10.0.0.1"),
]

# Details regarding the files that may be read from a TFTP server. .
env__zynqmp_secure_readable_file = {
    "fn": "auth_bhdr_ppk1.bin",
    "enckupfn": "auth_bhdr_enc_kup_load.bin",
    "addr": 0x1000000,
    "keyaddr": 0x100000,
    "keyfn": "aes.txt",
}
"""

import test_net

@pytest.mark.buildconfigspec('cmd_zynqmp')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net')
def test_zynqmp_secure_boot_image(u_boot_console):

    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)

    f = u_boot_console.config.env.get('env__zynqmp_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)
    if not addr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    expected_zynqmpsecure = 'Verified image at'
    output = u_boot_console.run_command('zynqmp secure %x $filesize' % (addr))
    assert expected_zynqmpsecure in output
    output = u_boot_console.run_command('pri zynqmp_verified_img_addr')
    assert "Error" not in output

@pytest.mark.buildconfigspec('cmd_zynqmp')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net')
def test_zynqmp_secure_boot_img_kup(u_boot_console):

    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)

    f = u_boot_console.config.env.get('env__zynqmp_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    keyaddr = f.get('keyaddr', None)
    if not keyaddr:
      addr = u_boot_utils.find_ram_base(u_boot_console)
    expected_tftp = 'Bytes transferred = '
    keyfn = f['keyfn']
    output = u_boot_console.run_command('tftpboot %x %s' % (keyaddr, keyfn))
    assert expected_tftp in output

    addr = f.get('addr', None)
    if not addr:
      addr = u_boot_utils.find_ram_base(u_boot_console)
    expected_tftp = 'Bytes transferred = '
    fn = f['enckupfn']
    output = u_boot_console.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    expected_zynqmpsecure = 'Verified image at'
    output = u_boot_console.run_command('zynqmp secure %x $filesize %x' % (addr, keyaddr))
    assert expected_zynqmpsecure in output
    output = u_boot_console.run_command('pri zynqmp_verified_img_addr')
    assert "Error" not in output

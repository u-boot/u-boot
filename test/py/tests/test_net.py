# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.

# Test various network-related functionality, such as the dhcp, ping, and
# tftpboot commands.

import pytest
import utils
import uuid
import datetime
import re

"""
Note: This test relies on boardenv_* containing configuration values to define
which network environment is available for testing. Without this, this test
will be automatically skipped.

For example:

# Boolean indicating whether the Ethernet device is attached to USB, and hence
# USB enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_usb = False

# Boolean indicating whether the Ethernet device is attached to PCI, and hence
# PCI enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_pci = True

# True if a DHCP server is attached to the network, and should be tested.
# If DHCP testing is not possible or desired, this variable may be omitted or
# set to False.
env__net_dhcp_server = True

# False or omitted if a DHCP server is attached to the network, and dhcp abort
# case should be tested.
# If DHCP abort testing is not possible or desired, set this variable to True.
# For example: On some setup, dhcp is too fast and this case may not work.
env__dhcp_abort_test_skip = True

# True if a DHCPv6 server is attached to the network, and should be tested.
# If DHCPv6 testing is not possible or desired, this variable may be omitted or
# set to False.
env__net_dhcp6_server = True

# A list of environment variables that should be set in order to configure a
# static IP. If solely relying on DHCP, this variable may be omitted or set to
# an empty list.
env__net_static_env_vars = [
    ('ipaddr', '10.0.0.100'),
    ('netmask', '255.255.255.0'),
    ('serverip', '10.0.0.1'),
]

# Details regarding a file that may be read from a TFTP server. This variable
# may be omitted or set to None if TFTP testing is not possible or desired.
env__net_tftp_readable_file = {
    'fn': 'ubtest-readable.bin',
    'addr': 0x10000000,
    'size': 5058624,
    'crc32': 'c2244b26',
    'timeout': 50000,
    'fnu': 'ubtest-upload.bin',
}

# Details regarding a file that may be read from a NFS server. This variable
# may be omitted or set to None if NFS testing is not possible or desired.
env__net_nfs_readable_file = {
    'fn': 'ubtest-readable.bin',
    'addr': 0x10000000,
    'size': 5058624,
    'crc32': 'c2244b26',
}

# Details regarding a file that may be read from a TFTP server. This variable
# may be omitted or set to None if PXE testing is not possible or desired.
env__net_pxe_readable_file = {
    'fn': 'default',
    'addr': 0x2000000,
    'size': 74,
    'timeout': 50000,
    'pattern': 'Linux',
}

# True if a router advertisement service is connected to the network, and should
# be tested. If router advertisement testing is not possible or desired, this
variable may be omitted or set to False.
env__router_on_net = True
"""

net_set_up = False
net6_set_up = False


@pytest.mark.buildconfigspec('cmd_net')
def test_net_pre_commands(ubman):
    """Execute any commands required to enable network hardware.

    These commands are provided by the boardenv_* file; see the comment at the
    beginning of this file.
    """

    init_usb = ubman.config.env.get('env__net_uses_usb', False)
    if init_usb:
        ubman.run_command('usb start')

    init_pci = ubman.config.env.get('env__net_uses_pci', False)
    if init_pci:
        ubman.run_command('pci enum')

    ubman.run_command('net list')

@pytest.mark.buildconfigspec('cmd_dhcp')
def test_net_dhcp(ubman):
    """Test the dhcp command.

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp = ubman.config.env.get('env__net_dhcp_server', False)
    if not test_dhcp:
        pytest.skip('No DHCP server available')

    ubman.run_command('setenv autoload no')
    output = ubman.run_command('dhcp')
    assert 'DHCP client bound to address ' in output

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('cmd_mii')
def test_net_dhcp_abort(ubman):
    """Test the dhcp command by pressing ctrl+c in the middle of dhcp request

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp = ubman.config.env.get('env__net_dhcp_server', False)
    if not test_dhcp:
        pytest.skip('No DHCP server available')

    if ubman.config.env.get('env__dhcp_abort_test_skip', True):
        pytest.skip('DHCP abort test is not enabled!')

    ubman.run_command('setenv autoload no')

    # Phy reset before running dhcp command
    output = ubman.run_command('mii device')
    if not re.search(r"Current device: '(.+?)'", output):
        pytest.skip('PHY device does not exist!')
    eth_num = re.search(r"Current device: '(.+?)'", output).groups()[0]
    ubman.run_command(f'mii device {eth_num}')
    output = ubman.run_command('mii info')
    eth_addr = hex(int(re.search(r'PHY (.+?):', output).groups()[0], 16))
    ubman.run_command(f'mii modify {eth_addr} 0 0x8000 0x8000')

    ubman.run_command('dhcp', wait_for_prompt=False)
    try:
        ubman.wait_for('Waiting for PHY auto negotiation to complete')
    except:
        pytest.skip('Timeout waiting for PHY auto negotiation to complete')

    ubman.wait_for('done')

    try:
        # Sending Ctrl-C
        output = ubman.run_command(
            chr(3), wait_for_echo=False, send_nl=False
        )
        assert 'TIMEOUT' not in output
        assert 'DHCP client bound to address ' not in output
        assert 'Abort' in output
    finally:
        # Provide a time to recover from Abort - if it is not performed
        # There is message like: ethernet@ff0e0000: No link.
        ubman.run_command('sleep 1')
        # Run the dhcp test to setup the network configuration
        test_net_dhcp(ubman)

@pytest.mark.buildconfigspec('cmd_dhcp6')
def test_net_dhcp6(ubman):
    """Test the dhcp6 command.

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp6 = ubman.config.env.get('env__net_dhcp6_server', False)
    if not test_dhcp6:
        pytest.skip('No DHCP6 server available')

    ubman.run_command('setenv autoload no')
    output = ubman.run_command('dhcp6')
    assert 'DHCP6 client bound to ' in output

    global net6_set_up
    net6_set_up = True

@pytest.mark.buildconfigspec('net')
def test_net_setup_static(ubman):
    """Set up a static IP configuration.

    The configuration is provided by the boardenv_* file; see the comment at
    the beginning of this file.
    """

    env_vars = ubman.config.env.get('env__net_static_env_vars', None)
    if not env_vars:
        pytest.skip('No static network configuration is defined')

    for (var, val) in env_vars:
        ubman.run_command('setenv %s %s' % (var, val))

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('cmd_ping')
def test_net_ping(ubman):
    """Test the ping command.

    The $serverip (as set up by either test_net_dhcp or test_net_setup_static)
    is pinged. The test validates that the host is alive, as reported by the
    ping command's output.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    output = ubman.run_command('ping $serverip')
    assert 'is alive' in output

@pytest.mark.buildconfigspec('IPV6_ROUTER_DISCOVERY')
def test_net_network_discovery(ubman):
    """Test the network discovery feature of IPv6.

    An IPv6 network command (ping6 in this case) is run to make U-Boot send a
    router solicitation packet, receive a router advertisement message, and
    parse it.
    A router advertisement service needs to be running for this test to succeed.
    U-Boot receives the RA, processes it, and if successful, assigns the gateway
    IP and prefix length.
    The configuration is provided by the boardenv_* file; see the comment at
    the beginning of this file.
    """

    router_on_net = ubman.config.env.get('env__router_on_net', False)
    if not router_on_net:
        pytest.skip('No router on network')

    fake_host_ip = 'fe80::215:5dff:fef6:2ec6'
    output = ubman.run_command('ping6 ' + fake_host_ip)
    assert 'ROUTER SOLICITATION 1' in output
    assert 'Set gatewayip6:' in output
    assert '0000:0000:0000:0000:0000:0000:0000:0000' not in output

@pytest.mark.buildconfigspec('cmd_tftpboot')
def test_net_tftpboot(ubman):
    """Test the tftpboot command.

    A file is downloaded from the TFTP server, its size and optionally its
    CRC32 are validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    f = ubman.config.env.get('env__net_tftp_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)

    fn = f['fn']
    if not addr:
        output = ubman.run_command('tftpboot %s' % (fn))
    else:
        output = ubman.run_command('tftpboot %x %s' % (addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return

    if ubman.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return

    output = ubman.run_command('crc32 $fileaddr $filesize')
    assert expected_crc in output

@pytest.mark.buildconfigspec('cmd_nfs')
def test_net_nfs(ubman):
    """Test the nfs command.

    A file is downloaded from the NFS server, its size and optionally its
    CRC32 are validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip('Network not initialized')

    f = ubman.config.env.get('env__net_nfs_readable_file', None)
    if not f:
        pytest.skip('No NFS readable file to read')

    addr = f.get('addr', None)
    if not addr:
        addr = utils.find_ram_base(ubman)

    fn = f['fn']
    output = ubman.run_command('nfs %x %s' % (addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return

    if ubman.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return

    output = ubman.run_command('crc32 %x $filesize' % addr)
    assert expected_crc in output

@pytest.mark.buildconfigspec("cmd_pxe")
def test_net_pxe_get(ubman):
    """Test the pxe get command.

    A pxe configuration file is downloaded from the TFTP server and interpreted
    to boot the images mentioned in pxe configuration file.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip("Network not initialized")

    test_net_setup_static(ubman)

    f = ubman.config.env.get("env__net_pxe_readable_file", None)
    if not f:
        pytest.skip("No PXE readable file to read")

    addr = f.get("addr", None)
    timeout = f.get("timeout", ubman.p.timeout)

    pxeuuid = uuid.uuid1()
    ubman.run_command(f"setenv pxeuuid {pxeuuid}")
    expected_text_uuid = f"Retrieving file: pxelinux.cfg/{pxeuuid}"

    ethaddr = ubman.run_command("echo $ethaddr")
    ethaddr = ethaddr.replace(':', '-')
    expected_text_ethaddr = f"Retrieving file: pxelinux.cfg/01-{ethaddr}"

    ip = ubman.run_command("echo $ipaddr")
    ip = ip.split('.')
    ipaddr_file = "".join(['%02x' % int(x) for x in ip]).upper()
    expected_text_ipaddr = f"Retrieving file: pxelinux.cfg/{ipaddr_file}"
    expected_text_default = f"Retrieving file: pxelinux.cfg/default"

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command("pxe get")

    assert "TIMEOUT" not in output
    assert expected_text_uuid in output
    assert expected_text_ethaddr in output
    assert expected_text_ipaddr in output

    i = 1
    for i in range(0, len(ipaddr_file) - 1):
        expected_text_ip = f"Retrieving file: pxelinux.cfg/{ipaddr_file[:-i]}"
        assert expected_text_ip in output
        i += 1

    assert expected_text_default in output
    assert "Config file 'default.boot' found" in output

@pytest.mark.buildconfigspec("cmd_crc32")
@pytest.mark.buildconfigspec("cmd_tftpboot")
@pytest.mark.buildconfigspec("cmd_tftpput")
def test_net_tftpput(ubman):
    """Test the tftpput command.

    A file is downloaded from the TFTP server and then uploaded to the TFTP
    server, its size and its CRC32 are validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """

    if not net_set_up:
        pytest.skip("Network not initialized")

    f = ubman.config.env.get("env__net_tftp_readable_file", None)
    if not f:
        pytest.skip("No TFTP readable file to read")

    addr = f.get("addr", None)
    if not addr:
        addr = utils.find_ram_base(ubman)

    sz = f.get("size", None)
    timeout = f.get("timeout", ubman.p.timeout)
    fn = f["fn"]
    fnu = f.get("fnu", "_".join([datetime.datetime.now().strftime("%y%m%d%H%M%S"), fn]))
    expected_text = "Bytes transferred = "
    if sz:
        expected_text += "%d" % sz

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command("tftpboot %x %s" % (addr, fn))

    assert "TIMEOUT" not in output
    assert expected_text in output

    expected_tftpb_crc = f.get("crc32", None)

    output = ubman.run_command("crc32 $fileaddr $filesize")
    assert expected_tftpb_crc in output

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command(
            "tftpput $fileaddr $filesize $serverip:%s" % (fnu)
        )

    expected_text = "Bytes transferred = "
    if sz:
        expected_text += "%d" % sz
        addr = addr + sz
    assert "TIMEOUT" not in output
    assert "Access violation" not in output
    assert expected_text in output

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command("tftpboot %x %s" % (addr, fnu))

    expected_text = "Bytes transferred = "
    if sz:
        expected_text += "%d" % sz
    assert "TIMEOUT" not in output
    assert expected_text in output

    output = ubman.run_command("crc32 $fileaddr $filesize")
    assert expected_tftpb_crc in output

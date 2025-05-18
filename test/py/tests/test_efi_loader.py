# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
# Copyright (c) 2016, Alexander Graf <agraf@suse.de>
#
# based on test_net.py.

# Test efi loader implementation

"""
Note: This test relies on boardenv_* containing configuration values to define
which network environment is available for testing. Without this, the parts
that rely on network will be automatically skipped.

For example:

.. code-block:: python

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
    env__efi_loader_helloworld_file = {
        'fn': 'lib/efi_loader/helloworld.efi', # file name
        'size': 5058624,                       # file length in bytes
        'crc32': 'c2244b26',                   # CRC32 check sum
        'addr': 0x40400000,                    # load address
    }

    # False if the helloworld EFI over HTTP boot test should be performed.
    # If HTTP boot testing is not possible or desired, set this variable to True or
    # ommit it.
    env__efi_helloworld_net_http_test_skip = True
"""

import pytest
import utils

PROTO_TFTP, PROTO_HTTP = range(0, 2)

net_set_up = False

def test_efi_pre_commands(ubman):
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

@pytest.mark.buildconfigspec('cmd_dhcp')
def test_efi_setup_dhcp(ubman):
    """Set up the network using DHCP.

    The boardenv_* file may be used to enable/disable this test; see the
    comment at the beginning of this file.
    """

    test_dhcp = ubman.config.env.get('env__net_dhcp_server', False)
    if not test_dhcp:
        env_vars = ubman.config.env.get('env__net_static_env_vars', None)
        if not env_vars:
            pytest.skip('No DHCP server available')
        return

    ubman.run_command('setenv autoload no')
    output = ubman.run_command('dhcp')
    assert 'DHCP client bound to address ' in output

    global net_set_up
    net_set_up = True

@pytest.mark.buildconfigspec('net', 'net_lwip')
def test_efi_setup_static(ubman):
    """Set up the network using a static IP configuration.

    The configuration is provided by the boardenv_* file; see the comment at
    the beginning of this file.
    """

    env_vars = ubman.config.env.get('env__net_static_env_vars', None)
    if not env_vars:
        test_dhcp = ubman.config.env.get('env__net_dhcp_server', False)
        if not test_dhcp:
            pytest.skip('No static network configuration is defined')
        return None

    for (var, val) in env_vars:
        ubman.run_command('setenv %s %s' % (var, val))

    global net_set_up
    net_set_up = True

def fetch_file(ubman, env_conf, proto):
    """Grab an env described file via TFTP or HTTP and return its address

    A file as described by an env config <env_conf> is downloaded from the
    server. The address to that file is returned.
    """
    if not net_set_up:
        pytest.skip('Network not initialized')

    f = ubman.config.env.get(env_conf, None)
    if not f:
        pytest.skip('No %s binary specified in environment' % env_conf)

    addr = f.get('addr', None)
    if not addr:
        addr = utils.find_ram_base(ubman)

    fn = f['fn']
    if proto == PROTO_TFTP:
        cmd = 'tftpboot'
    elif proto == PROTO_HTTP:
        cmd = 'wget'
    else:
        assert False
    output = ubman.run_command('%s %x %s' % (cmd, addr, fn))
    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    if not expected_crc:
        return addr

    if ubman.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
        return addr

    output = ubman.run_command('crc32 %x $filesize' % addr)
    assert expected_crc in output

    return addr

def do_test_efi_helloworld_net(ubman, proto):
    """Download and execute the helloworld appliation

    The helloworld.efi file is downloaded based on the value passed to us as a
    protocol and is executed using the fallback device tree at $fdtcontroladdr.
    """
    addr = fetch_file(ubman, 'env__efi_loader_helloworld_file', proto)

    output = ubman.run_command('bootefi %x' % addr)
    expected_text = 'Hello, world'
    assert expected_text in output
    expected_text = '## Application failed'
    assert expected_text not in output

@pytest.mark.buildconfigspec('of_control')
@pytest.mark.buildconfigspec('bootefi_hello_compile')
@pytest.mark.buildconfigspec('cmd_tftpboot')
def test_efi_helloworld_net_tftp(ubman):
    """Run the helloworld.efi binary via TFTP.

    Call the do_test_efi_helloworld_net function to execute the test via TFTP.
    """

    do_test_efi_helloworld_net(ubman, PROTO_TFTP);

@pytest.mark.buildconfigspec('of_control')
@pytest.mark.buildconfigspec('bootefi_hello_compile')
@pytest.mark.buildconfigspec('cmd_wget')
def test_efi_helloworld_net_http(ubman):
    """Run the helloworld.efi binary via HTTP.

    Call the do_test_efi_helloworld_net function to execute the test via HTTP.
    """
    if ubman.config.env.get('env__efi_helloworld_net_http_test_skip', True):
        pytest.skip('helloworld.efi HTTP test is not enabled!')

    do_test_efi_helloworld_net(ubman, PROTO_HTTP);

@pytest.mark.buildconfigspec('cmd_bootefi_hello')
def test_efi_helloworld_builtin(ubman):
    """Run the builtin helloworld.efi binary.

    The helloworld.efi file is included in U-Boot, execute it using the
    special "bootefi hello" command.
    """

    output = ubman.run_command('bootefi hello')
    expected_text = 'Hello, world'
    assert expected_text in output

@pytest.mark.buildconfigspec('of_control')
@pytest.mark.buildconfigspec('cmd_bootefi')
@pytest.mark.buildconfigspec('cmd_tftpboot')
def test_efi_grub_net(ubman):
    """Run the grub.efi binary via TFTP.

    The grub.efi file is downloaded from the TFTP server and gets
    executed.
    """

    addr = fetch_file(ubman, 'env__efi_loader_grub_file', PROTO_TFTP)

    ubman.run_command('bootefi %x' % addr, wait_for_prompt=False)

    # Verify that we have an SMBIOS table
    check_smbios = ubman.config.env.get('env__efi_loader_check_smbios', False)
    if check_smbios:
        ubman.wait_for('grub>')
        ubman.run_command('lsefisystab', wait_for_prompt=False, wait_for_echo=False)
        ubman.wait_for('SMBIOS')

    # Then exit cleanly
    ubman.wait_for('grub>')
    ubman.run_command('exit', wait_for_prompt=False, wait_for_echo=False)
    ubman.wait_for(ubman.prompt)
    # And give us our U-Boot prompt back
    ubman.run_command('')

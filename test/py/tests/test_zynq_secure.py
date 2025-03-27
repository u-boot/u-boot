# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import re
import utils
import test_net

"""
This test verifies different type of secure boot images to authentication and
decryption using AES and RSA features for AMD's Zynq SoC.

Note: This test relies on boardenv_* containing configuration values to define
the network available and files to be used for testing. Without this, this test
will be automatically skipped. It also relies on dhcp or setup_static net test
to support tftp to load files from a TFTP server.

For example:

# Details regarding the files that may be read from a TFTP server and addresses
# and size for aes and rsa cases respectively. This variable may be omitted or
# set to None if zynqmp secure testing is not possible or desired.
env__zynq_aes_readable_file = {
    'fn': 'zynq_aes_image.bin',
    'fnbit': 'zynq_aes_bit.bin',
    'fnpbit': 'zynq_aes_par_bit.bin',
    'srcaddr': 0x1000000,
    'dstaddr': 0x2000000,
    'dstlen': 0x1000000,
}

env__zynq_rsa_readable_file = {
    'fn': 'zynq_rsa_image.bin',
    'fninvalid': 'zynq_rsa_image_invalid.bin',
    'srcaddr': 0x1000000,
}
"""

def zynq_secure_pre_commands(ubman):
    output = ubman.run_command('print modeboot')
    if not 'modeboot=' in output:
        pytest.skip('bootmode cannnot be determined')
    m = re.search('modeboot=(.+?)boot', output)
    if not m:
        pytest.skip('bootmode cannnot be determined')
    bootmode = m.group(1)
    if bootmode == 'jtag':
        pytest.skip('skipping due to jtag bootmode')

@pytest.mark.buildconfigspec('cmd_zynq_aes')
def test_zynq_aes_image(ubman):
    f = ubman.config.env.get('env__zynq_aes_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynq secure aes case to read')

    dstaddr = f.get('dstaddr', None)
    if not dstaddr:
        pytest.skip('No dstaddr specified in env file to read')

    dstsize = f.get('dstlen', None)
    if not dstsize:
        pytest.skip('No dstlen specified in env file to read')

    zynq_secure_pre_commands(ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
        addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = ubman.run_command('tftpboot %x %s' % (srcaddr, fn))
    assert expected_tftp in output

    expected_op = 'zynq aes [operation type] <srcaddr>'
    output = ubman.run_command(
        'zynq aes %x $filesize %x %x' % (srcaddr, dstaddr, dstsize)
    )
    assert expected_op not in output
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_zynq_aes')
def test_zynq_aes_bitstream(ubman):
    f = ubman.config.env.get('env__zynq_aes_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynq secure aes case to read')

    zynq_secure_pre_commands(ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
        addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fn = f['fnbit']
    output = ubman.run_command('tftpboot %x %s' % (srcaddr, fn))
    assert expected_tftp in output

    expected_op = 'zynq aes [operation type] <srcaddr>'
    output = ubman.run_command(
        'zynq aes load %x $filesize' % (srcaddr)
    )
    assert expected_op not in output
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_zynq_aes')
def test_zynq_aes_partial_bitstream(ubman):
    f = ubman.config.env.get('env__zynq_aes_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynq secure aes case to read')

    zynq_secure_pre_commands(ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
        addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fn = f['fnpbit']
    output = ubman.run_command('tftpboot %x %s' % (srcaddr, fn))
    assert expected_tftp in output

    expected_op = 'zynq aes [operation type] <srcaddr>'
    output = ubman.run_command('zynq aes loadp %x $filesize' % (srcaddr))
    assert expected_op not in output
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_zynq_rsa')
def test_zynq_rsa_image(ubman):
    f = ubman.config.env.get('env__zynq_rsa_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynq secure rsa case to read')

    zynq_secure_pre_commands(ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
        addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = ubman.run_command('tftpboot %x %s' % (srcaddr, fn))
    assert expected_tftp in output

    expected_op = 'zynq rsa <baseaddr>'
    output = ubman.run_command('zynq rsa %x ' % (srcaddr))
    assert expected_op not in output
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_zynq_rsa')
def test_zynq_rsa_image_invalid(ubman):
    f = ubman.config.env.get('env__zynq_rsa_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file for zynq secure rsa case to read')

    zynq_secure_pre_commands(ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    srcaddr = f.get('srcaddr', None)
    if not srcaddr:
        addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fninvalid = f['fninvalid']
    output = ubman.run_command('tftpboot %x %s' % (srcaddr, fninvalid))
    assert expected_tftp in output

    expected_op = 'zynq rsa <baseaddr>'
    output = ubman.run_command('zynq rsa %x ' % (srcaddr))
    assert expected_op in output
    output = ubman.run_command('echo $?')
    assert not output.endswith('0')

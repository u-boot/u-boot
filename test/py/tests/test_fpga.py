# SPDX-License-Identifier: GPL-2.0
#
# Copyright (c) 2018, Xilinx Inc.
#
# Michal Simek
# Siva Durga Prasad Paladugu

import pytest
import re
import random
import utils

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
    ('ipaddr', '10.0.0.100'),
    ('netmask', '255.255.255.0'),
    ('serverip', '10.0.0.1'),
]

# Details regarding the files that may be read from a TFTP server. .
env__fpga_secure_readable_file = {
    'fn': 'auth_bhdr_ppk1_bit.bin',
    'enckupfn': 'auth_bhdr_enc_kup_load_bit.bin',
    'addr': 0x1000000,
    'keyaddr': 0x100000,
    'keyfn': 'key.txt',
}

env__fpga_under_test = {
    'dev': 0,
    'addr' : 0x1000000,
    'bitstream_load': 'compress.bin',
    'bitstream_load_size': 1831960,
    'bitstream_loadp': 'compress_pr.bin',
    'bitstream_loadp_size': 423352,
    'bitstream_loadb': 'compress.bit',
    'bitstream_loadb_size': 1832086,
    'bitstream_loadbp': 'compress_pr.bit',
    'bitstream_loadbp_size': 423491,
    'mkimage_legacy': 'download.ub',
    'mkimage_legacy_size': 13321468,
    'mkimage_legacy_gz': 'download.gz.ub',
    'mkimage_legacy_gz_size': 53632,
    'mkimage_fit': 'download-fit.ub',
    'mkimage_fit_size': 13322784,
    'loadfs': 'mmc 0 compress.bin',
    'loadfs_size': 1831960,
    'loadfs_block_size': 0x10000,
}
"""

import test_net

def check_dev(ubman):
    f = ubman.config.env.get('env__fpga_under_test', None)
    if not f:
        pytest.skip('No FPGA to test')

    dev = f.get('dev', -1)
    if dev < 0:
        pytest.fail('No dev specified via env__fpga_under_test')

    return dev, f

def load_file_from_var(ubman, name):
    dev, f = check_dev(ubman)

    addr = f.get('addr', -1)
    if addr < 0:
        pytest.fail('No address specified via env__fpga_under_test')

    test_net.test_net_dhcp(ubman)
    test_net.test_net_setup_static(ubman)
    bit = f['%s' % (name)]
    bit_size = f['%s_size' % (name)]

    expected_tftp = 'Bytes transferred = %d' % bit_size
    output = ubman.run_command('tftpboot %x %s' % (addr, bit))
    assert expected_tftp in output

    return f, dev, addr, bit, bit_size

###### FPGA FAIL test ######
expected_usage = 'fpga - loadable FPGA image support'

@pytest.mark.xfail
@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_fail(ubman):
    # Test non valid fpga subcommand
    expected = 'fpga: non existing command'
    output = ubman.run_command('fpga broken 0')
    #assert expected in output
    assert expected_usage in output

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_help(ubman):
    # Just show help
    output = ubman.run_command('fpga')
    assert expected_usage in output


###### FPGA DUMP tests ######

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_dump(ubman):
    pytest.skip('Not implemented now')

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_dump_variable(ubman):
    # Same as above but via "fpga" variable
    pytest.skip('Not implemented now')

###### FPGA INFO tests ######

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_info_fail(ubman):
    # Maybe this can be skipped completely
    dev, f = check_dev(ubman)

    # Multiple parameters to fpga info should fail
    expected = 'fpga: more parameters passed'
    output = ubman.run_command('fpga info 0 0')
    #assert expected in output
    assert expected_usage in output

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_info_list(ubman):
    # Maybe this can be skipped completely
    dev, f = check_dev(ubman)

    # Code is design in a way that if fpga dev is not passed it should
    # return list of all fpga devices in the system
    ubman.run_command('setenv fpga')
    output = ubman.run_command('fpga info')
    assert expected_usage not in output

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_info(ubman):
    dev, f = check_dev(ubman)

    output = ubman.run_command('fpga info %x' % (dev))
    assert expected_usage not in output

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_info_variable(ubman):
    dev, f = check_dev(ubman)

    #
    # fpga variable is storing device number which doesn't need to be passed
    #
    ubman.run_command('setenv fpga %x' % (dev))

    output = ubman.run_command('fpga info')
    # Variable cleanup
    ubman.run_command('setenv fpga')
    assert expected_usage not in output

###### FPGA LOAD tests ######

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_load_fail(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_load')

    for cmd in ['dump', 'load', 'loadb']:
        # missing dev parameter
        expected = 'fpga: incorrect parameters passed'
        output = ubman.run_command('fpga %s %x $filesize' % (cmd, addr))
        #assert expected in output
        assert expected_usage in output

        # more parameters - 0 at the end
        expected = 'fpga: more parameters passed'
        output = ubman.run_command('fpga %s %x %x $filesize 0' % (cmd, dev, addr))
        #assert expected in output
        assert expected_usage in output

        # 0 address
        expected = 'fpga: zero fpga_data address'
        output = ubman.run_command('fpga %s %x 0 $filesize' % (cmd, dev))
        #assert expected in output
        assert expected_usage in output

        # 0 filesize
        expected = 'fpga: zero size'
        output = ubman.run_command('fpga %s %x %x 0' % (cmd, dev, addr))
        #assert expected in output
        assert expected_usage in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_load(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_load')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga load %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadp')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadp(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_load')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga load %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

    # And load also partial bistream
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_loadp')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadp %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadb(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_loadb')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadb %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadbp')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadbp(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_loadb')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadb %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

    # And load also partial bistream in bit format
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'bitstream_loadbp')

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadbp %x %x $filesize && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

###### FPGA LOADMK tests ######

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
def test_fpga_loadmk_fail(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy')

    ubman.run_command('imi %x' % (addr))

    # load image but pass incorrect address to show error message
    expected = 'Unknown image type'
    output = ubman.run_command('fpga loadmk %x %x' % (dev, addr + 0x10))
    assert expected in output

    # Pass more parameters then command expects - 0 at the end
    output = ubman.run_command('fpga loadmk %x %x 0' % (dev, addr))
    #assert expected in output
    assert expected_usage in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
def test_fpga_loadmk_legacy(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy')

    ubman.run_command('imi %x' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x %x && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.xfail
@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
def test_fpga_loadmk_legacy_variable_fpga(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy')

    ubman.run_command('imi %x' % (addr))

    ubman.run_command('setenv fpga %x' % (dev))

    # this testcase should cover case which looks like it is supported but dev pointer is broken by loading mkimage address
    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x && echo %s' % (addr, expected_text))
    ubman.run_command('setenv fpga')
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
def test_fpga_loadmk_legacy_variable_fpgadata(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy')

    ubman.run_command('imi %x' % (addr))

    ubman.run_command('setenv fpgadata %x' % (addr))

    # this testcase should cover case which looks like it is supported but dev pointer is broken by loading mkimage address
    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x && echo %s' % (dev, expected_text))
    ubman.run_command('setenv fpgadata')
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
def test_fpga_loadmk_legacy_variable(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy')

    ubman.run_command('imi %x' % (addr))

    ubman.run_command('setenv fpga %x' % (dev))
    ubman.run_command('setenv fpgadata %x' % (addr))

    # this testcase should cover case which looks like it is supported but dev pointer is broken by loading mkimage address
    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk && echo %s' % (expected_text))
    ubman.run_command('setenv fpga')
    ubman.run_command('setenv fpgadata')
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('cmd_echo')
@pytest.mark.buildconfigspec('legacy_image_format')
@pytest.mark.buildconfigspec('gzip')
def test_fpga_loadmk_legacy_gz(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_legacy_gz')

    ubman.run_command('imi %x' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x %x && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadmk_fit_external(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_fit_external')

    ubman.run_command('imi %x' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x %x:fpga && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadmk_fit(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_fit')

    ubman.run_command('imi %x' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x %x:fpga && echo %s' % (dev, addr, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadmk_fit_variable_fpga(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_fit')

    ubman.run_command('imi %x' % (addr))
    # FIXME this should fail - broken support in past
    ubman.run_command('setenv fpga %x' % (dev))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x:fpga && echo %s' % (addr, expected_text))
    ubman.run_command('setenv fpga')
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadmk_fit_variable_fpgadata(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_fit')

    ubman.run_command('imi %x' % (addr))
    # FIXME this should fail - broken support in past
    ubman.run_command('setenv fpgadata %x:fpga' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk %x && echo %s' % (dev, expected_text))
    ubman.run_command('setenv fpgadata')
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_loadmk')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadmk_fit_variable(ubman):
    f, dev, addr, bit, bit_size = load_file_from_var(ubman, 'mkimage_fit')

    ubman.run_command('imi %x' % (addr))

    ubman.run_command('setenv fpga %x' % (dev))
    ubman.run_command('setenv fpgadata %x:fpga' % (addr))

    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadmk && echo %s' % (expected_text))
    ubman.run_command('setenv fpga')
    ubman.run_command('setenv fpgadata')
    assert expected_text in output

###### FPGA LOAD tests ######

@pytest.mark.buildconfigspec('cmd_fpga')
def test_fpga_loadfs_fail(ubman):
    dev, f = check_dev(ubman)

    addr = f.get('addr', -1)
    if addr < 0:
        pytest.fail('No address specified via env__fpga_under_test')

    bit = f['loadfs']
    bit_size = f['loadfs_size']
    block_size = f['loadfs_block_size']

    # less params - dev number removed
    expected = 'fpga: incorrect parameters passed'
    output = ubman.run_command('fpga loadfs %x %x %x %s' % (addr, bit_size, block_size, bit))
    #assert expected in output
    assert expected_usage in output

    # one more param - 0 at the end
    # This is the longest command that's why there is no message from cmd/fpga.c
    output = ubman.run_command('fpga loadfs %x %x %x %x %s 0' % (dev, addr, bit_size, block_size, bit))
    assert expected_usage in output

    # zero address 0
    expected = 'fpga: zero fpga_data address'
    output = ubman.run_command('fpga loadfs %x %x %x %x %s' % (dev, 0, bit_size, block_size, bit))
    #assert expected in output
    assert expected_usage in output

    # bit_size 0
    expected = 'fpga: zero size'
    output = ubman.run_command('fpga loadfs %x %x %x %x %s' % (dev, addr, 0, block_size, bit))
    #assert expected in output
    assert expected_usage in output

    # block size 0
    # FIXME this should pass but it failing too
    output = ubman.run_command('fpga loadfs %x %x %x %x %s' % (dev, addr, bit_size, 0, bit))
    assert expected_usage in output

    # non existing bitstream name
    expected = 'Unable to read file noname'
    output = ubman.run_command('fpga loadfs %x %x %x %x mmc 0 noname' % (dev, addr, bit_size, block_size))
    assert expected in output
    assert expected_usage in output

    # -1 dev number
    expected = 'fpga_fsload: Invalid device number -1'
    output = ubman.run_command('fpga loadfs %d %x %x %x mmc 0 noname' % (-1, addr, bit_size, block_size))
    assert expected in output
    assert expected_usage in output


@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_echo')
def test_fpga_loadfs(ubman):
    dev, f = check_dev(ubman)

    addr = f.get('addr', -1)
    if addr < 0:
        pytest.fail('No address specified via env__fpga_under_test')

    bit = f['loadfs']
    bit_size = f['loadfs_size']
    block_size = f['loadfs_block_size']

    # This should be done better
    expected_text = 'FPGA loaded successfully'
    output = ubman.run_command('fpga loadfs %x %x %x %x %s && echo %s' % (dev, addr, bit_size, block_size, bit, expected_text))
    assert expected_text in output

@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_load_secure')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net', 'net_lwip')
def test_fpga_secure_bit_auth(ubman):

    test_net.test_net_dhcp(ubman)
    test_net.test_net_setup_static(ubman)

    f = ubman.config.env.get('env__fpga_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)
    if not addr:
      addr = utils.find_ram_base(ubman)

    expected_tftp = 'Bytes transferred = '
    fn = f['fn']
    output = ubman.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    expected_zynqmpsecure = 'Bitstream successfully loaded'
    output = ubman.run_command('fpga loads 0 %x $filesize 0 2' % (addr))
    assert expected_zynqmpsecure in output


@pytest.mark.buildconfigspec('cmd_fpga')
@pytest.mark.buildconfigspec('cmd_fpga_load_secure')
@pytest.mark.buildconfigspec('cmd_net')
@pytest.mark.buildconfigspec('cmd_dhcp')
@pytest.mark.buildconfigspec('net', 'net_lwip')
def test_fpga_secure_bit_img_auth_kup(ubman):

    test_net.test_net_dhcp(ubman)
    test_net.test_net_setup_static(ubman)

    f = ubman.config.env.get('env__fpga_secure_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    keyaddr = f.get('keyaddr', None)
    if not keyaddr:
      addr = utils.find_ram_base(ubman)
    expected_tftp = 'Bytes transferred = '
    keyfn = f['keyfn']
    output = ubman.run_command('tftpboot %x %s' % (keyaddr, keyfn))
    assert expected_tftp in output

    addr = f.get('addr', None)
    if not addr:
      addr = utils.find_ram_base(ubman)
    expected_tftp = 'Bytes transferred = '
    fn = f['enckupfn']
    output = ubman.run_command('tftpboot %x %s' % (addr, fn))
    assert expected_tftp in output

    expected_zynqmpsecure = 'Bitstream successfully loaded'
    output = ubman.run_command('fpga loads 0 %x $filesize 0 1 %x' % (addr, keyaddr))
    assert expected_zynqmpsecure in output

# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

"""
Note: This test relies on boardenv_* containing configuration values to define
which the network environment available for testing. Without this, this test
will be automatically skipped.

For example:

.. code-block:: python

   # Details regarding a boot image file that may be read from a TFTP server. This
   # variable may be omitted or set to None if TFTP boot testing is not possible
   # or desired.
   env__net_tftp_bootable_file = {
       'fn': 'image.ub',
       'addr': 0x10000000,
       'size': 5058624,
       'crc32': 'c2244b26',
       'pattern': 'Linux',
       'config': 'config@2',
       'timeout': 50000,
       'check_type': 'boot_error',
       'check_pattern': 'ERROR',
   }

   # False or omitted if a TFTP boot test should be tested.
   # If TFTP boot testing is not possible or desired, set this variable to True.
   # For example: If FIT image is not proper to boot
   env__tftp_boot_test_skip = False


Here is the example of FIT image configurations:

.. code-block:: devicetree

   configurations {
       default = "config@1";
       config@1 {
           description = "Boot Linux kernel with config@1";
           kernel = "kernel@0";
           fdt = "fdt@0";
           ramdisk = "ramdisk@0";
           hash@1 {
               algo = "sha1";
           };
       };
       config@2 {
           description = "Boot Linux kernel with config@2";
           kernel = "kernel@1";
           fdt = "fdt@1";
           ramdisk = "ramdisk@1";
           hash@1 {
               algo = "sha1";
           };
       };
   };

.. code-block:: python

   # Details regarding a file that may be read from a TFTP server. This variable
   # may be omitted or set to None if PXE testing is not possible or desired.
   env__net_pxe_bootable_file = {
       'fn': 'default',
       'addr': 0x10000000,
       'size': 74,
       'timeout': 50000,
       'pattern': 'Linux',
       'valid_label': '1',
       'invalid_label': '2',
       'exp_str_invalid': 'Skipping install for failure retrieving',
       'local_label': '3',
       'exp_str_local': 'missing environment variable: localcmd',
       'empty_label': '4',
       'exp_str_empty': 'No kernel given, skipping boot',
       'check_type': 'boot_error',
       'check_pattern': 'ERROR',
   }

   # False if a PXE boot test should be tested.
   # If PXE boot testing is not possible or desired, set this variable to True.
   # For example: If pxe configuration file is not proper to boot
   env__pxe_boot_test_skip = False

Here is the example of pxe configuration file ordered based on the execution
flow:

1) /tftpboot/pxelinux.cfg/default-arm-zynqmp

.. code-block::

    menu include pxelinux.cfg/default-arm
    timeout 50

    default Linux

2) /tftpboot/pxelinux.cfg/default-arm

.. code-block::

    menu title Linux boot selections
    menu include pxelinux.cfg/default

    label install
        menu label Invalid boot
        kernel kernels/install.bin
        append console=ttyAMA0,38400 debug earlyprintk
        initrd initrds/uzInitrdDebInstall

    label local
        menu label Local boot
        append root=/dev/sdb1
        localboot 1

    label boot
        menu label Empty boot

3) /tftpboot/pxelinux.cfg/default

.. code-block::

    label Linux
        menu label Boot kernel
        kernel Image
        fdt system.dtb
        initrd rootfs.cpio.gz.u-boot
"""

import pytest
import utils
import test_net
import re

def setup_networking(ubman):
    """Setup networking

    Making use of the test_net test, first try and configure networking via
    DHCP. If this fails, fall back to static configuration.
    """
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

def setup_tftpboot_boot(ubman):
    """Setup for the tftpboot 'boot' test

    We check that a file to use has been configured. If it has, we download it
    and ensure it has the expected crc32 value.
    """
    f = ubman.config.env.get('env__net_tftp_bootable_file', None)
    if not f:
        pytest.skip('No TFTP bootable file to read')

    setup_networking(ubman)
    addr = f.get('addr', None)
    if not addr:
        addr = utils.find_ram_base(ubman)

    fn = f['fn']
    timeout = f.get('timeout', 50000)

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command('tftpboot %x %s' % (addr, fn))

    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert expected_text in output

    expected_crc = f.get('crc32', None)
    output = ubman.run_command('crc32 %x $filesize' % addr)
    if expected_crc:
        assert expected_crc in output

    pattern = f.get('pattern')
    chk_type = f.get('check_type', 'boot_error')
    chk_pattern = re.compile(f.get('check_pattern', 'ERROR'))
    config = f.get('config', None)

    return addr, timeout, pattern, chk_type, chk_pattern, config

@pytest.mark.buildconfigspec('cmd_tftpboot')
def test_net_tftpboot_boot(ubman):
    """Boot the loaded image

    A boot file (fit image) is downloaded from the TFTP server and booted using
    bootm command with the default fit configuration, its boot log pattern are
    validated.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """
    if ubman.config.env.get('env__tftp_boot_test_skip', True):
        pytest.skip('TFTP boot test is not enabled!')

    addr, timeout, pattern, chk_type, chk_pattern, imcfg = setup_tftpboot_boot(
        ubman
    )

    if imcfg:
        bootcmd = 'bootm %x#%s' % (addr, imcfg)
    else:
        bootcmd = 'bootm %x' % addr

    with ubman.enable_check(
        chk_type, chk_pattern
    ), ubman.temporary_timeout(timeout):
        try:
            # wait_for_prompt=False makes the core code not wait for the U-Boot
            # prompt code to be seen, since it won't be on a successful kernel
            # boot
            ubman.run_command(bootcmd, wait_for_prompt=False)

            # Wait for boot log pattern
            ubman.wait_for(pattern)
        finally:
            # This forces the console object to be shutdown, so any subsequent
            # test will reset the board back into U-Boot. We want to force this
            # no matter whether the kernel boot passed or failed.
            ubman.drain_console()
            ubman.cleanup_spawn()

def setup_pxe_boot(ubman):
    """Setup for the PXE 'boot' test

    Make sure that the file to load via PXE boot has been configured.
    """
    f = ubman.config.env.get('env__net_pxe_bootable_file', None)
    if not f:
        pytest.skip('No PXE bootable file to read')

    setup_networking(ubman)
    bootfile = ubman.run_command('echo $bootfile')
    if not bootfile:
        bootfile = '<NULL>'

    return f, bootfile

@pytest.mark.buildconfigspec('cmd_pxe')
def test_net_pxe_boot(ubman):
    """Test the pxe boot command.

    A pxe configuration file is downloaded from the TFTP server and interpreted
    to boot the images mentioned in pxe configuration file.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """
    if ubman.config.env.get('env__pxe_boot_test_skip', True):
        pytest.skip('PXE boot test is not enabled!')

    f, bootfile = setup_pxe_boot(ubman)
    addr = f.get('addr', None)
    timeout = f.get('timeout', ubman.p.timeout)
    fn = f['fn']

    if addr:
        ubman.run_command('setenv pxefile_addr_r %x' % addr)

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command('pxe get')

    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert 'TIMEOUT' not in output
    assert expected_text in output
    assert f"Config file '{bootfile}' found" in output

    pattern = f.get('pattern')
    chk_type = f.get('check_type', 'boot_error')
    chk_pattern = re.compile(f.get('check_pattern', 'ERROR'))

    if not addr:
        pxe_boot_cmd = 'pxe boot'
    else:
        pxe_boot_cmd = 'pxe boot %x' % addr

    with ubman.enable_check(
        chk_type, chk_pattern
    ), ubman.temporary_timeout(timeout):
        try:
            ubman.run_command(pxe_boot_cmd, wait_for_prompt=False)
            ubman.wait_for(pattern)
        finally:
            ubman.drain_console()
            ubman.cleanup_spawn()

@pytest.mark.buildconfigspec('cmd_pxe')
def test_net_pxe_boot_config(ubman):
    """Test the pxe boot command by selecting different combination of labels

    A pxe configuration file is downloaded from the TFTP server and interpreted
    to boot the images mentioned in pxe configuration file.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """
    if ubman.config.env.get('env__pxe_boot_test_skip', True):
        pytest.skip('PXE boot test is not enabled!')

    f, bootfile = setup_pxe_boot(ubman)
    addr = f.get('addr', None)
    timeout = f.get('timeout', ubman.p.timeout)
    fn = f['fn']
    local_label = f['local_label']
    empty_label = f['empty_label']
    exp_str_local = f['exp_str_local']
    exp_str_empty = f['exp_str_empty']

    if addr:
        ubman.run_command('setenv pxefile_addr_r %x' % addr)

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command('pxe get')

    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert 'TIMEOUT' not in output
    assert expected_text in output
    assert f"Config file '{bootfile}' found" in output

    pattern = f.get('pattern')
    chk_type = f.get('check_type', 'boot_error')
    chk_pattern = re.compile(f.get('check_pattern', 'ERROR'))

    if not addr:
        pxe_boot_cmd = 'pxe boot'
    else:
        pxe_boot_cmd = 'pxe boot %x' % addr

    with ubman.enable_check(
        chk_type, chk_pattern
    ), ubman.temporary_timeout(timeout):
        try:
            ubman.run_command(pxe_boot_cmd, wait_for_prompt=False)

            # pxe config is loaded where multiple labels are there and need to
            # select particular label to boot and check for expected string
            # In this case, local label is selected and it should look for
            # localcmd env variable and if that variable is not defined it
            # should not boot it and come out to u-boot prompt
            ubman.wait_for('Enter choice:')
            ubman.run_command(local_label, wait_for_prompt=False)
            expected_str = ubman.p.expect([exp_str_local])
            assert (
                expected_str == 0
            ), f'Expected string: {exp_str_local} did not match!'

            # In this case, empty label is selected and it should look for
            # kernel image path and if it is not set it should fail it and load
            # default label to boot
            ubman.run_command(pxe_boot_cmd, wait_for_prompt=False)
            ubman.wait_for('Enter choice:')
            ubman.run_command(empty_label, wait_for_prompt=False)
            expected_str = ubman.p.expect([exp_str_empty])
            assert (
                expected_str == 0
            ), f'Expected string: {exp_str_empty} did not match!'

            ubman.wait_for(pattern)
        finally:
            ubman.drain_console()
            ubman.cleanup_spawn()

@pytest.mark.buildconfigspec('cmd_pxe')
def test_net_pxe_boot_config_invalid(ubman):
    """Test the pxe boot command by selecting invalid label

    A pxe configuration file is downloaded from the TFTP server and interpreted
    to boot the images mentioned in pxe configuration file.

    The details of the file to download are provided by the boardenv_* file;
    see the comment at the beginning of this file.
    """
    if ubman.config.env.get('env__pxe_boot_test_skip', True):
        pytest.skip('PXE boot test is not enabled!')

    f, bootfile = setup_pxe_boot(ubman)
    addr = f.get('addr', None)
    timeout = f.get('timeout', ubman.p.timeout)
    fn = f['fn']
    invalid_label = f['invalid_label']
    exp_str_invalid = f['exp_str_invalid']

    if addr:
        ubman.run_command('setenv pxefile_addr_r %x' % addr)

    with ubman.temporary_timeout(timeout):
        output = ubman.run_command('pxe get')

    expected_text = 'Bytes transferred = '
    sz = f.get('size', None)
    if sz:
        expected_text += '%d' % sz
    assert 'TIMEOUT' not in output
    assert expected_text in output
    assert f"Config file '{bootfile}' found" in output

    pattern = f.get('pattern')
    if not addr:
        pxe_boot_cmd = 'pxe boot'
    else:
        pxe_boot_cmd = 'pxe boot %x' % addr

    with ubman.temporary_timeout(timeout):
        try:
            ubman.run_command(pxe_boot_cmd, wait_for_prompt=False)

            # pxe config is loaded where multiple labels are there and need to
            # select particular label to boot and check for expected string
            # In this case invalid label is selected, it should load invalid
            # label and if it fails it should load the default label to boot
            ubman.wait_for('Enter choice:')
            ubman.run_command(invalid_label, wait_for_prompt=False)
            expected_str = ubman.p.expect([exp_str_invalid])
            assert (
                expected_str == 0
            ), f'Expected string: {exp_str_invalid} did not match!'

            ubman.wait_for(pattern)
        finally:
            ubman.drain_console()
            ubman.cleanup_spawn()

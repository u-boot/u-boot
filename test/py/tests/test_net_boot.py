# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Test various network-related functionality, such as the dhcp, ping, and
# tftpboot commands.

import pytest
import u_boot_utils
import test_net

"""
Note: This test relies on boardenv_* containing configuration values to define
which the network environment available for testing. Without this, this test
will be automatically skipped.
"""

@pytest.mark.buildconfigspec('cmd_net')
def test_net_tftpboot_boot(u_boot_console):
    """ Boot loaded image"

    """
    if not test_net.net_set_up:
        pytest.skip('Network not initialized')

    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)
    test_net.test_net_tftpboot(u_boot_console)

    f = u_boot_console.config.env.get('env__net_tftp_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)
    if not addr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    timeout = 50000
    with u_boot_console.temporary_timeout(timeout):
	try:
	    # wait_for_prompt=False makes the core code not wait for the U-Boot
	    # prompt code to be seen, since it won't be on a successful kernel
	    # boot
	    u_boot_console.run_command('bootm %x' % addr, wait_for_prompt=False)
	    # You might want to expand wait_for() with options to add extra bad
	    # patterns which immediately indicate a failed boot, or add a new
	    # "with object" function u_boot_console.enable_check() that can
	    # cause extra patterns like the U-Boot console prompt, U-Boot boot
	    # error messages, kernel boot error messages, etc. to fail the
	    # wait_for().
	    u_boot_console.wait_for('login:')
	finally:
	    # This forces the console object to be shutdown, so any subsequent
	    # test will reset the board back into U-Boot. We want to force this
	    # no matter whether the kernel boot passed or failed.
	    u_boot_console.drain_console()
	    u_boot_console.cleanup_spawn()

def test_net_tftpboot_boot_config2(u_boot_console):
    if not test_net.net_set_up:
        pytest.skip('Network not initialized')

    test_net.test_net_dhcp(u_boot_console)
    test_net.test_net_setup_static(u_boot_console)
    test_net.test_net_tftpboot(u_boot_console)

    f = u_boot_console.config.env.get('env__net_tftp_readable_file', None)
    if not f:
        pytest.skip('No TFTP readable file to read')

    addr = f.get('addr', None)
    if not addr:
      addr = u_boot_utils.find_ram_base(u_boot_console)

    response = u_boot_console.run_command('imi %x' % addr)
    if not "config@2" in response:
	pytest.skip("Second configuration not found");

    timeout = 50000
    with u_boot_console.temporary_timeout(timeout):
	try:
	    # wait_for_prompt=False makes the core code not wait for the U-Boot
	    # prompt code to be seen, since it won't be on a successful kernel
	    # boot
	    u_boot_console.run_command('bootm %x#config@2' % addr, wait_for_prompt=False)
	    # You might want to expand wait_for() with options to add extra bad
	    # patterns which immediately indicate a failed boot, or add a new
	    # "with object" function u_boot_console.enable_check() that can
	    # cause extra patterns like the U-Boot console prompt, U-Boot boot
	    # error messages, kernel boot error messages, etc. to fail the
	    # wait_for().
	    u_boot_console.wait_for('login:')
	finally:
	    # This forces the console object to be shutdown, so any subsequent
	    # test will reset the board back into U-Boot. We want to force this
	    # no matter whether the kernel boot passed or failed.
	    u_boot_console.drain_console()
	    u_boot_console.cleanup_spawn()


# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

"""
Note: This test doesn't rely on boardenv_* configuration value but they can
change test behavior.

For example:

# Setup env__saveenv_test_skip to True if saveenv test is not possible or
# desired and should be skipped.
env__saveenv_test_skip = True

# Setup env__saveenv_test to set the bootmode if 'modeboot' u-boot environment
# variable is not set. Test will be skipped if bootmode is not set in both
# places i.e, boardenv and modeboot u-boot environment variable
env__saveenv_test = {
    'bootmode': 'qspiboot',
}

# This test will be also skipped if the bootmode is detected to JTAG.
"""

import pytest
import random
import ipaddress
import string
import uuid

# Setup the env
def setup_saveenv_env(u_boot_console):
    if u_boot_console.config.env.get('env__saveenv_test_skip', False):
        pytest.skip('saveenv test is not enabled')

    output = u_boot_console.run_command('echo $modeboot')
    if output:
        bootmode = output
    else:
        f = u_boot_console.config.env.get('env__saveenv_test', None)
        if not f:
            pytest.skip('bootmode cannot be determined')
        bootmode = f.get('bootmode', 'jtagboot')

    if 'jtag' in bootmode:
        pytest.skip('skipping saveenv test due to jtag bootmode')

# Check return code
def ret_code(u_boot_console):
    return u_boot_console.run_command('echo $?')

# Verify env variable
def check_env(u_boot_console, var_name, var_value):
    if var_value:
        output = u_boot_console.run_command(f'printenv {var_name}')
        var_value = str(var_value)
        if (var_value.startswith("'") and var_value.endswith("'")) or (
            var_value.startswith('"') and var_value.endswith('"')
        ):
            var_value = var_value.split(var_value[-1])[1]
        assert var_value in output
        assert ret_code(u_boot_console).endswith('0')
    else:
        u_boot_console.p.send(f'printenv {var_name}\n')
        output = u_boot_console.p.expect(['not defined'])
        assert output == 0
        assert ret_code(u_boot_console).endswith('1')

# Set env variable
def set_env(u_boot_console, var_name, var_value):
    u_boot_console.run_command(f'setenv {var_name} {var_value}')
    assert ret_code(u_boot_console).endswith('0')
    check_env(u_boot_console, var_name, var_value)

@pytest.mark.buildconfigspec('cmd_saveenv')
@pytest.mark.buildconfigspec('hush_parser')
def test_saveenv(u_boot_console):
    """Test the saveenv command in non-JTAG bootmode.
    It saves the U-Boot environment in persistent storage.
    """
    setup_saveenv_env(u_boot_console)

    # Set env for random mac address
    rand_mac = '%02x:%02x:%02x:%02x:%02x:%02x' % (
        random.randint(0, 255),
        random.randint(0, 255),
        random.randint(0, 255),
        random.randint(0, 255),
        random.randint(0, 255),
        random.randint(0, 255),
    )
    set_env(u_boot_console, 'mac_addr', rand_mac)

    # Set env for random IPv4 address
    rand_ipv4 = ipaddress.IPv4Address._string_from_ip_int(
        random.randint(0, ipaddress.IPv4Address._ALL_ONES)
    )
    set_env(u_boot_console, 'ipv4_addr', rand_ipv4)

    # Set env for random IPv6 address
    rand_ipv6 = ipaddress.IPv6Address._string_from_ip_int(
        random.randint(0, ipaddress.IPv6Address._ALL_ONES)
    )
    set_env(u_boot_console, 'ipv6_addr', rand_ipv6)

    # Set env for random number
    rand_num = random.randrange(1, 10**9)
    set_env(u_boot_console, 'num_var', rand_num)

    # Set env for uuid
    uuid_str = uuid.uuid4().hex.lower()
    set_env(u_boot_console, 'uuid_var', uuid_str)

    # Set env for random string including special characters
    sc = "!#%&()*+,-./:;<=>?@[\\]^_`{|}~"
    rand_str = ''.join(
        random.choices(' ' + string.ascii_letters + sc + string.digits, k=300)
    )
    set_env(u_boot_console, 'str_var', f'"{rand_str}"')

    # Set env for empty string
    set_env(u_boot_console, 'empty_var', '')

    # Save the env variables
    u_boot_console.run_command('saveenv')
    assert ret_code(u_boot_console).endswith('0')

    # Reboot
    u_boot_console.run_command('reset', wait_for_reboot=True)

    # Verify the saved env variables
    check_env(u_boot_console, 'mac_addr', rand_mac)
    check_env(u_boot_console, 'ipv4_addr', rand_ipv4)
    check_env(u_boot_console, 'ipv6_addr', rand_ipv6)
    check_env(u_boot_console, 'num_var', rand_num)
    check_env(u_boot_console, 'uuid_var', uuid_str)
    check_env(u_boot_console, 'str_var', rand_str)
    check_env(u_boot_console, 'empty_var', '')

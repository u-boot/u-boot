# SPDX-License-Identifier: GPL-2.0
# (C) Copyright 2023, Advanced Micro Devices, Inc.

import pytest
import random
import string
import test_net

"""
Note: This test relies on boardenv_* containing configuration values to define
RPU applications information for AMD's ZynqMP SoC which contains, application
names, processors, address where it is built, expected output and the tftp load
addresses. This test will be automatically skipped without this.

It also relies on dhcp or setup_static net test to support tftp to load
application on DDR. All the environment parameters are stored sequentially.
The length of all parameters values should be same. For example, if 2 app_names
are defined in a list as a value of parameter 'app_name' then the other
parameters value also should have a list with 2 items.
It will run RPU cases for all the applications defined in boardenv_*
configuration file.

Example:
env__zynqmp_rpu_apps = {
    'app_name': ['hello_world_r5_0_ddr.elf', 'hello_world_r5_1_ddr.elf'],
    'proc': ['rpu0', 'rpu1'],
    'cpu_num': [4, 5],
    'addr': [0xA00000, 0xB00000],
    'output': ['Successfully ran Hello World application on DDR from RPU0',
               'Successfully ran Hello World application on DDR from RPU1'],
    'tftp_addr': [0x100000, 0x200000],
}
"""

# Get rpu apps params from env
def get_rpu_apps_env(ubman):
    rpu_apps = ubman.config.env.get('env__zynqmp_rpu_apps', False)
    if not rpu_apps:
        pytest.skip('ZynqMP RPU application info not defined!')

    apps = rpu_apps.get('app_name', None)
    if not apps:
        pytest.skip('No RPU application found!')

    procs = rpu_apps.get('proc', None)
    if not procs:
        pytest.skip('No RPU application processor provided!')

    cpu_nums = rpu_apps.get('cpu_num', None)
    if not cpu_nums:
        pytest.skip('No CPU number for respective processor provided!')

    addrs = rpu_apps.get('addr', None)
    if not addrs:
        pytest.skip('No RPU application build address found!')

    outputs = rpu_apps.get('output', None)
    if not outputs:
        pytest.skip('Expected output not found!')

    tftp_addrs = rpu_apps.get('tftp_addr', None)
    if not tftp_addrs:
        pytest.skip('TFTP address to load application not found!')

    return apps, procs, cpu_nums, addrs, outputs, tftp_addrs

# Check return code
def ret_code(ubman):
    return ubman.run_command('echo $?')

# Initialize tcm
def tcminit(ubman, rpu_mode):
    output = ubman.run_command(f'zynqmp tcminit {rpu_mode}')
    assert 'Initializing TCM overwrites TCM content' in output
    return ret_code(ubman)

# Load application in DDR
def load_app_ddr(ubman, tftp_addr, app):
    output = ubman.run_command('tftpboot %x %s' % (tftp_addr, app))
    assert 'TIMEOUT' not in output
    assert 'Bytes transferred = ' in output

    # Load elf
    ubman.run_command('bootelf -p %x' % tftp_addr)
    assert ret_code(ubman).endswith('0')

# Disable cpus
def disable_cpus(ubman, cpu_nums):
    for num in cpu_nums:
        ubman.run_command(f'cpu {num} disable')

# Get random RPU mode between string and integer
def get_rpu_mode(rpu_mode):
    if rpu_mode == 0 or rpu_mode == 'lockstep':
        return random.choice(['lockstep', 0])
    elif rpu_mode == 1 or rpu_mode == 'split':
        return random.choice(['split', 1])

# Load apps on RPU cores
def rpu_apps_load(ubman, rpu_mode):
    apps, procs, cpu_nums, addrs, outputs, tftp_addrs = get_rpu_apps_env(
        ubman)
    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    try:
        assert tcminit(ubman, get_rpu_mode(rpu_mode)).endswith('0')

        for i in range(len(apps)):
            if rpu_mode == 'lockstep' and procs[i] != 'rpu0':
                continue

            load_app_ddr(ubman, tftp_addrs[i], apps[i])
            rel_addr = hex(int(addrs[i] + 0x3C))

            # Release cpu at app load address
            cpu_num = cpu_nums[i]
            cmd = f'cpu {cpu_num} release {rel_addr} {rpu_mode}'
            output = ubman.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {rel_addr}'
            assert exp_op in output
            assert f'R5 {rpu_mode} mode' in output
            ubman.wait_for(outputs[i])
            assert ret_code(ubman).endswith('0')
    finally:
        disable_cpus(ubman, cpu_nums)

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_split(ubman):
    rpu_apps_load(ubman, 'split')

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_lockstep(ubman):
    rpu_apps_load(ubman, 'lockstep')

@pytest.mark.buildconfigspec('cmd_zynqmp')
def test_zynqmp_rpu_app_load_negative(ubman):
    apps, procs, cpu_nums, addrs, outputs, tftp_addrs = get_rpu_apps_env(
        ubman)

    # Invalid commands
    rand_str = ''.join(random.choices(string.ascii_lowercase, k=4))
    rand_num = random.randint(2, 100)
    inv_modes = ['mode', rand_str, rand_num, 'splittt', 'locksteppp', '00', 11]

    for mode in inv_modes:
        ubman.run_command(f'zynqmp tcminit {mode}')
        assert ret_code(ubman).endswith('1')

    test_net.test_net_dhcp(ubman)
    if not test_net.net_set_up:
        test_net.test_net_setup_static(ubman)

    try:
        rpu_mode = 'split'
        assert tcminit(ubman, get_rpu_mode(rpu_mode)).endswith('0')

        inv_modes += [0, 1]
        for i in range(len(apps)):
            load_app_ddr(ubman, tftp_addrs[i], apps[i])

            # Run in split mode at different load address
            rel_addr = hex(int(addrs[i]) + random.randint(200, 1000))
            cpu_num = cpu_nums[i]
            cmd = f'cpu {cpu_num} release {rel_addr} {rpu_mode}'
            output = ubman.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {rel_addr}'
            assert exp_op in output
            assert f'R5 {rpu_mode} mode' in output
            assert not outputs[i] in output

            # Invalid rpu mode
            for mode in inv_modes:
                cmd = f'cpu {cpu_num} release {rel_addr} {mode}'
                output = ubman.run_command(cmd)
                assert exp_op in output
                assert f'Unsupported mode' in output
                assert not ret_code(ubman).endswith('0')

        # Switch to lockstep mode, without disabling CPUs
        rpu_mode = 'lockstep'
        output = ubman.run_command(
            f'zynqmp tcminit {get_rpu_mode(rpu_mode)}'
        )
        assert 'ERROR: ' in output

        # Disable cpus
        disable_cpus(ubman, cpu_nums)

        # Switch to lockstep mode, after disabling CPUs
        output = ubman.run_command(
            f'zynqmp tcminit {get_rpu_mode(rpu_mode)}'
        )
        assert 'Initializing TCM overwrites TCM content' in output
        assert ret_code(ubman).endswith('0')

        # Run lockstep mode for RPU1/RPU0
        for i in range(len(apps)):
            load_app_ddr(ubman, tftp_addrs[i], apps[i])
            rel_addr = hex(int(addrs[i] + 0x3C))
            cpu_num = cpu_nums[i]
            cmd = f'cpu {cpu_num} release {rel_addr} {rpu_mode}'
            output = ubman.run_command(cmd)
            exp_op = f'Using TCM jump trampoline for address {rel_addr}'
            assert exp_op in output

            if procs[i] == 'rpu1':
                assert 'Lockstep mode should run on ZYNQMP_CORE_RPU0' in output
                assert not ret_code(ubman).endswith('0')
            elif procs[i] == 'rpu0':
                assert f'R5 {rpu_mode} mode' in output
                ubman.wait_for(outputs[i])
                assert ret_code(ubman).endswith('0')
            else:
                assert False, 'ERROR: Invalid processor!'
    finally:
        disable_cpus(ubman, cpu_nums)
        # This forces the console object to be shutdown, so any subsequent test
        # will reset the board back into U-Boot.
        ubman.drain_console()
        ubman.cleanup_spawn()

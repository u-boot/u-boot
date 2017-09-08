# Copyright (c) 2015 Stephen Warren
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import random

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_bus(u_boot_console):
    expected_response = "Bus"
    response = u_boot_console.run_command("i2c bus")
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_dev(u_boot_console):
    expected_response = "Current bus"
    response = u_boot_console.run_command("i2c dev")
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe(u_boot_console):
    expected_response = "Valid chip addresses:"
    response = u_boot_console.run_command("i2c probe")
    assert(expected_response in response)

@pytest.mark.boardidentity("!qemu")
@pytest.mark.boardspec("zynq_zc702")
@pytest.mark.boardspec("zynq_zc706")
@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe_zc70x(u_boot_console):
    # Enable i2c mux bridge
    u_boot_console.run_command("i2c mw 74 0 4")
    u_boot_console.run_command("i2c probe")
    val = format(random.randint(0,255), '02x')
    u_boot_console.run_command("i2c mw 54 0 " + val + " 5")
    response = u_boot_console.run_command("i2c md 54 0 5")
    expected_response = "0000: " + val + " " + val + " " + val + " " + val + " " + val + " "
    assert(expected_response in response)

@pytest.mark.boardspec("xilinx_zynqmp_zcu102")
@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_probe_zcu102(u_boot_console):
    # This is using i2c mux wiring from config file
    u_boot_console.run_command("i2c dev 5")
    u_boot_console.run_command("i2c probe")
    val = format(random.randint(0,255), '02x')
    u_boot_console.run_command("i2c mw 54 0 " + val + " 5")
    response = u_boot_console.run_command("i2c md 54 0 5")
    expected_response = "0000: " + val + " " + val + " " + val + " " + val + " " + val + " "
    print expected_response
    assert(expected_response in response)

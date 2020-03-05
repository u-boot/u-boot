# Copyright (c) 2015 Stephen Warren
#
# SPDX-License-Identifier: GPL-2.0

import pytest
import random

"""
Note: This test doesn't rely on boardenv_* configuration value but they can
change test behavior.

# Setup env__i2c_device_test_skip to True if tests with i2c devices should be
# skipped. For example: Missing QEMU model or broken i2c device
env__i2c_device_test_skip = True

"""

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
    expected_response = "Setting bus to 0"
    response = u_boot_console.run_command("i2c dev 0")
    assert(expected_response in response)
    expected_response = "Valid chip addresses:"
    response = u_boot_console.run_command("i2c probe")
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_i2c")
def test_i2c_eeprom(u_boot_console):
    f = u_boot_console.config.env.get('env__i2c_eeprom_device_test', None)
    if not f:
        pytest.skip('No I2C eeprom to test')

    bus = f.get('bus', 0)
    if bus < 0:
        pytest.fail('No bus specified via env__i2c_eeprom_device_test')

    addr = f.get('eeprom_addr', -1)
    if addr < 0:
        pytest.fail('No eeprom address specified via env__i2c_eeprom_device_test')

    # Enable i2c mux bridge
    u_boot_console.run_command("i2c dev %x" % bus)
    u_boot_console.run_command("i2c probe")
    value = random.randint(0,255)
    val = format(value, '02x')
    u_boot_console.run_command("i2c mw %x 0 %x 5" % (addr, value))
    response = u_boot_console.run_command("i2c md %x 0 5" % addr)
    expected_response = "0000: " + val + " " + val + " " + val + " " + val + " " + val + " "
    assert(expected_response in response)

# SPDX-License-Identifier: GPL-2.0

import pytest
import u_boot_utils

@pytest.mark.buildconfigspec('cmd_pinmux')
def test_pinmux_usage_1(u_boot_console):
    """Test that 'pinmux' command without parameters displays
    pinmux usage."""
    output = u_boot_console.run_command('pinmux')
    assert 'Usage:' in output

@pytest.mark.buildconfigspec('cmd_pinmux')
def test_pinmux_usage_2(u_boot_console):
    """Test that 'pinmux status' executed without previous "pinmux dev"
    command displays pinmux usage."""
    output = u_boot_console.run_command('pinmux status')
    assert 'Usage:' in output

@pytest.mark.buildconfigspec('cmd_pinmux')
@pytest.mark.boardspec('sandbox')
def test_pinmux_status_all(u_boot_console):
    """Test that 'pinmux status -a' displays pin's muxing."""
    output = u_boot_console.run_command('pinmux status -a')

    assert ('pinctrl-gpio:' in output)
    assert ('a5        : gpio output .' in output)
    assert ('a6        : gpio output .' in output)

    assert ('pinctrl:' in output)
    assert ('SCL       : I2C SCL.' in output)
    assert ('SDA       : I2C SDA.' in output)
    assert ('TX        : Uart TX.' in output)
    assert ('RX        : Uart RX.' in output)
    assert ('W1        : 1-wire gpio.' in output)
    assert ('GPIO0     : gpio bias-pull-up input-disable.' in output)
    assert ('GPIO1     : gpio drive-open-drain.' in output)
    assert ('GPIO2     : gpio bias-pull-down input-enable.' in output)
    assert ('GPIO3     : gpio bias-disable.' in output)

@pytest.mark.buildconfigspec('cmd_pinmux')
@pytest.mark.boardspec('sandbox')
def test_pinmux_list(u_boot_console):
    """Test that 'pinmux list' returns the pin-controller list."""
    output = u_boot_console.run_command('pinmux list')
    assert 'sandbox_pinctrl' in output

@pytest.mark.buildconfigspec('cmd_pinmux')
def test_pinmux_dev_bad(u_boot_console):
    """Test that 'pinmux dev' returns an error when trying to select a
    wrong pin controller."""
    pincontroller = 'bad_pin_controller_name'
    output = u_boot_console.run_command('pinmux dev ' + pincontroller)
    expected_output = 'Can\'t get the pin-controller: ' + pincontroller + '!'
    assert (expected_output in output)

@pytest.mark.buildconfigspec('cmd_pinmux')
@pytest.mark.boardspec('sandbox')
def test_pinmux_dev(u_boot_console):
    """Test that 'pinmux dev' select the wanted pin controller."""
    pincontroller = 'pinctrl'
    output = u_boot_console.run_command('pinmux dev ' + pincontroller)
    expected_output = 'dev: ' + pincontroller
    assert (expected_output in output)

@pytest.mark.buildconfigspec('cmd_pinmux')
@pytest.mark.boardspec('sandbox')
def test_pinmux_status(u_boot_console):
    """Test that 'pinmux status' displays selected pincontroller's pin
    muxing descriptions."""
    output = u_boot_console.run_command('pinmux status')

    assert (not 'pinctrl-gpio:' in output)
    assert (not 'pinctrl:' in output)

    assert ('SCL       : I2C SCL.' in output)
    assert ('SDA       : I2C SDA.' in output)
    assert ('TX        : Uart TX.' in output)
    assert ('RX        : Uart RX.' in output)
    assert ('W1        : 1-wire gpio.' in output)
    assert ('GPIO0     : gpio bias-pull-up input-disable.' in output)
    assert ('GPIO1     : gpio drive-open-drain.' in output)
    assert ('GPIO2     : gpio bias-pull-down input-enable.' in output)
    assert ('GPIO3     : gpio bias-disable.' in output)

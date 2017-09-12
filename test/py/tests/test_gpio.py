# Copyright (c) 2017 Xilinx, Inc.
#
# SPDX-License-Identifier: GPL-2.0

# Test various gpio-related functionality, such as the input, set,
# clear and toggle.

import pytest
import random

"""
Note: This test relies on boardenv_* containing configuration values to define
which the gpio available for testing. Without this, this test will be  automat-
ically skipped.

For example:

# A list of gpio's that are going to be tested in order to validate the
# test.
env__gpio_val = {
     "gpio": [0,1,2],
}

# A list of gpio's that are shorted on the hardware board and first gpio of
# each group is configured as input and the other as output. If the pins are
# not shorted properly, then the test will be fail.
env__gpio_input_output = {
     "list_of_gpios": [ [36, 37], [38, 39]],
}
"""

def gpio_input(u_boot_console, gpio):
    u_boot_console.run_command("gpio input %d" %gpio)
    response = u_boot_console.run_command("gpio status -a %d" %gpio)
    expected_response = "%d: input:" %gpio
    assert(expected_response in response)

def gpio_set(u_boot_console, gpio):
    expected_response = "%d: output: 1" %gpio
    u_boot_console.run_command("gpio set %d" %gpio)
    response = u_boot_console.run_command("gpio status -a %d" %gpio)
    assert(expected_response in response)

def gpio_clear(u_boot_console, gpio):
    expected_response = "%d: output: 0" %gpio
    u_boot_console.run_command("gpio clear %d" %gpio)
    response = u_boot_console.run_command("gpio status -a %d" %gpio)
    assert(expected_response in response)

def gpio_toggle(u_boot_console, gpio):
    expected_response = "%d: output: 1" %gpio
    u_boot_console.run_command("gpio toggle %d" %gpio)
    response = u_boot_console.run_command("gpio status -a %d" %gpio)
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_gpio")
def test_gpio_status(u_boot_console):
    response = u_boot_console.run_command("gpio status -a")
    expected_response = "0: input:"
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_gpio")
def test_gpio(u_boot_console):
    f = u_boot_console.config.env.get('env__gpio_val', None)
    if not f:
        pytest.skip('No GPIO readable file to read')

    gpin = f.get("gpio", None)

    for gpio in gpin:
        gpio_input(u_boot_console, gpio)
        gpio_set(u_boot_console, gpio)
        gpio_clear(u_boot_console, gpio)
        gpio_toggle(u_boot_console, gpio)

@pytest.mark.buildconfigspec("cmd_gpio")
def test_gpio_input_output(u_boot_console):
    f = u_boot_console.config.env.get('env__gpio_input_output', None)
    if not f:
        pytest.skip('No GPIO readable file to read')

    list_of_gpios = f.get("list_of_gpios", None)

    flag = 0
    for list in list_of_gpios:
        for i in list:
            if flag == 0:
               gpio_in = i
               expected_response = "%d: input:" %gpio_in
               u_boot_console.run_command("gpio input %d" %gpio_in)
               response = u_boot_console.run_command("gpio status -a %d" %gpio_in)
               assert(expected_response in response)
               flag = 1

            else:
               gpio_out = i
               expected_response = "%d: output:" %gpio_out
               u_boot_console.run_command("gpio set %d" %gpio_out)
               response = u_boot_console.run_command("gpio status -a %d" %gpio_out)
               assert(expected_response in response)
               flag = 0

        expected_response = "%d: input: 0" %gpio_in
        u_boot_console.run_command("gpio clear %d" %gpio_out)
        response = u_boot_console.run_command("gpio status -a %d" %gpio_in)
        assert(expected_response in response)

        expected_response = "%d: input: 1" %gpio_in
        u_boot_console.run_command("gpio set %d" %gpio_out)
        response = u_boot_console.run_command("gpio status -a %d" %gpio_in)
        assert(expected_response in response)

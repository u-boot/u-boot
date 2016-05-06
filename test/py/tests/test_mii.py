# Copyright (c) 2016, Xilinx Inc. Michal Simek
#
# SPDX-License-Identifier: GPL-2.0

import pytest

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_info(u_boot_console):
    expected_response = "PHY"
    response = u_boot_console.run_command("mii info")
    assert(expected_response in response)

@pytest.mark.buildconfigspec("cmd_mii")
def test_mii_list(u_boot_console):
    expected_response = "Current device"
    response = u_boot_console.run_command("mii device")
    assert(expected_response in response)

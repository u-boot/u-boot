# Copyright (c) 2016, Xilinx Inc. Michal Simek
#
# SPDX-License-Identifier: GPL-2.0

import pytest

@pytest.mark.buildconfigspec("cmd_mii")
def test_mdio_list(u_boot_console):
    expected_response = "<-->"
    response = u_boot_console.run_command("mdio list")
    assert(expected_response in response)

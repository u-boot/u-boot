# Copyright (c) 2016, Xilinx Inc. Michal Simek
#
# SPDX-License-Identifier: GPL-2.0

import pytest

@pytest.mark.buildconfigspec("cmd_dm")
def test_dm_tree(u_boot_console):
    response = u_boot_console.run_command("dm tree")

@pytest.mark.buildconfigspec("cmd_dm")
def test_dm_uclass(u_boot_console):
    response = u_boot_console.run_command("dm uclass")

@pytest.mark.buildconfigspec("cmd_dm")
def test_dm_devres(u_boot_console):
    response = u_boot_console.run_command("dm devres")

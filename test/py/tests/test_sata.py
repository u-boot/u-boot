# Copyright (c) 2018 Michal Simek
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

@pytest.mark.boardspec("xilinx_zynqmp_zcu102_rev1_0")
@pytest.mark.boardspec("xilinx_zynqmp_zcu102_revA")
@pytest.mark.buildconfigspec("cmd_i2c")
@pytest.mark.buildconfigspec("cmd_scsi")
def test_sata_probe_zcu102(u_boot_console):
    test_skip = u_boot_console.config.env.get('env__i2c_device_test_skip', False)
    if test_skip:
        pytest.skip('i2c device test skipped')

    # This is using i2c mux wiring from config file
    u_boot_console.run_command("i2c dev 0")
    u_boot_console.run_command("i2c mw 20 6 0")
    u_boot_console.run_command("i2c mw 20 2 ef")
    response = u_boot_console.run_command("scsi reset")
    expected_response = "Type: Hard Disk"
    assert(expected_response in response)

@pytest.mark.boardspec("!xilinx_zynqmp_zcu102_rev1_0")
@pytest.mark.boardspec("!xilinx_zynqmp_zcu102_revA")
@pytest.mark.buildconfigspec("cmd_i2c")
@pytest.mark.buildconfigspec("cmd_scsi")
def test_sata_probe(u_boot_console):
    response = u_boot_console.run_command("scsi reset")
    expected_response = "Type: Hard Disk"
    assert(expected_response in response)

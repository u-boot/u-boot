# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
# Copyright (c) 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>
#
# SPDX-License-Identifier: GPL-2.0

# Test efi API implementation

import pytest
import u_boot_utils

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest(u_boot_console):
	"""
	Run bootefi selftest
	"""

	u_boot_console.run_command(cmd='bootefi selftest', wait_for_prompt=False)
	m = u_boot_console.p.expect(['Summary: 0 failures', 'Press any key'])
	if m != 0:
		raise Exception('Failures occured during the EFI selftest')
	u_boot_console.run_command(cmd='', wait_for_echo=False, wait_for_prompt=False);
	m = u_boot_console.p.expect(['resetting', 'U-Boot'])
	if m != 0:
		raise Exception('Reset failed during the EFI selftest')
	u_boot_console.restart_uboot();

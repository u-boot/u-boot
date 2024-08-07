# SPDX-License-Identifier: GPL-2.0+
# Copyright 2024 Google LLC
#
# Test addition of Universal Payload

import os

import pytest
import u_boot_utils

@pytest.mark.boardspec('sandbox_vpl')
def test_upl_handoff(u_boot_console):
    cons = u_boot_console
    ram = os.path.join(cons.config.build_dir, 'ram.bin')
    fdt = os.path.join(cons.config.build_dir, 'u-boot.dtb')

    # Remove any existing RAM file, so we don't have old data present
    if os.path.exists(ram):
        os.remove(ram)
    flags = ['-m', ram, '-d', fdt]
    cons.restart_uboot_with_flags(flags, use_dtb=False)

    # Make sure that Universal Payload is detected in U-Boot proper
    output = cons.run_command('upl info')
    assert output == 'UPL state: active'

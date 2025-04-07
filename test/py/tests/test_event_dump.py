# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest
import re
import utils

# This is only a partial test - coverting 64-bit sandbox. It does not test
# big-endian images, nor 32-bit images
@pytest.mark.boardspec('sandbox')
def test_event_dump(ubman):
    """Test that the "help" command can be executed."""
    sandbox = ubman.config.build_dir + '/u-boot'
    out = utils.run_and_log(ubman, ['scripts/event_dump.py', sandbox])
    expect = '''.*Event type            Id                              Source location
--------------------  ------------------------------  ------------------------------
EVT_FT_FIXUP          bootmeth_vbe_ft_fixup           .*boot/vbe_request.c:.*
EVT_FT_FIXUP          bootmeth_vbe_simple_ft_fixup    .*boot/vbe_simple_os.c:.*
EVT_LAST_STAGE_INIT   alloc_write_acpi_tables         .*lib/acpi/acpi_table.c:.*
EVT_LAST_STAGE_INIT   efi_block_device_create         .*lib/efi_driver/efi_block_device.c:.*
EVT_LAST_STAGE_INIT   install_smbios_table            .*lib/efi_loader/efi_smbios.c:.*
EVT_MISC_INIT_F       sandbox_early_getopt_check      .*arch/sandbox/cpu/start.c:.*
EVT_TEST              h_adder_simple                  .*test/common/event.c:'''
    assert re.match(expect, out, re.MULTILINE) is not None

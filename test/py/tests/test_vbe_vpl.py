# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
# Test addition of VBE

import os

import pytest
import utils

@pytest.mark.boardspec('sandbox_vpl')
@pytest.mark.requiredtool('dtc')
def test_vbe_vpl(ubman):
    #cmd = [ubman.config.build_dir + fname, '-v']
    ram = os.path.join(ubman.config.build_dir, 'ram.bin')
    fdt = os.path.join(ubman.config.build_dir, 'arch/sandbox/dts/test.dtb')
    image_fname = os.path.join(ubman.config.build_dir, 'image.bin')

    # Enable firmware1 and the mmc that it uses. These are needed for the full
    # VBE flow.
    utils.run_and_log(
        ubman, f'fdtput -t s {fdt} /bootstd/firmware0 status disabled')
    utils.run_and_log(
        ubman, f'fdtput -t s {fdt} /bootstd/firmware1 status okay')
    utils.run_and_log(
        ubman, f'fdtput -t s {fdt} /mmc3 status okay')
    utils.run_and_log(
        ubman, f'fdtput -t s {fdt} /mmc3 filename {image_fname}')

    # Remove any existing RAM file, so we don't have old data present
    if os.path.exists(ram):
        os.remove(ram)
    flags = ['-p', image_fname, '-w', '-s', 'state.dtb']
    ubman.restart_uboot_with_flags(flags)

    # Make sure that VBE was used in both VPL (to load SPL) and SPL (to load
    # U-Boot
    output = ubman.run_command('vbe state')
    assert output == 'Phases: VPL SPL'

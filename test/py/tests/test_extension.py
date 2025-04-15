# SPDX-License-Identifier:  GPL-2.0+
# Copyright (c) 2020
# Author: Kory Maincent <kory.maincent@bootlin.com>

# Test U-Boot's "extension" commands.

import os
import pytest
import utils

overlay_addr = 0x1000

SANDBOX_DTB='arch/sandbox/dts/sandbox.dtb'
OVERLAY_DIR='arch/sandbox/dts/'

def load_dtb(ubman):
    ubman.log.action('Loading devicetree to RAM...')
    ubman.run_command('host load hostfs - $fdt_addr_r %s' % (os.path.join(ubman.config.build_dir, SANDBOX_DTB)))
    ubman.run_command('fdt addr $fdt_addr_r')

@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.boardspec('sandbox')
def test_extension(ubman):
    """Test the 'extension' command."""

    load_dtb(ubman)

    output = ubman.run_command('extension list')
    # extension_bootdev_hunt may have already run.
    # Without reboot we cannot make any assumption here.
    # assert('No extension' in output)

    output = ubman.run_command('extension scan')
    assert output == 'Found 2 extension board(s).'

    output = ubman.run_command('extension list')
    assert('overlay0.dtbo' in output)
    assert('overlay1.dtbo' in output)

    ubman.run_command_list([
        'setenv extension_overlay_addr %s' % (overlay_addr),
        'setenv extension_overlay_cmd \'host load hostfs - ${extension_overlay_addr} %s${extension_overlay_name}\'' % (os.path.join(ubman.config.build_dir, OVERLAY_DIR))])

    output = ubman.run_command('extension apply 0')
    assert('bytes read' in output)

    output = ubman.run_command('fdt print')
    assert('button3' in output)

    output = ubman.run_command('extension apply all')
    assert('bytes read' in output)

    output = ubman.run_command('fdt print')
    assert('button4' in output)


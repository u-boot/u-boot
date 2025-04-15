# SPDX-License-Identifier: GPL-2.0+
# Copyright 2025 Canonical Ltd.
# Written by Simon Glass <simon.glass@canonical.com>

import pytest

# Enable early console so that the test can see if something goes wrong
CONSOLE = 'earlycon=uart8250,io,0x3f8 console=uart8250,io,0x3f8'

@pytest.mark.boardspec('qemu-x86_64')
@pytest.mark.role('qemu-x86_64')
def test_distro(ubman):
    """Test that of-platdata can be generated and used in sandbox"""
    with ubman.log.section('boot'):
        ubman.run_command('boot', wait_for_prompt=False)

    with ubman.log.section('Grub'):
        # Wait for grub to come up and offset a menu
        ubman.p.expect(['Try or Install Ubuntu'])

        # Press 'e' to edit the command line
        ubman.log.info("Pressing 'e'")
        ubman.run_command('e', wait_for_prompt=False, send_nl=False)

        # Wait until we see the editor appear
        ubman.p.expect(['/casper/initrd'])

        # Go down to the 'linux' line. Avoid using down-arrow as that includes
        # an Escape character, which may be parsed by Grub as such, causing it
        # to return to the top menu
        ubman.log.info("Going DOWN")
        ubman.ctrl('N')
        ubman.ctrl('N')
        ubman.ctrl('N')

        # Go to end of line
        ubman.log.info("Going to EOL")
        ubman.ctrl('E')

        # Backspace to remove 'quiet splash'
        ubman.log.info("Erasing quiet and splash")
        ubman.send('\b' * len('quiet splash'))

        # Send our noisy console
        ubman.log.info("Noisy console")
        ubman.send(CONSOLE)

        # Tell grub to boot
        ubman.log.info("boot")
        ubman.ctrl('X')
        ubman.p.expect(['Booting a command list'])

    with ubman.log.section('Linux'):
        # Linux should start immediately
        ubman.p.expect(['Linux version'])

    with ubman.log.section('Ubuntu'):
        # Shortly later, we should see this banner
        ubman.p.expect(['Welcome to .*Ubuntu 24.04.1 LTS.*!'])

    ubman.restart_uboot()

# SPDX-License-Identifier:      GPL-2.0+
# Copyright 2026 Free Mobile - Vincent Jardin

"""Regression test for `load semihosting - <addr> <file>`.

Companion to test_hostfs.py: same fixture, same crc32, different
fstype routing:
see the doc/usage/cmd/load.rst "Null-block-device interfaces" section.
"""

import pytest


@pytest.mark.buildconfigspec('semihosting')
def test_semihosting_load(ubman, semihosting_data):
    """Run `load semihosting - <addr> <file>` and check the bytes."""
    response = ubman.run_command(
        f'load semihosting - $loadaddr {semihosting_data}')

    # Fixture is "Das U-Boot\n" (11 bytes).
    assert '11 bytes read' in response

    # crc32("Das U-Boot\n")
    response = ubman.run_command('crc32 $loadaddr $filesize')
    assert '==> 60cfccfc' in response


@pytest.mark.buildconfigspec('semihosting')
def test_semihosting_load_offset(ubman, semihosting_data):
    """Run the [bytes] [pos] variant through the same dispatch."""
    response = ubman.run_command(
        f'load semihosting - $loadaddr {semihosting_data} 4 6')
    # bytes=4 pos=6 over "Das U-Boot\n" -> "Boot".
    assert '4 bytes read' in response

    # crc32("Boot")
    response = ubman.run_command('crc32 $loadaddr $filesize')
    assert '==> e6df01fa' in response

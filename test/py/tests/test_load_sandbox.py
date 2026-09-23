# SPDX-License-Identifier:      GPL-2.0+
# Copyright 2026 Free Mobile - Vincent Jardin

"""Regression test for `load sandbox - <addr> <file>`.

Exercises the null_dev_desc_ok dispatch added in
"fs: dispatch null_dev_desc_ok filesystems before block lookup".

It is the counterpart of test_load_semihosting.py
"""

import os
import pytest


@pytest.fixture(scope='session')
def sandbox_fixture(u_boot_config):
    """Host-staged fixture file read by `load sandbox`."""
    path = os.path.join(u_boot_config.persistent_data_dir,
                        'sandbox-fstype.txt')
    with open(path, 'w', encoding='utf-8') as f:
        f.write('Das U-Boot\n')   # 11 bytes, same as test_hostfs.py / semihosting
    yield path
    os.remove(path)


@pytest.mark.boardspec('sandbox')
def test_sandbox_load(ubman, sandbox_fixture):
    """Run `load sandbox - <addr> <file>` and check the bytes."""
    response = ubman.run_command(
        f'load sandbox - $loadaddr {sandbox_fixture}')

    # Fixture is "Das U-Boot\n" (11 bytes).
    assert '11 bytes read' in response

    # crc32("Das U-Boot\n") -- identical to the semihosting / hostfs checks.
    response = ubman.run_command('crc32 $loadaddr $filesize')
    assert '==> 60cfccfc' in response


@pytest.mark.boardspec('sandbox')
def test_sandbox_load_offset(ubman, sandbox_fixture):
    """Run the [bytes] [pos] variant through the same dispatch."""
    response = ubman.run_command(
        f'load sandbox - $loadaddr {sandbox_fixture} 4 6')
    # bytes=4 pos=6 over "Das U-Boot\n" -> "Boot".
    assert '4 bytes read' in response

    # crc32("Boot")
    response = ubman.run_command('crc32 $loadaddr $filesize')
    assert '==> e6df01fa' in response

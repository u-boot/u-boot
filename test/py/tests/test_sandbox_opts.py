# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest

import utils

# This is needed for Azure, since the default '..' directory is not writeable
TMPDIR = '/tmp/test_cmdline'

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_sandbox_cmdline(ubman):
    """Test building sandbox without CONFIG_CMDLINE"""

    utils.run_and_log(
        ubman, ['./tools/buildman/buildman', '-m', '--board', 'sandbox',
               '-a', '~CMDLINE', '-o', TMPDIR])

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_sandbox_lto(ubman):
    """Test building sandbox without CONFIG_LTO"""

    utils.run_and_log(
        ubman, ['./tools/buildman/buildman', '-m', '--board', 'sandbox',
               '-a', '~LTO', '-o', TMPDIR])

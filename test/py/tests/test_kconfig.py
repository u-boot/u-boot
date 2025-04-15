# SPDX-License-Identifier: GPL-2.0
# Copyright 2022 Google LLC
# Written by Simon Glass <sjg@chromium.org>

import pytest

import utils

# This is needed for Azure, since the default '..' directory is not writeable
TMPDIR = '/tmp/test_kconfig'

@pytest.mark.slow
@pytest.mark.boardspec('sandbox')
def test_kconfig(ubman):
    """Test build failures when IF_ENABLED_INT() option is not enabled"""

    # This detects build errors in test/lib/kconfig.c
    out = utils.run_and_log(
        ubman, ['./tools/buildman/buildman', '-m', '--board', 'sandbox',
               '-a', 'TEST_KCONFIG', '-o', TMPDIR], ignore_errors=True)
    assert 'invalid_use_of_IF_ENABLED_INT' in out
    assert 'invalid_use_of_CONFIG_IF_ENABLED_INT' in out

@pytest.mark.slow
@pytest.mark.boardspec('sandbox_spl')
def test_kconfig_spl(ubman):
    """Test build failures when IF_ENABLED_INT() option is not enabled"""

    # This detects build errors in test/lib/kconfig_spl.c
    out = utils.run_and_log(
        ubman, ['./tools/buildman/buildman', '-m', '--board', 'sandbox_spl',
               '-a', 'TEST_KCONFIG', '-o', TMPDIR], ignore_errors=True)
    assert 'invalid_use_of_IF_ENABLED_INT' in out

    # There is no CONFIG_SPL_TEST_KCONFIG, so the CONFIG_IF_ENABLED_INT()
    # line should not generate an error
    assert 'invalid_use_of_CONFIG_IF_ENABLED_INT' not in out

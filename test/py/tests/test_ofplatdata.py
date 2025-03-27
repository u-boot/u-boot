# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest
import utils

@pytest.mark.boardspec('sandbox_spl')
@pytest.mark.buildconfigspec('spl_of_platdata')
def test_spl_devicetree(ubman):
    """Test content of spl device-tree"""
    dtb = ubman.config.build_dir + '/spl/u-boot-spl.dtb'
    fdtgrep = ubman.config.build_dir + '/tools/fdtgrep'
    output = utils.run_and_log(ubman, [fdtgrep, '-l', dtb])

    assert "bootph-all" not in output
    assert "bootph-some-ram" not in output
    assert "bootph-pre-ram" not in output
    assert "bootph-pre-sram" not in output

    assert "spl-test5" not in output
    assert "spl-test6" not in output
    assert "spl-test7" in output

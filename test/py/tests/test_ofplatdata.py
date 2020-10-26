# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest
import u_boot_utils as util

@pytest.mark.buildconfigspec('spl_of_platdata')
def test_spl_devicetree(u_boot_console):
    """Test content of spl device-tree"""
    cons = u_boot_console
    dtb = cons.config.build_dir + '/spl/u-boot-spl.dtb'
    fdtgrep = cons.config.build_dir + '/tools/fdtgrep'
    output = util.run_and_log(cons, [fdtgrep, '-l', dtb])

    assert "u-boot,dm-pre-reloc" not in output
    assert "u-boot,dm-pre-proper" not in output
    assert "u-boot,dm-spl" not in output
    assert "u-boot,dm-tpl" not in output

    assert "spl-test4" in output
    assert "spl-test5" not in output
    assert "spl-test6" not in output
    assert "spl-test7" in output

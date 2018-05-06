# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc

import pytest

OF_PLATDATA_OUTPUT = ''

@pytest.mark.buildconfigspec('spl_of_platdata')
def test_ofplatdata(u_boot_console):
    """Test that of-platdata can be generated and used in sandbox"""
    cons = u_boot_console
    output = cons.get_spawn_output().replace('\r', '')
    assert OF_PLATDATA_OUTPUT in output

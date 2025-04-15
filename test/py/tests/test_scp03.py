# Copyright (c) 2021 Foundries.io Ltd
#
# SPDX-License-Identifier:  GPL-2.0+
#
# SCP03 command test

"""
This tests SCP03 command in U-Boot.

For additional details check doc/usage/scp03.rst
"""

import pytest
import utils

@pytest.mark.buildconfigspec('cmd_scp03')
def test_scp03(ubman):
    """Enable and provision keys with SCP03
    """

    success_str1 = "SCP03 is enabled"
    success_str2 = "SCP03 is provisioned"

    response = ubman.run_command('scp03 enable')
    assert success_str1 in response
    response = ubman.run_command('scp03 provision')
    assert success_str2 in response

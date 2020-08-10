# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import pytest
from sqfs_common import *

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_fs_generic')
@pytest.mark.buildconfigspec('cmd_squashfs')
@pytest.mark.buildconfigspec('fs_squashfs')
@pytest.mark.requiredtool('mksquashfs')
def test_sqfs_ls(u_boot_console):
    cons = u_boot_console
    sqfs_generate_image(cons)
    path = os.path.join(cons.config.build_dir, "sqfs")
    try:
        output = u_boot_console.run_command("host bind 0 " + path)
        output = u_boot_console.run_command("sqfsls host 0")
        assert "4 file(s), 0 dir(s)" in output
        assert "<SYM>   sym" in output
        output = u_boot_console.run_command("sqfsls host 0 xxx")
        assert "** Cannot find directory. **" in output
    except:
        sqfs_clean(cons)
    sqfs_clean(cons)

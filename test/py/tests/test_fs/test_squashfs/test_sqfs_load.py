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
def test_sqfs_load(u_boot_console):
    cons = u_boot_console
    sqfs_generate_image(cons)
    command = "sqfsload host 0 $kernel_addr_r "
    path = os.path.join(cons.config.build_dir, "sqfs")

    try:
        output = u_boot_console.run_command("host bind 0 " + path)
        output = u_boot_console.run_command(command + "xxx")
        assert "File not found." in output
        output = u_boot_console.run_command(command + "frag_only")
        assert "100 bytes read in" in output
        output = u_boot_console.run_command(command + "blks_frag")
        assert "5100 bytes read in" in output
        output = u_boot_console.run_command(command + "blks_only")
        assert "4096 bytes read in" in output
        output = u_boot_console.run_command(command + "sym")
        assert "100 bytes read in" in output
    except:
        sqfs_clean(cons)
    sqfs_clean(cons)

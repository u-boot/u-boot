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
    build_dir = u_boot_console.config.build_dir
    for opt in comp_opts:
        try:
            opt.gen_image(build_dir)
        except RuntimeError:
            opt.clean_source(build_dir)
            # skip unsupported compression types
            continue
        path = os.path.join(build_dir, "sqfs-" + opt.name)
        output = u_boot_console.run_command("host bind 0 " + path)

        try:
            # list files in root directory
            output = u_boot_console.run_command("sqfsls host 0")
            assert str(len(opt.files) + 1) + " file(s), 0 dir(s)" in output
            assert "<SYM>   sym" in output
            output = u_boot_console.run_command("sqfsls host 0 xxx")
            assert "** Cannot find directory. **" in output
        except:
            opt.cleanup(build_dir)
            assert False
        opt.cleanup(build_dir)

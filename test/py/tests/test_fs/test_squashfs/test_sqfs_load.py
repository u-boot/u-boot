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
    build_dir = u_boot_console.config.build_dir
    command = "sqfsload host 0 $kernel_addr_r "

    for opt in comp_opts:
        # generate and load the squashfs image
        try:
            opt.gen_image(build_dir)
        except RuntimeError:
            opt.clean_source(build_dir)
            # skip unsupported compression types
            continue

        path = os.path.join(build_dir, "sqfs-" + opt.name)
        output = u_boot_console.run_command("host bind 0 " + path)

        output = u_boot_console.run_command(command + "xxx")
        assert "File not found." in output

        for (f, s) in zip(opt.files, opt.sizes):
            try:
                output = u_boot_console.run_command(command + f)
                assert str(s) in output
            except:
                assert False
                opt.cleanup(build_dir)

        # test symbolic link
        output = u_boot_console.run_command(command + "sym")
        assert str(opt.sizes[0]) in output

        # remove generated files
        opt.cleanup(build_dir)

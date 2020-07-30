# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import random
import string

def sqfs_get_random_letters(size):
    letters = []
    for i in range(0, size):
            letters.append(random.choice(string.ascii_letters))

    return ''.join(letters)

def sqfs_generate_file(path, size):
    content = sqfs_get_random_letters(size)
    file = open(path, "w")
    file.write(content)
    file.close()

# generate image with three files and a symbolic link
def sqfs_generate_image():
    src = "test/py/tests/test_fs/test_squashfs/sqfs_src/"
    dest = "test/py/tests/test_fs/test_squashfs/sqfs"
    os.mkdir(src)
    sqfs_generate_file(src + "frag_only", 100)
    sqfs_generate_file(src + "blks_frag", 5100)
    sqfs_generate_file(src + "blks_only", 4096)
    os.symlink("frag_only", src + "sym")
    os.system("mksquashfs " + src + " " + dest + " -b 4096 -always-use-fragments")

# removes all files created by sqfs_generate_image()
def sqfs_clean():
    src = "test/py/tests/test_fs/test_squashfs/sqfs_src/"
    dest = "test/py/tests/test_fs/test_squashfs/sqfs"
    os.remove(src + "frag_only")
    os.remove(src + "blks_frag")
    os.remove(src + "blks_only")
    os.remove(src + "sym")
    os.rmdir(src)
    os.remove(dest)

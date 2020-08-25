# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2020 Bootlin
# Author: Joao Marcos Costa <joaomarcos.costa@bootlin.com>

import os
import random
import string
import subprocess

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

class Compression:
    def __init__(self, name, files, sizes, block_size = 4096):
        self.name = name
        self.files = files
        self.sizes = sizes
        self.mksquashfs_opts = " -b " + str(block_size) + " -comp " + self.name

    def add_opt(self, opt):
        self.mksquashfs_opts += " " + opt

    def gen_image(self, build_dir):
        src = os.path.join(build_dir, "sqfs_src/")
        os.mkdir(src)
        for (f, s) in zip(self.files, self.sizes):
            sqfs_generate_file(src + f, s)

        # the symbolic link always targets the first file
        os.symlink(self.files[0], src + "sym")

        sqfs_img = os.path.join(build_dir, "sqfs-" + self.name)
        i_o = src + " " + sqfs_img
        opts = self.mksquashfs_opts
        try:
            subprocess.run(["mksquashfs " + i_o + opts], shell = True, check = True)
        except:
            print("mksquashfs error. Compression type: " + self.name)
            raise RuntimeError

    def clean_source(self, build_dir):
        src = os.path.join(build_dir, "sqfs_src/")
        for f in self.files:
            os.remove(src + f)
        os.remove(src + "sym")
        os.rmdir(src)

    def cleanup(self, build_dir):
        self.clean_source(build_dir)
        sqfs_img = os.path.join(build_dir, "sqfs-" + self.name)
        os.remove(sqfs_img)

files = ["blks_only", "blks_frag", "frag_only"]
sizes = [4096, 5100, 100]
gzip = Compression("gzip", files, sizes)
zstd = Compression("zstd", files, sizes)
lzo = Compression("lzo", files, sizes)

# use fragment blocks for files larger than block_size
gzip.add_opt("-always-use-fragments")
zstd.add_opt("-always-use-fragments")

# avoid fragments if lzo is used
lzo.add_opt("-no-fragments")

comp_opts = [gzip, zstd, lzo]

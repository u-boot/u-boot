# SPDX-License-Identifier: GPL-2.0
# Regression test for the SquashFS directory-table integer overflow.

import os
import struct
import pytest

# metadata block size (SQFS_METADATA_BLOCK_SIZE) and metadata header size
SQFS_METADATA_BLOCK_SIZE = 8192
# metablks_count that makes metablks_count * SQFS_METADATA_BLOCK_SIZE wrap a
# 32-bit int back down to a tiny value: (2^19 + 1) * 8192 == 2^32 + 8192.
NR_METABLKS = (1 << 19) + 1

def make_overflow_image(path):
    """Build a SquashFS image whose directory table inflates metablks_count so
    that metablks_count * SQFS_METADATA_BLOCK_SIZE wraps a 32-bit int, then
    writes one block of data one metadata block past the resulting buffer."""
    def metahdr(size):
        # uncompressed metadata block header (bit 15 set)
        return struct.pack('<H', 0x8000 | (size & 0x7fff))

    # inode table: one small valid uncompressed metadata block
    inode_region = metahdr(32) + b'\x00' * 32
    inode_start = 96
    dir_start = inode_start + len(inode_region)

    # directory table: NR_METABLKS metadata blocks, only block 1 carries data
    dir_region = bytearray()
    dir_region += metahdr(0)                          # block 0: empty
    dir_region += metahdr(200) + b'A' * 200           # block 1: data
    dir_region += metahdr(0) * (NR_METABLKS - 2)      # blocks 2..N-1: empty
    frag_start = dir_start + len(dir_region)

    sb = bytearray(96)
    struct.pack_into('<I', sb, 0, 0x73717368)         # s_magic
    struct.pack_into('<I', sb, 4, 1)                  # inodes
    struct.pack_into('<I', sb, 12, 131072)            # block_size
    struct.pack_into('<H', sb, 20, 1)                 # compression = gzip/zlib
    struct.pack_into('<H', sb, 22, 17)                # block_log
    struct.pack_into('<H', sb, 26, 1)                 # no_ids
    struct.pack_into('<H', sb, 28, 4)                 # s_major
    struct.pack_into('<Q', sb, 48, frag_start)        # id_table_start
    struct.pack_into('<Q', sb, 56, 0xffffffffffffffff)  # xattr_id_table_start
    struct.pack_into('<Q', sb, 64, inode_start)       # inode_table_start
    struct.pack_into('<Q', sb, 72, dir_start)         # directory_table_start
    struct.pack_into('<Q', sb, 80, frag_start)        # fragment_table_start
    struct.pack_into('<Q', sb, 88, 0xffffffffffffffff)  # export_table_start

    img = bytearray(sb) + inode_region + dir_region
    # pad so the directory-table block read stays within the file
    need = ((len(img) + 511) // 512 + 1) * 512
    img += b'\x00' * (need - len(img))
    struct.pack_into('<Q', img, 40, len(img))         # bytes_used

    with open(path, 'wb') as f:
        f.write(img)

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_squashfs')
@pytest.mark.buildconfigspec('fs_squashfs')
@pytest.mark.singlethread
def test_sqfs_ls_dir_table_overflow(ubman):
    """Listing a crafted image whose directory table declares an oversized
    metadata-block count must be rejected without corrupting the heap.
    """
    ubman.restart_uboot()
    image_path = os.path.join(ubman.config.build_dir, 'sqfs_dir_table_overflow')
    make_overflow_image(image_path)
    try:
        ubman.run_command('host bind 0 {}'.format(image_path))
        ubman.run_command('sqfsls host 0')
        # The crafted image must not take the board down.
        assert 'alive' in ubman.run_command('echo alive')
    finally:
        os.remove(image_path)

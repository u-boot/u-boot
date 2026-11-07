# SPDX-License-Identifier: GPL-2.0+

"""Test ext4 handling of corrupt on-disk indices

Most tests make a small ext4 image with mkfs.ext4, corrupt one on-disk
field, with debugfs or by patching the image directly, and check that
U-Boot rejects the image and is still responsive afterwards. The others
check that U-Boot reads or replaces a valid file whose blocks or inode lie
at a boundary that it used to get wrong.
"""

from contextlib import contextmanager
import os
import struct
from subprocess import run, check_output, DEVNULL
import zlib
import pytest
from fstest_defs import ADDR

VICTIM = b'victim file\n'


@contextmanager
def ext4_image(ubman, prefix, opts=(), size=8 << 20):
    """Make an ext4 image and remove it once the test is done

    mk_fs() takes no mkfs.ext4 options, which some tests need. When the with
    block ends, whether or not the test passed, the image is removed and
    host device 0, which the test may have bound to it, is unbound.

    Args:
        ubman (ConsoleBase): U-Boot console
        prefix (str): prefix of the image file name
        opts (list of str): extra options for mkfs.ext4
        size (int): size of the image in bytes

    Yields:
        str: path of the image
    """
    img = os.path.join(ubman.config.persistent_data_dir, f'{prefix}.ext4.img')
    with open(img, 'wb') as fil:
        fil.truncate(size)
    try:
        # U-Boot does not write to a filesystem with metadata checksums
        run(['mkfs.ext4', '-q', '-F', '-O', '^metadata_csum', *opts, img],
            check=True)
        yield img
    finally:
        os.remove(img)
        ubman.run_command('host unbind 0')


def debugfs(img, cmds):
    """Run debugfs commands which write to an image

    Args:
        img (str): path of the image
        cmds (list of str): debugfs commands
    """
    script = '\n'.join(cmds) + '\n'
    run(['debugfs', '-w', '-f', '-', img], input=script.encode(),
        capture_output=True, check=True)


def add_file(img, name, chunks):
    """Write a file into the root directory of an image with debugfs

    Args:
        img (str): path of the image
        name (str): name of the file
        chunks (dict): data of the file (bytes) by offset (int), the rest
            of the file being left as holes
    """
    src = f'{img}.{name}'
    with open(src, 'wb') as fil:
        for offset, data in chunks.items():
            fil.seek(offset)
            fil.write(data)
    try:
        debugfs(img, [f'write {src} {name}'])
    finally:
        os.remove(src)


def read_at(img, offset, size):
    """Read size bytes of the image at offset"""
    with open(img, 'rb') as fil:
        fil.seek(offset)
        return fil.read(size)


def write_at(img, offset, data):
    """Write data into the image at offset"""
    with open(img, 'r+b') as fil:
        fil.seek(offset)
        fil.write(data)


def block_size(img):
    """Get the block size of the filesystem in an image"""
    return 1024 << struct.unpack('<I', read_at(img, 1024 + 24, 4))[0]


def file_block(img, path, blk):
    """Get the filesystem block which holds a block of a file"""
    return int(check_output(['debugfs', '-R', f'bmap {path} {blk}', img],
                            stderr=DEVNULL).decode())


def victim_has(img, text):
    """Check that debugfs shows some text in the inode of /victim"""
    out = check_output(['debugfs', '-R', 'stat victim', img],
                       stderr=DEVNULL).decode()
    return text in out


def load_crc(ubman, size, pos):
    """Read part of /victim from host device 0 and get its CRC32

    Args:
        ubman (ConsoleBase): U-Boot console
        size (int): number of bytes to read
        pos (int): offset in the file to read from

    Returns:
        str: output of the crc32 command
    """
    ubman.run_command(f'ext4load host 0:0 {ADDR:x} /victim {size:x} {pos:x}')
    return ubman.run_command(f'crc32 {ADDR:x} {size:x}')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_extent_header(ubman):
    """ext4fs_get_extent_block(): an extent header claiming more entries
    than the inode's i_block area holds must be rejected"""
    with ext4_image(ubman, 'ext_oob_eh') as img:
        add_file(img, 'victim', {0: VICTIM})
        # i_block[0] = eh_magic | eh_entries << 16: 255 entries, eh_max stays 4
        debugfs(img, ['sif victim block[0] 0x00fff30a'])
        assert victim_has(img, 'Flags: 0x80000')

        ubman.run_command(f'host bind 0 {img}')
        out = ubman.run_command(f'ext4load host 0:0 {ADDR:x} /victim')
        assert 'invalid extent block' in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_extent_loop(ubman):
    """ext4fs_get_extent_block(): an index node pointing back at itself, so
    that the depth does not drop, must be rejected rather than walked for
    ever"""
    with ext4_image(ubman, 'ext_oob_loop') as img:
        add_file(img, 'victim', {0: VICTIM})
        blksz = block_size(img)
        blk = file_block(img, 'victim', 0)
        # Make the data block an index node of depth 1 with one entry, which
        # points at the node itself: eh_magic, eh_entries, eh_max, eh_depth,
        # eh_generation, then ei_block, ei_leaf_lo, ei_leaf_hi, ei_unused
        write_at(img, blk * blksz,
                 struct.pack('<4HI2I2H', 0xf30a, 1, (blksz - 12) // 12, 1, 0,
                             0, blk, 0, 0))
        # Make the root an index node of depth 1 pointing at that block:
        # i_block[1] = eh_max | eh_depth << 16, i_block[4] = ei_leaf_lo
        debugfs(img, ['sif victim block[1] 0x00010004',
                      f'sif victim block[4] {blk}', 'sif victim block[5] 0'])

        ubman.run_command(f'host bind 0 {img}')
        out = ubman.run_command(f'ext4load host 0:0 {ADDR:x} /victim')
        assert 'invalid extent block' in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_triple_indirect(ubman):
    """read_allocated_block(): blocks reached through the second and third
    entries of the triple indirect block must be read correctly"""
    # With 1 KiB blocks an indirect block holds 256 block numbers. The file
    # blocks after the 12 direct, 256 single and 256 * 256 double indirect
    # ones are reached through the triple indirect block, each entry of which
    # maps 256 * 256 blocks.
    per = 256
    tind = 12 + per + per * per
    # The first block of entry 1, the last block of its first indirect block
    # and the first of its second, and the first block of entry 2
    blocks = [tind + per * per, tind + per * per + per - 1,
              tind + per * per + per, tind + 2 * per * per]
    chunks = {blk * 1024: f'{blk:08x}'.encode() * 128 for blk in blocks}
    with ext4_image(ubman, 'ext_oob_tind',
                    ['-b', '1024', '-O', '^extent,^64bit']) as img:
        add_file(img, 'victim', chunks)
        assert victim_has(img, '(TIND)')

        ubman.run_command(f'host bind 0 {img}')
        for first, count in ((blocks[0], 1), (blocks[1], 2), (blocks[3], 1)):
            data = b''.join(chunks[(first + i) * 1024] for i in range(count))
            out = load_crc(ubman, len(data), first * 1024)
            assert f'==> {zlib.crc32(data):08x}' in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_group_layout(ubman):
    """ext4fs_init(): a superblock with zero blocks per group must make
    writes fail rather than divide by zero"""
    with ext4_image(ubman, 'ext_oob_geo') as img:
        # s_blocks_per_group is at offset 32 of the superblock at byte 1024
        write_at(img, 1024 + 32, struct.pack('<I', 0))

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(
            f'ext4write host 0:0 {ADDR:x} /newfile 0x1000')
        assert 'Unsupported block group layout' in out
        assert 'written' not in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_delete_block(ubman):
    """delete_single_indirect_block(): replacing a file whose indirect block
    lies past the end of the filesystem must fail before it is released"""
    with ext4_image(ubman, 'ext_oob_del') as img:
        add_file(img, 'victim', {0: VICTIM})
        # Make /victim a block-mapped file whose only bad field is block[IND]
        cmds = ['sif victim flags 0', 'sif victim size 4096',
                'sif victim block[IND] 0x00ffff00']
        cmds += [f'sif victim block[{i}] 0' for i in range(12)]
        debugfs(img, cmds)
        assert victim_has(img, '(IND):16776960')

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(f'ext4write host 0:0 {ADDR:x} /victim 0x1000')
        assert 'Invalid block number' in out
        assert 'written' not in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
@pytest.mark.parametrize('inum', [1000000, 8])
def test_fs_ext_oob_delete_inode(ubman, inum):
    """ext4fs_delete_file(): replacing a file whose directory entry names an
    inode past the last block group, or a reserved inode such as the
    journal's (8), must fail"""
    with ext4_image(ubman, f'ext_oob_ino{inum}') as img:
        add_file(img, 'victim', {0: VICTIM})
        # Point the /victim entry of the root directory at the inode; debugfs
        # refuses to link a missing inode, so patch the entry in place
        blksz = block_size(img)
        blk = file_block(img, '/', 0)
        data = bytearray(read_at(img, blk * blksz, blksz))
        off = 0
        while off < blksz:
            rec_len, name_len = struct.unpack_from('<HB', data, off + 4)
            if data[off + 8:off + 8 + name_len] == b'victim':
                struct.pack_into('<I', data, off, inum)
                break
            off += rec_len
        else:
            pytest.fail('no /victim entry in the root directory')
        write_at(img, blk * blksz, data)

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(f'ext4write host 0:0 {ADDR:x} /victim 0x1000')
        assert f'Invalid inode number {inum}' in out
        assert 'written' not in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.buildconfigspec('cmd_crc32')
@pytest.mark.requiredtool('debugfs')
@pytest.mark.requiredtool('e2fsck')
def test_fs_ext_oob_last_inode(ubman):
    """ext4fs_delete_file(), ext4fs_write(): replacing a file whose inode is
    the last of its block group must use that group's inode bitmap and
    inode table"""
    # With 16 inodes per group, fill the free inodes of group 0 but the last
    # one, inode 16, which /victim then takes
    with ext4_image(ubman, 'ext_oob_last', ['-N', '16']) as img:
        out = check_output(['debugfs', '-R', 'ffi', img],
                           stderr=DEVNULL).decode()
        for num in range(int(out.split()[-1]), 16):
            add_file(img, f'file{num}', {0: VICTIM})
        add_file(img, 'victim', {0: VICTIM})
        assert victim_has(img, 'Inode: 16 ')

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(f'ext4write host 0:0 {ADDR:x} /victim 0x1000')
        assert 'written' in out
        out = load_crc(ubman, 0x1000, 0)
        assert f'==> {zlib.crc32(b"A" * 0x1000):08x}' in out
        assert 'alive' in ubman.run_command('echo alive')
        run(['e2fsck', '-fn', img], capture_output=True, check=True)


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_first_data_block(ubman):
    """ext4fs_init(): a filesystem whose first data block is not the one the
    bitmap code assumes must not be written to"""
    # Two block groups of 32768 4 KiB blocks. With a first data block of 1,
    # block 32768, the first block of group 1, lies in group 0.
    with ext4_image(ubman, 'ext_oob_fdb', ['-b', '4096'], 136 << 20) as img:
        # s_blocks_count and s_blocks_per_group are at offsets 4 and 32 of
        # the superblock
        assert struct.unpack('<I', read_at(img, 1024 + 4, 4))[0] > 32768
        assert struct.unpack('<I', read_at(img, 1024 + 32, 4))[0] == 32768
        add_file(img, 'victim', {0: VICTIM})
        # Make /victim a block-mapped file whose only block is 32768
        cmds = ['sif victim flags 0', 'sif victim size 4096']
        cmds += [f'sif victim block[{i}] 0' for i in range(1, 12)]
        cmds += ['sif victim block[0] 32768']
        debugfs(img, cmds)
        assert victim_has(img, '(0):32768')

        # s_first_data_block is at offset 20 of the superblock. The read
        # path looks for the group descriptors in the block after it, so
        # copy them from block 1 to block 2, a reserved GDT block.
        write_at(img, 1024 + 20, struct.pack('<I', 1))
        write_at(img, 2 * 4096, read_at(img, 4096, 4096))

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(f'ext4write host 0:0 {ADDR:x} /victim 0x1000')
        assert 'Unsupported block group layout' in out
        assert 'written' not in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_parent_dentry(ubman):
    """ext4fs_update_parent_dentry(): a directory block whose last entry
    claims a length past the end of the block must be rejected rather than
    walked past"""
    with ext4_image(ubman, 'ext_oob_den') as img:
        blksz = block_size(img)
        blk = file_block(img, '/', 0)

        # Rewrite the root directory block: '.' and '..', then entries with
        # no free space after their names, then a last entry that ends 4
        # bytes past the block with too little free space for the new name
        data = bytearray(blksz)
        off = 0
        entries = [(2, 12, b'.'), (2, 12, b'..')]
        entries += [(0, 12, b'fill')] * ((blksz - 16 - 24) // 12)
        for inode, rec_len, name in entries:
            struct.pack_into('<IHBB', data, off, inode, rec_len, len(name), 2)
            data[off + 8:off + 8 + len(name)] = name
            off += rec_len
        rec_len = blksz - off + 4
        name = b'l' * (rec_len - 12)
        struct.pack_into('<IHBB', data, off, 0, rec_len, len(name), 2)
        data[off + 8:blksz] = name[:blksz - off - 8]
        write_at(img, blk * blksz, data)

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        out = ubman.run_command(
            f'ext4write host 0:0 {ADDR:x} /newfile 0x1000')
        assert 'Corrupt directory block, not writing' in out
        assert 'written' not in out
        assert 'alive' in ubman.run_command('echo alive')


@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_ext4_write')
@pytest.mark.requiredtool('debugfs')
def test_fs_ext_oob_journal_block_type(ubman):
    """ext4fs_check_journal_state(): a journal block of an unknown type must
    end the journal scan rather than stall it"""
    with ext4_image(ubman, 'ext_oob_jnl') as img:
        blksz = block_size(img)

        # The journal superblock holds (big endian) magic, type, sequence,
        # block size, maxlen, first, sequence and start
        jsb = file_block(img, '<8>', 0) * blksz
        magic, _, _, _, _, first, seq = struct.unpack('>7I',
                                                      read_at(img, jsb, 28))
        assert magic == 0xc03b3998

        # Mark the journal as needing recovery from its first log block,
        # which holds a block of an unknown type with the current sequence
        write_at(img, jsb + 28, struct.pack('>I', first))
        write_at(img, file_block(img, '<8>', first) * blksz,
                 struct.pack('>3I', magic, 7, seq))
        # s_feature_incompat is at offset 0x60 of the superblock; 4 is
        # EXT3_FEATURE_INCOMPAT_RECOVER
        incompat = struct.unpack('<I', read_at(img, 1024 + 0x60, 4))[0]
        write_at(img, 1024 + 0x60, struct.pack('<I', incompat | 4))

        ubman.run_command(f'host bind 0 {img}')
        ubman.run_command(f'mw.b {ADDR:x} 41 1000')
        with ubman.temporary_timeout(10000):
            out = ubman.run_command(
                f'ext4write host 0:0 {ADDR:x} /newfile 0x1000')
        assert 'Journal Recovery Completed' in out
        assert 'written' in out
        assert 'alive' in ubman.run_command('echo alive')

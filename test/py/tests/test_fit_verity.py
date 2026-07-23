# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2026 Daniel Golle <daniel@makrotopia.org>

"""
Test mkimage dm-verity Merkle-tree generation

Build a minimal .its with a dm-verity subnode (user-provided properties only),
run mkimage -E, and verify that the computed properties (digest, salt,
num-data-blocks, hash-start-block) are written into the resulting FIT.
The computed digest is then re-verified by running ``veritysetup verify``
against the external data section of the .itb.

This test does not run the sandbox. It only exercises the host tool 'mkimage'.
Requires 'veritysetup' from the cryptsetup package on the build host.
"""

import os
import struct
import pytest
import utils

ITS_TEMPLATE = """\
/dts-v1/;

/ {
    description = "dm-verity test";
    #address-cells = <1>;

    images {
        rootfs {
            description = "test filesystem";
            data = /incbin/("./rootfs.bin");
            type = "filesystem";
            arch = "sandbox";
            compression = "none";

            dm-verity {
                algo = "sha256";
                data-block-size = <%d>;
                hash-block-size = <%d>;
            };
        };
    };

    configurations {
        default = "conf-1";
        conf-1 {
            description = "test config";
            loadables = "rootfs";
        };
    };
};
"""

def _fdt_totalsize(path):
    """Read the totalsize field from an FDT header (offset 4, big-endian u32)."""
    with open(path, 'rb') as f:
        magic, totalsize = struct.unpack('>II', f.read(8))
    assert magic == 0xd00dfeed, f'not an FDT: magic={magic:#x}'
    return totalsize


def _run_round_trip(ubman, tempdir, data_block_size, hash_block_size):
    """Build a FIT with dm-verity, verify written properties, re-verify with veritysetup."""
    mkimage = ubman.config.build_dir + '/tools/mkimage'

    rootfs_file = os.path.join(tempdir, 'rootfs.bin')
    its_file = os.path.join(tempdir, 'image.its')
    fit_file = os.path.join(tempdir, 'image.itb')

    # 64 data blocks of 0xa5
    num_blocks = 64
    data_size = data_block_size * num_blocks
    with open(rootfs_file, 'wb') as f:
        f.write(bytes([0xa5]) * data_size)

    with open(its_file, 'w') as f:
        f.write(ITS_TEMPLATE % (data_block_size, hash_block_size))

    dtc_args = f'-I dts -O dtb -i {tempdir}'
    utils.run_and_log(ubman,
                      [mkimage, '-E', '-D', dtc_args, '-f', its_file, fit_file])

    def fdt_get(node, prop):
        val = utils.run_and_log(ubman, f'fdtget {fit_file} {node} {prop}')
        return val.strip()

    def fdt_get_hex(node, prop):
        val = utils.run_and_log(ubman, f'fdtget -tbx {fit_file} {node} {prop}')
        return ''.join(b.zfill(2) for b in val.strip().split())

    verity_path = '/images/rootfs/dm-verity'

    assert fdt_get(verity_path, 'algo') == 'sha256'
    assert int(fdt_get(verity_path, 'data-block-size')) == data_block_size
    assert int(fdt_get(verity_path, 'hash-block-size')) == hash_block_size

    nblk = int(fdt_get(verity_path, 'num-data-blocks'))
    assert nblk == num_blocks, f'num-data-blocks {nblk} != {num_blocks}'

    hblk = int(fdt_get(verity_path, 'hash-start-block'))
    # With --no-superblock, hash-start-block = data_size / hash-block-size
    assert hblk == data_size // hash_block_size, \
        f'hash-start-block {hblk} != {data_size // hash_block_size}'

    digest = fdt_get_hex(verity_path, 'digest')
    assert len(digest) == 64 and digest != '0' * 64
    salt = fdt_get_hex(verity_path, 'salt')
    assert len(salt) == 64

    # Re-verify the digest with veritysetup against the .itb's external data.
    # With -E, image data sits after the FIT FDT at (fdt_totalsize + data-offset).
    data_offset = int(fdt_get('/images/rootfs', 'data-offset'))
    data_size_full = int(fdt_get('/images/rootfs', 'data-size'))
    ext_pos = _fdt_totalsize(fit_file) + data_offset
    expanded = os.path.join(tempdir, 'expanded.bin')
    with open(fit_file, 'rb') as src, open(expanded, 'wb') as dst:
        src.seek(ext_pos)
        dst.write(src.read(data_size_full))

    utils.run_and_log(ubman, [
        'veritysetup', 'verify', expanded, expanded, digest,
        '--no-superblock',
        f'--data-block-size={data_block_size}',
        f'--hash-block-size={hash_block_size}',
        f'--data-blocks={nblk}',
        '--hash=sha256',
        f'--salt={salt}',
        f'--hash-offset={data_size}',
    ])


@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('fdtget')
@pytest.mark.requiredtool('veritysetup')
@pytest.mark.parametrize('data_block_size,hash_block_size,subdir', [
    (4096, 4096, 'verity-equal'),
    (4096, 1024, 'verity-unequal'),
])
def test_mkimage_verity(ubman, data_block_size, hash_block_size, subdir):
    """mkimage writes correct dm-verity properties and the digest verifies.

    Run with matching and mismatched block sizes so the
    ``hash-start-block != num-data-blocks`` path is exercised.
    """
    tempdir = os.path.join(ubman.config.result_dir, subdir)
    os.makedirs(tempdir, exist_ok=True)
    _run_round_trip(ubman, tempdir, data_block_size, hash_block_size)


@pytest.mark.requiredtool('dtc')
@pytest.mark.requiredtool('veritysetup')
def test_mkimage_verity_requires_external(ubman):
    """mkimage rejects dm-verity without -E with the expected diagnostic."""

    mkimage = ubman.config.build_dir + '/tools/mkimage'
    tempdir = os.path.join(ubman.config.result_dir, 'verity_no_ext')
    os.makedirs(tempdir, exist_ok=True)

    rootfs_file = os.path.join(tempdir, 'rootfs.bin')
    its_file = os.path.join(tempdir, 'image.its')
    fit_file = os.path.join(tempdir, 'image.itb')

    with open(rootfs_file, 'wb') as f:
        f.write(bytes([0xa5]) * 4096 * 8)

    with open(its_file, 'w') as f:
        f.write(ITS_TEMPLATE % (4096, 4096))

    dtc_args = f'-I dts -O dtb -i {tempdir}'
    utils.run_and_log_expect_exception(
        ubman,
        [mkimage, '-D', dtc_args, '-f', its_file, fit_file],
        1, 'dm-verity requires external data')

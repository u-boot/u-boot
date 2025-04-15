# SPDX-License-Identifier:      GPL-2.0+
#
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>

"""Helper functions for dealing with filesystems"""

import re
import os
from subprocess import call, check_call, check_output, CalledProcessError

def mk_fs(config, fs_type, size, prefix, src_dir=None, size_gran = 0x100000):
    """Create a file system volume

    Args:
        config (u_boot_config): U-Boot configuration
        fs_type (str): File system type, e.g. 'ext4'
        size (int): Size of file system in bytes
        prefix (str): Prefix string of volume's file name
        src_dir (str): Root directory to use, or None for none
        size_gran (int): Size granularity of file system image in bytes

    Raises:
        CalledProcessError: if any error occurs when creating the filesystem
    """
    fs_img = f'{prefix}.{fs_type}.img'
    fs_img = os.path.join(config.persistent_data_dir, fs_img)

    if fs_type == 'fat12':
        mkfs_opt = '-F 12'
    elif fs_type == 'fat16':
        mkfs_opt = '-F 16'
    elif fs_type == 'fat32':
        mkfs_opt = '-F 32'
    else:
        mkfs_opt = ''

    if fs_type == 'exfat':
        fs_lnxtype = 'exfat'
    elif re.match('fat', fs_type) or fs_type == 'fs_generic':
        fs_lnxtype = 'vfat'
    else:
        fs_lnxtype = fs_type

    if src_dir:
        if fs_lnxtype == 'ext4':
            mkfs_opt = mkfs_opt + ' -d ' + src_dir
        elif fs_lnxtype != 'vfat' and fs_lnxtype != 'exfat':
            raise ValueError(f'src_dir not implemented for fs {fs_lnxtype}')

    count = (size + size_gran - 1) // size_gran

    # Some distributions do not add /sbin to the default PATH, where mkfs lives
    if '/sbin' not in os.environ["PATH"].split(os.pathsep):
        os.environ["PATH"] += os.pathsep + '/sbin'

    try:
        check_call(f'rm -f {fs_img}', shell=True)
        check_call(f'truncate -s $(( {size_gran} * {count} )) {fs_img}',
                   shell=True)
        check_call(f'mkfs.{fs_lnxtype} {mkfs_opt} {fs_img}', shell=True)
        if fs_type == 'ext4':
            sb_content = check_output(f'tune2fs -l {fs_img}',
                                      shell=True).decode()
            if 'metadata_csum' in sb_content:
                check_call(f'tune2fs -O ^metadata_csum {fs_img}', shell=True)
        elif fs_lnxtype == 'vfat' and src_dir:
            check_call(f'mcopy -i {fs_img} -vsmpQ {src_dir}/* ::/', shell=True)
        elif fs_lnxtype == 'exfat' and src_dir:
            check_call(f'fattools cp {src_dir}/* {fs_img}', shell=True)
        return fs_img
    except CalledProcessError:
        call(f'rm -f {fs_img}', shell=True)
        raise

def setup_image(ubman, devnum, part_type, img_size=20, second_part=False,
                basename='mmc'):
    """Create a disk image with a single partition

    Args:
        ubman (ConsoleBase): Console to use
        devnum (int): Device number to use, e.g. 1
        part_type (int): Partition type, e.g. 0xc for FAT32
        img_size (int): Image size in MiB
        second_part (bool): True to contain a small second partition
        basename (str): Base name to use in the filename, e.g. 'mmc'

    Returns:
        tuple:
            str: Filename of MMC image
            str: Directory name of scratch directory
    """
    fname = os.path.join(ubman.config.source_dir, f'{basename}{devnum}.img')
    mnt = os.path.join(ubman.config.persistent_data_dir, 'scratch')

    spec = f'type={part_type:x}, size={img_size - 2}M, start=1M, bootable'
    if second_part:
        spec += '\ntype=c'

    try:
        check_call(f'mkdir -p {mnt}', shell=True)
        check_call(f'qemu-img create {fname} {img_size}M', shell=True)
        check_call(f'printf "{spec}" | sfdisk {fname}', shell=True)
    except CalledProcessError:
        call(f'rm -f {fname}', shell=True)
        raise

    return fname, mnt

# Just for trying out
if __name__ == "__main__":
    import collections

    CNF= collections.namedtuple('config', 'persistent_data_dir')

    mk_fs(CNF('.'), 'ext4', 0x1000000, 'pref')

# SPDX-License-Identifier:      GPL-2.0+
#
# Helper functions for dealing with filesystems
#
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>

import re
import os
from subprocess import call, check_call, check_output, CalledProcessError

def mk_fs(config, fs_type, size, prefix):
    """Create a file system volume

    Args:
        fs_type (str): File system type, e.g. 'ext4'
        size (int): Size of file system in bytes
        prefix (str): Prefix string of volume's file name
    """
    fs_img = '%s.%s.img' % (prefix, fs_type)
    fs_img = config.persistent_data_dir + '/' + fs_img

    if fs_type == 'fat16':
        mkfs_opt = '-F 16'
    elif fs_type == 'fat32':
        mkfs_opt = '-F 32'
    else:
        mkfs_opt = ''

    if re.match('fat', fs_type):
        fs_lnxtype = 'vfat'
    else:
        fs_lnxtype = fs_type

    count = (size + 1048576 - 1) / 1048576

    # Some distributions do not add /sbin to the default PATH, where mkfs lives
    if '/sbin' not in os.environ["PATH"].split(os.pathsep):
        os.environ["PATH"] += os.pathsep + '/sbin'

    try:
        check_call('rm -f %s' % fs_img, shell=True)
        check_call('dd if=/dev/zero of=%s bs=1M count=%d'
            % (fs_img, count), shell=True)
        check_call('mkfs.%s %s %s'
            % (fs_lnxtype, mkfs_opt, fs_img), shell=True)
        if fs_type == 'ext4':
            sb_content = check_output('tune2fs -l %s' % fs_img, shell=True).decode()
            if 'metadata_csum' in sb_content:
                check_call('tune2fs -O ^metadata_csum %s' % fs_img, shell=True)
        return fs_img
    except CalledProcessError:
        call('rm -f %s' % fs_img, shell=True)
        raise

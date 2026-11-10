# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2019, Texas Instrument
# Author: JJ Hiblot <jjhiblot@ti.com>
#

import os
from subprocess import check_call, CalledProcessError

def assert_fs_integrity(fs_type, fs_img):
    # Some distributions do not add /sbin to the default PATH, where fsck lives
    local_env = os.environ.copy()
    if '/sbin' not in local_env["PATH"].split(os.pathsep):
        local_env["PATH"] += os.pathsep + '/sbin'

    try:
        if fs_type == 'ext4':
            check_call('fsck.ext4 -n -f %s' % fs_img, shell=True, env=local_env)
        elif fs_type == 'exfat':
            check_call('fsck.exfat -n %s' % fs_img, shell=True, env=local_env)
        elif fs_type in ['fat12', 'fat16', 'fat32']:
            check_call('fsck.fat -n %s' % fs_img, shell=True, env=local_env)
    except CalledProcessError:
        raise

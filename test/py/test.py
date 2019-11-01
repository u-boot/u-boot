#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0

# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.

# Wrapper script to invoke pytest with the directory name that contains the
# U-Boot tests.

import os
import os.path
import sys
from pkg_resources import load_entry_point

# argv; py.test test_directory_name user-supplied-arguments
args = [os.path.dirname(__file__) + '/tests']
args.extend(sys.argv)

if __name__ == '__main__':
    sys.exit(load_entry_point('pytest', 'console_scripts', 'pytest')(args))

#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import struct

def fdt32_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        Value to convert (4-character string representing the cell value)

    Return:
        A native-endian integer value
    """
    return struct.unpack(">I", val)[0]

# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot binary
#

from binman.entry import Entry
from binman.etype.blob import Entry_blob

class Entry_u_boot(Entry_blob):
    """U-Boot flat binary

    Properties / Entry arguments:
        - filename: Filename of u-boot.bin (default 'u-boot.bin')

    This is the U-Boot binary, containing relocation information to allow it
    to relocate itself at runtime. The binary typically includes a device tree
    blob at the end of it. Use u-boot-nodtb if you want to package the device
    tree separately.

    U-Boot can access binman symbols at runtime. See:

        'Access to binman entry offsets at run time (fdt)'

    in the binman README for more information.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def GetDefaultFilename(self):
        return 'u-boot.bin'

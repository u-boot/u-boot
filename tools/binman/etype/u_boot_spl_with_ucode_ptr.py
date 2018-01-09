# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for an SPL binary with an embedded microcode pointer
#

import struct

import command
from entry import Entry
from blob import Entry_blob
from u_boot_with_ucode_ptr import Entry_u_boot_with_ucode_ptr
import tools

class Entry_u_boot_spl_with_ucode_ptr(Entry_u_boot_with_ucode_ptr):
    """U-Boot SPL with embedded microcode pointer

    See Entry_u_boot_ucode for full details of the entries involved in this
    process.
    """
    def __init__(self, image, etype, node):
        Entry_u_boot_with_ucode_ptr.__init__(self, image, etype, node)
        self.elf_fname = 'spl/u-boot-spl'

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl-nodtb.bin'

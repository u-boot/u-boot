# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for BSS padding for spl/u-boot-spl.bin. This padding
# can be added after the SPL binary to ensure that anything concatenated
# to it will appear to SPL to be at the end of BSS rather than the start.
#

import command
import elf
from entry import Entry
from blob import Entry_blob
import tools

class Entry_u_boot_spl_bss_pad(Entry_blob):
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)

    def ObtainContents(self):
        fname = tools.GetInputFilename('spl/u-boot-spl')
        bss_size = elf.GetSymbolAddress(fname, '__bss_size')
        if not bss_size:
            self.Raise('Expected __bss_size symbol in spl/u-boot-spl')
        self.data = chr(0) * bss_size
        self.contents_size = bss_size

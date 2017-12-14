# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for a U-Boot binary with an embedded microcode pointer
#

import struct

import command
import elf
from entry import Entry
from blob import Entry_blob
import fdt_util
import tools

class Entry_u_boot_with_ucode_ptr(Entry_blob):
    """U-Boot with embedded microcode pointer

    See Entry_u_boot_ucode for full details of the 3 entries involved in this
    process.
    """
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)
        self.elf_fname = 'u-boot'
        self.target_pos = None

    def GetDefaultFilename(self):
        return 'u-boot-nodtb.bin'

    def ObtainContents(self):
        # Figure out where to put the microcode pointer
        fname = tools.GetInputFilename(self.elf_fname)
        sym = elf.GetSymbolAddress(fname, '_dt_ucode_base_size')
        if sym:
           self.target_pos = sym
        elif not fdt_util.GetBool(self._node, 'optional-ucode'):
            self.Raise('Cannot locate _dt_ucode_base_size symbol in u-boot')

        return Entry_blob.ObtainContents(self)

    def ProcessContents(self):
        # If the image does not need microcode, there is nothing to do
        if not self.target_pos:
            return

        # Get the position of the microcode
        ucode_entry = self.image.FindEntryType('u-boot-ucode')
        if not ucode_entry:
            self.Raise('Cannot find microcode region u-boot-ucode')

        # Check the target pos is in the image. If it is not, then U-Boot is
        # being linked incorrectly, or is being placed at the wrong position
        # in the image.
        #
        # The image must be set up so that U-Boot is placed at the
        # flash address to which it is linked. For example, if
        # CONFIG_SYS_TEXT_BASE is 0xfff00000, and the ROM is 8MB, then
        # the U-Boot region must start at position 7MB in the image. In this
        # case the ROM starts at 0xff800000, so the position of the first
        # entry in the image corresponds to that.
        if (self.target_pos < self.pos or
                self.target_pos >= self.pos + self.size):
            self.Raise('Microcode pointer _dt_ucode_base_size at %08x is '
                'outside the image ranging from %08x to %08x' %
                (self.target_pos, self.pos, self.pos + self.size))

        # Get the microcode, either from u-boot-ucode or u-boot-dtb-with-ucode.
        # If we have left the microcode in the device tree, then it will be
        # in the former. If we extracted the microcode from the device tree
        # and collated it in one place, it will be in the latter.
        if ucode_entry.size:
            pos, size = ucode_entry.pos, ucode_entry.size
        else:
            dtb_entry = self.image.FindEntryType('u-boot-dtb-with-ucode')
            if not dtb_entry:
                self.Raise('Cannot find microcode region u-boot-dtb-with-ucode')
            pos = dtb_entry.pos + dtb_entry.ucode_offset
            size = dtb_entry.ucode_size

        # Write the microcode position and size into the entry
        pos_and_size = struct.pack('<2L', pos, size)
        self.target_pos -= self.pos
        self.data = (self.data[:self.target_pos] + pos_and_size +
                     self.data[self.target_pos + 8:])

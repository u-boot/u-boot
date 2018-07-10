# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree with the microcode removed
#

import control
import fdt
from entry import Entry
from blob import Entry_blob
import tools

class Entry_u_boot_dtb_with_ucode(Entry_blob):
    """A U-Boot device tree file, with the microcode removed

    See Entry_u_boot_ucode for full details of the 3 entries involved in this
    process.
    """
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)
        self.ucode_data = ''
        self.collate = False
        self.ucode_offset = None
        self.ucode_size = None
        self.ucode = None
        self.ready = False

    def GetDefaultFilename(self):
        return 'u-boot.dtb'

    def ProcessFdt(self, fdt):
        # If the section does not need microcode, there is nothing to do
        ucode_dest_entry = self.section.FindEntryType(
            'u-boot-spl-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_pos:
            ucode_dest_entry = self.section.FindEntryType(
                'u-boot-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_pos:
            return True

        # Remove the microcode
        fname = self.GetDefaultFilename()
        fdt = control.GetFdt(fname)
        self.ucode = fdt.GetNode('/microcode')
        if not self.ucode:
            raise self.Raise("No /microcode node found in '%s'" % fname)

        # There's no need to collate it (move all microcode into one place)
        # if we only have one chunk of microcode.
        self.collate = len(self.ucode.subnodes) > 1
        for node in self.ucode.subnodes:
            data_prop = node.props.get('data')
            if data_prop:
                self.ucode_data += ''.join(data_prop.bytes)
                if self.collate:
                    node.DeleteProp('data')
        return True

    def ObtainContents(self):
        # Call the base class just in case it does something important.
        Entry_blob.ObtainContents(self)
        self._pathname = control.GetFdtPath(self._filename)
        self.ReadContents()
        if self.ucode:
            for node in self.ucode.subnodes:
                data_prop = node.props.get('data')
                if data_prop and not self.collate:
                    # Find the offset in the device tree of the ucode data
                    self.ucode_offset = data_prop.GetOffset() + 12
                    self.ucode_size = len(data_prop.bytes)
        self.ready = True
        return True

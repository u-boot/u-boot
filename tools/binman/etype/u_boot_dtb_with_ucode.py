# Copyright (c) 2016 Google, Inc
## Written by Simon Glass <sjg@chromium.org>

# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for U-Boot device tree with the microcode removed
#

import fdt
from entry import Entry
from blob import Entry_blob
import tools

class Entry_u_boot_dtb_with_ucode(Entry_blob):
    """A U-Boot device tree file, with the microcode removed

    See Entry_u_boot_ucode for full details of the 3 entries involved in this
    process.
    """
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)
        self.ucode_data = ''
        self.collate = False
        self.ucode_offset = None
        self.ucode_size = None

    def GetDefaultFilename(self):
        return 'u-boot.dtb'

    def ObtainContents(self):
        Entry_blob.ObtainContents(self)

        # If the image does not need microcode, there is nothing to do
        ucode_dest_entry = self.image.FindEntryType('u-boot-spl-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_pos:
            ucode_dest_entry = self.image.FindEntryType('u-boot-with-ucode-ptr')
        if not ucode_dest_entry or not ucode_dest_entry.target_pos:
            return True

        # Create a new file to hold the copied device tree
        dtb_name = 'u-boot-dtb-with-ucode.dtb'
        fname = tools.GetOutputFilename(dtb_name)
        with open(fname, 'wb') as fd:
            fd.write(self.data)

        # Remove the microcode
        dtb = fdt.FdtScan(fname)
        ucode = dtb.GetNode('/microcode')
        if not ucode:
            raise self.Raise("No /microcode node found in '%s'" % fname)

        # There's no need to collate it (move all microcode into one place)
        # if we only have one chunk of microcode.
        self.collate = len(ucode.subnodes) > 1
        for node in ucode.subnodes:
            data_prop = node.props.get('data')
            if data_prop:
                self.ucode_data += ''.join(data_prop.bytes)
                if self.collate:
                    prop = node.DeleteProp('data')
                else:
                    # Find the offset in the device tree of the ucode data
                    self.ucode_offset = data_prop.GetOffset() + 12
                    self.ucode_size = len(data_prop.bytes)
        if self.collate:
            dtb.Pack()
            dtb.Flush()

            # Make this file the contents of this entry
            self._pathname = fname
            self.ReadContents()
        return True

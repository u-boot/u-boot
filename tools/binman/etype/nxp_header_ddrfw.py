# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 NXP

from binman.etype.section import Entry_section

class Entry_nxp_header_ddrfw(Entry_section):
    """Add a header to DDR PHY firmware images

    This entry is used for i.MX95 to combine DDR PHY firmware images and their
    byte counts together.

    See imx95_evk.rst for how to get DDR PHY Firmware Images.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)

    def BuildSectionData(self, required):
        section_data = bytearray()
        header_data = bytearray()

        for entry in self._entries.values():
            entry_data = entry.GetData(required)

            section_data += entry_data
            header_data += entry.contents_size.to_bytes(4, 'little')

        return header_data + section_data

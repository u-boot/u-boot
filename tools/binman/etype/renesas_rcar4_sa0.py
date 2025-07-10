# SPDX-License-Identifier: GPL-2.0+
# Copyright 2025 Marek Vasut <marek.vasut+renesas@mailbox.org>
#
# Entry-type module for generating the Renesas R-Car Gen4 SA0 header.
#

import os
import struct

from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

RCAR_GEN4_SF_HEADER_SIZE    = 0x40000
RCAR_GEN4_SF_MAX_LOAD_SIZE  = 0xec000

class Entry_renesas_rcar4_sa0(Entry_section):
    """Renesas R-Car Gen4 SA0 generator"""

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['renesas,loader-address']

    def ReadNode(self):
        self.loader_address = fdt_util.GetInt(self._node, 'renesas,loader-address')
        super().ReadNode()

    def BuildSectionData(self, required):
        data = super().BuildSectionData(required)[RCAR_GEN4_SF_HEADER_SIZE:]

        # Offset 0x0000 / Value 0x00000000
        header = struct.pack('<I', 0)
        header += tools.get_bytes(0xff, 0x3008)
        # Offset 0x300c / Value 0x00000000
        header += struct.pack('<I', 0)
        header += tools.get_bytes(0xff, 0x144)
        # Offset 0x3154 / Value (payload load address)
        header += struct.pack('<I', self.loader_address)
        header += tools.get_bytes(0xff, 0x10c)
        # Offset 0x3264 / Value (payload size in 4-byte words, aligned to 4k)
        header += struct.pack('<I', int(tools.align(len(data), 0x1000) / 4))
        header += tools.get_bytes(0xff, 0x3cd98)
        if len(data) > RCAR_GEN4_SF_MAX_LOAD_SIZE:
            self.Raise(f'SRAM data longer than {RCAR_GEN4_SF_MAX_LOAD_SIZE} Bytes')

        return header + data

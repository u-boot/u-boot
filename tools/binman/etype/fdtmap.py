# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>

"""# Entry-type module for a full map of the firmware image

This handles putting an FDT into the image with just the information about the
image.
"""

import libfdt

from entry import Entry
from fdt import Fdt
import state
import tools

FDTMAP_MAGIC = b'_FDTMAP_'

class Entry_fdtmap(Entry):
    """An entry which contains an FDT map

    Properties / Entry arguments:
        None

    An FDT map is just a header followed by an FDT containing a list of all the
    entries in the image.

    The header is the string _FDTMAP_ followed by 8 unused bytes.

    When used, this entry will be populated with an FDT map which reflects the
    entries in the current image. Hierarchy is preserved, and all offsets and
    sizes are included.

    Note that the -u option must be provided to ensure that binman updates the
    FDT with the position of each entry.

    Example output for a simple image with U-Boot and an FDT map:

    / {
        size = <0x00000112>;
        image-pos = <0x00000000>;
        offset = <0x00000000>;
        u-boot {
            size = <0x00000004>;
            image-pos = <0x00000000>;
            offset = <0x00000000>;
        };
        fdtmap {
            size = <0x0000010e>;
            image-pos = <0x00000004>;
            offset = <0x00000004>;
        };
    };
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)

    def _GetFdtmap(self):
        """Build an FDT map from the entries in the current image

        Returns:
            FDT map binary data
        """
        def _AddNode(node):
            """Add a node to the FDT map"""
            for pname, prop in node.props.items():
                fsw.property(pname, prop.bytes)
            for subnode in node.subnodes:
                with fsw.add_node(subnode.name):
                    _AddNode(subnode)

        # Get the FDT data into an Fdt object
        data = state.GetFdtContents()[1]
        infdt = Fdt.FromData(data)
        infdt.Scan()

        # Find the node for the image containing the Fdt-map entry
        path = self.section.GetPath()
        node = infdt.GetNode(path)
        if not node:
            self.Raise("Internal error: Cannot locate node for path '%s'" %
                       path)

        # Build a new tree with all nodes and properties starting from that node
        fsw = libfdt.FdtSw()
        fsw.finish_reservemap()
        with fsw.add_node(''):
            _AddNode(node)
        fdt = fsw.as_fdt()

        # Pack this new FDT and return its contents
        fdt.pack()
        outfdt = Fdt.FromData(fdt.as_bytearray())
        data = FDTMAP_MAGIC + tools.GetBytes(0, 8) + outfdt.GetContents()
        return data

    def ObtainContents(self):
        """Obtain a placeholder for the fdt-map contents"""
        self.SetContents(self._GetFdtmap())
        return True

    def ProcessContents(self):
        """Write an updated version of the FDT map to this entry

        This is necessary since new data may have been written back to it during
        processing, e.g. the image-pos properties.
        """
        self.ProcessContentsUpdate(self._GetFdtmap())

# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Class for an image, the output of binman
#

from __future__ import print_function

from collections import OrderedDict
from operator import attrgetter
import re
import sys

from entry import Entry
from etype import fdtmap
from etype import image_header
from etype import section
import fdt
import fdt_util
import tools

class Image(section.Entry_section):
    """A Image, representing an output from binman

    An image is comprised of a collection of entries each containing binary
    data. The image size must be large enough to hold all of this data.

    This class implements the various operations needed for images.

    Attributes:
        filename: Output filename for image

    Args:
        test: True if this is being called from a test of Images. This this case
            there is no device tree defining the structure of the section, so
            we create a section manually.
    """
    def __init__(self, name, node, test=False):
        self.image = self
        section.Entry_section.__init__(self, None, 'section', node, test)
        self.name = 'main-section'
        self.image_name = name
        self._filename = '%s.bin' % self.image_name
        if not test:
            filename = fdt_util.GetString(self._node, 'filename')
            if filename:
                self._filename = filename

    @classmethod
    def FromFile(cls, fname):
        """Convert an image file into an Image for use in binman

        Args:
            fname: Filename of image file to read

        Returns:
            Image object on success

        Raises:
            ValueError if something goes wrong
        """
        data = tools.ReadFile(fname)
        size = len(data)

        # First look for an image header
        pos = image_header.LocateHeaderOffset(data)
        if pos is None:
            # Look for the FDT map
            pos = fdtmap.LocateFdtmap(data)
        if pos is None:
            raise ValueError('Cannot find FDT map in image')

        # We don't know the FDT size, so check its header first
        probe_dtb = fdt.Fdt.FromData(
            data[pos + fdtmap.FDTMAP_HDR_LEN:pos + 256])
        dtb_size = probe_dtb.GetFdtObj().totalsize()
        fdtmap_data = data[pos:pos + dtb_size + fdtmap.FDTMAP_HDR_LEN]
        dtb = fdt.Fdt.FromData(fdtmap_data[fdtmap.FDTMAP_HDR_LEN:])
        dtb.Scan()

        # Return an Image with the associated nodes
        return Image('image', dtb.GetRoot())

    def Raise(self, msg):
        """Convenience function to raise an error referencing an image"""
        raise ValueError("Image '%s': %s" % (self._node.path, msg))

    def PackEntries(self):
        """Pack all entries into the image"""
        section.Entry_section.Pack(self, 0)

    def SetImagePos(self):
        # This first section in the image so it starts at 0
        section.Entry_section.SetImagePos(self, 0)

    def ProcessEntryContents(self):
        """Call the ProcessContents() method for each entry

        This is intended to adjust the contents as needed by the entry type.

        Returns:
            True if the new data size is OK, False if expansion is needed
        """
        sizes_ok = True
        for entry in self._entries.values():
            if not entry.ProcessContents():
                sizes_ok = False
                print("Entry '%s' size change" % self._node.path)
        return sizes_ok

    def WriteSymbols(self):
        """Write symbol values into binary files for access at run time"""
        section.Entry_section.WriteSymbols(self, self)

    def BuildSection(self, fd, base_offset):
        """Write the section to a file"""
        fd.seek(base_offset)
        fd.write(self.GetData())

    def BuildImage(self):
        """Write the image to a file"""
        fname = tools.GetOutputFilename(self._filename)
        with open(fname, 'wb') as fd:
            self.BuildSection(fd, 0)

    def WriteMap(self):
        """Write a map of the image to a .map file

        Returns:
            Filename of map file written
        """
        filename = '%s.map' % self.image_name
        fname = tools.GetOutputFilename(filename)
        with open(fname, 'w') as fd:
            print('%8s  %8s  %8s  %s' % ('ImagePos', 'Offset', 'Size', 'Name'),
                  file=fd)
            section.Entry_section.WriteMap(self, fd, 0)
        return fname

    def BuildEntryList(self):
        """List the files in an image

        Returns:
            List of entry.EntryInfo objects describing all entries in the image
        """
        entries = []
        self.ListEntries(entries, 0)
        return entries

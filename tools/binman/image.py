# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Class for an image, the output of binman
#

from __future__ import print_function

from collections import OrderedDict
import fnmatch
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
import tout

class Image(section.Entry_section):
    """A Image, representing an output from binman

    An image is comprised of a collection of entries each containing binary
    data. The image size must be large enough to hold all of this data.

    This class implements the various operations needed for images.

    Attributes:
        filename: Output filename for image
        image_node: Name of node containing the description for this image
        fdtmap_dtb: Fdt object for the fdtmap when loading from a file
        fdtmap_data: Contents of the fdtmap when loading from a file

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
        self.fdtmap_dtb = None
        self.fdtmap_data = None
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
        root = dtb.GetRoot()
        image = Image('image', root)
        image.image_node = fdt_util.GetString(root, 'image-node', 'image')
        image.fdtmap_dtb = dtb
        image.fdtmap_data = fdtmap_data
        image._data = data
        return image

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
                tout.Debug("Entry '%s' size change" % self._node.path)
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

    def FindEntryPath(self, entry_path):
        """Find an entry at a given path in the image

        Args:
            entry_path: Path to entry (e.g. /ro-section/u-boot')

        Returns:
            Entry object corresponding to that past

        Raises:
            ValueError if no entry found
        """
        parts = entry_path.split('/')
        entries = self.GetEntries()
        parent = '/'
        for part in parts:
            entry = entries.get(part)
            if not entry:
                raise ValueError("Entry '%s' not found in '%s'" %
                                 (part, parent))
            parent = entry.GetPath()
            entries = entry.GetEntries()
        return entry

    def ReadData(self, decomp=True):
        return self._data

    def GetListEntries(self, entry_paths):
        """List the entries in an image

        This decodes the supplied image and returns a list of entries from that
        image, preceded by a header.

        Args:
            entry_paths: List of paths to match (each can have wildcards). Only
                entries whose names match one of these paths will be printed

        Returns:
            String error message if something went wrong, otherwise
            3-Tuple:
                List of EntryInfo objects
                List of lines, each
                    List of text columns, each a string
                List of widths of each column
        """
        def _EntryToStrings(entry):
            """Convert an entry to a list of strings, one for each column

            Args:
                entry: EntryInfo object containing information to output

            Returns:
                List of strings, one for each field in entry
            """
            def _AppendHex(val):
                """Append a hex value, or an empty string if val is None

                Args:
                    val: Integer value, or None if none
                """
                args.append('' if val is None else '>%x' % val)

            args = ['  ' * entry.indent + entry.name]
            _AppendHex(entry.image_pos)
            _AppendHex(entry.size)
            args.append(entry.etype)
            _AppendHex(entry.offset)
            _AppendHex(entry.uncomp_size)
            return args

        def _DoLine(lines, line):
            """Add a line to the output list

            This adds a line (a list of columns) to the output list. It also updates
            the widths[] array with the maximum width of each column

            Args:
                lines: List of lines to add to
                line: List of strings, one for each column
            """
            for i, item in enumerate(line):
                widths[i] = max(widths[i], len(item))
            lines.append(line)

        def _NameInPaths(fname, entry_paths):
            """Check if a filename is in a list of wildcarded paths

            Args:
                fname: Filename to check
                entry_paths: List of wildcarded paths (e.g. ['*dtb*', 'u-boot*',
                                                             'section/u-boot'])

            Returns:
                True if any wildcard matches the filename (using Unix filename
                    pattern matching, not regular expressions)
                False if not
            """
            for path in entry_paths:
                if fnmatch.fnmatch(fname, path):
                    return True
            return False

        entries = self.BuildEntryList()

        # This is our list of lines. Each item in the list is a list of strings, one
        # for each column
        lines = []
        HEADER = ['Name', 'Image-pos', 'Size', 'Entry-type', 'Offset',
                  'Uncomp-size']
        num_columns = len(HEADER)

        # This records the width of each column, calculated as the maximum width of
        # all the strings in that column
        widths = [0] * num_columns
        _DoLine(lines, HEADER)

        # We won't print anything unless it has at least this indent. So at the
        # start we will print nothing, unless a path matches (or there are no
        # entry paths)
        MAX_INDENT = 100
        min_indent = MAX_INDENT
        path_stack = []
        path = ''
        indent = 0
        selected_entries = []
        for entry in entries:
            if entry.indent > indent:
                path_stack.append(path)
            elif entry.indent < indent:
                path_stack.pop()
            if path_stack:
                path = path_stack[-1] + '/' + entry.name
            indent = entry.indent

            # If there are entry paths to match and we are not looking at a
            # sub-entry of a previously matched entry, we need to check the path
            if entry_paths and indent <= min_indent:
                if _NameInPaths(path[1:], entry_paths):
                    # Print this entry and all sub-entries (=higher indent)
                    min_indent = indent
                else:
                    # Don't print this entry, nor any following entries until we get
                    # a path match
                    min_indent = MAX_INDENT
                    continue
            _DoLine(lines, _EntryToStrings(entry))
            selected_entries.append(entry)
        return selected_entries, lines, widths

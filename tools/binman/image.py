# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Class for an image, the output of binman
#

from collections import OrderedDict
from operator import attrgetter

import entry
from entry import Entry
import fdt_util
import tools

class Image:
    """A Image, representing an output from binman

    An image is comprised of a collection of entries each containing binary
    data. The image size must be large enough to hold all of this data.

    This class implements the various operations needed for images.

    Atrtributes:
        _node: Node object that contains the image definition in device tree
        _name: Image name
        _size: Image size in bytes, or None if not known yet
        _align_size: Image size alignment, or None
        _pad_before: Number of bytes before the first entry starts. This
            effectively changes the place where entry position 0 starts
        _pad_after: Number of bytes after the last entry ends. The last
            entry will finish on or before this boundary
        _pad_byte: Byte to use to pad the image where there is no entry
        _filename: Output filename for image
        _sort: True if entries should be sorted by position, False if they
            must be in-order in the device tree description
        _skip_at_start: Number of bytes before the first entry starts. These
            effecively adjust the starting position of entries. For example,
            if _pad_before is 16, then the first entry would start at 16.
            An entry with pos = 20 would in fact be written at position 4
            in the image file.
        _end_4gb: Indicates that the image ends at the 4GB boundary. This is
            used for x86 images, which want to use positions such that a
             memory address (like 0xff800000) is the first entry position.
             This causes _skip_at_start to be set to the starting memory
             address.
        _entries: OrderedDict() of entries
    """
    def __init__(self, name, node):
        self._node = node
        self._name = name
        self._size = None
        self._align_size = None
        self._pad_before = 0
        self._pad_after = 0
        self._pad_byte = 0
        self._filename = '%s.bin' % self._name
        self._sort = False
        self._skip_at_start = 0
        self._end_4gb = False
        self._entries = OrderedDict()

        self._ReadNode()
        self._ReadEntries()

    def _ReadNode(self):
        """Read properties from the image node"""
        self._size = fdt_util.GetInt(self._node, 'size')
        self._align_size = fdt_util.GetInt(self._node, 'align-size')
        if tools.NotPowerOfTwo(self._align_size):
            self._Raise("Alignment size %s must be a power of two" %
                        self._align_size)
        self._pad_before = fdt_util.GetInt(self._node, 'pad-before', 0)
        self._pad_after = fdt_util.GetInt(self._node, 'pad-after', 0)
        self._pad_byte = fdt_util.GetInt(self._node, 'pad-byte', 0)
        filename = fdt_util.GetString(self._node, 'filename')
        if filename:
            self._filename = filename
        self._sort = fdt_util.GetBool(self._node, 'sort-by-pos')
        self._end_4gb = fdt_util.GetBool(self._node, 'end-at-4gb')
        if self._end_4gb and not self._size:
            self._Raise("Image size must be provided when using end-at-4gb")
        if self._end_4gb:
            self._skip_at_start = 0x100000000 - self._size

    def CheckSize(self):
        """Check that the image contents does not exceed its size, etc."""
        contents_size = 0
        for entry in self._entries.values():
            contents_size = max(contents_size, entry.pos + entry.size)

        contents_size -= self._skip_at_start

        size = self._size
        if not size:
            size = self._pad_before + contents_size + self._pad_after
            size = tools.Align(size, self._align_size)

        if self._size and contents_size > self._size:
            self._Raise("contents size %#x (%d) exceeds image size %#x (%d)" %
                       (contents_size, contents_size, self._size, self._size))
        if not self._size:
            self._size = size
        if self._size != tools.Align(self._size, self._align_size):
            self._Raise("Size %#x (%d) does not match align-size %#x (%d)" %
                  (self._size, self._size, self._align_size, self._align_size))

    def _Raise(self, msg):
        """Raises an error for this image

        Args:
            msg: Error message to use in the raise string
        Raises:
            ValueError()
        """
        raise ValueError("Image '%s': %s" % (self._node.path, msg))

    def _ReadEntries(self):
        for node in self._node.subnodes:
            self._entries[node.name] = Entry.Create(self, node)

    def FindEntryType(self, etype):
        """Find an entry type in the image

        Args:
            etype: Entry type to find
        Returns:
            entry matching that type, or None if not found
        """
        for entry in self._entries.values():
            if entry.etype == etype:
                return entry
        return None

    def GetEntryContents(self):
        """Call ObtainContents() for each entry

        This calls each entry's ObtainContents() a few times until they all
        return True. We stop calling an entry's function once it returns
        True. This allows the contents of one entry to depend on another.

        After 3 rounds we give up since it's likely an error.
        """
        todo = self._entries.values()
        for passnum in range(3):
            next_todo = []
            for entry in todo:
                if not entry.ObtainContents():
                    next_todo.append(entry)
            todo = next_todo
            if not todo:
                break

    def _SetEntryPosSize(self, name, pos, size):
        """Set the position and size of an entry

        Args:
            name: Entry name to update
            pos: New position
            size: New size
        """
        entry = self._entries.get(name)
        if not entry:
            self._Raise("Unable to set pos/size for unknown entry '%s'" % name)
        entry.SetPositionSize(self._skip_at_start + pos, size)

    def GetEntryPositions(self):
        """Handle entries that want to set the position/size of other entries

        This calls each entry's GetPositions() method. If it returns a list
        of entries to update, it updates them.
        """
        for entry in self._entries.values():
            pos_dict = entry.GetPositions()
            for name, info in pos_dict.iteritems():
                self._SetEntryPosSize(name, *info)

    def PackEntries(self):
        """Pack all entries into the image"""
        pos = self._skip_at_start
        for entry in self._entries.values():
            pos = entry.Pack(pos)

    def _SortEntries(self):
        """Sort entries by position"""
        entries = sorted(self._entries.values(), key=lambda entry: entry.pos)
        self._entries.clear()
        for entry in entries:
            self._entries[entry._node.name] = entry

    def CheckEntries(self):
        """Check that entries do not overlap or extend outside the image"""
        if self._sort:
            self._SortEntries()
        pos = 0
        prev_name = 'None'
        for entry in self._entries.values():
            if (entry.pos < self._skip_at_start or
                entry.pos >= self._skip_at_start + self._size):
                entry.Raise("Position %#x (%d) is outside the image starting "
                            "at %#x (%d)" %
                            (entry.pos, entry.pos, self._skip_at_start,
                             self._skip_at_start))
            if entry.pos < pos:
                entry.Raise("Position %#x (%d) overlaps with previous entry '%s' "
                            "ending at %#x (%d)" %
                            (entry.pos, entry.pos, prev_name, pos, pos))
            pos = entry.pos + entry.size
            prev_name = entry.GetPath()

    def ProcessEntryContents(self):
        """Call the ProcessContents() method for each entry

        This is intended to adjust the contents as needed by the entry type.
        """
        for entry in self._entries.values():
            entry.ProcessContents()

    def BuildImage(self):
        """Write the image to a file"""
        fname = tools.GetOutputFilename(self._filename)
        with open(fname, 'wb') as fd:
            fd.write(chr(self._pad_byte) * self._size)

            for entry in self._entries.values():
                data = entry.GetData()
                fd.seek(self._pad_before + entry.pos - self._skip_at_start)
                fd.write(data)

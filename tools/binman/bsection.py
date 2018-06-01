# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Base class for sections (collections of entries)
#

from __future__ import print_function

from collections import OrderedDict
import sys

import fdt_util
import re
import tools

class Section(object):
    """A section which contains multiple entries

    A section represents a collection of entries. There must be one or more
    sections in an image. Sections are used to group entries together.

    Attributes:
        _node: Node object that contains the section definition in device tree
        _size: Section size in bytes, or None if not known yet
        _align_size: Section size alignment, or None
        _pad_before: Number of bytes before the first entry starts. This
            effectively changes the place where entry position 0 starts
        _pad_after: Number of bytes after the last entry ends. The last
            entry will finish on or before this boundary
        _pad_byte: Byte to use to pad the section where there is no entry
        _sort: True if entries should be sorted by position, False if they
            must be in-order in the device tree description
        _skip_at_start: Number of bytes before the first entry starts. These
            effectively adjust the starting position of entries. For example,
            if _pad_before is 16, then the first entry would start at 16.
            An entry with pos = 20 would in fact be written at position 4
            in the image file.
        _end_4gb: Indicates that the section ends at the 4GB boundary. This is
            used for x86 images, which want to use positions such that a
             memory address (like 0xff800000) is the first entry position.
             This causes _skip_at_start to be set to the starting memory
             address.
        _name_prefix: Prefix to add to the name of all entries within this
            section
        _entries: OrderedDict() of entries
    """
    def __init__(self, name, node, test=False):
        global entry
        global Entry
        import entry
        from entry import Entry

        self._node = node
        self._size = None
        self._align_size = None
        self._pad_before = 0
        self._pad_after = 0
        self._pad_byte = 0
        self._sort = False
        self._skip_at_start = 0
        self._end_4gb = False
        self._name_prefix = ''
        self._entries = OrderedDict()
        if not test:
            self._ReadNode()
            self._ReadEntries()

    def _ReadNode(self):
        """Read properties from the section node"""
        self._size = fdt_util.GetInt(self._node, 'size')
        self._align_size = fdt_util.GetInt(self._node, 'align-size')
        if tools.NotPowerOfTwo(self._align_size):
            self._Raise("Alignment size %s must be a power of two" %
                        self._align_size)
        self._pad_before = fdt_util.GetInt(self._node, 'pad-before', 0)
        self._pad_after = fdt_util.GetInt(self._node, 'pad-after', 0)
        self._pad_byte = fdt_util.GetInt(self._node, 'pad-byte', 0)
        self._sort = fdt_util.GetBool(self._node, 'sort-by-pos')
        self._end_4gb = fdt_util.GetBool(self._node, 'end-at-4gb')
        if self._end_4gb and not self._size:
            self._Raise("Section size must be provided when using end-at-4gb")
        if self._end_4gb:
            self._skip_at_start = 0x100000000 - self._size
        self._name_prefix = fdt_util.GetString(self._node, 'name-prefix')

    def _ReadEntries(self):
        for node in self._node.subnodes:
            entry = Entry.Create(self, node)
            entry.SetPrefix(self._name_prefix)
            self._entries[node.name] = entry

    def CheckSize(self):
        """Check that the section contents does not exceed its size, etc."""
        contents_size = 0
        for entry in self._entries.values():
            contents_size = max(contents_size, entry.pos + entry.size)

        contents_size -= self._skip_at_start

        size = self._size
        if not size:
            size = self._pad_before + contents_size + self._pad_after
            size = tools.Align(size, self._align_size)

        if self._size and contents_size > self._size:
            self._Raise("contents size %#x (%d) exceeds section size %#x (%d)" %
                       (contents_size, contents_size, self._size, self._size))
        if not self._size:
            self._size = size
        if self._size != tools.Align(self._size, self._align_size):
            self._Raise("Size %#x (%d) does not match align-size %#x (%d)" %
                  (self._size, self._size, self._align_size, self._align_size))
        return size

    def _Raise(self, msg):
        """Raises an error for this section

        Args:
            msg: Error message to use in the raise string
        Raises:
            ValueError()
        """
        raise ValueError("Section '%s': %s" % (self._node.path, msg))

    def GetPath(self):
        """Get the path of an image (in the FDT)

        Returns:
            Full path of the node for this image
        """
        return self._node.path

    def FindEntryType(self, etype):
        """Find an entry type in the section

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
        """Pack all entries into the section"""
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
        """Check that entries do not overlap or extend outside the section"""
        if self._sort:
            self._SortEntries()
        pos = 0
        prev_name = 'None'
        for entry in self._entries.values():
            entry.CheckPosition()
            if (entry.pos < self._skip_at_start or
                entry.pos >= self._skip_at_start + self._size):
                entry.Raise("Position %#x (%d) is outside the section starting "
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

    def WriteSymbols(self):
        """Write symbol values into binary files for access at run time"""
        for entry in self._entries.values():
            entry.WriteSymbols(self)

    def BuildSection(self, fd, base_pos):
        """Write the section to a file"""
        fd.seek(base_pos)
        fd.write(self.GetData())

    def GetData(self):
        """Write the section to a file"""
        section_data = chr(self._pad_byte) * self._size

        for entry in self._entries.values():
            data = entry.GetData()
            base = self._pad_before + entry.pos - self._skip_at_start
            section_data = (section_data[:base] + data +
                            section_data[base + len(data):])
        return section_data

    def LookupSymbol(self, sym_name, optional, msg):
        """Look up a symbol in an ELF file

        Looks up a symbol in an ELF file. Only entry types which come from an
        ELF image can be used by this function.

        At present the only entry property supported is pos.

        Args:
            sym_name: Symbol name in the ELF file to look up in the format
                _binman_<entry>_prop_<property> where <entry> is the name of
                the entry and <property> is the property to find (e.g.
                _binman_u_boot_prop_pos). As a special case, you can append
                _any to <entry> to have it search for any matching entry. E.g.
                _binman_u_boot_any_prop_pos will match entries called u-boot,
                u-boot-img and u-boot-nodtb)
            optional: True if the symbol is optional. If False this function
                will raise if the symbol is not found
            msg: Message to display if an error occurs

        Returns:
            Value that should be assigned to that symbol, or None if it was
                optional and not found

        Raises:
            ValueError if the symbol is invalid or not found, or references a
                property which is not supported
        """
        m = re.match(r'^_binman_(\w+)_prop_(\w+)$', sym_name)
        if not m:
            raise ValueError("%s: Symbol '%s' has invalid format" %
                             (msg, sym_name))
        entry_name, prop_name = m.groups()
        entry_name = entry_name.replace('_', '-')
        entry = self._entries.get(entry_name)
        if not entry:
            if entry_name.endswith('-any'):
                root = entry_name[:-4]
                for name in self._entries:
                    if name.startswith(root):
                        rest = name[len(root):]
                        if rest in ['', '-img', '-nodtb']:
                            entry = self._entries[name]
        if not entry:
            err = ("%s: Entry '%s' not found in list (%s)" %
                   (msg, entry_name, ','.join(self._entries.keys())))
            if optional:
                print('Warning: %s' % err, file=sys.stderr)
                return None
            raise ValueError(err)
        if prop_name == 'pos':
            return entry.pos
        else:
            raise ValueError("%s: No such property '%s'" % (msg, prop_name))

    def GetEntries(self):
        return self._entries

    def WriteMap(self, fd, indent):
        """Write a map of the section to a .map file

        Args:
            fd: File to write the map to
        """
        for entry in self._entries.values():
            entry.WriteMap(fd, indent)

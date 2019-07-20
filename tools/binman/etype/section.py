# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>

"""Entry-type module for sections (groups of entries)

Sections are entries which can contain other entries. This allows hierarchical
images to be created.
"""

from __future__ import print_function

from collections import OrderedDict
import re
import sys

from entry import Entry
import fdt_util
import tools


class Entry_section(Entry):
    """Entry that contains other entries

    Properties / Entry arguments: (see binman README for more information)
        pad-byte: Pad byte to use when padding
        sort-by-offset: True if entries should be sorted by offset, False if
            they must be in-order in the device tree description
        end-at-4gb: Used to build an x86 ROM which ends at 4GB (2^32)
        skip-at-start: Number of bytes before the first entry starts. These
            effectively adjust the starting offset of entries. For example,
            if this is 16, then the first entry would start at 16. An entry
            with offset = 20 would in fact be written at offset 4 in the image
            file, since the first 16 bytes are skipped when writing.
        name-prefix: Adds a prefix to the name of every entry in the section
            when writing out the map

    Since a section is also an entry, it inherits all the properies of entries
    too.

    A section is an entry which can contain other entries, thus allowing
    hierarchical images to be created. See 'Sections and hierarchical images'
    in the binman README for more information.
    """
    def __init__(self, section, etype, node, test=False):
        if not test:
            Entry.__init__(self, section, etype, node)
        self._entries = OrderedDict()
        self._pad_byte = 0
        self._sort = False
        self._skip_at_start = None
        self._end_4gb = False

    def ReadNode(self):
        """Read properties from the image node"""
        Entry.ReadNode(self)
        self._pad_byte = fdt_util.GetInt(self._node, 'pad-byte', 0)
        self._sort = fdt_util.GetBool(self._node, 'sort-by-offset')
        self._end_4gb = fdt_util.GetBool(self._node, 'end-at-4gb')
        self._skip_at_start = fdt_util.GetInt(self._node, 'skip-at-start')
        if self._end_4gb:
            if not self.size:
                self.Raise("Section size must be provided when using end-at-4gb")
            if self._skip_at_start is not None:
                self.Raise("Provide either 'end-at-4gb' or 'skip-at-start'")
            else:
                self._skip_at_start = 0x100000000 - self.size
        else:
            if self._skip_at_start is None:
                self._skip_at_start = 0
        self._name_prefix = fdt_util.GetString(self._node, 'name-prefix')
        filename = fdt_util.GetString(self._node, 'filename')
        if filename:
            self._filename = filename

        self._ReadEntries()

    def _ReadEntries(self):
        for node in self._node.subnodes:
            if node.name == 'hash':
                continue
            entry = Entry.Create(self, node)
            entry.ReadNode()
            entry.SetPrefix(self._name_prefix)
            self._entries[node.name] = entry

    def _Raise(self, msg):
        """Raises an error for this section

        Args:
            msg: Error message to use in the raise string
        Raises:
            ValueError()
        """
        raise ValueError("Section '%s': %s" % (self._node.path, msg))

    def GetFdts(self):
        fdts = {}
        for entry in self._entries.values():
            fdts.update(entry.GetFdts())
        return fdts

    def ProcessFdt(self, fdt):
        """Allow entries to adjust the device tree

        Some entries need to adjust the device tree for their purposes. This
        may involve adding or deleting properties.
        """
        todo = self._entries.values()
        for passnum in range(3):
            next_todo = []
            for entry in todo:
                if not entry.ProcessFdt(fdt):
                    next_todo.append(entry)
            todo = next_todo
            if not todo:
                break
        if todo:
            self.Raise('Internal error: Could not complete processing of Fdt: remaining %s' %
                       todo)
        return True

    def ExpandEntries(self):
        """Expand out any entries which have calculated sub-entries

        Some entries are expanded out at runtime, e.g. 'files', which produces
        a section containing a list of files. Process these entries so that
        this information is added to the device tree.
        """
        Entry.ExpandEntries(self)
        for entry in self._entries.values():
            entry.ExpandEntries()

    def AddMissingProperties(self):
        """Add new properties to the device tree as needed for this entry"""
        Entry.AddMissingProperties(self)
        for entry in self._entries.values():
            entry.AddMissingProperties()

    def ObtainContents(self):
        return self.GetEntryContents()

    def GetData(self):
        section_data = tools.GetBytes(self._pad_byte, self.size)

        for entry in self._entries.values():
            data = entry.GetData()
            base = self.pad_before + entry.offset - self._skip_at_start
            section_data = (section_data[:base] + data +
                            section_data[base + len(data):])
        self.Detail('GetData: %d entries, total size %#x' %
                    (len(self._entries), len(section_data)))
        return section_data

    def GetOffsets(self):
        """Handle entries that want to set the offset/size of other entries

        This calls each entry's GetOffsets() method. If it returns a list
        of entries to update, it updates them.
        """
        self.GetEntryOffsets()
        return {}

    def ResetForPack(self):
        """Reset offset/size fields so that packing can be done again"""
        Entry.ResetForPack(self)
        for entry in self._entries.values():
            entry.ResetForPack()

    def Pack(self, offset):
        """Pack all entries into the section"""
        self._PackEntries()
        return Entry.Pack(self, offset)

    def _PackEntries(self):
        """Pack all entries into the image"""
        offset = self._skip_at_start
        for entry in self._entries.values():
            offset = entry.Pack(offset)
        self.size = self.CheckSize()

    def _ExpandEntries(self):
        """Expand any entries that are permitted to"""
        exp_entry = None
        for entry in self._entries.values():
            if exp_entry:
                exp_entry.ExpandToLimit(entry.offset)
                exp_entry = None
            if entry.expand_size:
                exp_entry = entry
        if exp_entry:
            exp_entry.ExpandToLimit(self.size)

    def _SortEntries(self):
        """Sort entries by offset"""
        entries = sorted(self._entries.values(), key=lambda entry: entry.offset)
        self._entries.clear()
        for entry in entries:
            self._entries[entry._node.name] = entry

    def CheckEntries(self):
        """Check that entries do not overlap or extend outside the image"""
        if self._sort:
            self._SortEntries()
        self._ExpandEntries()
        offset = 0
        prev_name = 'None'
        for entry in self._entries.values():
            entry.CheckOffset()
            if (entry.offset < self._skip_at_start or
                    entry.offset + entry.size > self._skip_at_start +
                    self.size):
                entry.Raise("Offset %#x (%d) is outside the section starting "
                            "at %#x (%d)" %
                            (entry.offset, entry.offset, self._skip_at_start,
                             self._skip_at_start))
            if entry.offset < offset:
                entry.Raise("Offset %#x (%d) overlaps with previous entry '%s' "
                            "ending at %#x (%d)" %
                            (entry.offset, entry.offset, prev_name, offset, offset))
            offset = entry.offset + entry.size
            prev_name = entry.GetPath()

    def WriteSymbols(self, section):
        """Write symbol values into binary files for access at run time"""
        for entry in self._entries.values():
            entry.WriteSymbols(self)

    def SetCalculatedProperties(self):
        Entry.SetCalculatedProperties(self)
        for entry in self._entries.values():
            entry.SetCalculatedProperties()

    def SetImagePos(self, image_pos):
        Entry.SetImagePos(self, image_pos)
        for entry in self._entries.values():
            entry.SetImagePos(image_pos + self.offset)

    def ProcessContents(self):
        sizes_ok_base = super(Entry_section, self).ProcessContents()
        sizes_ok = True
        for entry in self._entries.values():
            if not entry.ProcessContents():
                sizes_ok = False
        return sizes_ok and sizes_ok_base

    def CheckOffset(self):
        self.CheckEntries()

    def WriteMap(self, fd, indent):
        """Write a map of the section to a .map file

        Args:
            fd: File to write the map to
        """
        Entry.WriteMapLine(fd, indent, self.name, self.offset or 0,
                           self.size, self.image_pos)
        for entry in self._entries.values():
            entry.WriteMap(fd, indent + 1)

    def GetEntries(self):
        return self._entries

    def GetContentsByPhandle(self, phandle, source_entry):
        """Get the data contents of an entry specified by a phandle

        This uses a phandle to look up a node and and find the entry
        associated with it. Then it returnst he contents of that entry.

        Args:
            phandle: Phandle to look up (integer)
            source_entry: Entry containing that phandle (used for error
                reporting)

        Returns:
            data from associated entry (as a string), or None if not found
        """
        node = self._node.GetFdt().LookupPhandle(phandle)
        if not node:
            source_entry.Raise("Cannot find node for phandle %d" % phandle)
        for entry in self._entries.values():
            if entry._node == node:
                return entry.GetData()
        source_entry.Raise("Cannot find entry for node '%s'" % node.name)

    def LookupSymbol(self, sym_name, optional, msg):
        """Look up a symbol in an ELF file

        Looks up a symbol in an ELF file. Only entry types which come from an
        ELF image can be used by this function.

        At present the only entry property supported is offset.

        Args:
            sym_name: Symbol name in the ELF file to look up in the format
                _binman_<entry>_prop_<property> where <entry> is the name of
                the entry and <property> is the property to find (e.g.
                _binman_u_boot_prop_offset). As a special case, you can append
                _any to <entry> to have it search for any matching entry. E.g.
                _binman_u_boot_any_prop_offset will match entries called u-boot,
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
        if prop_name == 'offset':
            return entry.offset
        elif prop_name == 'image_pos':
            return entry.image_pos
        else:
            raise ValueError("%s: No such property '%s'" % (msg, prop_name))

    def GetRootSkipAtStart(self):
        """Get the skip-at-start value for the top-level section

        This is used to find out the starting offset for root section that
        contains this section. If this is a top-level section then it returns
        the skip-at-start offset for this section.

        This is used to get the absolute position of section within the image.

        Returns:
            Integer skip-at-start value for the root section containing this
                section
        """
        if self.section:
            return self.section.GetRootSkipAtStart()
        return self._skip_at_start

    def GetStartOffset(self):
        """Get the start offset for this section

        Returns:
            The first available offset in this section (typically 0)
        """
        return self._skip_at_start

    def GetImageSize(self):
        """Get the size of the image containing this section

        Returns:
            Image size as an integer number of bytes, which may be None if the
                image size is dynamic and its sections have not yet been packed
        """
        return self.GetImage().size

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
        """Call ObtainContents() for the section
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
        if todo:
            self.Raise('Internal error: Could not complete processing of contents: remaining %s' %
                       todo)
        return True

    def _SetEntryOffsetSize(self, name, offset, size):
        """Set the offset and size of an entry

        Args:
            name: Entry name to update
            offset: New offset, or None to leave alone
            size: New size, or None to leave alone
        """
        entry = self._entries.get(name)
        if not entry:
            self._Raise("Unable to set offset/size for unknown entry '%s'" %
                        name)
        entry.SetOffsetSize(self._skip_at_start + offset if offset else None,
                            size)

    def GetEntryOffsets(self):
        """Handle entries that want to set the offset/size of other entries

        This calls each entry's GetOffsets() method. If it returns a list
        of entries to update, it updates them.
        """
        for entry in self._entries.values():
            offset_dict = entry.GetOffsets()
            for name, info in offset_dict.items():
                self._SetEntryOffsetSize(name, *info)


    def CheckSize(self):
        """Check that the image contents does not exceed its size, etc."""
        contents_size = 0
        for entry in self._entries.values():
            contents_size = max(contents_size, entry.offset + entry.size)

        contents_size -= self._skip_at_start

        size = self.size
        if not size:
            size = self.pad_before + contents_size + self.pad_after
            size = tools.Align(size, self.align_size)

        if self.size and contents_size > self.size:
            self._Raise("contents size %#x (%d) exceeds section size %#x (%d)" %
                        (contents_size, contents_size, self.size, self.size))
        if not self.size:
            self.size = size
        if self.size != tools.Align(self.size, self.align_size):
            self._Raise("Size %#x (%d) does not match align-size %#x (%d)" %
                        (self.size, self.size, self.align_size,
                         self.align_size))
        return size

    def ListEntries(self, entries, indent):
        """List the files in the section"""
        Entry.AddEntryInfo(entries, indent, self.name, 'section', self.size,
                           self.image_pos, None, self.offset, self)
        for entry in self._entries.values():
            entry.ListEntries(entries, indent + 1)

    def LoadData(self, decomp=True):
        for entry in self._entries.values():
            entry.LoadData(decomp)
        self.Detail('Loaded data')

    def GetImage(self):
        """Get the image containing this section

        Note that a top-level section is actually an Image, so this function may
        return self.

        Returns:
            Image object containing this section
        """
        if not self.section:
            return self
        return self.section.GetImage()

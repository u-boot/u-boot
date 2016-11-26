# Copyright (c) 2016 Google, Inc
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Base class for all entries
#

# importlib was introduced in Python 2.7 but there was a report of it not
# working in 2.7.12, so we work around this:
# http://lists.denx.de/pipermail/u-boot/2016-October/269729.html
try:
    import importlib
    have_importlib = True
except:
    have_importlib = False

import fdt_util
import tools

modules = {}

class Entry(object):
    """An Entry in the image

    An entry corresponds to a single node in the device-tree description
    of the image. Each entry ends up being a part of the final image.
    Entries can be placed either right next to each other, or with padding
    between them. The type of the entry determines the data that is in it.

    This class is not used by itself. All entry objects are subclasses of
    Entry.

    Attributes:
        image: The image containing this entry
        node: The node that created this entry
        pos: Absolute position of entry within the image, None if not known
        size: Entry size in bytes, None if not known
        contents_size: Size of contents in bytes, 0 by default
        align: Entry start position alignment, or None
        align_size: Entry size alignment, or None
        align_end: Entry end position alignment, or None
        pad_before: Number of pad bytes before the contents, 0 if none
        pad_after: Number of pad bytes after the contents, 0 if none
        data: Contents of entry (string of bytes)
    """
    def __init__(self, image, etype, node, read_node=True):
        self.image = image
        self.etype = etype
        self._node = node
        self.pos = None
        self.size = None
        self.contents_size = 0
        self.align = None
        self.align_size = None
        self.align_end = None
        self.pad_before = 0
        self.pad_after = 0
        self.pos_unset = False
        if read_node:
            self.ReadNode()

    @staticmethod
    def Create(image, node, etype=None):
        """Create a new entry for a node.

        Args:
            image:  Image object containing this node
            node:   Node object containing information about the entry to create
            etype:  Entry type to use, or None to work it out (used for tests)

        Returns:
            A new Entry object of the correct type (a subclass of Entry)
        """
        if not etype:
            etype = fdt_util.GetString(node, 'type', node.name)
        module_name = etype.replace('-', '_')
        module = modules.get(module_name)

        # Import the module if we have not already done so.
        if not module:
            try:
                if have_importlib:
                    module = importlib.import_module(module_name)
                else:
                    module = __import__(module_name)
            except ImportError:
                raise ValueError("Unknown entry type '%s' in node '%s'" %
                        (etype, node.path))
            modules[module_name] = module

        # Call its constructor to get the object we want.
        obj = getattr(module, 'Entry_%s' % module_name)
        return obj(image, etype, node)

    def ReadNode(self):
        """Read entry information from the node

        This reads all the fields we recognise from the node, ready for use.
        """
        self.pos = fdt_util.GetInt(self._node, 'pos')
        self.size = fdt_util.GetInt(self._node, 'size')
        self.align = fdt_util.GetInt(self._node, 'align')
        if tools.NotPowerOfTwo(self.align):
            raise ValueError("Node '%s': Alignment %s must be a power of two" %
                             (self._node.path, self.align))
        self.pad_before = fdt_util.GetInt(self._node, 'pad-before', 0)
        self.pad_after = fdt_util.GetInt(self._node, 'pad-after', 0)
        self.align_size = fdt_util.GetInt(self._node, 'align-size')
        if tools.NotPowerOfTwo(self.align_size):
            raise ValueError("Node '%s': Alignment size %s must be a power "
                             "of two" % (self._node.path, self.align_size))
        self.align_end = fdt_util.GetInt(self._node, 'align-end')
        self.pos_unset = fdt_util.GetBool(self._node, 'pos-unset')

    def ObtainContents(self):
        """Figure out the contents of an entry.

        Returns:
            True if the contents were found, False if another call is needed
            after the other entries are processed.
        """
        # No contents by default: subclasses can implement this
        return True

    def Pack(self, pos):
        """Figure out how to pack the entry into the image

        Most of the time the entries are not fully specified. There may be
        an alignment but no size. In that case we take the size from the
        contents of the entry.

        If an entry has no hard-coded position, it will be placed at @pos.

        Once this function is complete, both the position and size of the
        entry will be know.

        Args:
            Current image position pointer

        Returns:
            New image position pointer (after this entry)
        """
        if self.pos is None:
            if self.pos_unset:
                self.Raise('No position set with pos-unset: should another '
                           'entry provide this correct position?')
            self.pos = tools.Align(pos, self.align)
        needed = self.pad_before + self.contents_size + self.pad_after
        needed = tools.Align(needed, self.align_size)
        size = self.size
        if not size:
            size = needed
        new_pos = self.pos + size
        aligned_pos = tools.Align(new_pos, self.align_end)
        if aligned_pos != new_pos:
            size = aligned_pos - self.pos
            new_pos = aligned_pos

        if not self.size:
            self.size = size

        if self.size < needed:
            self.Raise("Entry contents size is %#x (%d) but entry size is "
                       "%#x (%d)" % (needed, needed, self.size, self.size))
        # Check that the alignment is correct. It could be wrong if the
        # and pos or size values were provided (i.e. not calculated), but
        # conflict with the provided alignment values
        if self.size != tools.Align(self.size, self.align_size):
            self.Raise("Size %#x (%d) does not match align-size %#x (%d)" %
                  (self.size, self.size, self.align_size, self.align_size))
        if self.pos != tools.Align(self.pos, self.align):
            self.Raise("Position %#x (%d) does not match align %#x (%d)" %
                  (self.pos, self.pos, self.align, self.align))

        return new_pos

    def Raise(self, msg):
        """Convenience function to raise an error referencing a node"""
        raise ValueError("Node '%s': %s" % (self._node.path, msg))

    def GetPath(self):
        """Get the path of a node

        Returns:
            Full path of the node for this entry
        """
        return self._node.path

    def GetData(self):
        return self.data

    def GetPositions(self):
        return {}

    def SetPositionSize(self, pos, size):
        self.pos = pos
        self.size = size

    def ProcessContents(self):
        pass

#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import struct
import sys

import fdt_util
import libfdt

# This deals with a device tree, presenting it as an assortment of Node and
# Prop objects, representing nodes and properties, respectively. This file
# contains the base classes and defines the high-level API. You can use
# FdtScan() as a convenience function to create and scan an Fdt.

# This implementation uses a libfdt Python library to access the device tree,
# so it is fairly efficient.

# A list of types we support
(TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL, TYPE_INT64) = range(5)

def CheckErr(errnum, msg):
    if errnum:
        raise ValueError('Error %d: %s: %s' %
            (errnum, libfdt.fdt_strerror(errnum), msg))

class Prop:
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, node, offset, name, bytes):
        self._node = node
        self._offset = offset
        self.name = name
        self.value = None
        self.bytes = str(bytes)
        if not bytes:
            self.type = TYPE_BOOL
            self.value = True
            return
        self.type, self.value = self.BytesToValue(bytes)

    def GetPhandle(self):
        """Get a (single) phandle value from a property

        Gets the phandle valuie from a property and returns it as an integer
        """
        return fdt_util.fdt32_to_cpu(self.value[:4])

    def Widen(self, newprop):
        """Figure out which property type is more general

        Given a current property and a new property, this function returns the
        one that is less specific as to type. The less specific property will
        be ble to represent the data in the more specific property. This is
        used for things like:

            node1 {
                compatible = "fred";
                value = <1>;
            };
            node1 {
                compatible = "fred";
                value = <1 2>;
            };

        He we want to use an int array for 'value'. The first property
        suggests that a single int is enough, but the second one shows that
        it is not. Calling this function with these two propertes would
        update the current property to be like the second, since it is less
        specific.
        """
        if newprop.type < self.type:
            self.type = newprop.type

        if type(newprop.value) == list and type(self.value) != list:
            self.value = [self.value]

        if type(self.value) == list and len(newprop.value) > len(self.value):
            val = self.GetEmpty(self.type)
            while len(self.value) < len(newprop.value):
                self.value.append(val)

    def BytesToValue(self, bytes):
        """Converts a string of bytes into a type and value

        Args:
            A string containing bytes

        Return:
            A tuple:
                Type of data
                Data, either a single element or a list of elements. Each element
                is one of:
                    TYPE_STRING: string value from the property
                    TYPE_INT: a byte-swapped integer stored as a 4-byte string
                    TYPE_BYTE: a byte stored as a single-byte string
        """
        bytes = str(bytes)
        size = len(bytes)
        strings = bytes.split('\0')
        is_string = True
        count = len(strings) - 1
        if count > 0 and not strings[-1]:
            for string in strings[:-1]:
                if not string:
                    is_string = False
                    break
                for ch in string:
                    if ch < ' ' or ch > '~':
                        is_string = False
                        break
        else:
            is_string = False
        if is_string:
            if count == 1:
                return TYPE_STRING, strings[0]
            else:
                return TYPE_STRING, strings[:-1]
        if size % 4:
            if size == 1:
                return TYPE_BYTE, bytes[0]
            else:
                return TYPE_BYTE, list(bytes)
        val = []
        for i in range(0, size, 4):
            val.append(bytes[i:i + 4])
        if size == 4:
            return TYPE_INT, val[0]
        else:
            return TYPE_INT, val

    def GetEmpty(self, type):
        """Get an empty / zero value of the given type

        Returns:
            A single value of the given type
        """
        if type == TYPE_BYTE:
            return chr(0)
        elif type == TYPE_INT:
            return struct.pack('<I', 0);
        elif type == TYPE_STRING:
            return ''
        else:
            return True

    def GetOffset(self):
        """Get the offset of a property

        Returns:
            The offset of the property (struct fdt_property) within the file
        """
        return self._node._fdt.GetStructOffset(self._offset)

class Node:
    """A device tree node

    Properties:
        offset: Integer offset in the device tree
        name: Device tree node tname
        path: Full path to node, along with the node name itself
        _fdt: Device tree object
        subnodes: A list of subnodes for this node, each a Node object
        props: A dict of properties for this node, each a Prop object.
            Keyed by property name
    """
    def __init__(self, fdt, parent, offset, name, path):
        self._fdt = fdt
        self.parent = parent
        self._offset = offset
        self.name = name
        self.path = path
        self.subnodes = []
        self.props = {}

    def _FindNode(self, name):
        """Find a node given its name

        Args:
            name: Node name to look for
        Returns:
            Node object if found, else None
        """
        for subnode in self.subnodes:
            if subnode.name == name:
                return subnode
        return None

    def Offset(self):
        """Returns the offset of a node, after checking the cache

        This should be used instead of self._offset directly, to ensure that
        the cache does not contain invalid offsets.
        """
        self._fdt.CheckCache()
        return self._offset

    def Scan(self):
        """Scan a node's properties and subnodes

        This fills in the props and subnodes properties, recursively
        searching into subnodes so that the entire tree is built.
        """
        self.props = self._fdt.GetProps(self)
        phandle = self.props.get('phandle')
        if phandle:
            val = fdt_util.fdt32_to_cpu(phandle.value)
            self._fdt.phandle_to_node[val] = self

        offset = libfdt.fdt_first_subnode(self._fdt.GetFdt(), self.Offset())
        while offset >= 0:
            sep = '' if self.path[-1] == '/' else '/'
            name = self._fdt._fdt_obj.get_name(offset)
            path = self.path + sep + name
            node = Node(self._fdt, self, offset, name, path)
            self.subnodes.append(node)

            node.Scan()
            offset = libfdt.fdt_next_subnode(self._fdt.GetFdt(), offset)

    def Refresh(self, my_offset):
        """Fix up the _offset for each node, recursively

        Note: This does not take account of property offsets - these will not
        be updated.
        """
        if self._offset != my_offset:
            #print '%s: %d -> %d\n' % (self.path, self._offset, my_offset)
            self._offset = my_offset
        offset = libfdt.fdt_first_subnode(self._fdt.GetFdt(), self._offset)
        for subnode in self.subnodes:
            subnode.Refresh(offset)
            offset = libfdt.fdt_next_subnode(self._fdt.GetFdt(), offset)

    def DeleteProp(self, prop_name):
        """Delete a property of a node

        The property is deleted and the offset cache is invalidated.

        Args:
            prop_name: Name of the property to delete
        Raises:
            ValueError if the property does not exist
        """
        CheckErr(libfdt.fdt_delprop(self._fdt.GetFdt(), self.Offset(), prop_name),
                 "Node '%s': delete property: '%s'" % (self.path, prop_name))
        del self.props[prop_name]
        self._fdt.Invalidate()

class Fdt:
    """Provides simple access to a flat device tree blob using libfdts.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """
    def __init__(self, fname):
        self._fname = fname
        self._cached_offsets = False
        self.phandle_to_node = {}
        if self._fname:
            self._fname = fdt_util.EnsureCompiled(self._fname)

            with open(self._fname) as fd:
                self._fdt = bytearray(fd.read())
                self._fdt_obj = libfdt.Fdt(self._fdt)

    def Scan(self, root='/'):
        """Scan a device tree, building up a tree of Node objects

        This fills in the self._root property

        Args:
            root: Ignored

        TODO(sjg@chromium.org): Implement the 'root' parameter
        """
        self._root = self.Node(self, None, 0, '/', '/')
        self._root.Scan()

    def GetRoot(self):
        """Get the root Node of the device tree

        Returns:
            The root Node object
        """
        return self._root

    def GetNode(self, path):
        """Look up a node from its path

        Args:
            path: Path to look up, e.g. '/microcode/update@0'
        Returns:
            Node object, or None if not found
        """
        node = self._root
        for part in path.split('/')[1:]:
            node = node._FindNode(part)
            if not node:
                return None
        return node

    def Flush(self):
        """Flush device tree changes back to the file

        If the device tree has changed in memory, write it back to the file.
        """
        with open(self._fname, 'wb') as fd:
            fd.write(self._fdt)

    def Pack(self):
        """Pack the device tree down to its minimum size

        When nodes and properties shrink or are deleted, wasted space can
        build up in the device tree binary.
        """
        CheckErr(libfdt.fdt_pack(self._fdt), 'pack')
        fdt_len = libfdt.fdt_totalsize(self._fdt)
        del self._fdt[fdt_len:]

    def GetFdt(self):
        """Get the contents of the FDT

        Returns:
            The FDT contents as a string of bytes
        """
        return self._fdt

    def CheckErr(errnum, msg):
        if errnum:
            raise ValueError('Error %d: %s: %s' %
                (errnum, libfdt.fdt_strerror(errnum), msg))


    def GetProps(self, node):
        """Get all properties from a node.

        Args:
            node: Full path to node name to look in.

        Returns:
            A dictionary containing all the properties, indexed by node name.
            The entries are Prop objects.

        Raises:
            ValueError: if the node does not exist.
        """
        props_dict = {}
        poffset = libfdt.fdt_first_property_offset(self._fdt, node._offset)
        while poffset >= 0:
            p = self._fdt_obj.get_property_by_offset(poffset)
            prop = Prop(node, poffset, p.name, p.value)
            props_dict[prop.name] = prop

            poffset = libfdt.fdt_next_property_offset(self._fdt, poffset)
        return props_dict

    def Invalidate(self):
        """Mark our offset cache as invalid"""
        self._cached_offsets = False

    def CheckCache(self):
        """Refresh the offset cache if needed"""
        if self._cached_offsets:
            return
        self.Refresh()
        self._cached_offsets = True

    def Refresh(self):
        """Refresh the offset cache"""
        self._root.Refresh(0)

    def GetStructOffset(self, offset):
        """Get the file offset of a given struct offset

        Args:
            offset: Offset within the 'struct' region of the device tree
        Returns:
            Position of @offset within the device tree binary
        """
        return libfdt.fdt_off_dt_struct(self._fdt) + offset

    @classmethod
    def Node(self, fdt, parent, offset, name, path):
        """Create a new node

        This is used by Fdt.Scan() to create a new node using the correct
        class.

        Args:
            fdt: Fdt object
            parent: Parent node, or None if this is the root node
            offset: Offset of node
            name: Node name
            path: Full path to node
        """
        node = Node(fdt, parent, offset, name, path)
        return node

def FdtScan(fname):
    """Returns a new Fdt object from the implementation we are using"""
    dtb = Fdt(fname)
    dtb.Scan()
    return dtb

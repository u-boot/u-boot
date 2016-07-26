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

# This deals with a device tree, presenting it as an assortment of Node and
# Prop objects, representing nodes and properties, respectively. This file
# contains the base classes and defines the high-level API. Most of the
# implementation is in the FdtFallback and FdtNormal subclasses. See
# fdt_select.py for how to create an Fdt object.

# A list of types we support
(TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL) = range(4)

def CheckErr(errnum, msg):
    if errnum:
        raise ValueError('Error %d: %s: %s' %
            (errnum, libfdt.fdt_strerror(errnum), msg))

class PropBase:
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, node, offset, name):
        self._node = node
        self._offset = offset
        self.name = name
        self.value = None

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

        This can be implemented by subclasses.

        Returns:
            The offset of the property (struct fdt_property) within the
            file, or None if not known.
        """
        return None

class NodeBase:
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
    def __init__(self, fdt, offset, name, path):
        self._fdt = fdt
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

    def Scan(self):
        """Scan the subnodes of a node

        This should be implemented by subclasses
        """
        raise NotImplementedError()

    def DeleteProp(self, prop_name):
        """Delete a property of a node

        This should be implemented by subclasses

        Args:
            prop_name: Name of the property to delete
        """
        raise NotImplementedError()

class Fdt:
    """Provides simple access to a flat device tree blob.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """
    def __init__(self, fname):
        self._fname = fname

    def Scan(self, root='/'):
        """Scan a device tree, building up a tree of Node objects

        This fills in the self._root property

        Args:
            root: Ignored

        TODO(sjg@chromium.org): Implement the 'root' parameter
        """
        self._root = self.Node(self, 0, '/', '/')
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
        Subclasses can implement this if needed.
        """
        pass

    def Pack(self):
        """Pack the device tree down to its minimum size

        When nodes and properties shrink or are deleted, wasted space can
        build up in the device tree binary. Subclasses can implement this
        to remove that spare space.
        """
        pass

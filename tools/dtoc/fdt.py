#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import fdt_util
import libfdt
import sys

# This deals with a device tree, presenting it as a list of Node and Prop
# objects, representing nodes and properties, respectively.
#
# This implementation uses a libfdt Python library to access the device tree,
# so it is fairly efficient.

class Prop:
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, name, bytes):
        self.name = name
        self.value = None
        if not bytes:
            self.type = fdt_util.TYPE_BOOL
            self.value = True
            return
        self.type, self.value = fdt_util.BytesToValue(bytes)

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
            val = fdt_util.GetEmpty(self.type)
            while len(self.value) < len(newprop.value):
                self.value.append(val)


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
    def __init__(self, fdt, offset, name, path):
        self.offset = offset
        self.name = name
        self.path = path
        self._fdt = fdt
        self.subnodes = []
        self.props = {}

    def Scan(self):
        """Scan a node's properties and subnodes

        This fills in the props and subnodes properties, recursively
        searching into subnodes so that the entire tree is built.
        """
        self.props = self._fdt.GetProps(self.path)

        offset = libfdt.fdt_first_subnode(self._fdt.GetFdt(), self.offset)
        while offset >= 0:
            sep = '' if self.path[-1] == '/' else '/'
            name = libfdt.Name(self._fdt.GetFdt(), offset)
            path = self.path + sep + name
            node = Node(self._fdt, offset, name, path)
            self.subnodes.append(node)

            node.Scan()
            offset = libfdt.fdt_next_subnode(self._fdt.GetFdt(), offset)


class Fdt:
    """Provides simple access to a flat device tree blob.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """

    def __init__(self, fname):
        self.fname = fname
        with open(fname) as fd:
            self._fdt = fd.read()

    def GetFdt(self):
        """Get the contents of the FDT

        Returns:
            The FDT contents as a string of bytes
        """
        return self._fdt

    def Scan(self):
        """Scan a device tree, building up a tree of Node objects

        This fills in the self._root property
        """
        self._root = Node(self, 0, '/', '/')
        self._root.Scan()

    def GetRoot(self):
        """Get the root Node of the device tree

        Returns:
            The root Node object
        """
        return self._root

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
        offset = libfdt.fdt_path_offset(self._fdt, node)
        if offset < 0:
            libfdt.Raise(offset)
        props_dict = {}
        poffset = libfdt.fdt_first_property_offset(self._fdt, offset)
        while poffset >= 0:
            dprop, plen = libfdt.fdt_get_property_by_offset(self._fdt, poffset)
            prop = Prop(libfdt.String(self._fdt, dprop.nameoff), libfdt.Data(dprop))
            props_dict[prop.name] = prop

            poffset = libfdt.fdt_next_property_offset(self._fdt, poffset)
        return props_dict

#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import command
import fdt_util
import sys

# This deals with a device tree, presenting it as a list of Node and Prop
# objects, representing nodes and properties, respectively.
#
# This implementation uses the fdtget tool to access the device tree, so it
# is not very efficient for larger trees. The tool is called once for each
# node and property in the tree.

class Prop:
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, name, byte_list_str):
        self.name = name
        self.value = None
        if not byte_list_str.strip():
            self.type = fdt_util.TYPE_BOOL
            return
        bytes = [chr(int(byte, 16)) for byte in byte_list_str.strip().split(' ')]
        self.type, self.value = fdt_util.BytesToValue(''.join(bytes))

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
            self.value = newprop.value

        if type(self.value) == list and len(newprop.value) > len(self.value):
            val = fdt_util.GetEmpty(self.type)
            while len(self.value) < len(newprop.value):
                self.value.append(val)


class Node:
    """A device tree node

    Properties:
        name: Device tree node tname
        path: Full path to node, along with the node name itself
        _fdt: Device tree object
        subnodes: A list of subnodes for this node, each a Node object
        props: A dict of properties for this node, each a Prop object.
            Keyed by property name
    """
    def __init__(self, fdt, name, path):
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
        for name, byte_list_str in self._fdt.GetProps(self.path).iteritems():
            prop = Prop(name, byte_list_str)
            self.props[name] = prop

        for name in self._fdt.GetSubNodes(self.path):
            sep = '' if self.path[-1] == '/' else '/'
            path = self.path + sep + name
            node = Node(self._fdt, name, path)
            self.subnodes.append(node)

            node.Scan()


class Fdt:
    """Provides simple access to a flat device tree blob.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """

    def __init__(self, fname):
        self.fname = fname

    def Scan(self):
        """Scan a device tree, building up a tree of Node objects

        This fills in the self._root property
        """
        self._root = Node(self, '/', '/')
        self._root.Scan()

    def GetRoot(self):
        """Get the root Node of the device tree

        Returns:
            The root Node object
        """
        return self._root

    def GetSubNodes(self, node):
        """Returns a list of sub-nodes of a given node

        Args:
            node: Node name to return children from

        Returns:
            List of children in the node (each a string node name)

        Raises:
            CmdError: if the node does not exist.
        """
        out = command.Output('fdtget', self.fname, '-l', node)
        return out.strip().splitlines()

    def GetProps(self, node, convert_dashes=False):
        """Get all properties from a node

        Args:
            node: full path to node name to look in
            convert_dashes: True to convert - to _ in node names

        Returns:
            A dictionary containing all the properties, indexed by node name.
            The entries are simply strings - no decoding of lists or numbers
            is done.

        Raises:
            CmdError: if the node does not exist.
        """
        out = command.Output('fdtget', self.fname, node, '-p')
        props = out.strip().splitlines()
        props_dict = {}
        for prop in props:
            name = prop
            if convert_dashes:
                prop = re.sub('-', '_', prop)
            props_dict[prop] = self.GetProp(node, name)
        return props_dict

    def GetProp(self, node, prop, default=None, typespec=None):
        """Get a property from a device tree.

        This looks up the given node and property, and returns the value as a
        string,

        If the node or property does not exist, this will return the default
        value.

        Args:
            node: Full path to node to look up.
            prop: Property name to look up.
            default: Default value to return if nothing is present in the fdt,
                or None to raise in this case. This will be converted to a
                string.
            typespec: Type character to use (None for default, 's' for string)

        Returns:
            string containing the property value.

        Raises:
            CmdError: if the property does not exist and no default is provided.
        """
        args = [self.fname, node, prop, '-t', 'bx']
        if default is not None:
          args += ['-d', str(default)]
        if typespec is not None:
          args += ['-t%s' % typespec]
        out = command.Output('fdtget', *args)
        return out.strip()

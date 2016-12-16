#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import command
import fdt
from fdt import Fdt, NodeBase, PropBase
import fdt_util
import sys

# This deals with a device tree, presenting it as a list of Node and Prop
# objects, representing nodes and properties, respectively.
#
# This implementation uses the fdtget tool to access the device tree, so it
# is not very efficient for larger trees. The tool is called once for each
# node and property in the tree.

class Prop(PropBase):
    """A device tree property

    Properties:
        name: Property name (as per the device tree)
        value: Property value as a string of bytes, or a list of strings of
            bytes
        type: Value type
    """
    def __init__(self, node, name, byte_list_str):
        PropBase.__init__(self, node, 0, name)
        if not byte_list_str.strip():
            self.type = fdt.TYPE_BOOL
            return
        self.bytes = [chr(int(byte, 16))
                      for byte in byte_list_str.strip().split(' ')]
        self.type, self.value = self.BytesToValue(''.join(self.bytes))


class Node(NodeBase):
    """A device tree node

    Properties:
        name: Device tree node tname
        path: Full path to node, along with the node name itself
        _fdt: Device tree object
        subnodes: A list of subnodes for this node, each a Node object
        props: A dict of properties for this node, each a Prop object.
            Keyed by property name
    """
    def __init__(self, fdt, offset, name, path):
        NodeBase.__init__(self, fdt, offset, name, path)

    def Scan(self):
        """Scan a node's properties and subnodes

        This fills in the props and subnodes properties, recursively
        searching into subnodes so that the entire tree is built.
        """
        for name, byte_list_str in self._fdt.GetProps(self.path).items():
            prop = Prop(self, name, byte_list_str)
            self.props[name] = prop

        for name in self._fdt.GetSubNodes(self.path):
            sep = '' if self.path[-1] == '/' else '/'
            path = self.path + sep + name
            node = Node(self._fdt, 0, name, path)
            self.subnodes.append(node)

            node.Scan()

    def DeleteProp(self, prop_name):
        """Delete a property of a node

        The property is deleted using fdtput.

        Args:
            prop_name: Name of the property to delete
        Raises:
            CommandError if the property does not exist
        """
        args = [self._fdt._fname, '-d', self.path, prop_name]
        command.Output('fdtput', *args)
        del self.props[prop_name]

class FdtFallback(Fdt):
    """Provides simple access to a flat device tree blob using fdtget/fdtput

    Properties:
        See superclass
    """

    def __init__(self, fname):
        Fdt.__init__(self, fname)
        if self._fname:
            self._fname = fdt_util.EnsureCompiled(self._fname)

    def GetSubNodes(self, node):
        """Returns a list of sub-nodes of a given node

        Args:
            node: Node name to return children from

        Returns:
            List of children in the node (each a string node name)

        Raises:
            CmdError: if the node does not exist.
        """
        out = command.Output('fdtget', self._fname, '-l', node)
        return out.strip().splitlines()

    def GetProps(self, node):
        """Get all properties from a node

        Args:
            node: full path to node name to look in

        Returns:
            A dictionary containing all the properties, indexed by node name.
            The entries are simply strings - no decoding of lists or numbers
            is done.

        Raises:
            CmdError: if the node does not exist.
        """
        out = command.Output('fdtget', self._fname, node, '-p')
        props = out.strip().splitlines()
        props_dict = {}
        for prop in props:
            name = prop
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
        args = [self._fname, node, prop, '-t', 'bx']
        if default is not None:
          args += ['-d', str(default)]
        if typespec is not None:
          args += ['-t', typespec]
        out = command.Output('fdtget', *args)
        return out.strip()

    @classmethod
    def Node(self, fdt, offset, name, path):
        """Create a new node

        This is used by Fdt.Scan() to create a new node using the correct
        class.

        Args:
            fdt: Fdt object
            offset: Offset of node
            name: Node name
            path: Full path to node
        """
        node = Node(fdt, offset, name, path)
        return node

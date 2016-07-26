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

class Fdt:
    """Provides simple access to a flat device tree blob.

    Properties:
      fname: Filename of fdt
      _root: Root of device tree (a Node object)
    """
    def __init__(self, fname):
        self._fname = fname

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

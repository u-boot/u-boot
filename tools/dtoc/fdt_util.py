#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import os
import struct
import sys
import tempfile

import command
import tools

def fdt32_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        Value to convert (4-character string representing the cell value)

    Return:
        A native-endian integer value
    """
    if sys.version_info > (3, 0):
        if isinstance(val, bytes):
            val = val.decode('utf-8')
        val = val.encode('raw_unicode_escape')
    return struct.unpack('>I', val)[0]

def fdt_cells_to_cpu(val, cells):
    """Convert one or two cells to a long integer

    Args:
        Value to convert (array of one or more 4-character strings)

    Return:
        A native-endian long value
    """
    if not cells:
        return 0
    out = long(fdt32_to_cpu(val[0]))
    if cells == 2:
        out = out << 32 | fdt32_to_cpu(val[1])
    return out

def EnsureCompiled(fname):
    """Compile an fdt .dts source file into a .dtb binary blob if needed.

    Args:
        fname: Filename (if .dts it will be compiled). It not it will be
            left alone

    Returns:
        Filename of resulting .dtb file
    """
    _, ext = os.path.splitext(fname)
    if ext != '.dts':
        return fname

    dts_input = tools.GetOutputFilename('source.dts')
    dtb_output = tools.GetOutputFilename('source.dtb')

    search_paths = [os.path.join(os.getcwd(), 'include')]
    root, _ = os.path.splitext(fname)
    args = ['-E', '-P', '-x', 'assembler-with-cpp', '-D__ASSEMBLY__']
    args += ['-Ulinux']
    for path in search_paths:
        args.extend(['-I', path])
    args += ['-o', dts_input, fname]
    command.Run('cc', *args)

    # If we don't have a directory, put it in the tools tempdir
    search_list = []
    for path in search_paths:
        search_list.extend(['-i', path])
    args = ['-I', 'dts', '-o', dtb_output, '-O', 'dtb']
    args.extend(search_list)
    args.append(dts_input)
    command.Run('dtc', *args)
    return dtb_output

def GetInt(node, propname, default=None):
    prop = node.props.get(propname)
    if not prop:
        return default
    value = fdt32_to_cpu(prop.value)
    if type(value) == type(list):
        raise ValueError("Node '%s' property '%' has list value: expecting"
                         "a single integer" % (node.name, propname))
    return value

def GetString(node, propname, default=None):
    prop = node.props.get(propname)
    if not prop:
        return default
    value = prop.value
    if type(value) == type(list):
        raise ValueError("Node '%s' property '%' has list value: expecting"
                         "a single string" % (node.name, propname))
    return value

def GetBool(node, propname, default=False):
    if propname in node.props:
        return True
    return default

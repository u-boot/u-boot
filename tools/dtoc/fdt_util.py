#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#

import struct

# A list of types we support
(TYPE_BYTE, TYPE_INT, TYPE_STRING, TYPE_BOOL) = range(4)

def BytesToValue(bytes):
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

def GetEmpty(type):
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

def fdt32_to_cpu(val):
    """Convert a device tree cell to an integer

    Args:
        Value to convert (4-character string representing the cell value)

    Return:
        A native-endian integer value
    """
    return struct.unpack(">I", val)[0]

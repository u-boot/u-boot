# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Handle various things related to ELF images
#

from collections import namedtuple, OrderedDict
import command
import os
import re
import struct

import tools

Symbol = namedtuple('Symbol', ['section', 'address', 'size', 'weak'])

# Used for tests which don't have an ELF file to read
ignore_missing_files = False


def GetSymbols(fname, patterns):
    """Get the symbols from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        None, if the file does not exist, or Dict:
          key: Name of symbol
          value: Hex value of symbol
    """
    stdout = command.Output('objdump', '-t', fname, raise_on_error=False)
    lines = stdout.splitlines()
    if patterns:
        re_syms = re.compile('|'.join(patterns))
    else:
        re_syms = None
    syms = {}
    syms_started = False
    for line in lines:
        if not line or not syms_started:
            if 'SYMBOL TABLE' in line:
                syms_started = True
            line = None  # Otherwise code coverage complains about 'continue'
            continue
        if re_syms and not re_syms.search(line):
            continue

        space_pos = line.find(' ')
        value, rest = line[:space_pos], line[space_pos + 1:]
        flags = rest[:7]
        parts = rest[7:].split()
        section, size =  parts[:2]
        if len(parts) > 2:
            name = parts[2]
            syms[name] = Symbol(section, int(value, 16), int(size,16),
                                flags[1] == 'w')
    return syms

def GetSymbolAddress(fname, sym_name):
    """Get a value of a symbol from an ELF file

    Args:
        fname: Filename of the ELF file to read
        patterns: List of regex patterns to search for, each a string

    Returns:
        Symbol value (as an integer) or None if not found
    """
    syms = GetSymbols(fname, [sym_name])
    sym = syms.get(sym_name)
    if not sym:
        return None
    return sym.address

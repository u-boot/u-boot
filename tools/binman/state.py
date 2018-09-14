# SPDX-License-Identifier: GPL-2.0+
# Copyright 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Holds and modifies the state information held by binman
#

import re
from sets import Set

import os
import tools

# Records the device-tree files known to binman, keyed by filename (e.g.
# 'u-boot-spl.dtb')
fdt_files = {}

# Arguments passed to binman to provide arguments to entries
entry_args = {}

def GetFdt(fname):
    """Get the Fdt object for a particular device-tree filename

    Binman keeps track of at least one device-tree file called u-boot.dtb but
    can also have others (e.g. for SPL). This function looks up the given
    filename and returns the associated Fdt object.

    Args:
        fname: Filename to look up (e.g. 'u-boot.dtb').

    Returns:
        Fdt object associated with the filename
    """
    return fdt_files[fname]

def GetFdtPath(fname):
    """Get the full pathname of a particular Fdt object

    Similar to GetFdt() but returns the pathname associated with the Fdt.

    Args:
        fname: Filename to look up (e.g. 'u-boot.dtb').

    Returns:
        Full path name to the associated Fdt
    """
    return fdt_files[fname]._fname

def SetEntryArgs(args):
    """Set the value of the entry args

    This sets up the entry_args dict which is used to supply entry arguments to
    entries.

    Args:
        args: List of entry arguments, each in the format "name=value"
    """
    global entry_args

    entry_args = {}
    if args:
        for arg in args:
            m = re.match('([^=]*)=(.*)', arg)
            if not m:
                raise ValueError("Invalid entry arguemnt '%s'" % arg)
            entry_args[m.group(1)] = m.group(2)

def GetEntryArg(name):
    """Get the value of an entry argument

    Args:
        name: Name of argument to retrieve

    Returns:
        String value of argument
    """
    return entry_args.get(name)

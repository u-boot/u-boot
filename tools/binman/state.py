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

# Set of all device tree files references by images
fdt_set = Set()

# Same as above, but excluding the main one
fdt_subset = Set()

# The DTB which contains the full image information
main_dtb = None

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

def Prepare(dtb):
    """Get device tree files ready for use

    This sets up a set of device tree files that can be retrieved by GetFdts().
    At present there is only one, that for U-Boot proper.

    Args:
        dtb: Main dtb
    """
    global fdt_set, fdt_subset, fdt_files, main_dtb
    # Import these here in case libfdt.py is not available, in which case
    # the above help option still works.
    import fdt
    import fdt_util

    # If we are updating the DTBs we need to put these updated versions
    # where Entry_blob_dtb can find them. We can ignore 'u-boot.dtb'
    # since it is assumed to be the one passed in with options.dt, and
    # was handled just above.
    main_dtb = dtb
    fdt_files.clear()
    fdt_files['u-boot.dtb'] = dtb
    fdt_set = Set()
    fdt_subset = Set()

def GetFdts():
    """Yield all device tree files being used by binman

    Yields:
        Device trees being used (U-Boot proper, SPL, TPL)
    """
    yield main_dtb

def GetUpdateNodes(node):
    """Yield all the nodes that need to be updated in all device trees

    The property referenced by this node is added to any device trees which
    have the given node. Due to removable of unwanted notes, SPL and TPL may
    not have this node.

    Args:
        node: Node object in the main device tree to look up

    Yields:
        Node objects in each device tree that is in use (U-Boot proper, which
            is node, SPL and TPL)
    """
    yield node

def AddZeroProp(node, prop):
    """Add a new property to affected device trees with an integer value of 0.

    Args:
        prop_name: Name of property
    """
    for n in GetUpdateNodes(node):
        n.AddZeroProp(prop)

def SetInt(node, prop, value):
    """Update an integer property in affected device trees with an integer value

    This is not allowed to change the size of the FDT.

    Args:
        prop_name: Name of property
    """
    for n in GetUpdateNodes(node):
        n.SetInt(prop, value)

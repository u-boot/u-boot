# SPDX-License-Identifier: GPL-2.0+
# Copyright 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Holds and modifies the state information held by binman
#

import hashlib
import re

import fdt
import os
import tools
import tout

# Records the device-tree files known to binman, keyed by entry type (e.g.
# 'u-boot-spl-dtb'). These are the output FDT files, which can be updated by
# binman. They have been copied to <xxx>.out files.
#
#   key: entry type
#   value: tuple:
#       Fdt object
#       Filename
#       Entry object, or None if not known
output_fdt_info = {}

# Prefix to add to an fdtmap path to turn it into a path to the /binman node
fdt_path_prefix = ''

# Arguments passed to binman to provide arguments to entries
entry_args = {}

# True to use fake device-tree files for testing (see U_BOOT_DTB_DATA in
# ftest.py)
use_fake_dtb = False

# The DTB which contains the full image information
main_dtb = None

# Allow entries to expand after they have been packed. This is detected and
# forces a re-pack. If not allowed, any attempted expansion causes an error in
# Entry.ProcessContentsUpdate()
allow_entry_expansion = True

def GetFdtForEtype(etype):
    """Get the Fdt object for a particular device-tree entry

    Binman keeps track of at least one device-tree file called u-boot.dtb but
    can also have others (e.g. for SPL). This function looks up the given
    entry and returns the associated Fdt object.

    Args:
        etype: Entry type of device tree (e.g. 'u-boot-dtb')

    Returns:
        Fdt object associated with the entry type
    """
    value = output_fdt_info.get(etype);
    if not value:
        return None
    return value[0]

def GetFdtPath(etype):
    """Get the full pathname of a particular Fdt object

    Similar to GetFdtForEtype() but returns the pathname associated with the
    Fdt.

    Args:
        etype: Entry type of device tree (e.g. 'u-boot-dtb')

    Returns:
        Full path name to the associated Fdt
    """
    return output_fdt_info[etype][0]._fname

def GetFdtContents(etype='u-boot-dtb'):
    """Looks up the FDT pathname and contents

    This is used to obtain the Fdt pathname and contents when needed by an
    entry. It supports a 'fake' dtb, allowing tests to substitute test data for
    the real dtb.

    Args:
        etype: Entry type to look up (e.g. 'u-boot.dtb').

    Returns:
        tuple:
            pathname to Fdt
            Fdt data (as bytes)
    """
    if etype not in output_fdt_info:
        return None, None
    if not use_fake_dtb:
        pathname = GetFdtPath(etype)
        data = GetFdtForEtype(etype).GetContents()
    else:
        fname = output_fdt_info[etype][1]
        pathname = tools.GetInputFilename(fname)
        data = tools.ReadFile(pathname)
    return pathname, data

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

def Prepare(images, dtb):
    """Get device tree files ready for use

    This sets up a set of device tree files that can be retrieved by
    GetAllFdts(). This includes U-Boot proper and any SPL device trees.

    Args:
        images: List of images being used
        dtb: Main dtb
    """
    global output_fdt_info, main_dtb, fdt_path_prefix
    # Import these here in case libfdt.py is not available, in which case
    # the above help option still works.
    import fdt
    import fdt_util

    # If we are updating the DTBs we need to put these updated versions
    # where Entry_blob_dtb can find them. We can ignore 'u-boot.dtb'
    # since it is assumed to be the one passed in with options.dt, and
    # was handled just above.
    main_dtb = dtb
    output_fdt_info.clear()
    fdt_path_prefix = ''
    output_fdt_info['u-boot-dtb'] = [dtb, 'u-boot.dtb', None]
    output_fdt_info['u-boot-spl-dtb'] = [dtb, 'spl/u-boot-spl.dtb', None]
    output_fdt_info['u-boot-tpl-dtb'] = [dtb, 'tpl/u-boot-tpl.dtb', None]
    if not use_fake_dtb:
        fdt_set = {}
        for image in images.values():
            fdt_set.update(image.GetFdts())
        for etype, other in fdt_set.items():
            entry, other_fname = other
            infile = tools.GetInputFilename(other_fname)
            other_fname_dtb = fdt_util.EnsureCompiled(infile)
            out_fname = tools.GetOutputFilename('%s.out' %
                    os.path.split(other_fname)[1])
            tools.WriteFile(out_fname, tools.ReadFile(other_fname_dtb))
            other_dtb = fdt.FdtScan(out_fname)
            output_fdt_info[etype] = [other_dtb, out_fname, entry]

def PrepareFromLoadedData(image):
    """Get device tree files ready for use with a loaded image

    Loaded images are different from images that are being created by binman,
    since there is generally already an fdtmap and we read the description from
    that. This provides the position and size of every entry in the image with
    no calculation required.

    This function uses the same output_fdt_info[] as Prepare(). It finds the
    device tree files, adds a reference to the fdtmap and sets the FDT path
    prefix to translate from the fdtmap (where the root node is the image node)
    to the normal device tree (where the image node is under a /binman node).

    Args:
        images: List of images being used
    """
    global output_fdt_info, main_dtb, fdt_path_prefix

    tout.Info('Preparing device trees')
    output_fdt_info.clear()
    fdt_path_prefix = ''
    output_fdt_info['fdtmap'] = [image.fdtmap_dtb, 'u-boot.dtb', None]
    main_dtb = None
    tout.Info("   Found device tree type 'fdtmap' '%s'" % image.fdtmap_dtb.name)
    for etype, value in image.GetFdts().items():
        entry, fname = value
        out_fname = tools.GetOutputFilename('%s.dtb' % entry.etype)
        tout.Info("   Found device tree type '%s' at '%s' path '%s'" %
                  (etype, out_fname, entry.GetPath()))
        entry._filename = entry.GetDefaultFilename()
        data = entry.ReadData()

        tools.WriteFile(out_fname, data)
        dtb = fdt.Fdt(out_fname)
        dtb.Scan()
        image_node = dtb.GetNode('/binman')
        if 'multiple-images' in image_node.props:
            image_node = dtb.GetNode('/binman/%s' % image.image_node)
        fdt_path_prefix = image_node.path
        output_fdt_info[etype] = [dtb, None, entry]
    tout.Info("   FDT path prefix '%s'" % fdt_path_prefix)


def GetAllFdts():
    """Yield all device tree files being used by binman

    Yields:
        Device trees being used (U-Boot proper, SPL, TPL)
    """
    if main_dtb:
        yield main_dtb
    for etype in output_fdt_info:
        dtb = output_fdt_info[etype][0]
        if dtb != main_dtb:
            yield dtb

def GetUpdateNodes(node, for_repack=False):
    """Yield all the nodes that need to be updated in all device trees

    The property referenced by this node is added to any device trees which
    have the given node. Due to removable of unwanted notes, SPL and TPL may
    not have this node.

    Args:
        node: Node object in the main device tree to look up
        for_repack: True if we want only nodes which need 'repack' properties
            added to them (e.g. 'orig-offset'), False to return all nodes. We
            don't add repack properties to SPL/TPL device trees.

    Yields:
        Node objects in each device tree that is in use (U-Boot proper, which
            is node, SPL and TPL)
    """
    yield node
    for dtb, fname, entry in output_fdt_info.values():
        if dtb != node.GetFdt():
            if for_repack and entry.etype != 'u-boot-dtb':
                continue
            other_node = dtb.GetNode(fdt_path_prefix + node.path)
            if other_node:
                yield other_node

def AddZeroProp(node, prop, for_repack=False):
    """Add a new property to affected device trees with an integer value of 0.

    Args:
        prop_name: Name of property
        for_repack: True is this property is only needed for repacking
    """
    for n in GetUpdateNodes(node, for_repack):
        n.AddZeroProp(prop)

def AddSubnode(node, name):
    """Add a new subnode to a node in affected device trees

    Args:
        node: Node to add to
        name: name of node to add

    Returns:
        New subnode that was created in main tree
    """
    first = None
    for n in GetUpdateNodes(node):
        subnode = n.AddSubnode(name)
        if not first:
            first = subnode
    return first

def AddString(node, prop, value):
    """Add a new string property to affected device trees

    Args:
        prop_name: Name of property
        value: String value (which will be \0-terminated in the DT)
    """
    for n in GetUpdateNodes(node):
        n.AddString(prop, value)

def SetInt(node, prop, value, for_repack=False):
    """Update an integer property in affected device trees with an integer value

    This is not allowed to change the size of the FDT.

    Args:
        prop_name: Name of property
        for_repack: True is this property is only needed for repacking
    """
    for n in GetUpdateNodes(node, for_repack):
        tout.Detail("File %s: Update node '%s' prop '%s' to %#x" %
                    (node.GetFdt().name, node.path, prop, value))
        n.SetInt(prop, value)

def CheckAddHashProp(node):
    hash_node = node.FindNode('hash')
    if hash_node:
        algo = hash_node.props.get('algo')
        if not algo:
            return "Missing 'algo' property for hash node"
        if algo.value == 'sha256':
            size = 32
        else:
            return "Unknown hash algorithm '%s'" % algo
        for n in GetUpdateNodes(hash_node):
            n.AddEmptyProp('value', size)

def CheckSetHashValue(node, get_data_func):
    hash_node = node.FindNode('hash')
    if hash_node:
        algo = hash_node.props.get('algo').value
        if algo == 'sha256':
            m = hashlib.sha256()
            m.update(get_data_func())
            data = m.digest()
        for n in GetUpdateNodes(hash_node):
            n.SetData('value', data)

def SetAllowEntryExpansion(allow):
    """Set whether post-pack expansion of entries is allowed

    Args:
       allow: True to allow expansion, False to raise an exception
    """
    global allow_entry_expansion

    allow_entry_expansion = allow

def AllowEntryExpansion():
    """Check whether post-pack expansion of entries is allowed

    Returns:
        True if expansion should be allowed, False if an exception should be
            raised
    """
    return allow_entry_expansion

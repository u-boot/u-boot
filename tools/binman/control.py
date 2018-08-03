# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

from collections import OrderedDict
import os
import re
import sys
import tools

import command
import elf
from image import Image
import tout

# List of images we plan to create
# Make this global so that it can be referenced from tests
images = OrderedDict()

# Records the device-tree files known to binman, keyed by filename (e.g.
# 'u-boot-spl.dtb')
fdt_files = {}

# Arguments passed to binman to provide arguments to entries
entry_args = {}


def _ReadImageDesc(binman_node):
    """Read the image descriptions from the /binman node

    This normally produces a single Image object called 'image'. But if
    multiple images are present, they will all be returned.

    Args:
        binman_node: Node object of the /binman node
    Returns:
        OrderedDict of Image objects, each of which describes an image
    """
    images = OrderedDict()
    if 'multiple-images' in binman_node.props:
        for node in binman_node.subnodes:
            images[node.name] = Image(node.name, node)
    else:
        images['image'] = Image('image', binman_node)
    return images

def _FindBinmanNode(dtb):
    """Find the 'binman' node in the device tree

    Args:
        dtb: Fdt object to scan
    Returns:
        Node object of /binman node, or None if not found
    """
    for node in dtb.GetRoot().subnodes:
        if node.name == 'binman':
            return node
    return None

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
    return fdt_files[fname]._fname

def SetEntryArgs(args):
    global entry_args

    entry_args = {}
    if args:
        for arg in args:
            m = re.match('([^=]*)=(.*)', arg)
            if not m:
                raise ValueError("Invalid entry arguemnt '%s'" % arg)
            entry_args[m.group(1)] = m.group(2)

def GetEntryArg(name):
    return entry_args.get(name)

def WriteEntryDocs(modules, test_missing=None):
    from entry import Entry
    Entry.WriteDocs(modules, test_missing)

def Binman(options, args):
    """The main control code for binman

    This assumes that help and test options have already been dealt with. It
    deals with the core task of building images.

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
    """
    global images

    if options.full_help:
        pager = os.getenv('PAGER')
        if not pager:
            pager = 'more'
        fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'README')
        command.Run(pager, fname)
        return 0

    # Try to figure out which device tree contains our image description
    if options.dt:
        dtb_fname = options.dt
    else:
        board = options.board
        if not board:
            raise ValueError('Must provide a board to process (use -b <board>)')
        board_pathname = os.path.join(options.build_dir, board)
        dtb_fname = os.path.join(board_pathname, 'u-boot.dtb')
        if not options.indir:
            options.indir = ['.']
        options.indir.append(board_pathname)

    try:
        # Import these here in case libfdt.py is not available, in which case
        # the above help option still works.
        import fdt
        import fdt_util

        tout.Init(options.verbosity)
        elf.debug = options.debug
        try:
            tools.SetInputDirs(options.indir)
            tools.PrepareOutputDir(options.outdir, options.preserve)
            SetEntryArgs(options.entry_arg)

            # Get the device tree ready by compiling it and copying the compiled
            # output into a file in our output directly. Then scan it for use
            # in binman.
            dtb_fname = fdt_util.EnsureCompiled(dtb_fname)
            fname = tools.GetOutputFilename('u-boot-out.dtb')
            with open(dtb_fname) as infd:
                with open(fname, 'wb') as outfd:
                    outfd.write(infd.read())
            dtb = fdt.FdtScan(fname)

            # Note the file so that GetFdt() can find it
            fdt_files['u-boot.dtb'] = dtb
            node = _FindBinmanNode(dtb)
            if not node:
                raise ValueError("Device tree '%s' does not have a 'binman' "
                                 "node" % dtb_fname)

            images = _ReadImageDesc(node)

            # Prepare the device tree by making sure that any missing
            # properties are added (e.g. 'pos' and 'size'). The values of these
            # may not be correct yet, but we add placeholders so that the
            # size of the device tree is correct. Later, in
            # SetCalculatedProperties() we will insert the correct values
            # without changing the device-tree size, thus ensuring that our
            # entry offsets remain the same.
            for image in images.values():
                if options.update_fdt:
                    image.AddMissingProperties()
                image.ProcessFdt(dtb)

            dtb.Pack()
            dtb.Flush()

            for image in images.values():
                # Perform all steps for this image, including checking and
                # writing it. This means that errors found with a later
                # image will be reported after earlier images are already
                # completed and written, but that does not seem important.
                image.GetEntryContents()
                image.GetEntryOffsets()
                image.PackEntries()
                image.CheckSize()
                image.CheckEntries()
                image.SetImagePos()
                if options.update_fdt:
                    image.SetCalculatedProperties()
                image.ProcessEntryContents()
                image.WriteSymbols()
                image.BuildImage()
                if options.map:
                    image.WriteMap()
            with open(fname, 'wb') as outfd:
                outfd.write(dtb.GetContents())
        finally:
            tools.FinaliseOutputDir()
    finally:
        tout.Uninit()

    return 0

# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Creates binary images from input files controlled by a description
#

from collections import OrderedDict
import os
import sys
import tools

import command
import fdt_select
import fdt_util
from image import Image
import tout

# List of images we plan to create
# Make this global so that it can be referenced from tests
images = OrderedDict()

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

def _FindBinmanNode(fdt):
    """Find the 'binman' node in the device tree

    Args:
        fdt: Fdt object to scan
    Returns:
        Node object of /binman node, or None if not found
    """
    for node in fdt.GetRoot().subnodes:
        if node.name == 'binman':
            return node
    return None

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
        tout.Init(options.verbosity)
        try:
            tools.SetInputDirs(options.indir)
            tools.PrepareOutputDir(options.outdir, options.preserve)
            fdt = fdt_select.FdtScan(dtb_fname)
            node = _FindBinmanNode(fdt)
            if not node:
                raise ValueError("Device tree '%s' does not have a 'binman' "
                                 "node" % dtb_fname)
            images = _ReadImageDesc(node)
            for image in images.values():
                # Perform all steps for this image, including checking and
                # writing it. This means that errors found with a later
                # image will be reported after earlier images are already
                # completed and written, but that does not seem important.
                image.GetEntryContents()
                image.GetEntryPositions()
                image.PackEntries()
                image.CheckSize()
                image.CheckEntries()
                image.ProcessEntryContents()
                image.BuildImage()
        finally:
            tools.FinaliseOutputDir()
    finally:
        tout.Uninit()

    return 0

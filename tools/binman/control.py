# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

from __future__ import print_function

from collections import OrderedDict
import os
import sys
import tools

import cbfs_util
import command
import elf
from image import Image
import state
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

def WriteEntryDocs(modules, test_missing=None):
    """Write out documentation for all entries

    Args:
        modules: List of Module objects to get docs for
        test_missing: Used for testing only, to force an entry's documeentation
            to show as missing even if it is present. Should be set to None in
            normal use.
    """
    from entry import Entry
    Entry.WriteDocs(modules, test_missing)


def ListEntries(image_fname, entry_paths):
    """List the entries in an image

    This decodes the supplied image and displays a table of entries from that
    image, preceded by a header.

    Args:
        image_fname: Image filename to process
        entry_paths: List of wildcarded paths (e.g. ['*dtb*', 'u-boot*',
                                                     'section/u-boot'])
    """
    image = Image.FromFile(image_fname)

    entries, lines, widths = image.GetListEntries(entry_paths)

    num_columns = len(widths)
    for linenum, line in enumerate(lines):
        if linenum == 1:
            # Print header line
            print('-' * (sum(widths) + num_columns * 2))
        out = ''
        for i, item in enumerate(line):
            width = -widths[i]
            if item.startswith('>'):
                width = -width
                item = item[1:]
            txt = '%*s  ' % (width, item)
            out += txt
        print(out.rstrip())


def ReadEntry(image_fname, entry_path, decomp=True):
    """Extract an entry from an image

    This extracts the data from a particular entry in an image

    Args:
        image_fname: Image filename to process
        entry_path: Path to entry to extract
        decomp: True to return uncompressed data, if the data is compress
            False to return the raw data

    Returns:
        data extracted from the entry
    """
    image = Image.FromFile(image_fname)
    entry = image.FindEntryPath(entry_path)
    return entry.ReadData(decomp)


def Binman(args):
    """The main control code for binman

    This assumes that help and test options have already been dealt with. It
    deals with the core task of building images.

    Args:
        args: Command line arguments Namespace object
    """
    global images

    if args.full_help:
        pager = os.getenv('PAGER')
        if not pager:
            pager = 'more'
        fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'README')
        command.Run(pager, fname)
        return 0

    if args.cmd == 'ls':
        ListEntries(args.image, args.paths)
        return 0

    # Try to figure out which device tree contains our image description
    if args.dt:
        dtb_fname = args.dt
    else:
        board = args.board
        if not board:
            raise ValueError('Must provide a board to process (use -b <board>)')
        board_pathname = os.path.join(args.build_dir, board)
        dtb_fname = os.path.join(board_pathname, 'u-boot.dtb')
        if not args.indir:
            args.indir = ['.']
        args.indir.append(board_pathname)

    try:
        # Import these here in case libfdt.py is not available, in which case
        # the above help option still works.
        import fdt
        import fdt_util

        tout.Init(args.verbosity)
        elf.debug = args.debug
        cbfs_util.VERBOSE = args.verbosity > 2
        state.use_fake_dtb = args.fake_dtb
        try:
            tools.SetInputDirs(args.indir)
            tools.PrepareOutputDir(args.outdir, args.preserve)
            tools.SetToolPaths(args.toolpath)
            state.SetEntryArgs(args.entry_arg)

            # Get the device tree ready by compiling it and copying the compiled
            # output into a file in our output directly. Then scan it for use
            # in binman.
            dtb_fname = fdt_util.EnsureCompiled(dtb_fname)
            fname = tools.GetOutputFilename('u-boot.dtb.out')
            tools.WriteFile(fname, tools.ReadFile(dtb_fname))
            dtb = fdt.FdtScan(fname)

            node = _FindBinmanNode(dtb)
            if not node:
                raise ValueError("Device tree '%s' does not have a 'binman' "
                                 "node" % dtb_fname)

            images = _ReadImageDesc(node)

            if args.image:
                skip = []
                new_images = OrderedDict()
                for name, image in images.items():
                    if name in args.image:
                        new_images[name] = image
                    else:
                        skip.append(name)
                images = new_images
                if skip and args.verbosity >= 2:
                    print('Skipping images: %s' % ', '.join(skip))

            state.Prepare(images, dtb)

            # Prepare the device tree by making sure that any missing
            # properties are added (e.g. 'pos' and 'size'). The values of these
            # may not be correct yet, but we add placeholders so that the
            # size of the device tree is correct. Later, in
            # SetCalculatedProperties() we will insert the correct values
            # without changing the device-tree size, thus ensuring that our
            # entry offsets remain the same.
            for image in images.values():
                image.ExpandEntries()
                if args.update_fdt:
                    image.AddMissingProperties()
                image.ProcessFdt(dtb)

            for dtb_item in state.GetFdts():
                dtb_item.Sync(auto_resize=True)
                dtb_item.Pack()
                dtb_item.Flush()

            for image in images.values():
                # Perform all steps for this image, including checking and
                # writing it. This means that errors found with a later
                # image will be reported after earlier images are already
                # completed and written, but that does not seem important.
                image.GetEntryContents()
                image.GetEntryOffsets()

                # We need to pack the entries to figure out where everything
                # should be placed. This sets the offset/size of each entry.
                # However, after packing we call ProcessEntryContents() which
                # may result in an entry changing size. In that case we need to
                # do another pass. Since the device tree often contains the
                # final offset/size information we try to make space for this in
                # AddMissingProperties() above. However, if the device is
                # compressed we cannot know this compressed size in advance,
                # since changing an offset from 0x100 to 0x104 (for example) can
                # alter the compressed size of the device tree. So we need a
                # third pass for this.
                passes = 3
                for pack_pass in range(passes):
                    try:
                        image.PackEntries()
                        image.CheckSize()
                        image.CheckEntries()
                    except Exception as e:
                        if args.map:
                            fname = image.WriteMap()
                            print("Wrote map file '%s' to show errors"  % fname)
                        raise
                    image.SetImagePos()
                    if args.update_fdt:
                        image.SetCalculatedProperties()
                        for dtb_item in state.GetFdts():
                            dtb_item.Sync()
                    sizes_ok = image.ProcessEntryContents()
                    if sizes_ok:
                        break
                    image.ResetForPack()
                if not sizes_ok:
                    image.Raise('Entries expanded after packing (tried %s passes)' %
                                passes)

                image.WriteSymbols()
                image.BuildImage()
                if args.map:
                    image.WriteMap()

            # Write the updated FDTs to our output files
            for dtb_item in state.GetFdts():
                tools.WriteFile(dtb_item._fname, dtb_item.GetContents())

        finally:
            tools.FinaliseOutputDir()
    finally:
        tout.Uninit()

    return 0

# SPDX-License-Identifier:      GPL-2.0+
# Copyright 2024 Google LLC
# Written by Simon Glass <sjg@chromium.org>

"""Entry-type module for producing multiple alternate sections"""

import glob
import os

from binman.entry import EntryArg
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_alternates_fdt(Entry_section):
    """Entry that generates alternative sections for each devicetree provided

    When creating an image designed to boot on multiple models, each model
    requires its own devicetree. This entry deals with selecting the correct
    devicetree from a directory containing them. Each one is read in turn, then
    used to produce section contents which are written to a file. This results
    in a number of images, one for each model.

    For example this produces images for each .dtb file in the 'dtb' directory::

        alternates-fdt {
            fdt-list-dir = "dtb";
            filename-pattern = "NAME.bin";
            fdt-phase = "tpl";

            section {
                u-boot-tpl {
                };
            };
        };

    Each output file is named based on its input file, so an input file of
    `model1.dtb` results in an output file of `model1.bin` (i.e. the `NAME` in
    the `filename-pattern` property is replaced with the .dtb basename).

    Note that this entry type still produces contents for the 'main' image, in
    that case using the normal dtb provided to Binman, e.g. `u-boot-tpl.dtb`.
    But that image is unlikely to be useful, since it relates to whatever dtb
    happened to be the default when U-Boot builds
    (i.e. `CONFIG_DEFAULT_DEVICE_TREE`). However, Binman ensures that the size
    of each of the alternates is the same as the 'default' one, so they can in
    principle be 'slotted in' to the appropriate place in the main image.

    The optional `fdt-phase` property indicates the phase to build. In this
    case, it etype runs fdtgrep to obtain the devicetree subset for that phase,
    respecting the `bootph-xxx` tags in the devicetree.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.fdt_list_dir = None
        self.filename_pattern = None
        self.required_props = ['fdt-list-dir']
        self._cur_fdt = None
        self._fdt_phase = None
        self.fdtgrep = None
        self._fdt_dir = None
        self._fdts = None
        self._fname_pattern = None
        self._remove_props = None
        self.alternates = None

    def ReadNode(self):
        """Read properties from the node"""
        super().ReadNode()
        self._fdt_dir = fdt_util.GetString(self._node, 'fdt-list-dir')
        fname = tools.get_input_filename(self._fdt_dir)
        fdts = glob.glob('*.dtb', root_dir=fname)
        self._fdts = [os.path.splitext(f)[0] for f in fdts]

        self._fdt_phase = fdt_util.GetString(self._node, 'fdt-phase')

        # This is used by Image.WriteAlternates()
        self.alternates = self._fdts

        self._fname_pattern = fdt_util.GetString(self._node, 'filename-pattern')

        self._remove_props = []
        props, = self.GetEntryArgsOrProps(
            [EntryArg('of-spl-remove-props', str)], required=False)
        if props:
            self._remove_props = props.split()

    def FdtContents(self, fdt_etype):
        # If there is no current FDT, just use the normal one
        if not self._cur_fdt:
            return self.section.FdtContents(fdt_etype)

        # Find the file to use
        fname = os.path.join(self._fdt_dir, f'{self._cur_fdt}.dtb')
        infile = tools.get_input_filename(fname)

        # Run fdtgrep if needed, to remove unwanted nodes and properties
        if self._fdt_phase:
            uniq = self.GetUniqueName()
            outfile = tools.get_output_filename(
                f'{uniq}.{self._cur_fdt}-{self._fdt_phase}.dtb')
            self.fdtgrep.create_for_phase(infile, self._fdt_phase, outfile,
                                          self._remove_props)
            return outfile, tools.read_file(outfile)
        return fname, tools.read_file(infile)

    def ProcessWithFdt(self, alt):
        """Produce the contents of this entry, using a particular FDT blob

        Args:
            alt (str): Name of the alternate

        Returns:
            tuple:
                str: Filename to use for the alternate's .bin file
                bytes: Contents of this entry's section, using the selected FDT
        """
        pattern = self._fname_pattern or 'NAME.bin'
        fname = pattern.replace('NAME', alt)

        data = b''
        try:
            self._cur_fdt = alt
            self.ProcessContents()
            data = self.GetPaddedData()
        finally:
            self._cur_fdt = None
        return fname, data

    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.fdtgrep = self.AddBintool(btools, 'fdtgrep')

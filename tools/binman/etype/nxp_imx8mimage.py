# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023-2024 Marek Vasut <marex@denx.de>
# Written with much help from Simon Glass <sjg@chromium.org>
#
# Entry-type module for generating the i.MX8M mkimage -T imx8mimage
# configuration file and invocation of mkimage -T imx8mimage on the
# configuration file and input data.
#

from collections import OrderedDict

from binman.entry import Entry
from binman.etype.mkimage import Entry_mkimage
from binman.etype.section import Entry_section
from binman import elf
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_nxp_imx8mimage(Entry_mkimage):
    """NXP i.MX8M imx8mimage .cfg file generator and mkimage invoker

    Properties / Entry arguments:
        - nxp,boot-from - device to boot from (e.g. 'sd')
        - nxp,loader-address - loader address (SPL text base)
        - nxp,rom-version - BootROM version ('2' for i.MX8M Nano and Plus)
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['nxp,boot-from', 'nxp,rom-version',
                               'nxp,loader-address']

    def ReadNode(self):
        super().ReadNode()
        self.boot_from = fdt_util.GetString(self._node, 'nxp,boot-from')
        self.loader_address = fdt_util.GetInt(self._node, 'nxp,loader-address')
        self.rom_version = fdt_util.GetInt(self._node, 'nxp,rom-version')
        self.ReadEntries()

    def BuildSectionData(self, required):
        data, input_fname, uniq = self.collect_contents_to_file(
            self._entries.values(), 'input')
        # Generate mkimage configuration file similar to imx8mimage.cfg
        # and pass it to mkimage to generate SPL image for us here.
        cfg_fname = tools.get_output_filename('nxp.imx8mimage.cfg.%s' % uniq)
        with open(cfg_fname, 'w') as outf:
            print('ROM_VERSION v%d' % self.rom_version, file=outf)
            print('BOOT_FROM %s' % self.boot_from, file=outf)
            print('LOADER %s %#x' % (input_fname, self.loader_address), file=outf)

        output_fname = tools.get_output_filename(f'cfg-out.{uniq}')
        args = ['-d', input_fname, '-n', cfg_fname, '-T', 'imx8mimage',
                output_fname]
        if self.mkimage.run_cmd(*args) is not None:
            return tools.read_file(output_fname)
        else:
            # Bintool is missing; just use the input data as the output
            self.record_missing_bintool(self.mkimage)
            return data

    def SetImagePos(self, image_pos):
        # Customized SoC specific SetImagePos which skips the mkimage etype
        # implementation and removes the 0x48 offset introduced there. That
        # offset is only used for uImage/fitImage, which is not the case in
        # here.
        upto = 0x00
        for entry in super().GetEntries().values():
            entry.SetOffsetSize(upto, None)

            # Give up if any entries lack a size
            if entry.size is None:
                return
            upto += entry.size

        Entry_section.SetImagePos(self, image_pos)

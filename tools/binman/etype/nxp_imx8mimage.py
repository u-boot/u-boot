# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023-2024 Marek Vasut <marex@denx.de>
# Written with much help from Simon Glass <sjg@chromium.org>
#
# Entry-type module for generating the i.MX8M mkimage -T imx8mimage
# configuration file and invocation of mkimage -T imx8mimage on the
# configuration file and input data.
#

import struct

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

    Properties / Entry arguments for FSPI boot mode (nxp,boot-from = "fspi"):
        - nxp,fspi-columnaddresswidth - FSPI column address width
            (3 - HyperFlash, 12/13 - Serial NAND, 0 - Otherwise (default))
        - nxp,fspi-controllermisc-diffclk - FSPI differential clock enable (default off)
        - nxp,fspi-controllermisc-wordaddr - FSPI word addressable enable (default off)
        - nxp,fspi-controllermisc-safecfg - FSPI safe configuration frequency (default off)
        - nxp,fspi-controllermisc-padovr - FSPI pad setting override (default off)
        - nxp,fspi-controllermisc-ddrmode - FSPI DDR mode (default off)
        - nxp,fspi-lutcustomseq - FSPI use LUT sequence parameters (default off)
        - nxp,fspi-devicetype - FSPI device type
            (1 - SPI NOR (default), 2 - Serial NAND)
        - nxp,fspi-flasha1size - FSPI device size (default 0x10000000)
        - nxp,fspi-flashpadtype - FSPI flash pad type
            (1 - Single pad (default), 2 - Dual pads, 4 - Quad pads, 8 - Octal pads)
        - nxp,fspi-readsampleclksrc - FSPI clock source
            (0 - Internal loopback (default), 1 - loopback from DQS pad, 3 - Flash provided DQS).
        - nxp,fspi-serialclkfreq - FSPI clock frequency
            (1 - 30 MHz, 2 - 50 MHz (default), 3 - 60 MHz, 4 - 75 MHz, 5 - 80 MHz,
             6 - 100 MHz, 7 - 133 MHz, 8 - 166 MHz).
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['nxp,boot-from', 'nxp,rom-version',
                               'nxp,loader-address']

    def ReadNode(self):
        super().ReadNode()
        self.boot_from = fdt_util.GetString(self._node, 'nxp,boot-from')
        self.fspi_columnadresswidth = fdt_util.GetInt(self._node, 'nxp,fspi-columnaddresswidth', 0)
        self.fspi_controllermisc_diffclk = fdt_util.GetBool(self._node, 'nxp,fspi-controllermisc-diffclk')
        self.fspi_controllermisc_wordaddr = fdt_util.GetBool(self._node, 'nxp,fspi-controllermisc-wordaddr')
        self.fspi_controllermisc_safecfg = fdt_util.GetBool(self._node, 'nxp,fspi-controllermisc-safecfg')
        self.fspi_controllermisc_padovr = fdt_util.GetBool(self._node, 'nxp,fspi-controllermisc-padovr')
        self.fspi_controllermisc_ddrmode = fdt_util.GetBool(self._node, 'nxp,fspi-controllermisc-ddrmode')
        self.fspi_devicetype = fdt_util.GetInt(self._node, 'nxp,fspi-devicetype', 1)
        self.fspi_flasha1size = fdt_util.GetInt(self._node, 'nxp,fspi-flasha1size', 0x10000000)
        self.fspi_flashpadtype = fdt_util.GetInt(self._node, 'nxp,fspi-flashpadtype', 1)
        self.fspi_lutcustomseq = fdt_util.GetBool(self._node, 'nxp,fspi-lutcustomseq')
        self.fspi_readsampleclksrc = fdt_util.GetInt(self._node, 'nxp,fspi-readsampleclksrc', 0)
        self.fspi_serialclkfreq = fdt_util.GetInt(self._node, 'nxp,fspi-serialclkfreq', 2)
        self.loader_address = fdt_util.GetInt(self._node, 'nxp,loader-address')
        self.rom_version = fdt_util.GetInt(self._node, 'nxp,rom-version')
        if not self.fspi_columnadresswidth in [ 0, 3, 12, 13 ]:
            self.Raise('nxp,fspi-columnaddresswidth can be 0, 3, 12, 13 only.')
        if not self.fspi_devicetype in [ 1, 2 ]:
            self.Raise('nxp,fspi-devicetype can be 1, 2 only.')
        if not self.fspi_flashpadtype in [ 1, 2, 4, 8 ]:
            self.Raise('nxp,fspi-flashpadtype can be 1, 2, 4, 8 only.')
        if not self.fspi_readsampleclksrc in [ 0, 1, 3 ]:
            self.Raise('nxp,fspi-readsampleclksrc can be 0, 1, 3 only.')
        if not self.fspi_serialclkfreq in [ 1, 2, 3, 4, 5, 6, 7, 8 ]:
            self.Raise('nxp,fspi-serialclkfreq can be 1..8 only.')
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
            outdata = tools.read_file(output_fname)
            if self.boot_from == 'fspi':
                # 0x00 ... Tag
                spidata = struct.pack('<I', 0x42464346)
                # 0x04 ... Version
                spidata += struct.pack('<I', 0x56010000)
                # 0x08 ... Reserved
                spidata += struct.pack('<I', 0)
                # 0x0c ... readSampleClkSrc (LSByte at 0x0c), dataHoldTime,
                #          dataSetupTime, columnAdressWidth (MSByte at 0x0f)
                spidata += struct.pack('<I', 0x00030300 |
                    (self.fspi_columnadresswidth << 24) |
                    self.fspi_readsampleclksrc)

                # 0x10..0x3f ... Padding
                spidata += tools.get_bytes(0, 0x30)

                # 0x40 ... controllerMiscOption
                spidata += struct.pack('<I',
                    ((1 << 0) if self.fspi_controllermisc_diffclk else 0) |
                    ((1 << 3) if self.fspi_controllermisc_wordaddr else 0) |
                    ((1 << 4) if self.fspi_controllermisc_safecfg else 0) |
                    ((1 << 5) if self.fspi_controllermisc_padovr else 0) |
                    ((1 << 6) if self.fspi_controllermisc_ddrmode else 0))

                # 0x44 ... deviceType (LSByte at 0x44), sflashPadType,
                #          serialClkFreq, lutCustomSeqEnable (MSByte at 0x47)
                spidata += struct.pack('<I',
                    ((1 << 24) if self.fspi_lutcustomseq else 0) |
                    (self.fspi_serialclkfreq << 16) |
                    (self.fspi_flashpadtype << 8) |
                    self.fspi_devicetype)

                # 0x48..0x4f ... Padding
                spidata += tools.get_bytes(0, 0x8)

                # 0x50 ... flashA1Size
                spidata += struct.pack('<I', self.fspi_flasha1size)

                # 0x54..0x7f ... Padding
                spidata += tools.get_bytes(0, 0x2c)

                # 0x80 ... lookupTable
                spidata += struct.pack('<I', 0x0818040b)
                spidata += struct.pack('<I', 0x24043008)

                # 0x88..0xfff ... Padding (end of FSPI block is 0x1bf, align to 4k)
                spidata += tools.get_bytes(0, 0x1000 - len(spidata))
                outdata = spidata + outdata
            return outdata
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

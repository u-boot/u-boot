# SPDX-License-Identifier: GPL-2.0+
# Copyright 2026 (C) Bootlin
# Author: Jérémie Dautheribes <jeremie.dautheribes@bootlin.com>
#
# Derived from nxp_imx8mcst.py
# Copyright 2023-2024 Marek Vasut <marex@denx.de>

# Entry-type module for NXP i.MX Code Signing Tool (CST) base class
#

import configparser
import os

from binman.etype.mkimage import Entry_mkimage
from binman.etype.section import Entry_section
from dtoc import fdt_util
from u_boot_pylib import tools


class Entry_nxp_imxcst(Entry_mkimage):
    """NXP i.MX CST .cfg file generator and cst invoker base class

    Properties / Entry arguments:
        - nxp,srk-table - full path to SRK_1_2_3_4_table.bin
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.cst = None
        self.srk_table = None

    def ReadNode(self):
        super().ReadNode()
        self.srk_table = os.getenv(
            'SRK_TABLE', fdt_util.GetString(self._node, 'nxp,srk-table',
                                            'SRK_1_2_3_4_table.bin'))

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

    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.cst = self.AddBintool(btools, 'cst')

    def write_input_data(self, data, uniq):
        """Write input data to a temporary file for CST

        Args:
            data: Data to write
            uniq: Unique string for naming output files

        Returns:
            str: Path to the written file
        """
        output_dname = tools.get_output_filename(f'nxp.cst-input-data.{uniq}')
        tools.write_file(output_dname, data)
        return output_dname

    def get_config(self, template):
        """Get a ConfigParser loaded with the given template

        Args:
            template: Configuration template string

        Returns:
            ConfigParser instance
        """
        config = configparser.ConfigParser()
        # Do not make key names lowercase
        config.optionxform = str
        config.read_string(template)
        return config

    def write_config(self, config, uniq):
        """Write ConfigParser object to a temporary file for CST

        Args:
            config: ConfigParser instance
            uniq: Unique string for naming output files

        Returns:
            str: Path to the written configuration file
        """
        cfg_fname = tools.get_output_filename(f'nxp.csf-config-txt.{uniq}')
        with open(cfg_fname, 'w') as cfgf:
            config.write(cfgf)
        return cfg_fname

    def run_cst(self, cfg_fname, uniq, backend=None):
        """Run CST tool with given configuration file

        Args:
            cfg_fname: Filename of the CST configuration file
            uniq: Unique string for naming output files
            backend: Optional backend (e.g. 'ssl' or 'pkcs11')

        Returns:
            bytes: Output data from CST, or None if bintool is missing
        """
        output_fname = tools.get_output_filename(f'nxp.csf-output-blob.{uniq}')
        args = ['-i', cfg_fname, '-o', output_fname]
        if backend:
            args.extend(['-b', backend])
        if self.cst.run_cmd(*args) is not None:
            return tools.read_file(output_fname)
        self.record_missing_bintool(self.cst)
        return None

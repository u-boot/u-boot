# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for producing a EFI capsule through a
# config file.
#

import os

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools

class Entry_efi_capsule_cfg_file(Entry):
    """Entry for generating EFI capsules through config file

    This is an entry for generating EFI capsules through a
    config file.

    The parameters needed for generation of the capsules are
    provided through a config file. This results in generation
    of one or multiple capsules, corresponding to the entries
    in the config file.

    Properties / Entry arguments:
    - cfg-file: Config file for providing capsule parameters. These are
      parameters needed for generating the capsules. The parameters can
      be listed by running the './tools/mkeficapsule -h' command.

    For more details on the description of the capsule format, and the capsule
    update functionality, refer Section 8.5 and Chapter 23 in the `UEFI
    specification`_.

    A typical capsule entry node would then look something like this

    capsule {
            type = "efi-capsule-cfg-file";
            cfg-file = "path/to/the/config/file";
    };

    In the above example, the entry only contains the path to the config file.
    All parameters needed for generation of the capsule, including the input
    payload image and the output capsule file are specified through the entries
    in the config file.

    .. _`UEFI specification`: https://uefi.org/sites/default/files/resources/UEFI_Spec_2_10_Aug29.pdf
"""
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['cfg-file']

    def ReadNode(self):
        super().ReadNode()

        self.cfg_file = fdt_util.GetString(self._node, 'cfg-file')
        if not os.path.isabs(self.cfg_file):
            self.cfg_file = tools.get_input_filename(self.cfg_file)

    def _GenCapsule(self):
        self.mkeficapsule.generate_capsule_cfg_file(self.cfg_file)

    def ObtainContents(self):
        self._GenCapsule()

    def AddBintools(self, btools):
        self.mkeficapsule = self.AddBintool(btools, 'mkeficapsule')

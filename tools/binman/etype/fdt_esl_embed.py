# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2023 Linaro Limited
#
# Entry-type module for Embedding ESL into FDTs
#

import os

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools
from u_boot_pylib import tout

class Entry_fdt_esl_embed(Entry):
    """Entry for embedding the ESL file into devicetree(s)

    This is an entry for embedding the EFI Signature List(ESL)
    file into one or multiple devicetrees.

    The ESL is a public key which is inserted as a property
    under the signature node in the DTB. One or more dtb files
    can be specified under the fdt-esl-embed node to embed the
    ESL file.

    Properties / Entry arguments:
        - esl-file: Path to the ESL file. Usually specified
          through the CONFIG_EFI_CAPSULE_ESL_FILE Kconfig
          symbol. Mandatory property.
        - dtb: A list of DTB file names into which the ESL
          needs to be embedded. Provided as a string list,
          similar to the compatible property. At least one
          DTB file needs to be specified. Mandatory property.
        - concat: An optional property which specifies the
          files which need to be concatenated, along with
          the third file into which the result needs to be
          put into.
          E.g. concat = "./u-boot-nodtb.bin",  "./u-boot.dtb", "./u-boot.bin";
          The above concatenates u-boot-nodtb.bin and the
          u-boot.dtb files, and puts the result into
          u-boot.bin.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.dtbs = []

    def ReadNode(self):
        super().ReadNode()

        self.esl = fdt_util.GetString(self._node, 'esl-file')
        self.dtbs = fdt_util.GetStringList(self._node, 'dtb')
        self.concat = fdt_util.GetStringList(self._node, 'concat')

        if not self.esl:
            self.Raise('Path to the ESL file needs to be provided')
        if not self.dtbs:
            self.Raise('At least one DTB needs to be provided')

    def dtb_embed_esl(self):
        self.keypair = os.path.split(self.esl)

        for dtb in self.dtbs:
            self.fdt_add_pubkey.add_esl(self.keypair,
                                        tools.get_input_filename(dtb))

    def concat_binaries(self):
        bins = self.concat
        bin0_fname = tools.get_input_filename(bins[0])
        bin1_fname = tools.get_input_filename(bins[1])
        bin2_fname = tools.get_input_filename(bins[2])
        with open(bin0_fname, 'rb') as fd1, open(bin1_fname, 'rb') as fd2, open(bin2_fname, 'wb') as fd3:
            bin1 = fd1.read()
            bin2 = fd2.read()
            bin1 += bin2

            fd3.write(bin1)

    def ObtainContents(self):
        self.dtb_embed_esl()

        if self.concat:
            self.concat_binaries()

    def AddBintools(self, btools):
        self.fdt_add_pubkey = self.AddBintool(btools, 'fdt_add_pubkey')

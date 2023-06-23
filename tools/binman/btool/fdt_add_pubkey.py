# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Linaro Limited
#
"""Bintool implementation for fdt_add_pubkey tool

fdt_add_pubkey is a tool used for embedding a public key
into the DTB file.

Currently, this is being used for embedding the EFI Signature
List(ESL) file into the DTB provided. The contents of the ESL
file get added as a property, capsule-key under the signature
node.

The following are the command line options to be provided to the tool.
Options:
        -a <algo>       Cryptographic algorithm. Optional parameter, default value: sha1,rsa2048
        -e <esl file>   EFI Signature List(ESL) file to embed into the FDT
        -k <keydir>     Directory with public key. Optional parameter, default value: .
        -n <keyname>    Public key name. Optional parameter, default value: key
        -r <conf|image> Required: If present this indicates that the key must be verified for the image / configuration to be considered valid.
        <fdt blob>      FDT blob file for adding of the public key. Required parameter.

"""

from binman import bintool

class Bintoolfdt_add_pubkey(bintool.Bintool):
    """Handles the 'fdt_add_pubkey' tool

    This bintool supports running the fdt_add_pubkey tool for
    embedding the public key into the dtb file provided.

    Currently, this is being used for embedding the EFI Signature
    List(ESL) file into the DTB provided.
    """
    def __init__(self, name):
        super().__init__(name, 'Tool for generating adding pubkey to platform dtb')

    def add_esl(self, esl_fname, dtb_fname):
        """Add an ESL public key into the DTB

        Args:
            esl_fname: Path to the ESL file
            dtb_name: Path to the DTB file

        Returns:
            None
        """
        args = [
            f'-e',
            esl_fname,
            dtb_fname
        ]

        self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for fdt_add_pubkey

        This builds the tool from source

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        if method != bintool.FETCH_BUILD:
            return None
        result = self.build_from_git(
            'https://source.denx.de/u-boot/u-boot.git',
            'tools',
            'tools/fdt_add_pubkey')
        return result

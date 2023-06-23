# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Linaro Limited
#
"""Bintool implementation for embed_capsule_key tool

embed_capsule_key is a tool used for embedding a public key
into the DTB file.

This tool is being used for embedding the EFI Signature
List(ESL) file into the DTB provided. The contents of the ESL
file get added as a property, capsule-key under the signature
node.

"""

from binman import bintool

class Bintoolembed_capsule_key(bintool.Bintool):
    """Handles the 'embed_capsule_key' tool

    This bintool supports running the embed_capsule_key tool for
    embedding the public key into the dtb file provided.

    Used for embedding the EFI Signature List(ESL) file into the
    DTB provided.
    """
    def __init__(self, name):
        super().__init__(name, 'Tool for adding pubkey to platform dtb')

    def add_esl(self, esl_fname, dtb_fname):
        """Add an ESL public key into the DTB

        Args:
            esl_fname: Path to the ESL file
            dtb_fname: Path to the DTB file

        Returns:
            None
        """
        args = [
            esl_fname,
            dtb_fname
        ]

        self.run_cmd(*args)

    def fetch(self, method):
        """Fetch handler for embed_capsule_key

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
            'tools/embed_capsule_key')
        return result

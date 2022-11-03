# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Linaro Limited
#

from binman import bintool

class Bintoolfdt_add_pubkey(bintool.Bintool):
    def __init__(self, name):
        super().__init__(name, 'Tool for generating adding pubkey to platform dtb')

    def add_esl(self, keypair, dtb_fname):
        args = [
            f'-k',
            keypair[0],
            f'-n',
            keypair[1],
            dtb_fname
        ]

        return self.run_cmd(*args)

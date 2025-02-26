# SPDX-License-Identifier: GPL-2.0+
# Copyright 2024 Marek Vasut <marex@denx.de>
#
"""Bintool implementation for cst"""

import re

from binman import bintool

class Bintoolcst(bintool.Bintool):
    """Image generation for U-Boot

    This bintool supports running `cst` with some basic parameters as
    needed by binman.

    cst (imx code signing tool) is used for sigining bootloader binaries for
    various i.MX SoCs.

    See `Code Signing Tool Users Guide`_ for more information.

    .. _`Code Signing Tool Users Guide`:
        https://community.nxp.com/pwmxy87654/attachments/pwmxy87654/imx-processors/202591/1/CST_UG.pdf
    """
    def __init__(self, name):
        super().__init__(name, 'Sign NXP i.MX image')

    # pylint: disable=R0913
    def run(self, output_fname=None):
        """Run cst

        Args:
            output_fname: Output filename to write to
        """
        args = []
        if output_fname:
            args += ['-o', output_fname]
        return self.run_cmd(*args)

    def fetch(self, method):
        """Build cst from git"""
        if method != bintool.FETCH_BUILD:
            return None

        from platform import architecture
        arch = 'linux64' if architecture()[0] == '64bit' else 'linux32'
        result = self.build_from_git(
            'https://gitlab.apertis.org/pkg/imx-code-signing-tool',
            ['all'],
            f'code/obj.{arch}/cst',
            flags=[f'OSTYPE={arch}', 'ENCRYPTION=yes'],
            git_branch='debian/unstable',
            make_path=f'code/obj.{arch}/')
        return result

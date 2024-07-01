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
        """Fetch handler for cst

        This installs cst using the apt utility.

        Args:
            method (FETCH_...): Method to use

        Returns:
            True if the file was fetched and now installed, None if a method
            other than FETCH_BIN was requested

        Raises:
            Valuerror: Fetching could not be completed
        """
        if method != bintool.FETCH_BIN:
            return None
        return self.apt_install('imx-code-signing-tool')

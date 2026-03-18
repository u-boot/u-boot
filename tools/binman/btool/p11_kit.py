# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2026 Mettler Toledo Technologies GmbH
#
"""Bintool implementation for p11-kit"""

from binman import bintool


class Bintoolp11_kit(bintool.Bintool):
    """p11-kit -- support tool for pkcs#11 libraries"""
    def __init__(self, name):
        super().__init__('p11-kit',
                         'Pkcs11 library modules tool',
                         version_args='list modules')

    def fetch(self, method):
        """Install p11-kit via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('p11-kit')

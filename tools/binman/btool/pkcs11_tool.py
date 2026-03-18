# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2026 Mettler Toledo Technologies GmbH
#
"""Bintool implementation for pkcs11-tool"""

from binman import bintool


class Bintoolpkcs11_tool(bintool.Bintool):
    """pkcs11-tool -- support tool for managing pkcs#11 tokens"""
    def __init__(self, name):
        super().__init__('pkcs11-tool',
                         'PKCS #11 tokens managing tool',
                         version_args='--show-info')

    def fetch(self, method):
        """Install opensc via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('opensc')

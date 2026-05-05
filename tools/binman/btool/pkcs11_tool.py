# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2026 Mettler Toledo Technologies GmbH
#
"""Bintool implementation for pkcs11-tool"""

from binman import bintool


class Bintoolpkcs11_tool(bintool.Bintool):
    """pkcs11-tool -- support tool for managing pkcs#11 tokens

    This bintool wraps the `pkcs11-tool` command from the OpenSC project for
    managing objects stored in PKCS#11 tokens. Binman uses this wrapper only
    to check that pkcs11-tool is installed (and to fetch it if missing); any
    actual key or token management for signing FIT images or capsules is done
    outside binman.

    See https://github.com/OpenSC/OpenSC/wiki for more details.
    """
    def __init__(self, name):
        super().__init__('pkcs11-tool',
                         'PKCS #11 tokens managing tool',
                         version_args='--show-info')

    def fetch(self, method):
        """Install opensc via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('opensc')

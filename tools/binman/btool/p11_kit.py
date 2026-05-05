# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2026 Mettler Toledo Technologies GmbH
#
"""Bintool implementation for p11-kit"""

from binman import bintool


class Bintoolp11_kit(bintool.Bintool):
    """p11-kit -- support tool for pkcs#11 libraries

    This bintool wraps the `p11-kit` command, a support tool for PKCS#11
    modules. Binman uses this wrapper only to check that p11-kit is installed
    (and to fetch it if missing); any actual PKCS#11 module discovery for
    signing FIT images or capsules is done outside binman, by mkimage and the
    OpenSSL PKCS#11 engine.

    See https://p11-glue.github.io/p11-glue/p11-kit.html for more details.
    """
    def __init__(self, name):
        super().__init__('p11-kit',
                         'Pkcs11 library modules tool',
                         version_args='list modules')

    def fetch(self, method):
        """Install p11-kit via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('p11-kit')

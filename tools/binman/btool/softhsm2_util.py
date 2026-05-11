# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2025 Cherry Embedded Solutions GmbH
#
"""Bintool implementation for SoftHSMv2 (softhsm2-util)"""

from binman import bintool


class Bintoolsofthsm2_util(bintool.Bintool):
    """SoftHSMv2 -- support tool for libsofthsm2

    This bintool wraps the `softhsm2-util` command shipped with SoftHSMv2 (a
    software implementation of a PKCS#11 token). Binman uses this wrapper only
    to check that softhsm2-util is installed (and to fetch it if missing); any
    actual token initialisation or key import for signing FIT images or
    capsules is done outside binman, typically via mkimage and the OpenSSL
    PKCS#11 engine.

    See https://www.opendnssec.org/softhsm/ for more details.
    """
    def __init__(self, name):
        super().__init__('softhsm2-util',
                         'SoftHSMv2 support tool for libsofthsm2',
                         version_args='-v')

    def fetch(self, method):
        """Install softhsm2-util via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('softhsm2')

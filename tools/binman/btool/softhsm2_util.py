# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2025 Cherry Embedded Solutions GmbH
#
"""Bintool implementation for SoftHSMv2 (softhsm2-util)"""

from binman import bintool


class Bintoolsofthsm2_util(bintool.Bintool):
    """SoftHSMv2 -- support tool for libsofthsm2"""
    def __init__(self, name):
        super().__init__('softhsm2-util',
                         'SoftHSMv2 support tool for libsofthsm2',
                         version_args='-v')

    def fetch(self, method):
        """Install softhsm2-util via APT """
        if method != bintool.FETCH_BIN:
            return None

        return self.apt_install('softhsm2')

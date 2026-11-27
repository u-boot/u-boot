# SPDX-License-Identifier: GPL-2.0+
"""Binman entry for a RustSBI dynamic firmware binary."""

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg


class Entry_rustsbi(Entry_blob_named_by_arg):
    """RISC-V RustSBI dynamic firmware blob

    Properties / Entry arguments:
        - rustsbi-path: Filename of the RustSBI firmware binary

    This entry holds the M-mode firmware started by U-Boot SPL.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'rustsbi')
        self.external = True

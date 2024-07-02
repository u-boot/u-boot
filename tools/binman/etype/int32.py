# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Timesys
# Written by Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
#

from binman.entry import Entry
from dtoc import fdt_util
import sys

class Entry_int32(Entry):
    """An entry which is filled with arbitrary int32 data

    Properties / Entry arguments:
        - value: int32 value

    Useful where a magic header is needed.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._int32_value = fdt_util.GetInt(self._node, 'value', 0)

    def ObtainContents(self):
        self.SetContents(self._int32_value.to_bytes(4, sys.byteorder))
        return True

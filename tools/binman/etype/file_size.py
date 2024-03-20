# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023 Timesys
# Written by Piotr Wojtaszczyk <piotr.wojtaszczyk@timesys.com>
#

from binman.entry import Entry
from dtoc import fdt_util
from u_boot_pylib import tools
import sys

class Entry_file_size(Entry):
    """An entry which is filled size of a file

    Properties / Entry arguments:
        - file: path to the file

    Useful where a firmware header needs length of the firmware.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._filename = fdt_util.GetString(self._node, 'filename', self.etype)

    def ObtainContents(self):
        self._pathname = tools.get_input_filename(self._filename,
            self.external and self.section.GetAllowMissing())
        self._file_size = len(tools.read_file(self._pathname))
        self.SetContents(self._file_size.to_bytes(4, sys.byteorder))
        return True

# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# Entry type module for TI SYSFW binary blob
#

import os
import struct
import sys
import zlib

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg
from dtoc import fdt_util
from patman import tools


class Entry_ti_sysfw(Entry_blob_named_by_arg):
    """Entry containing Texas Instruments System Firmware (SYSFW) blob

    Properties / Entry arguments:
        - ti-sysfw-path: Filename of file to read into the entry, typically sysfw.bin

    This entry contains system firmware necessary for booting of K3 architecture devices.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'ti-sysfw')
        self.external = True

# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# Entry type for TI Device Manager

import os

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg


class Entry_ti_dm(Entry_blob_named_by_arg):
    """Entry containing a Texas Instruments Device Manager (DM)

    Properties / Entry arguments:
        - ti-dm-path: Filename of file to read into the entry, typically dm.bin

    This entry holds the device manager responsible for resource and power management
    in K3 devices.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'ti-dm')
        self.external = True

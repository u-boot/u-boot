# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for U-Boot device tree
#

from entry import Entry
from blob import Entry_blob

class Entry_u_boot_spl_dtb(Entry_blob):
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'spl/u-boot-spl.dtb'

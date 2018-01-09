#
# Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
# Entry-type module for Intel Video BIOS Table binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_vbt(Entry_blob):
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)

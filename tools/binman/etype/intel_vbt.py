# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
#
# Entry-type module for Intel Video BIOS Table binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_vbt(Entry_blob):
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

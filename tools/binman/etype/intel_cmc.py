# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for Intel Chip Microcode binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_cmc(Entry_blob):
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)

    def GetDefaultFilename(self):
        return 'cmc.bin'

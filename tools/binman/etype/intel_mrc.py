# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Memory Reference Code binary blob
#

from entry import Entry
from blob import Entry_blob

class Entry_intel_mrc(Entry_blob):
    def __init__(self, section, etype, node):
        Entry_blob.__init__(self, section, etype, node)

    def GetDefaultFilename(self):
        return 'mrc.bin'

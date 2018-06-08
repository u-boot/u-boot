# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for testing purposes. Not used in real images.
#

from entry import Entry
import fdt_util
import tools

class Entry__testing(Entry):
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)

    def ObtainContents(self):
        self.data = 'a'
        self.contents_size = len(self.data)
        return True

    def ReadContents(self):
        return True

    def GetPositions(self):
        return {'invalid-entry': [1, 2]}

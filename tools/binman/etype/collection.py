# SPDX-License-Identifier: GPL-2.0+
# Copyright 2021 Google LLC
# Written by Simon Glass <sjg@chromium.org>
#

# Support for a collection of entries from other parts of an image

from collections import OrderedDict
import os

from binman.entry import Entry
from dtoc import fdt_util

class Entry_collection(Entry):
    """An entry which contains a collection of other entries

    Properties / Entry arguments:
        - content: List of phandles to entries to include

    This allows reusing the contents of other entries. The contents of the
    listed entries are combined to form this entry. This serves as a useful
    base class for entry types which need to process data from elsewhere in
    the image, not necessarily child entries.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.content = fdt_util.GetPhandleList(self._node, 'content')
        if not self.content:
            self.Raise("Collection must have a 'content' property")

    def GetContents(self):
        """Get the contents of this entry

        Returns:
            bytes content of the entry
        """
        # Join up all the data
        self.Info('Getting content')
        data = b''
        for entry_phandle in self.content:
            entry_data = self.section.GetContentsByPhandle(entry_phandle, self)
            if entry_data is None:
                # Data not available yet
                return None
            data += entry_data

        self.Info('Returning contents size %x' % len(data))

        return data

    def ObtainContents(self):
        data = self.GetContents()
        if data is None:
            return False
        self.SetContents(data)
        return True

    def ProcessContents(self):
        # The blob may have changed due to WriteSymbols()
        data = self.GetContents()
        return self.ProcessContentsUpdate(data)

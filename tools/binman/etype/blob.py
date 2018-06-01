# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for blobs, which are binary objects read from files
#

from entry import Entry
import fdt_util
import tools

class Entry_blob(Entry):
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self._filename = fdt_util.GetString(self._node, "filename", self.etype)

    def ObtainContents(self):
        self._filename = self.GetDefaultFilename()
        self._pathname = tools.GetInputFilename(self._filename)
        self.ReadContents()
        return True

    def ReadContents(self):
        with open(self._pathname) as fd:
            # We assume the data is small enough to fit into memory. If this
            # is used for large filesystem image that might not be true.
            # In that case, Image.BuildImage() could be adjusted to use a
            # new Entry method which can read in chunks. Then we could copy
            # the data in chunks and avoid reading it all at once. For now
            # this seems like an unnecessary complication.
            self.data = fd.read()
            self.contents_size = len(self.data)
        return True

    def GetDefaultFilename(self):
        return self._filename

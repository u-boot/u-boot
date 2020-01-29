# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for blobs, which are binary objects read from files
#

from entry import Entry
import fdt_util
import tools
import tout

class Entry_blob(Entry):
    """Entry containing an arbitrary binary blob

    Note: This should not be used by itself. It is normally used as a parent
    class by other entry types.

    Properties / Entry arguments:
        - filename: Filename of file to read into entry
        - compress: Compression algorithm to use:
            none: No compression
            lz4: Use lz4 compression (via 'lz4' command-line utility)

    This entry reads data from a file and places it in the entry. The
    default filename is often specified specified by the subclass. See for
    example the 'u_boot' entry which provides the filename 'u-boot.bin'.

    If compression is enabled, an extra 'uncomp-size' property is written to
    the node (if enabled with -u) which provides the uncompressed size of the
    data.
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self._filename = fdt_util.GetString(self._node, 'filename', self.etype)
        self.compress = fdt_util.GetString(self._node, 'compress', 'none')

    def ObtainContents(self):
        self._filename = self.GetDefaultFilename()
        self._pathname = tools.GetInputFilename(self._filename)
        self.ReadBlobContents()
        return True

    def CompressData(self, indata):
        if self.compress != 'none':
            self.uncomp_size = len(indata)
        data = tools.Compress(indata, self.compress)
        return data

    def ReadBlobContents(self):
        """Read blob contents into memory

        This function compresses the data before storing if needed.

        We assume the data is small enough to fit into memory. If this
        is used for large filesystem image that might not be true.
        In that case, Image.BuildImage() could be adjusted to use a
        new Entry method which can read in chunks. Then we could copy
        the data in chunks and avoid reading it all at once. For now
        this seems like an unnecessary complication.
        """
        indata = tools.ReadFile(self._pathname)
        data = self.CompressData(indata)
        self.SetContents(data)
        return True

    def GetDefaultFilename(self):
        return self._filename

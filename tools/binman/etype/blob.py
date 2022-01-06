# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for blobs, which are binary objects read from files
#

import pathlib

from binman.entry import Entry
from binman import state
from dtoc import fdt_util
from patman import tools
from patman import tout

class Entry_blob(Entry):
    """Arbitrary binary blob

    Note: This should not be used by itself. It is normally used as a parent
    class by other entry types.

    Properties / Entry arguments:
        - filename: Filename of file to read into entry
        - compress: Compression algorithm to use:
            none: No compression
            lz4: Use lz4 compression (via 'lz4' command-line utility)

    This entry reads data from a file and places it in the entry. The
    default filename is often specified specified by the subclass. See for
    example the 'u-boot' entry which provides the filename 'u-boot.bin'.

    If compression is enabled, an extra 'uncomp-size' property is written to
    the node (if enabled with -u) which provides the uncompressed size of the
    data.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._filename = fdt_util.GetString(self._node, 'filename', self.etype)

    def ObtainContents(self):
        if self.allow_fake and not pathlib.Path(self._filename).is_file():
            with open(self._filename, "wb") as out:
                out.truncate(1024)
            self.faked = True

        self._filename = self.GetDefaultFilename()
        self._pathname = tools.GetInputFilename(self._filename,
            self.external and self.section.GetAllowMissing())
        # Allow the file to be missing
        if not self._pathname:
            self.SetContents(b'')
            self.missing = True
            return True

        self.ReadBlobContents()
        return True

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
        state.TimingStart('read')
        indata = tools.ReadFile(self._pathname)
        state.TimingAccum('read')
        state.TimingStart('compress')
        data = self.CompressData(indata)
        state.TimingAccum('compress')
        self.SetContents(data)
        return True

    def GetDefaultFilename(self):
        return self._filename

    def ProcessContents(self):
        # The blob may have changed due to WriteSymbols()
        return self.ProcessContentsUpdate(self.data)

    def CheckFakedBlobs(self, faked_blobs_list):
        """Check if any entries in this section have faked external blobs

        If there are faked blobs, the entries are added to the list

        Args:
            fake_blobs_list: List of Entry objects to be added to
        """
        if self.faked:
            faked_blobs_list.append(self)

# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for a set of files which are placed in individual
# sub-entries
#

import glob
import os

from binman.etype.section import Entry_section
from dtoc import fdt_util
from patman import tools


class Entry_files(Entry_section):
    """Entry containing a set of files

    Properties / Entry arguments:
        - pattern: Filename pattern to match the files to include
        - files-compress: Compression algorithm to use:
            none: No compression
            lz4: Use lz4 compression (via 'lz4' command-line utility)

    This entry reads a number of files and places each in a separate sub-entry
    within this entry. To access these you need to enable device-tree updates
    at run-time so you can obtain the file positions.
    """
    def __init__(self, section, etype, node):
        # Put this here to allow entry-docs and help to work without libfdt
        global state
        from binman import state

        super().__init__(section, etype, node)
        self._pattern = fdt_util.GetString(self._node, 'pattern')
        if not self._pattern:
            self.Raise("Missing 'pattern' property")
        self._files_compress = fdt_util.GetString(self._node, 'files-compress',
                                                  'none')
        self._require_matches = fdt_util.GetBool(self._node,
                                                'require-matches')

    def ExpandEntries(self):
        files = tools.GetInputFilenameGlob(self._pattern)
        if self._require_matches and not files:
            self.Raise("Pattern '%s' matched no files" % self._pattern)
        for fname in files:
            if not os.path.isfile(fname):
                continue
            name = os.path.basename(fname)
            subnode = self._node.FindNode(name)
            if not subnode:
                subnode = state.AddSubnode(self._node, name)
            state.AddString(subnode, 'type', 'blob')
            state.AddString(subnode, 'filename', fname)
            state.AddString(subnode, 'compress', self._files_compress)

        # Read entries again, now that we have some
        self._ReadEntries()

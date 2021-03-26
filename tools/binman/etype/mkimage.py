# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for producing an image using mkimage
#

from collections import OrderedDict

from binman.entry import Entry
from dtoc import fdt_util
from patman import tools

class Entry_mkimage(Entry):
    """Binary produced by mkimage

    Properties / Entry arguments:
        - datafile: Filename for -d argument
        - args: Other arguments to pass

    The data passed to mkimage is collected from subnodes of the mkimage node,
    e.g.::

        mkimage {
            args = "-n test -T imximage";

            u-boot-spl {
            };
        };

    This calls mkimage to create an imximage with u-boot-spl.bin as the input
    file. The output from mkimage then becomes part of the image produced by
    binman.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._args = fdt_util.GetString(self._node, 'args').split(' ')
        self._mkimage_entries = OrderedDict()
        self._ReadSubnodes()

    def ObtainContents(self):
        data = b''
        for entry in self._mkimage_entries.values():
            # First get the input data and put it in a file. If not available,
            # try later.
            if not entry.ObtainContents():
                return False
            data += entry.GetData()
        uniq = self.GetUniqueName()
        input_fname = tools.GetOutputFilename('mkimage.%s' % uniq)
        tools.WriteFile(input_fname, data)
        output_fname = tools.GetOutputFilename('mkimage-out.%s' % uniq)
        tools.Run('mkimage', '-d', input_fname, *self._args, output_fname)
        self.SetContents(tools.ReadFile(output_fname))
        return True

    def _ReadSubnodes(self):
        """Read the subnodes to find out what should go in this image"""
        for node in self._node.subnodes:
            entry = Entry.Create(self, node)
            entry.ReadNode()
            self._mkimage_entries[entry.name] = entry

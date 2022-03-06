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

    To use CONFIG options in the arguments, use a string list instead, as in
    this example which also produces four arguments::

        mkimage {
            args = "-n", CONFIG_SYS_SOC, "-T imximage";

            u-boot-spl {
            };
        };

    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._args = fdt_util.GetArgs(self._node, 'args')
        self._mkimage_entries = OrderedDict()
        self.align_default = None
        self.ReadEntries()

    def ObtainContents(self):
        # Use a non-zero size for any fake files to keep mkimage happy
        data, input_fname, uniq = self.collect_contents_to_file(
            self._mkimage_entries.values(), 'mkimage', 1024)
        if data is None:
            return False
        output_fname = tools.get_output_filename('mkimage-out.%s' % uniq)
        if self.mkimage.run_cmd('-d', input_fname, *self._args,
                                output_fname) is not None:
            self.SetContents(tools.read_file(output_fname))
        else:
            # Bintool is missing; just use the input data as the output
            self.record_missing_bintool(self.mkimage)
            self.SetContents(data)

        return True

    def ReadEntries(self):
        """Read the subnodes to find out what should go in this image"""
        for node in self._node.subnodes:
            entry = Entry.Create(self, node)
            entry.ReadNode()
            self._mkimage_entries[entry.name] = entry

    def SetAllowMissing(self, allow_missing):
        """Set whether a section allows missing external blobs

        Args:
            allow_missing: True if allowed, False if not allowed
        """
        self.allow_missing = allow_missing
        for entry in self._mkimage_entries.values():
            entry.SetAllowMissing(allow_missing)

    def SetAllowFakeBlob(self, allow_fake):
        """Set whether the sub nodes allows to create a fake blob

        Args:
            allow_fake: True if allowed, False if not allowed
        """
        for entry in self._mkimage_entries.values():
            entry.SetAllowFakeBlob(allow_fake)

    def CheckFakedBlobs(self, faked_blobs_list):
        """Check if any entries in this section have faked external blobs

        If there are faked blobs, the entries are added to the list

        Args:
            faked_blobs_list: List of Entry objects to be added to
        """
        for entry in self._mkimage_entries.values():
            entry.CheckFakedBlobs(faked_blobs_list)

    def AddBintools(self, btools):
        self.mkimage = self.AddBintool(btools, 'mkimage')

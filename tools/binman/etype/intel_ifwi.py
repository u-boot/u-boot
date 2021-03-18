# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Entry-type module for Intel Management Engine binary blob
#

from collections import OrderedDict

from binman.entry import Entry
from binman.etype.blob_ext import Entry_blob_ext
from dtoc import fdt_util
from patman import tools

class Entry_intel_ifwi(Entry_blob_ext):
    """Entry containing an Intel Integrated Firmware Image (IFWI) file

    Properties / Entry arguments:
        - filename: Filename of file to read into entry. This is either the
            IFWI file itself, or a file that can be converted into one using a
            tool
        - convert-fit: If present this indicates that the ifwitool should be
            used to convert the provided file into a IFWI.

    This file contains code and data used by the SoC that is required to make
    it work. It includes U-Boot TPL, microcode, things related to the CSE
    (Converged Security Engine, the microcontroller that loads all the firmware)
    and other items beyond the wit of man.

    A typical filename is 'ifwi.bin' for an IFWI file, or 'fitimage.bin' for a
    file that will be converted to an IFWI.

    The position of this entry is generally set by the intel-descriptor entry.

    The contents of the IFWI are specified by the subnodes of the IFWI node.
    Each subnode describes an entry which is placed into the IFWFI with a given
    sub-partition (and optional entry name).

    Properties for subnodes:
        - ifwi-subpart: sub-parition to put this entry into, e.g. "IBBP"
        - ifwi-entry: entry name t use, e.g. "IBBL"
        - ifwi-replace: if present, indicates that the item should be replaced
          in the IFWI. Otherwise it is added.

    See README.x86 for information about x86 binary blobs.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self._convert_fit = fdt_util.GetBool(self._node, 'convert-fit')
        self._ifwi_entries = OrderedDict()

    def ReadNode(self):
        self._ReadSubnodes()
        super().ReadNode()

    def _BuildIfwi(self):
        """Build the contents of the IFWI and write it to the 'data' property"""
        # Create the IFWI file if needed
        if self._convert_fit:
            inname = self._pathname
            outname = tools.GetOutputFilename('ifwi.bin')
            tools.RunIfwiTool(inname, tools.CMD_CREATE, outname)
            self._filename = 'ifwi.bin'
            self._pathname = outname
        else:
            # Provide a different code path here to ensure we have test coverage
            outname = self._pathname

        # Delete OBBP if it is there, then add the required new items.
        tools.RunIfwiTool(outname, tools.CMD_DELETE, subpart='OBBP')

        for entry in self._ifwi_entries.values():
            # First get the input data and put it in a file
            data = entry.GetPaddedData()
            uniq = self.GetUniqueName()
            input_fname = tools.GetOutputFilename('input.%s' % uniq)
            tools.WriteFile(input_fname, data)

            tools.RunIfwiTool(outname,
                tools.CMD_REPLACE if entry._ifwi_replace else tools.CMD_ADD,
                input_fname, entry._ifwi_subpart, entry._ifwi_entry_name)

        self.ReadBlobContents()
        return True

    def ObtainContents(self):
        """Get the contents for the IFWI

        Unfortunately we cannot create anything from scratch here, as Intel has
        tools which create precursor binaries with lots of data and settings,
        and these are not incorporated into binman.

        The first step is to get a file in the IFWI format. This is either
        supplied directly or is extracted from a fitimage using the 'create'
        subcommand.

        After that we delete the OBBP sub-partition and add each of the files
        that we want in the IFWI file, one for each sub-entry of the IWFI node.
        """
        self._pathname = tools.GetInputFilename(self._filename,
                                                self.section.GetAllowMissing())
        # Allow the file to be missing
        if not self._pathname:
            self.SetContents(b'')
            self.missing = True
            return True
        for entry in self._ifwi_entries.values():
            if not entry.ObtainContents():
                return False
        return self._BuildIfwi()

    def ProcessContents(self):
        if self.missing:
            return True
        orig_data = self.data
        self._BuildIfwi()
        same = orig_data == self.data
        return same

    def _ReadSubnodes(self):
        """Read the subnodes to find out what should go in this IFWI"""
        for node in self._node.subnodes:
            entry = Entry.Create(self.section, node)
            entry.ReadNode()
            entry._ifwi_replace = fdt_util.GetBool(node, 'ifwi-replace')
            entry._ifwi_subpart = fdt_util.GetString(node, 'ifwi-subpart')
            entry._ifwi_entry_name = fdt_util.GetString(node, 'ifwi-entry')
            self._ifwi_entries[entry._ifwi_subpart] = entry

    def WriteSymbols(self, section):
        """Write symbol values into binary files for access at run time"""
        if not self.missing:
            for entry in self._ifwi_entries.values():
                entry.WriteSymbols(self)

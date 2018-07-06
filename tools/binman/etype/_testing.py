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
    """A fake entry used for testing

    Properties:
        return_invalid_entry: Return an invalid entry from GetPositions()
    """
    def __init__(self, section, etype, node):
        Entry.__init__(self, section, etype, node)
        self.return_invalid_entry = fdt_util.GetBool(self._node,
                                                     'return-invalid-entry')
        self.return_unknown_contents = fdt_util.GetBool(self._node,
                                                     'return-unknown-contents')
        self.bad_update_contents = fdt_util.GetBool(self._node,
                                                    'bad-update-contents')

    def ObtainContents(self):
        if self.return_unknown_contents:
            return False
        self.data = 'a'
        self.contents_size = len(self.data)
        return True

    def GetPositions(self):
        if self.return_invalid_entry :
            return {'invalid-entry': [1, 2]}
        return {}

    def ProcessContents(self):
        if self.bad_update_contents:
            # Request to update the conents with something larger, to cause a
            # failure.
            self.ProcessContentsUpdate('aa')

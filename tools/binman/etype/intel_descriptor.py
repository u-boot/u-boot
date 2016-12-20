# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:      GPL-2.0+
#
# Entry-type module for 'u-boot'
#

import struct

from entry import Entry
from blob import Entry_blob

FD_SIGNATURE   = struct.pack('<L', 0x0ff0a55a)
MAX_REGIONS    = 5

(REGION_DESCRIPTOR, REGION_BIOS, REGION_ME, REGION_GBE,
        REGION_PDATA) = range(5)

class Region:
    def __init__(self, data, frba, region_num):
        pos = frba + region_num * 4
        val = struct.unpack('<L', data[pos:pos + 4])[0]
        self.base = (val & 0xfff) << 12
        self.limit = ((val & 0x0fff0000) >> 4) | 0xfff
        self.size = self.limit - self.base + 1

class Entry_intel_descriptor(Entry_blob):
    """Intel flash descriptor block (4KB)

    This is placed at the start of flash and provides information about
    the SPI flash regions. In particular it provides the base address and
    size of the ME region, allowing us to place the ME binary in the right
    place.
    """
    def __init__(self, image, etype, node):
        Entry_blob.__init__(self, image, etype, node)
        self._regions = []

    def GetDefaultFilename(self):
        return 'descriptor.bin'

    def GetPositions(self):
        pos = self.data.find(FD_SIGNATURE)
        if pos == -1:
            self.Raise('Cannot find FD signature')
        flvalsig, flmap0, flmap1, flmap2 = struct.unpack('<LLLL',
                                                    self.data[pos:pos + 16])
        frba = ((flmap0 >> 16) & 0xff) << 4
        for i in range(MAX_REGIONS):
            self._regions.append(Region(self.data, frba, i))

        # Set the offset for ME only, for now, since the others are not used
        return {'intel-me': [self._regions[REGION_ME].base,
                             self._regions[REGION_ME].size]}

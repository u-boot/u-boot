# SPDX-License-Identifier: GPL-2.0+
# Entry-type module for Samsung Android DTBH tables

import struct

from binman.android_vendor_dt_table import Entry_Android_vendor_dt_table
from dtoc import fdt_util


DTBH_MAGIC = b'DTBH'
DTBH_VERSION = 2
DTBH_PLATFORM_CODE_DEF = 0x50a6
DTBH_SUBTYPE_CODE_DEF = 0x217584da
DTBH_HEADER = '<4sII'
DTBH_HEADER_SIZE = struct.calcsize(DTBH_HEADER)
DTBH_RECORD = '<8I'
DTBH_RECORD_SIZE = struct.calcsize(DTBH_RECORD)
# Fixed per-record "space delimiter" used by dtbTool-exynos.
# It's unclear what the 0x20 magic actually means, if anything.
DTBH_RECORD_SPACE = 0x20
# Zero end-of-table marker emitted after all DTBH records.
DTBH_EOT = '<I'
DTBH_EOT_SIZE = struct.calcsize(DTBH_EOT)


class Entry_dtbh(Entry_Android_vendor_dt_table):
    """Samsung Android device tree table

    This creates a DTBH table, the legacy device-tree table format used by
    some Samsung Android bootloaders.

    Properties / Entry arguments:
        - page-size: DTBH page size, defaults to the parent android-boot page
          size or 2048 when used elsewhere
        - platform: DTBH platform code, defaults to 0x50a6
        - subtype: DTBH subtype code, defaults to 0x217584da

    This entry uses the following subnodes:
        - dtb-*: DTB records, each containing model_info-chip,
          model_info-hw_rev, model_info-hw_rev_end and exactly one DTB payload
          entry

    Each dtb-* subnode must contain these properties:
        - model_info-chip
        - model_info-hw_rev
        - model_info-hw_rev_end

    Example::

        dtbh {
            dtb-0 {
                model_info-chip = <7870>;
                model_info-hw_rev = <123>;
                model_info-hw_rev_end = <321>;

                u-boot-dtb {
                };
            };
        };
    """

    def ReadNode(self):
        super().ReadNode()
        self.platform = fdt_util.GetInt(self._node, 'platform',
                                        DTBH_PLATFORM_CODE_DEF)
        self.subtype = fdt_util.GetInt(self._node, 'subtype',
                                       DTBH_SUBTYPE_CODE_DEF)

    def _GetDtbRecordData(self, node, required):
        chip = self._GetU32Tuple(node, 'model_info-chip', 1)[0]
        hw_rev = self._GetU32Tuple(node, 'model_info-hw_rev', 1)[0]
        hw_rev_end = self._GetU32Tuple(node, 'model_info-hw_rev_end', 1)[0]
        data = super()._GetDtbRecordData(node, required)
        if data is None and not required:
            return None

        return (chip, hw_rev, hw_rev_end, data)

    def _ReadDtbRecord(self, node, data):
        chip, hw_rev, hw_rev_end, data = data
        return (chip, self.platform, self.subtype, hw_rev, hw_rev_end, data)

    def BuildSectionData(self, required):
        page_size = self._GetPageSize()
        dtbs = self._ReadDtbRecords(required, self._ReadDtbRecord)
        if dtbs is None:
            return None

        size = (DTBH_HEADER_SIZE + len(dtbs) * DTBH_RECORD_SIZE +
                DTBH_EOT_SIZE)
        header_size = self.AlignUp(size, page_size)
        dtb_offset = header_size
        records = []
        payloads = bytearray()
        for chip, platform, subtype, hw_rev, hw_rev_end, dtb in dtbs:
            dtb_size = self.AlignUp(len(dtb), page_size)
            records.append((chip, platform, subtype, hw_rev, hw_rev_end,
                            dtb_offset, dtb_size, DTBH_RECORD_SPACE))
            payloads += self.PadToAlignment(dtb, page_size)
            dtb_offset += dtb_size

        dtbh = bytearray(struct.pack(DTBH_HEADER, DTBH_MAGIC, DTBH_VERSION,
                                     len(records)))
        for record in records:
            dtbh += struct.pack(DTBH_RECORD, *record)
        dtbh += struct.pack(DTBH_EOT, 0)

        return self.PadToAlignment(dtbh, page_size) + bytes(payloads)

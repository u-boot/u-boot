# SPDX-License-Identifier: GPL-2.0+
# Entry-type module for Qualcomm Android device tree tables

import struct

from binman.android_vendor_dt_table import Entry_Android_vendor_dt_table


QCDT_MAGIC = b'QCDT'
QCDT_VERSION = 2
QCDT_HEADER = '<4sII'
QCDT_HEADER_SIZE = struct.calcsize(QCDT_HEADER)
QCDT_RECORD = '<IIIIII'
QCDT_RECORD_SIZE = struct.calcsize(QCDT_RECORD)


class Entry_qcdt(Entry_Android_vendor_dt_table):
    """Qualcomm Android device tree table

    This creates a QCDT table, the legacy device-tree table format used by
    some Qualcomm Android bootloaders.

    Properties / Entry arguments:
        - page-size: QCDT page size, defaults to 2048, unless there's a parent
          android-boot node with an explicit page-size

    This entry uses the following subnodes:
        - dtb-*: DTB records, each containing qcom,msm-id, qcom,board-id and
          exactly one DTB payload entry

    Example::

        qcdt {
            dtb-0 {
                qcom,msm-id = <206 0>;
                qcom,board-id = <0xce08ff01 1>;

                u-boot-dtb {
                };
            };
        };
    """

    def _GetDtbRecordData(self, node, required):
        msm_id = self._GetU32Tuple(node, 'qcom,msm-id', 2)
        board_id = self._GetU32Tuple(node, 'qcom,board-id', 2)
        data = super()._GetDtbRecordData(node, required)
        if data is None and not required:
            return None

        return (msm_id, board_id, data)

    def _ReadDtbRecord(self, node, data):
        return data

    def BuildSectionData(self, required):
        page_size = self._GetPageSize()
        dtbs = self._ReadDtbRecords(required, self._ReadDtbRecord)
        if dtbs is None:
            return None

        size = QCDT_HEADER_SIZE + len(dtbs) * QCDT_RECORD_SIZE
        dtb_offset = self.AlignUp(size, page_size)
        records = []
        payloads = bytearray()
        for msm_id, board_id, dtb in dtbs:
            platform_id, soc_rev = msm_id
            variant_id, board_hw_subtype = board_id
            dtb_size = self.AlignUp(len(dtb), page_size)
            records.append((platform_id, variant_id, board_hw_subtype,
                            soc_rev, dtb_offset, dtb_size))
            payloads += self.PadToAlignment(dtb, page_size)
            dtb_offset += dtb_size

        qcdt = bytearray(struct.pack(QCDT_HEADER, QCDT_MAGIC, QCDT_VERSION,
                                     len(records)))
        for record in records:
            qcdt += struct.pack(QCDT_RECORD, *record)

        return self.PadToAlignment(qcdt, page_size) + bytes(payloads)

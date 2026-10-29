# SPDX-License-Identifier: GPL-2.0+
# Copyright 2026 (C) Bootlin
# Author: Jérémie Dautheribes <jeremie.dautheribes@bootlin.com>
#
# Derived from nxp_imx8mcst.py
# Copyright 2023-2024 Marek Vasut <marex@denx.de>

# Entry-type module for generating the i.MX93 code signing tool
# input configuration file and invocation of cst on generated
# input configuration file and input data to be signed.
#

import os
import struct

from binman.etype.nxp_imxcst import Entry_nxp_imxcst
from dtoc import fdt_util

CONTAINER_HDR_TAG = 0x87
SPL_CONTAINER_OFFSET = 1024  # 0x400
CONTAINER_HDR_SIZE = 16
AHAB_IMAGE_ENTRY_FLAGS_OFFSET = 24
ELE_IMAGE_CORE_AND_TYPE = 0x66

KEY_NAME = 'sha384_secp384r1_v3_usr_crt'

CSF_CONFIG_TEMPLATE = f'''
[Header]
  Target = AHAB
  Version = 1.0

[Install SRK]
  File = "SRK_1_2_3_4_table.bin"
  Source = "SRK1_{KEY_NAME}.pem"
  Source index = 0
  Source set = OEM
  Revocations = 0x0

[Authenticate Data]
  File = "data.bin"
  Offsets = 0x0 0x0

'''


class Entry_nxp_imx93cst(Entry_nxp_imxcst):
    """NXP i.MX93 CST .cfg file generator and cst invoker

    Properties / Entry arguments:
        - nxp,srk-table - full path to SRK_1_2_3_4_table.bin
        - nxp,srk-crt - full path to the SRK Key SRK1_sha384_secp384r1_v3_usr_crt.pem

    The nxp,srk-table and nxp,srk-crt properties can be overridden with
    the SRK_TABLE and SRK_KEY environment variables, respectively.
    """

    def ReadNode(self):
        super().ReadNode()
        self.srk_crt = os.getenv(
            'SRK_KEY',
            fdt_util.GetString(self._node, 'nxp,srk-crt', f'SRK1_{KEY_NAME}.pem'),
        )

    def BuildSectionData(self, required):
        data, _, uniq = self.collect_contents_to_file(self._entries.values(), 'input')

        flags_offset = CONTAINER_HDR_SIZE + AHAB_IMAGE_ENTRY_FLAGS_OFFSET

        # Give up early if the input is too short to contain the container
        # header fields read below
        if len(data) < flags_offset + 4:
            return data

        if data[3] != CONTAINER_HDR_TAG:
            # Unknown section type, pass input data through.
            return data

        hdr_addr = 0

        # The SPL AHAB image can optionally contain and start with the ELE FW,
        # which is already signed by NXP.
        # In this case, the SPL container header address is not 0x0.

        image_flags = struct.unpack('<I', data[flags_offset : flags_offset + 4])[0]
        # Detect the ELE FW from the core/type fields of its image entries
        if (image_flags & 0xFF) == ELE_IMAGE_CORE_AND_TYPE:
            hdr_addr = SPL_CONTAINER_OFFSET

        # Extract the signing offset from the i.MX container
        signoffset = struct.unpack('<H', data[hdr_addr + 12 : hdr_addr + 14])[0]

        # The signing offset is relative to the container header address,
        # so compute the absolute signing offset address
        signoffset = signoffset + hdr_addr

        # Write out customized data to be signed
        output_dname = self.write_input_data(data, uniq)

        # Generate CST configuration file used to sign payload
        config = self.get_config(CSF_CONFIG_TEMPLATE)
        config['Install SRK']['File'] = f'"{self.srk_table}"'
        config['Install SRK']['Source'] = f'"{self.srk_crt}"'
        config['Authenticate Data']['File'] = f'"{output_dname}"'
        config['Authenticate Data']['Offsets'] = f'{hdr_addr:#x} {signoffset:#x}'

        cfg_fname = self.write_config(config, uniq)

        outdata = self.run_cst(cfg_fname, uniq)
        if outdata is not None:
            return outdata
        return data

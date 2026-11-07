# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023-2024 Marek Vasut <marex@denx.de>
# Written with much help from Simon Glass <sjg@chromium.org>
#
# Entry-type module for generating the i.MX8M code signing tool
# input configuration file and invocation of cst on generated
# input configuration file and input data to be signed.
#

import os
import struct

from binman.etype.nxp_imxcst import Entry_nxp_imxcst
from dtoc import fdt_util
from u_boot_pylib import tools

MAGIC_NXP_IMX_IVT  = 0x412000d1
MAGIC_NXP_IMX_FCFB = 0x42464346
MAGIC_FITIMAGE     = 0xedfe0dd0

KEY_NAME = 'sha256_4096_65537_v3_usr_crt'

CSF_CONFIG_TEMPLATE = f'''
[Header]
  Version = 4.3
  Hash Algorithm = sha256
  Engine = CAAM
  Engine Configuration = 0
  Certificate Format = X509
  Signature Format = CMS

[Install SRK]
  File = "SRK_1_2_3_4_table.bin"
  Source index = 0

[Install NOCAK]
  File = "SRK1_{KEY_NAME}.pem"

[Install CSFK]
  File = "CSF1_1_{KEY_NAME}.pem"

[Authenticate CSF]

[Unlock]
  Engine = CAAM
  Features = MID

[Install Key]
  Verification index = 0
  Target Index = 2
  File = "IMG1_1_{KEY_NAME}.pem"

[Authenticate Data]
  Verification index = 2
  Blocks = 0x1234 0x78 0xabcd "data.bin"
'''


class Entry_nxp_imx8mcst(Entry_nxp_imxcst):
    """NXP i.MX8M CST .cfg file generator and cst invoker

    Properties / Entry arguments:
        - nxp,loader-address - loader address (SPL text base)
        - nxp,srk-table - full path to SRK_1_2_3_4_table.bin
        - nxp,srk-index - index of the key in the SRK table, default is 0
        - nxp,csf-crt - full path to the CSF Key CSF1_1_sha256_4096_65537_v3_usr_crt.pem
        - nxp,img-crt - full path to the IMG Key IMG1_1_sha256_4096_65537_v3_usr_crt.pem
        - nxp,fast-auth - enable fast authentication method
        - nxp,srk-crt - full path to the SRK Key SRK1_sha256_4096_65537_v3_ca_crt.pem
        - nxp,unlock - unlock CAAM in SPL
        - nxp,cst-backend - CST tool backend, default is 'ssl', or selectable 'pkcs11'
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['nxp,loader-address']

    def ReadNode(self):
        super().ReadNode()
        self.loader_address = fdt_util.GetInt(self._node, 'nxp,loader-address')
        self.srk_index = os.getenv(
            'SRK_INDEX', fdt_util.GetInt(self._node, 'nxp,srk-index', 0))
        self.fast_auth = fdt_util.GetBool(self._node, 'nxp,fast-auth')
        if not self.fast_auth:
            self.csf_crt = os.getenv(
                'CSF_KEY', fdt_util.GetString(self._node, 'nxp,csf-crt',
                                              f'CSF1_1_{KEY_NAME}.pem'))
            self.img_crt = os.getenv(
                'IMG_KEY', fdt_util.GetString(self._node, 'nxp,img-crt',
                                              f'IMG1_1_{KEY_NAME}.pem'))
        else:
            self.srk_crt = os.getenv(
                'SRK_KEY', fdt_util.GetString(self._node, 'nxp,srk-crt',
                                              f'SRK1_{KEY_NAME}.pem'))

        self.backend = os.getenv(
            'CST_BACKEND', fdt_util.GetString(self._node, 'nxp,cst-backend',
                                              'ssl'))

        self.unlock = fdt_util.GetBool(self._node, 'nxp,unlock')
        self.ReadEntries()

    def BuildSectionData(self, required):
        data, input_fname, uniq = self.collect_contents_to_file(
            self._entries.values(), 'input')

        # Parse the input data and figure out what it is that is being signed.
        # - If it is mkimage'd imx8mimage, then extract to be signed data size
        #   from imx8mimage header, and calculate CSF blob offset right past
        #   the SPL from this information.
        # - If it is mkimage'd imx8mimage wrapped in FCFB, then extract to be
        #   signed data size from imx8mimage header past the FCFB header, and
        #   calculate CSF blob offset right past the SPL from this information.
        # - If it is fitImage, then pad the image to 4k, add generated IVT and
        #   sign the whole payload, then append CSF blob at the end right past
        #   the IVT.
        signtype = struct.unpack('<I', data[:4])[0]
        signbase = self.loader_address
        signsize = 0
        if signtype == MAGIC_NXP_IMX_IVT: # SPL/imx8mimage
            # Sign the payload including imx8mimage header
            # (extra 0x40 bytes before the payload)
            signbase -= 0x40
            signsize = struct.unpack('<I', data[24:28])[0] - signbase
            # Remove mkimage generated padding from the end of data
            data = data[:signsize]
        elif signtype == MAGIC_NXP_IMX_FCFB: # SPL/imx8mimage with FCFB
            # Sign the payload including FCFB and imx8mimage headers
            # (extra 0x1000 and 0x40 bytes before the payload)
            signbase -= 0x1040
            # Pull the end address from IVT offset 24 Bytes and subtract
            # the start address to get amount of data to sign. The IVT
            # itself is at offset 4096 Bytes, so the total end address
            # offset in the image is 4120 Bytes.
            signsize = struct.unpack('<I', data[4120:4124])[0] - signbase
            # Remove mkimage generated padding from the end of data
            data = data[:signsize]
        elif signtype == MAGIC_FITIMAGE: # fitImage
            # Align fitImage to 4k
            signsize = tools.align(len(data), 0x1000)
            data += tools.get_bytes(0, signsize - len(data))
            # Add generated IVT
            data += struct.pack('<I', MAGIC_NXP_IMX_IVT)
            data += struct.pack('<I', signbase + signsize) # IVT base
            data += struct.pack('<I', 0)
            data += struct.pack('<I', 0)
            data += struct.pack('<I', 0)
            data += struct.pack('<I', signbase + signsize) # IVT base
            data += struct.pack('<I', signbase + signsize + 0x20) # CSF base
            data += struct.pack('<I', 0)
        else:
            # Unknown section type, pass input data through.
            return data

        # Write out customized data to be signed
        output_dname = self.write_input_data(data, uniq)

        # Generate CST configuration file used to sign payload
        config = self.get_config(CSF_CONFIG_TEMPLATE)
        config['Install SRK']['File'] = f'"{self.srk_table}"'
        config['Install SRK']['Source index'] = f'{self.srk_index}'
        if not self.fast_auth:
            config.remove_section('Install NOCAK')
            config['Install CSFK']['File'] = f'"{self.csf_crt}"'
            config['Install Key']['File']  = f'"{self.img_crt}"'
        else:
            config.remove_section('Install CSFK')
            config.remove_section('Install Key')
            config['Install NOCAK']['File'] = f'"{self.srk_crt}"'
            config['Authenticate Data']['Verification index'] = '0'

        config['Authenticate Data']['Blocks'] = \
            f'{signbase:#x} 0 {len(data):#x} "{output_dname}"'

        if not self.unlock:
            config.remove_section('Unlock')
        cfg_fname = self.write_config(config, uniq)

        # SSL is the default backend, PKCS11 backend is optional
        if self.backend == "pkcs11":
            cst_backend = "pkcs11"
        else:
            cst_backend = "ssl"

        outdata = self.run_cst(cfg_fname, uniq, cst_backend)
        if outdata is not None:
            # fixme: 0x2000 should be CONFIG_CSF_SIZE
            outdata += tools.get_bytes(0, 0x2000 - 0x20 - len(outdata))
            return data + outdata
        return data

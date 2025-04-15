# SPDX-License-Identifier: GPL-2.0+
# Copyright 2023-2024 Marek Vasut <marex@denx.de>
# Written with much help from Simon Glass <sjg@chromium.org>
#
# Entry-type module for generating the i.MX8M code signing tool
# input configuration file and invocation of cst on generated
# input configuration file and input data to be signed.
#

import configparser
import os
import struct

from collections import OrderedDict

from binman.entry import Entry
from binman.etype.mkimage import Entry_mkimage
from binman.etype.section import Entry_section
from binman import elf
from dtoc import fdt_util
from u_boot_pylib import tools

MAGIC_NXP_IMX_IVT = 0x412000d1
MAGIC_FITIMAGE    = 0xedfe0dd0

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

class Entry_nxp_imx8mcst(Entry_mkimage):
    """NXP i.MX8M CST .cfg file generator and cst invoker

    Properties / Entry arguments:
        - nxp,loader-address - loader address (SPL text base)
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node)
        self.required_props = ['nxp,loader-address']

    def ReadNode(self):
        super().ReadNode()
        self.loader_address = fdt_util.GetInt(self._node, 'nxp,loader-address')
        self.srk_table = os.getenv(
            'SRK_TABLE', fdt_util.GetString(self._node, 'nxp,srk-table',
                                            'SRK_1_2_3_4_table.bin'))
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

        self.unlock = fdt_util.GetBool(self._node, 'nxp,unlock')
        self.ReadEntries()

    def BuildSectionData(self, required):
        data, input_fname, uniq = self.collect_contents_to_file(
            self._entries.values(), 'input')

        # Parse the input data and figure out what it is that is being signed.
        # - If it is mkimage'd imx8mimage, then extract to be signed data size
        #   from imx8mimage header, and calculate CSF blob offset right past
        #   the SPL from this information.
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
        output_dname = tools.get_output_filename(f'nxp.cst-input-data.{uniq}')
        tools.write_file(output_dname, data)

        # Generate CST configuration file used to sign payload
        cfg_fname = tools.get_output_filename(f'nxp.csf-config-txt.{uniq}')
        config = configparser.ConfigParser()
        # Do not make key names lowercase
        config.optionxform = str
        # Load configuration template and modify keys of interest
        config.read_string(CSF_CONFIG_TEMPLATE)
        config['Install SRK']['File']  = f'"{self.srk_table}"'
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
        with open(cfg_fname, 'w') as cfgf:
            config.write(cfgf)

        output_fname = tools.get_output_filename(f'nxp.csf-output-blob.{uniq}')
        args = ['-i', cfg_fname, '-o', output_fname]
        if self.cst.run_cmd(*args) is not None:
            outdata = tools.read_file(output_fname)
            # fixme: 0x2000 should be CONFIG_CSF_SIZE
            outdata += tools.get_bytes(0, 0x2000 - 0x20 - len(outdata))
            return data + outdata
        else:
            # Bintool is missing; just use the input data as the output
            self.record_missing_bintool(self.cst)
            return data

    def SetImagePos(self, image_pos):
        # Customized SoC specific SetImagePos which skips the mkimage etype
        # implementation and removes the 0x48 offset introduced there. That
        # offset is only used for uImage/fitImage, which is not the case in
        # here.
        upto = 0x00
        for entry in super().GetEntries().values():
            entry.SetOffsetSize(upto, None)

            # Give up if any entries lack a size
            if entry.size is None:
                return
            upto += entry.size

        Entry_section.SetImagePos(self, image_pos)

    def AddBintools(self, btools):
        super().AddBintools(btools)
        self.cst = self.AddBintool(btools, 'cst')

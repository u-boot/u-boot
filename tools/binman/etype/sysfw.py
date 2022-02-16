# SPDX-License-Identifier: GPL-2.0+
# Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
#
# Entry type module for TI SYSFW binary blob
#

import struct
import zlib
import os
import sys

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg
from dtoc import fdt_util
from patman import tools


class Entry_sysfw(Entry_blob_named_by_arg):
    """Entry containing System Firmware (SYSFW) blob

    Properties / Entry arguments:
        - sysfw-path: Filename of file to read into the entry, typically sysfw.bin

This entry contains system firmware necessary for booting of K3 architecture devices.
    """

    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'scp')
        self.core = "0"
        self.missing_msg = "sysfw"

    def ReadNode(self):
        self._load_addr = fdt_util.GetInt(self._node, 'load', 0)
        self._args = []

    def _SignSysfw(self, out):
        """Sign the sysfw image and write it to the output directory"""
        # Try running the K3 x509 certificate signing script
        try:
            args = [
                '-c', "0",
                '-b', self._filename,
                '-l', str(self._load_addr),
                '-o', out
            ]
            k3_cert_gen_path = os.environ['srctree'] + \
                "/tools/k3_gen_x509_cert.sh"
            tools.run(k3_cert_gen_path, *args)
            self.SetContents(tools.read_file(out))
            return True
        # If not available (example, in the case of binman tests, set entry contents as dummy binary)
        except KeyError:
            self.missing = True
            self.SetContents(b'sysfw')
            return True

    def ObtainContents(self):
        self.missing = False
        out = tools.get_output_filename("sysfwint")
        self._SignSysfw(out)
        return True

# SPDX-License-Identifier: GPL-2.0+
# Copyright 2025 Texas Instruments Incorporated
#
# Entry-type module for Application Processor Trusted ROM (BL1)
#

from binman.etype.blob_named_by_arg import Entry_blob_named_by_arg

class Entry_atf_bl1(Entry_blob_named_by_arg):
    """Application Processor (AP) Trusted ROM BL1 blob

    Properties / Entry arguments:
        - atf-bl1-path: Filename of file to read into entry. This is typically
            called bl1.bin or bl1.elf

    This entry holds the boot code initialization like exception vectors and
    processor and platform initialization.

    See https://github.com/TrustedFirmware-A/trusted-firmware-a for more information.
    """
    def __init__(self, section, etype, node):
        super().__init__(section, etype, node, 'atf-bl1')
        self.external = True

#!/usr/bin/python
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

"""Device tree to C tool

This tool converts a device tree binary file (.dtb) into two C files. The
indent is to allow a C program to access data from the device tree without
having to link against libfdt. By putting the data from the device tree into
C structures, normal C code can be used. This helps to reduce the size of the
compiled program.

Dtoc produces two output files:

   dt-structs.h  - contains struct definitions
   dt-platdata.c - contains data from the device tree using the struct
                      definitions, as well as U-Boot driver definitions.

This tool is used in U-Boot to provide device tree data to SPL without
increasing the code size of SPL. This supports the CONFIG_SPL_OF_PLATDATA
options. For more information about the use of this options and tool please
see doc/driver-model/of-plat.txt
"""

from optparse import OptionParser
import os
import sys

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../patman'))

import dtb_platdata


if __name__ != "__main__":
    pass

parser = OptionParser()
parser.add_option('-d', '--dtb-file', action='store',
                  help='Specify the .dtb input file')
parser.add_option('--include-disabled', action='store_true',
                  help='Include disabled nodes')
parser.add_option('-o', '--output', action='store', default='-',
                  help='Select output filename')
(options, args) = parser.parse_args()

if not args:
    raise ValueError('Please specify a command: struct, platdata')

plat = dtb_platdata.DtbPlatdata(options.dtb_file, options)
plat.ScanDtb()
plat.ScanTree()
plat.SetupOutput(options.output)
structs = plat.ScanStructs()
plat.ScanPhandles()

for cmd in args[0].split(','):
    if cmd == 'struct':
        plat.GenerateStructs(structs)
    elif cmd == 'platdata':
        plat.GenerateTables()
    else:
        raise ValueError("Unknown command '%s': (use: struct, platdata)" % cmd)

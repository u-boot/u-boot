#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

"""
Expo utility - used for testing of expo features

Copyright 2023 Google LLC
Written by Simon Glass <sjg@chromium.org>
"""

import argparse
import collections
import io
import re
import subprocess
import sys

#from u_boot_pylib import cros_subprocess
from u_boot_pylib import tools

# Parse:
#	SCENE1		= 7,
# or    SCENE1		= EXPOID_BASE_ID,
# or	SCENE2,
RE_ENUM = re.compile(r'(\S*)(\s*= ([0-9A-Z_]+))?,')

# Parse #define <name>  "string"
RE_DEF = re.compile(r'#define (\S*)\s*"(.*)"')

# Parse EXPOID_BASE_ID = 5,
RE_BASE_ID = re.compile(r'\s*EXPOID_BASE_ID\s*= (\d+),')

def calc_ids(fname, base_id):
    """Figure out the value of the enums in a C file

    Args:
        fname (str): Filename to parse
        base_id (int): Base ID (value of EXPOID_BASE_ID)

    Returns:
        OrderedDict():
            key (str): enum name
            value (int or str):
                Value of enum, if int
                Value of #define, if string
    """
    vals = collections.OrderedDict()
    with open(fname, 'r', encoding='utf-8') as inf:
        in_enum = False
        cur_id = 0
        for line in inf.readlines():
            line = line.strip()
            if line == 'enum {':
                in_enum = True
                continue
            if in_enum and line == '};':
                in_enum = False

            if in_enum:
                if not line or line.startswith('/*'):
                    continue
                m_enum = RE_ENUM.match(line)
                enum_name = m_enum.group(3)
                if enum_name:
                    if enum_name == 'EXPOID_BASE_ID':
                        cur_id = base_id
                    else:
                        cur_id = int(enum_name)
                vals[m_enum.group(1)] = cur_id
                cur_id += 1
            else:
                m_def = RE_DEF.match(line)
                if m_def:
                    vals[m_def.group(1)] = tools.to_bytes(m_def.group(2))

    return vals


def find_base_id():
    fname = 'include/expo.h'
    base_id = None
    with open(fname, 'r', encoding='utf-8') as inf:
        for line in inf.readlines():
            m_base_id = RE_BASE_ID.match(line)
            if m_base_id:
                base_id = int(m_base_id.group(1))
    if base_id is None:
        raise ValueError('EXPOID_BASE_ID not found in expo.h')
    #print(f'EXPOID_BASE_ID={base_id}')
    return base_id

def run_expo(args):
    """Run the expo program"""
    base_id = find_base_id()
    fname = args.enum_fname or args.layout
    ids = calc_ids(fname, base_id)
    if not ids:
        print(f"Warning: No enum ID values found in file '{fname}'")

    indata = tools.read_file(args.layout)

    outf = io.BytesIO()

    for name, val in ids.items():
        if isinstance(val, int):
            outval = b'%d' % val
        else:
            outval = b'"%s"' % val
        find_str = r'\b%s\b' % name
        indata = re.sub(tools.to_bytes(find_str), outval, indata)

    outf.write(indata)
    data = outf.getvalue()

    with open('/tmp/asc', 'wb') as outf:
        outf.write(data)
    proc = subprocess.run('dtc', input=data, capture_output=True)
    edtb = proc.stdout
    if proc.stderr:
        print(f"Devicetree compiler error:\n{proc.stderr.decode('utf-8')}")
        return 1
    tools.write_file(args.outfile, edtb)
    return 0


def parse_args(argv):
    """Parse the command-line arguments

    Args:
        argv (list of str): List of string arguments

    Returns:
        tuple: (options, args) with the command-line options and arugments.
            options provides access to the options (e.g. option.debug)
            args is a list of string arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-D', '--debug', action='store_true',
        help='Enable full debug traceback')
    parser.add_argument('-e', '--enum-fname', type=str,
        help='.dts or C file containing enum declaration for expo items')
    parser.add_argument('-l', '--layout', type=str, required=True,
        help='Devicetree file source .dts for expo layout (and perhaps enums)')
    parser.add_argument('-o', '--outfile', type=str, required=True,
        help='Filename to write expo layout dtb')

    return parser.parse_args(argv)

def start_expo():
    """Start the expo program"""
    args = parse_args(sys.argv[1:])

    if not args.debug:
        sys.tracebacklimit = 0

    ret_code = run_expo(args)
    sys.exit(ret_code)


if __name__ == "__main__":
    start_expo()

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
# or	SCENE2,
RE_ENUM = re.compile(r'(\S*)(\s*= (\d))?,')

# Parse #define <name>  "string"
RE_DEF = re.compile(r'#define (\S*)\s*"(.*)"')

def calc_ids(fname):
    """Figure out the value of the enums in a C file

    Args:
        fname (str): Filename to parse

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
                if m_enum.group(3):
                    cur_id = int(m_enum.group(3))
                vals[m_enum.group(1)] = cur_id
                cur_id += 1
            else:
                m_def = RE_DEF.match(line)
                if m_def:
                    vals[m_def.group(1)] = tools.to_bytes(m_def.group(2))

    return vals


def run_expo(args):
    """Run the expo program"""
    ids = calc_ids(args.enum_fname)

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
    proc = subprocess.run('dtc', input=data, capture_output=True, check=True)
    edtb = proc.stdout
    if proc.stderr:
        print(proc.stderr)
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
    parser.add_argument('-e', '--enum-fname', type=str,
        help='C file containing enum declaration for expo items')
    parser.add_argument('-l', '--layout', type=str,
        help='Devicetree file source .dts for expo layout')
    parser.add_argument('-o', '--outfile', type=str,
        help='Filename to write expo layout dtb')

    return parser.parse_args(argv)

def start_expo():
    """Start the expo program"""
    args = parse_args(sys.argv[1:])

    ret_code = run_expo(args)
    sys.exit(ret_code)


if __name__ == "__main__":
    start_expo()

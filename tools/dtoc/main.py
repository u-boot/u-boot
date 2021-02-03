#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Device tree to C tool

This tool converts a device tree binary file (.dtb) into two C files. The
indent is to allow a C program to access data from the device tree without
having to link against libfdt. By putting the data from the device tree into
C structures, normal C code can be used. This helps to reduce the size of the
compiled program.

Dtoc produces several output files - see OUTPUT_FILES in dtb_platdata.py

This tool is used in U-Boot to provide device tree data to SPL without
increasing the code size of SPL. This supports the CONFIG_SPL_OF_PLATDATA
options. For more information about the use of this options and tool please
see doc/driver-model/of-plat.rst
"""

from optparse import OptionParser
import os
import sys
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '..'))

# Bring in the libfdt module
sys.path.insert(0, 'scripts/dtc/pylibfdt')
sys.path.insert(0, os.path.join(our_path,
                '../../build-sandbox_spl/scripts/dtc/pylibfdt'))

from dtoc import dtb_platdata
from patman import test_util

def run_tests(processes, args):
    """Run all the test we have for dtoc

    Args:
        processes: Number of processes to use to run tests (None=same as #CPUs)
        args: List of positional args provided to dtoc. This can hold a test
            name to execute (as in 'dtoc -t test_empty_file', for example)
    """
    from dtoc import test_src_scan
    from dtoc import test_dtoc

    result = unittest.TestResult()
    sys.argv = [sys.argv[0]]
    test_name = args and args[0] or None

    test_dtoc.setup()

    test_util.RunTestSuites(
        result, debug=True, verbosity=1, test_preserve_dirs=False,
        processes=processes, test_name=test_name, toolpath=[],
        test_class_list=[test_dtoc.TestDtoc,test_src_scan.TestSrcScan])

    return test_util.ReportResult('binman', test_name, result)

def RunTestCoverage():
    """Run the tests and check that we get 100% coverage"""
    sys.argv = [sys.argv[0]]
    test_util.RunTestCoverage('tools/dtoc/dtoc', '/main.py',
            ['tools/patman/*.py', '*/fdt*', '*test*'], options.build_dir)


if __name__ != '__main__':
    sys.exit(1)

parser = OptionParser()
parser.add_option('-B', '--build-dir', type='string', default='b',
        help='Directory containing the build output')
parser.add_option('-c', '--c-output-dir', action='store',
                  help='Select output directory for C files')
parser.add_option('-C', '--h-output-dir', action='store',
                  help='Select output directory for H files (defaults to --c-output-di)')
parser.add_option('-d', '--dtb-file', action='store',
                  help='Specify the .dtb input file')
parser.add_option('--include-disabled', action='store_true',
                  help='Include disabled nodes')
parser.add_option('-o', '--output', action='store',
                  help='Select output filename')
parser.add_option('-p', '--phase', type=str,
                  help='set phase of U-Boot this invocation is for (spl/tpl)')
parser.add_option('-P', '--processes', type=int,
                  help='set number of processes to use for running tests')
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-T', '--test-coverage', action='store_true',
                default=False, help='run tests and check for 100% coverage')
(options, args) = parser.parse_args()

# Run our meagre tests
if options.test:
    ret_code = run_tests(options.processes, args)
    sys.exit(ret_code)

elif options.test_coverage:
    RunTestCoverage()

else:
    dtb_platdata.run_steps(args, options.dtb_file, options.include_disabled,
                           options.output,
                           [options.c_output_dir, options.h_output_dir],
                           phase=options.phase)

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

"""See README for more information"""

from distutils.sysconfig import get_python_lib
import glob
import os
import site
import sys
import traceback
import unittest

# Bring in the patman and dtoc libraries (but don't override the first path
# in PYTHONPATH)
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(2, os.path.join(our_path, '..'))

from patman import test_util

# Bring in the libfdt module
sys.path.insert(2, 'scripts/dtc/pylibfdt')
sys.path.insert(2, os.path.join(our_path,
                '../../build-sandbox_spl/scripts/dtc/pylibfdt'))

# When running under python-coverage on Ubuntu 16.04, the dist-packages
# directories are dropped from the python path. Add them in so that we can find
# the elffile module. We could use site.getsitepackages() here but unfortunately
# that is not available in a virtualenv.
sys.path.append(get_python_lib())

from binman import cmdline
from binman import control
from patman import test_util

def RunTests(debug, verbosity, processes, test_preserve_dirs, args, toolpath):
    """Run the functional tests and any embedded doctests

    Args:
        debug: True to enable debugging, which shows a full stack trace on error
        verbosity: Verbosity level to use
        test_preserve_dirs: True to preserve the input directory used by tests
            so that it can be examined afterwards (only useful for debugging
            tests). If a single test is selected (in args[0]) it also preserves
            the output directory for this test. Both directories are displayed
            on the command line.
        processes: Number of processes to use to run tests (None=same as #CPUs)
        args: List of positional args provided to binman. This can hold a test
            name to execute (as in 'binman test testSections', for example)
        toolpath: List of paths to use for tools
    """
    from binman import cbfs_util_test
    from binman import elf_test
    from binman import entry_test
    from binman import fdt_test
    from binman import ftest
    from binman import image_test
    from binman import test
    import doctest

    result = unittest.TestResult()
    test_name = args and args[0] or None

    # Run the entry tests first ,since these need to be the first to import the
    # 'entry' module.
    test_util.RunTestSuites(
        result, debug, verbosity, test_preserve_dirs, processes, test_name,
        toolpath,
        [entry_test.TestEntry, ftest.TestFunctional, fdt_test.TestFdt,
         elf_test.TestElf, image_test.TestImage, cbfs_util_test.TestCbfs])

    return test_util.ReportResult('binman', test_name, result)

def GetEntryModules(include_testing=True):
    """Get a set of entry class implementations

    Returns:
        Set of paths to entry class filenames
    """
    glob_list = glob.glob(os.path.join(our_path, 'etype/*.py'))
    return set([os.path.splitext(os.path.basename(item))[0]
                for item in glob_list
                if include_testing or '_testing' not in item])

def RunTestCoverage():
    """Run the tests and check that we get 100% coverage"""
    glob_list = GetEntryModules(False)
    all_set = set([os.path.splitext(os.path.basename(item))[0]
                   for item in glob_list if '_testing' not in item])
    test_util.RunTestCoverage('tools/binman/binman', None,
            ['*test*', '*main.py', 'tools/patman/*', 'tools/dtoc/*'],
            args.build_dir, all_set)

def RunBinman(args):
    """Main entry point to binman once arguments are parsed

    Args:
        args: Command line arguments Namespace object
    """
    ret_code = 0

    if not args.debug:
        sys.tracebacklimit = 0

    if args.cmd == 'test':
        if args.test_coverage:
            RunTestCoverage()
        else:
            ret_code = RunTests(args.debug, args.verbosity, args.processes,
                                args.test_preserve_dirs, args.tests,
                                args.toolpath)

    elif args.cmd == 'entry-docs':
        control.WriteEntryDocs(GetEntryModules())

    else:
        try:
            ret_code = control.Binman(args)
        except Exception as e:
            print('binman: %s' % e)
            if args.debug:
                print()
                traceback.print_exc()
            ret_code = 1
    return ret_code


if __name__ == "__main__":
    args = cmdline.ParseArgs(sys.argv[1:])

    ret_code = RunBinman(args)
    sys.exit(ret_code)

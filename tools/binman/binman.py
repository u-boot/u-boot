#!/usr/bin/env python2
# SPDX-License-Identifier: GPL-2.0+

# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# Creates binary images from input files controlled by a description
#

"""See README for more information"""

import glob
import os
import sys
import traceback
import unittest

# Bring in the patman and dtoc libraries
our_path = os.path.dirname(os.path.realpath(__file__))
for dirname in ['../patman', '../dtoc', '..']:
    sys.path.insert(0, os.path.join(our_path, dirname))

# Bring in the libfdt module
sys.path.insert(0, 'scripts/dtc/pylibfdt')

import cmdline
import command
import control
import test_util

def RunTests(debug, args):
    """Run the functional tests and any embedded doctests

    Args:
        debug: True to enable debugging, which shows a full stack trace on error
        args: List of positional args provided to binman. This can hold a test
            name to execute (as in 'binman -t testSections', for example)
    """
    import elf_test
    import entry_test
    import fdt_test
    import ftest
    import image_test
    import test
    import doctest

    result = unittest.TestResult()
    for module in []:
        suite = doctest.DocTestSuite(module)
        suite.run(result)

    sys.argv = [sys.argv[0]]
    if debug:
        sys.argv.append('-D')

    # Run the entry tests first ,since these need to be the first to import the
    # 'entry' module.
    test_name = args and args[0] or None
    for module in (entry_test.TestEntry, ftest.TestFunctional, fdt_test.TestFdt,
                   elf_test.TestElf, image_test.TestImage):
        if test_name:
            try:
                suite = unittest.TestLoader().loadTestsFromName(test_name, module)
            except AttributeError:
                continue
        else:
            suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    print result
    for test, err in result.errors:
        print test.id(), err
    for test, err in result.failures:
        print err, result.failures
    if result.errors or result.failures:
      print 'binman tests FAILED'
      return 1
    return 0

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
    test_util.RunTestCoverage('tools/binman/binman.py', None,
            ['*test*', '*binman.py', 'tools/patman/*', 'tools/dtoc/*'],
            options.build_dir, all_set)

def RunBinman(options, args):
    """Main entry point to binman once arguments are parsed

    Args:
        options: Command-line options
        args: Non-option arguments
    """
    ret_code = 0

    # For testing: This enables full exception traces.
    #options.debug = True

    if not options.debug:
        sys.tracebacklimit = 0

    if options.test:
        ret_code = RunTests(options.debug, args[1:])

    elif options.test_coverage:
        RunTestCoverage()

    elif options.entry_docs:
        control.WriteEntryDocs(GetEntryModules())

    else:
        try:
            ret_code = control.Binman(options, args)
        except Exception as e:
            print 'binman: %s' % e
            if options.debug:
                print
                traceback.print_exc()
            ret_code = 1
    return ret_code


if __name__ == "__main__":
    (options, args) = cmdline.ParseArgs(sys.argv)
    ret_code = RunBinman(options, args)
    sys.exit(ret_code)

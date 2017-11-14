#!/usr/bin/env python2

# Copyright (c) 2016 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#
# SPDX-License-Identifier:	GPL-2.0+
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

# Also allow entry-type modules to be brought in from the etype directory.
sys.path.insert(0, os.path.join(our_path, 'etype'))

import cmdline
import command
import control

def RunTests(debug):
    """Run the functional tests and any embedded doctests"""
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
    suite = unittest.TestLoader().loadTestsFromTestCase(entry_test.TestEntry)
    suite.run(result)
    for module in (ftest.TestFunctional, fdt_test.TestFdt, elf_test.TestElf,
                   image_test.TestImage):
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

def RunTestCoverage():
    """Run the tests and check that we get 100% coverage"""
    # This uses the build output from sandbox_spl to get _libfdt.so
    cmd = ('PYTHONPATH=$PYTHONPATH:%s/sandbox_spl/tools coverage run '
            '--include "tools/binman/*.py" --omit "*test*,*binman.py" '
            'tools/binman/binman.py -t' % options.build_dir)
    os.system(cmd)
    stdout = command.Output('coverage', 'report')
    lines = stdout.splitlines()

    test_set= set([os.path.basename(line.split()[0])
                     for line in lines if '/etype/' in line])
    glob_list = glob.glob(os.path.join(our_path, 'etype/*.py'))
    all_set = set([os.path.basename(item) for item in glob_list])
    missing_list = all_set
    missing_list.difference_update(test_set)
    missing_list.remove('_testing.py')
    coverage = lines[-1].split(' ')[-1]
    ok = True
    if missing_list:
        print 'Missing tests for %s' % (', '.join(missing_list))
        ok = False
    if coverage != '100%':
        print stdout
        print "Type 'coverage html' to get a report in htmlcov/index.html"
        print 'Coverage error: %s, but should be 100%%' % coverage
        ok = False
    if not ok:
      raise ValueError('Test coverage failure')

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
        ret_code = RunTests(options.debug)

    elif options.test_coverage:
        RunTestCoverage()

    elif options.full_help:
        pager = os.getenv('PAGER')
        if not pager:
            pager = 'more'
        fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'README')
        command.Run(pager, fname)

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

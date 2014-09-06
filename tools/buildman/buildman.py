#!/usr/bin/env python
#
# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

"""See README for more information"""

import multiprocessing
import os
import re
import sys
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../patman'))

# Our modules
import board
import builder
import checkpatch
import cmdline
import control
import doctest
import gitutil
import patchstream
import terminal
import toolchain

def RunTests():
    import test
    import doctest

    result = unittest.TestResult()
    for module in ['toolchain']:
        suite = doctest.DocTestSuite(module)
        suite.run(result)

    # TODO: Surely we can just 'print' result?
    print result
    for test, err in result.errors:
        print err
    for test, err in result.failures:
        print err

    sys.argv = [sys.argv[0]]
    suite = unittest.TestLoader().loadTestsFromTestCase(test.TestBuild)
    result = unittest.TestResult()
    suite.run(result)

    # TODO: Surely we can just 'print' result?
    print result
    for test, err in result.errors:
        print err
    for test, err in result.failures:
        print err


options, args = cmdline.ParseArgs()

# Run our meagre tests
if options.test:
    RunTests()
elif options.full_help:
    pager = os.getenv('PAGER')
    if not pager:
        pager = 'more'
    fname = os.path.join(os.path.dirname(sys.argv[0]), 'README')
    command.Run(pager, fname)

# Build selected commits for selected boards
else:
    ret_code = control.DoBuildman(options, args)
    sys.exit(ret_code)

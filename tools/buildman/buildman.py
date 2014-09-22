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
import bsettings
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
    import func_test
    import test
    import doctest

    result = unittest.TestResult()
    for module in ['toolchain', 'gitutil']:
        suite = doctest.DocTestSuite(module)
        suite.run(result)

    sys.argv = [sys.argv[0]]
    for module in (test.TestBuild, func_test.TestFunctional):
        suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    print result
    for test, err in result.errors:
        print err
    for test, err in result.failures:
        print err


options, args = cmdline.ParseArgs()

# Run our meagre tests
if options.test:
    RunTests()

# Build selected commits for selected boards
else:
    bsettings.Setup(options.config_file)
    ret_code = control.DoBuildman(options, args)
    sys.exit(ret_code)

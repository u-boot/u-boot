#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2012 The Chromium OS Authors.
#

"""See README for more information"""

import doctest
import multiprocessing
import os
import re
import sys

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, os.path.join(our_path, '..'))

# Our modules
from buildman import board
from buildman import bsettings
from buildman import builder
from buildman import cmdline
from buildman import control
from buildman import toolchain
from patman import patchstream
from patman import gitutil
from patman import terminal
from patman import test_util

def RunTests(skip_net_tests, verboose, args):
    from buildman import func_test
    from buildman import test
    import doctest

    test_name = args and args[0] or None
    if skip_net_tests:
        test.use_network = False

    # Run the entry tests first ,since these need to be the first to import the
    # 'entry' module.
    result = test_util.run_test_suites(
        'buildman', False, verboose, False, None, test_name, [],
        [test.TestBuild, func_test.TestFunctional,
         'buildman.toolchain', 'patman.gitutil'])

    return (0 if result.wasSuccessful() else 1)

options, args = cmdline.ParseArgs()

if not options.debug:
    sys.tracebacklimit = 0

# Run our meagre tests
if options.test:
    RunTests(options.skip_net_tests, options.verbose, args)

# Build selected commits for selected boards
else:
    bsettings.Setup(options.config_file)
    ret_code = control.DoBuildman(options, args)
    sys.exit(ret_code)

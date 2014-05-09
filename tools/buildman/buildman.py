#!/usr/bin/env python
#
# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

"""See README for more information"""

import multiprocessing
from optparse import OptionParser
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
import command
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


parser = OptionParser()
parser.add_option('-b', '--branch', type='string',
       help='Branch name to build')
parser.add_option('-B', '--bloat', dest='show_bloat',
       action='store_true', default=False,
       help='Show changes in function code size for each board')
parser.add_option('-c', '--count', dest='count', type='int',
       default=-1, help='Run build on the top n commits')
parser.add_option('-e', '--show_errors', action='store_true',
       default=False, help='Show errors and warnings')
parser.add_option('-f', '--force-build', dest='force_build',
       action='store_true', default=False,
       help='Force build of boards even if already built')
parser.add_option('-d', '--detail', dest='show_detail',
       action='store_true', default=False,
       help='Show detailed information for each board in summary')
parser.add_option('-g', '--git', type='string',
       help='Git repo containing branch to build', default='.')
parser.add_option('-H', '--full-help', action='store_true', dest='full_help',
       default=False, help='Display the README file')
parser.add_option('-j', '--jobs', dest='jobs', type='int',
       default=None, help='Number of jobs to run at once (passed to make)')
parser.add_option('-k', '--keep-outputs', action='store_true',
       default=False, help='Keep all build output files (e.g. binaries)')
parser.add_option('--list-tool-chains', action='store_true', default=False,
       help='List available tool chains')
parser.add_option('-n', '--dry-run', action='store_true', dest='dry_run',
       default=False, help="Do a try run (describe actions, but no nothing)")
parser.add_option('-Q', '--quick', action='store_true',
       default=False, help='Do a rough build, with limited warning resolution')
parser.add_option('-s', '--summary', action='store_true',
       default=False, help='Show a build summary')
parser.add_option('-S', '--show-sizes', action='store_true',
       default=False, help='Show image size variation in summary')
parser.add_option('--step', type='int',
       default=1, help='Only build every n commits (0=just first and last)')
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-T', '--threads', type='int',
       default=None, help='Number of builder threads to use')
parser.add_option('-u', '--show_unknown', action='store_true',
       default=False, help='Show boards with unknown build result')
parser.add_option('-o', '--output-dir', type='string',
       dest='output_dir', default='..',
       help='Directory where all builds happen and buildman has its workspace (default is ../)')

parser.usage = """buildman -b <branch> [options]

Build U-Boot for all commits in a branch. Use -n to do a dry run"""

(options, args) = parser.parse_args()

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
    control.DoBuildman(options, args)

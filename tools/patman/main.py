#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#

"""See README for more information"""

from optparse import OptionParser
import os
import re
import sys
import unittest

if __name__ == "__main__":
    # Allow 'from patman import xxx to work'
    our_path = os.path.dirname(os.path.realpath(__file__))
    sys.path.append(os.path.join(our_path, '..'))

# Our modules
from patman import command
from patman import control
from patman import gitutil
from patman import project
from patman import settings
from patman import terminal
from patman import test_util
from patman import test_checkpatch


parser = OptionParser()
parser.add_option('-H', '--full-help', action='store_true', dest='full_help',
       default=False, help='Display the README file')
parser.add_option('-c', '--count', dest='count', type='int',
       default=-1, help='Automatically create patches from top n commits')
parser.add_option('-i', '--ignore-errors', action='store_true',
       dest='ignore_errors', default=False,
       help='Send patches email even if patch errors are found')
parser.add_option('-l', '--limit-cc', dest='limit', type='int',
       default=None, help='Limit the cc list to LIMIT entries [default: %default]')
parser.add_option('-m', '--no-maintainers', action='store_false',
       dest='add_maintainers', default=True,
       help="Don't cc the file maintainers automatically")
parser.add_option('-n', '--dry-run', action='store_true', dest='dry_run',
       default=False, help="Do a dry run (create but don't email patches)")
parser.add_option('-p', '--project', default=project.DetectProject(),
                  help="Project name; affects default option values and "
                  "aliases [default: %default]")
parser.add_option('-r', '--in-reply-to', type='string', action='store',
                  help="Message ID that this series is in reply to")
parser.add_option('-s', '--start', dest='start', type='int',
       default=0, help='Commit to start creating patches from (0 = HEAD)')
parser.add_option('-t', '--ignore-bad-tags', action='store_true',
                  default=False, help='Ignore bad tags / aliases')
parser.add_option('-v', '--verbose', action='store_true', dest='verbose',
       default=False, help='Verbose output of errors and warnings')
parser.add_option('-T', '--thread', action='store_true', dest='thread',
                  default=False, help='Create patches as a single thread')
parser.add_option('--cc-cmd', dest='cc_cmd', type='string', action='store',
       default=None, help='Output cc list for patch file (used by git)')
parser.add_option('--no-binary', action='store_true', dest='ignore_binary',
                  default=False,
                  help="Do not output contents of changes in binary files")
parser.add_option('--no-check', action='store_false', dest='check_patch',
                  default=True,
                  help="Don't check for patch compliance")
parser.add_option('--no-tags', action='store_false', dest='process_tags',
                  default=True, help="Don't process subject tags as aliases")
parser.add_option('--smtp-server', type='str',
                  help="Specify the SMTP server to 'git send-email'")
parser.add_option('--test', action='store_true', dest='test',
                  default=False, help='run tests')

parser.usage += """

Create patches from commits in a branch, check them and email them as
specified by tags you place in the commits. Use -n to do a dry run first."""


# Parse options twice: first to get the project and second to handle
# defaults properly (which depends on project).
(options, args) = parser.parse_args()
settings.Setup(gitutil, parser, options.project, '')
(options, args) = parser.parse_args()

if __name__ != "__main__":
    pass

# Run our meagre tests
elif options.test:
    import doctest
    from patman import func_test

    sys.argv = [sys.argv[0]]
    result = unittest.TestResult()
    for module in (test_checkpatch.TestPatch, func_test.TestFunctional):
        suite = unittest.TestLoader().loadTestsFromTestCase(module)
        suite.run(result)

    for module in ['gitutil', 'settings', 'terminal']:
        suite = doctest.DocTestSuite(module)
        suite.run(result)

    sys.exit(test_util.ReportResult('patman', None, result))

# Called from git with a patch filename as argument
# Printout a list of additional CC recipients for this patch
elif options.cc_cmd:
    fd = open(options.cc_cmd, 'r')
    re_line = re.compile('(\S*) (.*)')
    for line in fd.readlines():
        match = re_line.match(line)
        if match and match.group(1) == args[0]:
            for cc in match.group(2).split('\0'):
                cc = cc.strip()
                if cc:
                    print(cc)
    fd.close()

elif options.full_help:
    pager = os.getenv('PAGER')
    if not pager:
        pager = 'more'
    fname = os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                         'README')
    command.Run(pager, fname)

# Process commits, produce patches files, check them, email them
else:
    control.send(options)

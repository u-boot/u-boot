#!/usr/bin/python
#
# Copyright (c) 2011 The Chromium OS Authors.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

"""See README for more information"""

from optparse import OptionParser
import os
import re
import sys
import unittest

# Our modules
import checkpatch
import command
import gitutil
import patchstream
import terminal
import test


parser = OptionParser()
parser.add_option('-H', '--full-help', action='store_true', dest='full_help',
       default=False, help='Display the README file')
parser.add_option('-c', '--count', dest='count', type='int',
       default=-1, help='Automatically create patches from top n commits')
parser.add_option('-i', '--ignore-errors', action='store_true',
       dest='ignore_errors', default=False,
       help='Send patches email even if patch errors are found')
parser.add_option('-n', '--dry-run', action='store_true', dest='dry_run',
       default=False, help="Do a try run (create but don't email patches)")
parser.add_option('-s', '--start', dest='start', type='int',
       default=0, help='Commit to start creating patches from (0 = HEAD)')
parser.add_option('-t', '--test', action='store_true', dest='test',
                  default=False, help='run tests')
parser.add_option('-v', '--verbose', action='store_true', dest='verbose',
       default=False, help='Verbose output of errors and warnings')
parser.add_option('--cc-cmd', dest='cc_cmd', type='string', action='store',
       default=None, help='Output cc list for patch file (used by git)')
parser.add_option('--no-tags', action='store_false', dest='process_tags',
                  default=True, help="Don't process subject tags as aliaes")

parser.usage = """patman [options]

Create patches from commits in a branch, check them and email them as
specified by tags you place in the commits. Use -n to """

(options, args) = parser.parse_args()

# Run our meagre tests
if options.test:
    import doctest

    sys.argv = [sys.argv[0]]
    suite = unittest.TestLoader().loadTestsFromTestCase(test.TestPatch)
    result = unittest.TestResult()
    suite.run(result)

    suite = doctest.DocTestSuite('gitutil')
    suite.run(result)

    # TODO: Surely we can just 'print' result?
    print result
    for test, err in result.errors:
        print err
    for test, err in result.failures:
        print err

# Called from git with a patch filename as argument
# Printout a list of additional CC recipients for this patch
elif options.cc_cmd:
    fd = open(options.cc_cmd, 'r')
    re_line = re.compile('(\S*) (.*)')
    for line in fd.readlines():
        match = re_line.match(line)
        if match and match.group(1) == args[0]:
            for cc in match.group(2).split(', '):
                cc = cc.strip()
                if cc:
                    print cc
    fd.close()

elif options.full_help:
    pager = os.getenv('PAGER')
    if not pager:
        pager = 'more'
    fname = os.path.join(os.path.dirname(sys.argv[0]), 'README')
    command.Run(pager, fname)

# Process commits, produce patches files, check them, email them
else:
    gitutil.Setup()

    if options.count == -1:
        # Work out how many patches to send if we can
        options.count = gitutil.CountCommitsToBranch() - options.start

    col = terminal.Color()
    if not options.count:
        str = 'No commits found to process - please use -c flag'
        print col.Color(col.RED, str)
        sys.exit(1)

    # Read the metadata from the commits
    if options.count:
        series = patchstream.GetMetaData(options.start, options.count)
        cover_fname, args = gitutil.CreatePatches(options.start, options.count,
                series)

    # Fix up the patch files to our liking, and insert the cover letter
    series = patchstream.FixPatches(series, args)
    if series and cover_fname and series.get('cover'):
        patchstream.InsertCoverLetter(cover_fname, series, options.count)

    # Do a few checks on the series
    series.DoChecks()

    # Check the patches, and run them through 'git am' just to be sure
    ok = checkpatch.CheckPatches(options.verbose, args)
    if not gitutil.ApplyPatches(options.verbose, args,
            options.count + options.start):
        ok = False

    # Email the patches out (giving the user time to check / cancel)
    cmd = ''
    if ok or options.ignore_errors:
        cc_file = series.MakeCcFile(options.process_tags)
        cmd = gitutil.EmailPatches(series, cover_fname, args,
                options.dry_run, cc_file)
        os.remove(cc_file)

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        series.ShowActions(args, cmd, options.process_tags)

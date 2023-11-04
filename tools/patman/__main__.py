#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#

"""See README for more information"""

try:
    import importlib.resources
except ImportError:
    # for Python 3.6
    import importlib_resources
import os
import re
import sys
import traceback

if __name__ == "__main__":
    # Allow 'from patman import xxx to work'
    our_path = os.path.dirname(os.path.realpath(__file__))
    sys.path.append(os.path.join(our_path, '..'))

# Our modules
from patman import cmdline
from patman import control
from patman import func_test
from u_boot_pylib import terminal
from u_boot_pylib import test_util
from u_boot_pylib import tools


if __name__ != "__main__":
    pass

args = cmdline.parse_args()

if not args.debug:
    sys.tracebacklimit = 0

# Run our meagre tests
if args.cmd == 'test':
    from patman import func_test
    from patman import test_checkpatch

    result = test_util.run_test_suites(
        'patman', False, False, False, None, None, None,
        [test_checkpatch.TestPatch, func_test.TestFunctional,
         'gitutil', 'settings'])

    sys.exit(0 if result.wasSuccessful() else 1)

# Process commits, produce patches files, check them, email them
elif args.cmd == 'send':
    # Called from git with a patch filename as argument
    # Printout a list of additional CC recipients for this patch
    if args.cc_cmd:
        fd = open(args.cc_cmd, 'r')
        re_line = re.compile('(\S*) (.*)')
        for line in fd.readlines():
            match = re_line.match(line)
            if match and match.group(1) == args.patchfiles[0]:
                for cc in match.group(2).split('\0'):
                    cc = cc.strip()
                    if cc:
                        print(cc)
        fd.close()

    elif args.full_help:
        with importlib.resources.path('patman', 'README.rst') as readme:
            tools.print_full_help(str(readme))
    else:
        # If we are not processing tags, no need to warning about bad ones
        if not args.process_tags:
            args.ignore_bad_tags = True
        control.send(args)

# Check status of patches in patchwork
elif args.cmd == 'status':
    ret_code = 0
    try:
        control.patchwork_status(args.branch, args.count, args.start, args.end,
                                 args.dest_branch, args.force,
                                 args.show_comments, args.patchwork_url)
    except Exception as e:
        terminal.tprint('patman: %s: %s' % (type(e).__name__, e),
                        colour=terminal.Color.RED)
        if args.debug:
            print()
            traceback.print_exc()
        ret_code = 1
    sys.exit(ret_code)

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2011 The Chromium OS Authors.
#

"""See README for more information"""

import os
import sys

# Allow 'from patman import xxx to work'
# pylint: disable=C0413
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '..'))

# Our modules
from u_boot_pylib import test_util
from u_boot_pylib import tout
from patman import cmdline
from patman import control


def run_patman():
    """Run patamn

    This is the main program. It collects arguments and runs either the tests or
    the control module.
    """
    args = cmdline.parse_args()

    if not args.debug:
        sys.tracebacklimit = 0

    tout.init(tout.INFO if args.verbose else tout.WARNING)

    # Run our reasonably good tests
    if args.cmd == 'test':
        # pylint: disable=C0415
        from patman import func_test
        from patman import test_checkpatch
        from patman import test_cseries

        to_run = args.testname if args.testname not in [None, 'test'] else None
        result = test_util.run_test_suites(
            'patman', False, args.verbose, args.no_capture,
            args.test_preserve_dirs, None, to_run, None,
            [test_checkpatch.TestPatch, func_test.TestFunctional, 'settings',
             test_cseries.TestCseries])
        sys.exit(0 if result.wasSuccessful() else 1)

    # Process commits, produce patches files, check them, email them
    else:
        exit_code = control.do_patman(args)
        sys.exit(exit_code)


if __name__ == "__main__":
    sys.exit(run_patman())

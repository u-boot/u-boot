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
from patman import cmdline
from patman import control
from u_boot_pylib import test_util


def run_patman():
    """Run patamn

    This is the main program. It collects arguments and runs either the tests or
    the control module.
    """
    args = cmdline.parse_args()

    if not args.debug:
        sys.tracebacklimit = 0

    # Run our meagre tests
    if args.cmd == 'test':
        # pylint: disable=C0415
        from patman import func_test
        from patman import test_checkpatch

        result = test_util.run_test_suites(
            'patman', False, False, False, None, None, None,
            [test_checkpatch.TestPatch, func_test.TestFunctional,
             'settings'])

        sys.exit(0 if result.wasSuccessful() else 1)

    # Process commits, produce patches files, check them, email them
    else:
        control.do_patman(args)


if __name__ == "__main__":
    sys.exit(run_patman())

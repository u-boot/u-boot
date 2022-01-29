# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
#

import os

from patman import command

def find_get_maintainer(try_list):
    """Look for the get_maintainer.pl script.

    Args:
        try_list: List of directories to try for the get_maintainer.pl script

    Returns:
        If the script is found we'll return a path to it; else None.
    """
    # Look in the list
    for path in try_list:
        fname = os.path.join(path, 'get_maintainer.pl')
        if os.path.isfile(fname):
            return fname

    return None

def get_maintainer(dir_list, fname, verbose=False):
    """Run get_maintainer.pl on a file if we find it.

    We look for get_maintainer.pl in the 'scripts' directory at the top of
    git.  If we find it we'll run it.  If we don't find get_maintainer.pl
    then we fail silently.

    Args:
        dir_list: List of directories to try for the get_maintainer.pl script
        fname: Path to the patch file to run get_maintainer.pl on.

    Returns:
        A list of email addresses to CC to.
    """
    get_maintainer = find_get_maintainer(dir_list)
    if not get_maintainer:
        if verbose:
            print("WARNING: Couldn't find get_maintainer.pl")
        return []

    stdout = command.output(get_maintainer, '--norolestats', fname)
    lines = stdout.splitlines()
    return [ x.replace('"', '') for x in lines ]

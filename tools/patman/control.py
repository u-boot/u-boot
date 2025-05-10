# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 Google LLC
#
"""Handles the main control logic of patman

This module provides various functions called by the main program to implement
the features of patman.
"""

import re
import traceback

try:
    from importlib import resources
except ImportError:
    # for Python 3.6
    import importlib_resources as resources

from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tools
from u_boot_pylib import tout
from patman import cseries
from patman import cser_helper
from patman import patchstream
from patman.patchwork import Patchwork
from patman import send
from patman import settings


def setup():
    """Do required setup before doing anything"""
    gitutil.setup()
    alias_fname = gitutil.get_alias_file()
    if alias_fname:
        settings.ReadGitAliases(alias_fname)


def do_send(args):
    """Create, check and send patches by email

    Args:
        args (argparse.Namespace): Arguments to patman
    """
    setup()
    send.send(args)


def patchwork_status(branch, count, start, end, dest_branch, force,
                     show_comments, url, single_thread=False):
    """Check the status of patches in patchwork

    This finds the series in patchwork using the Series-link tag, checks for new
    comments and review tags, displays then and creates a new branch with the
    review tags.

    Args:
        branch (str): Branch to create patches from (None = current)
        count (int): Number of patches to produce, or -1 to produce patches for
            the current branch back to the upstream commit
        start (int): Start partch to use (0=first / top of branch)
        end (int): End patch to use (0=last one in series, 1=one before that,
            etc.)
        dest_branch (str): Name of new branch to create with the updated tags
            (None to not create a branch)
        force (bool): With dest_branch, force overwriting an existing branch
        show_comments (bool): True to display snippets from the comments
            provided by reviewers
        url (str): URL of patchwork server, e.g. 'https://patchwork.ozlabs.org'.
            This is ignored if the series provides a Series-patchwork-url tag.

    Raises:
        ValueError: if the branch has no Series-link value
    """
    if not branch:
        branch = gitutil.get_branch()
    if count == -1:
        # Work out how many patches to send if we can
        count = gitutil.count_commits_to_branch(branch) - start

    series = patchstream.get_metadata(branch, start, count - end)
    warnings = 0
    for cmt in series.commits:
        if cmt.warn:
            print('%d warnings for %s:' % (len(cmt.warn), cmt.hash))
            for warn in cmt.warn:
                print('\t', warn)
                warnings += 1
            print
    if warnings:
        raise ValueError('Please fix warnings before running status')
    links = series.get('links')
    if not links:
        raise ValueError("Branch has no Series-links value")

    _, version = cser_helper.split_name_version(branch)
    link = series.get_link_for_version(version, links)
    if not link:
        raise ValueError('Series-links has no link for v{version}')
    tout.debug(f"Link '{link}")

    # Allow the series to override the URL
    if 'patchwork_url' in series:
        url = series.patchwork_url
    pwork = Patchwork(url, single_thread=single_thread)

    # Import this here to avoid failing on other commands if the dependencies
    # are not present
    from patman import status
    pwork = Patchwork(url)
    status.check_and_show_status(series, link, branch, dest_branch, force,
                                 show_comments, False, pwork)


def patchwork(args, test_db=None, pwork=None):
    """Process a 'patchwork' subcommand
    Args:
        args (Namespace): Arguments to process
        test_db (str or None): Directory containing the test database, None to
            use the normal one
        pwork (Patchwork): Patchwork object to use
    """
    cser = cseries.Cseries(test_db)
    try:
        cser.open_database()
        if args.subcmd == 'set-project':
            if not pwork:
                pwork = Patchwork(args.patchwork_url)
            cser.project_set(pwork, args.project_name)
        elif args.subcmd == 'get-project':
            info = cser.project_get()
            if not info:
                raise ValueError("Project has not been set; use 'patman patchwork set-project'")
            name, pwid, link_name = info
            print(f"Project '{name}' patchwork-ID {pwid} link-name {link_name}")
        else:
            raise ValueError(f"Unknown patchwork subcommand '{args.subcmd}'")
    finally:
        cser.close_database()

def do_patman(args, test_db=None, pwork=None, cser=None):
    """Process a patman command

    Args:
        args (Namespace): Arguments to process
        test_db (str or None): Directory containing the test database, None to
            use the normal one
        pwork (Patchwork): Patchwork object to use, or None to create one
        cser (Cseries): Cseries object to use when executing the command,
            or None to create one
    """
    if args.full_help:
        with resources.path('patman', 'README.rst') as readme:
            tools.print_full_help(str(readme))
        return 0
    if args.cmd == 'send':
        # Called from git with a patch filename as argument
        # Printout a list of additional CC recipients for this patch
        if args.cc_cmd:
            re_line = re.compile(r'(\S*) (.*)')
            with open(args.cc_cmd, 'r', encoding='utf-8') as inf:
                for line in inf.readlines():
                    match = re_line.match(line)
                    if match and match.group(1) == args.patchfiles[0]:
                        for cca in match.group(2).split('\0'):
                            cca = cca.strip()
                            if cca:
                                print(cca)
        else:
            # If we are not processing tags, no need to warning about bad ones
            if not args.process_tags:
                args.ignore_bad_tags = True
            do_send(args)
        return 0

    ret_code = 0
    try:
        # Check status of patches in patchwork
        if args.cmd == 'status':
            patchwork_status(args.branch, args.count, args.start, args.end,
                             args.dest_branch, args.force, args.show_comments,
                             args.patchwork_url)
        elif args.cmd == 'patchwork':
            patchwork(args, test_db, pwork)
    except Exception as exc:
        terminal.tprint(f'patman: {type(exc).__name__}: {exc}',
                        colour=terminal.Color.RED)
        if args.debug:
            print()
            traceback.print_exc()
        ret_code = 1
    return ret_code

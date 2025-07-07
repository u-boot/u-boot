# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2023 Google LLC
#

"""Handles parsing of buildman arguments

This creates the argument parser and uses it to parse the arguments passed in
"""

import argparse
import os
import pathlib
import sys

from u_boot_pylib import gitutil
from patman import project
from patman import settings

PATMAN_DIR = pathlib.Path(__file__).parent
HAS_TESTS = os.path.exists(PATMAN_DIR / "func_test.py")

# Aliases for subcommands
ALIASES = {
    'series': ['s', 'ser'],
    'status': ['st'],
    'patchwork': ['pw'],
    'upstream': ['us'],

    # Series aliases
    'archive': ['ar'],
    'autolink': ['au'],
    'gather': ['g'],
    'open': ['o'],
    'progress': ['p', 'pr', 'prog'],
    'rm-version': ['rmv'],
    'unarchive': ['unar'],
    }


class ErrorCatchingArgumentParser(argparse.ArgumentParser):
    def __init__(self, **kwargs):
        self.exit_state = None
        self.catch_error = False
        super().__init__(**kwargs)

    def error(self, message):
        if self.catch_error:
            self.message = message
        else:
            super().error(message)

    def exit(self, status=0, message=None):
        if self.catch_error:
            self.exit_state = True
        else:
            super().exit(status, message)


def add_send_args(par):
    """Add arguments for the 'send' command

    Arguments:
        par (ArgumentParser): Parser to add to
    """
    par.add_argument(
        '-c', '--count', dest='count', type=int, default=-1,
        help='Automatically create patches from top n commits')
    par.add_argument(
        '-e', '--end', type=int, default=0,
        help='Commits to skip at end of patch list')
    par.add_argument(
        '-i', '--ignore-errors', action='store_true',
        dest='ignore_errors', default=False,
        help='Send patches email even if patch errors are found')
    par.add_argument(
        '-l', '--limit-cc', dest='limit', type=int, default=None,
        help='Limit the cc list to LIMIT entries [default: %(default)s]')
    par.add_argument(
        '-m', '--no-maintainers', action='store_false',
        dest='add_maintainers', default=True,
        help="Don't cc the file maintainers automatically")
    default_arg = None
    top_level = gitutil.get_top_level()
    if top_level:
        default_arg = os.path.join(top_level, 'scripts',
                                   'get_maintainer.pl') + ' --norolestats'
    par.add_argument(
        '--get-maintainer-script', dest='get_maintainer_script', type=str,
        action='store',
        default=default_arg,
        help='File name of the get_maintainer.pl (or compatible) script.')
    par.add_argument(
        '-r', '--in-reply-to', type=str, action='store',
        help="Message ID that this series is in reply to")
    par.add_argument(
        '-s', '--start', dest='start', type=int, default=0,
        help='Commit to start creating patches from (0 = HEAD)')
    par.add_argument(
        '-t', '--ignore-bad-tags', action='store_true', default=False,
        help='Ignore bad tags / aliases (default=warn)')
    par.add_argument(
        '--no-binary', action='store_true', dest='ignore_binary',
        default=False,
        help="Do not output contents of changes in binary files")
    par.add_argument(
        '--no-check', action='store_false', dest='check_patch', default=True,
        help="Don't check for patch compliance")
    par.add_argument(
        '--tree', dest='check_patch_use_tree', default=False,
        action='store_true',
        help=("Set `tree` to True. If `tree` is False then we'll pass "
              "'--no-tree' to checkpatch (default: tree=%(default)s)"))
    par.add_argument(
        '--no-tree', dest='check_patch_use_tree', action='store_false',
        help="Set `tree` to False")
    par.add_argument(
        '--no-tags', action='store_false', dest='process_tags', default=True,
        help="Don't process subject tags as aliases")
    par.add_argument(
        '--no-signoff', action='store_false', dest='add_signoff',
        default=True, help="Don't add Signed-off-by to patches")
    par.add_argument(
        '--smtp-server', type=str,
        help="Specify the SMTP server to 'git send-email'")
    par.add_argument(
        '--keep-change-id', action='store_true',
        help='Preserve Change-Id tags in patches to send.')


def _add_show_comments(parser):
    parser.add_argument('-c', '--show-comments', action='store_true',
                        help='Show comments from each patch')


def _add_show_cover_comments(parser):
    parser.add_argument('-C', '--show-cover-comments', action='store_true',
                        help='Show comments from the cover letter')


def add_patchwork_subparser(subparsers):
    """Add the 'patchwork' subparser

    Args:
        subparsers (argparse action): Subparser parent

    Return:
        ArgumentParser: patchwork subparser
    """
    patchwork = subparsers.add_parser(
        'patchwork', aliases=ALIASES['patchwork'],
        help='Manage patchwork connection')
    patchwork.defaults_cmds = [
        ['set-project', 'U-Boot'],
    ]
    patchwork_subparsers = patchwork.add_subparsers(dest='subcmd')
    patchwork_subparsers.add_parser('get-project')
    uset = patchwork_subparsers.add_parser('set-project')
    uset.add_argument(
        'project_name', help="Patchwork project name, e.g. 'U-Boot'")
    return patchwork


def add_series_subparser(subparsers):
    """Add the 'series' subparser

    Args:
        subparsers (argparse action): Subparser parent

    Return:
        ArgumentParser: series subparser
    """
    def _add_allow_unmarked(parser):
        parser.add_argument('-M', '--allow-unmarked', action='store_true',
                            default=False,
                            help="Don't require commits to be marked")

    def _add_mark(parser):
        parser.add_argument(
            '-m', '--mark', action='store_true',
            help='Mark unmarked commits with a Change-Id field')

    def _add_update(parser):
        parser.add_argument('-u', '--update', action='store_true',
                            help='Update the branch commit')

    def _add_wait(parser, default_s):
        """Add a -w option to a parser

        Args:
            parser (ArgumentParser): Parser to adjust
            default_s (int): Default value to use, in seconds
        """
        parser.add_argument(
            '-w', '--autolink-wait', type=int, default=default_s,
            help='Seconds to wait for patchwork to get a sent series')

    def _upstream_add(parser):
        parser.add_argument('-U', '--upstream', help='Commit to end before')

    def _add_gather(parser):
        parser.add_argument(
            '-G', '--no-gather-tags', dest='gather_tags', default=True,
            action='store_false',
            help="Don't gather review/test tags / update local series")

    series = subparsers.add_parser('series', aliases=ALIASES['series'],
                                   help='Manage series of patches')
    series.defaults_cmds = [
        ['set-link', 'fred'],
    ]
    series.add_argument(
        '-n', '--dry-run', action='store_true', dest='dry_run', default=False,
        help="Do a dry run (create but don't email patches)")
    series.add_argument('-s', '--series', help='Name of series')
    series.add_argument('-V', '--version', type=int,
                        help='Version number to link')
    series_subparsers = series.add_subparsers(dest='subcmd')

    # This causes problem at present, perhaps due to the 'defaults' handling in
    # settings
    # series_subparsers.required = True

    add = series_subparsers.add_parser('add')
    add.add_argument('-D', '--desc',
                     help='Series description / cover-letter title')
    add.add_argument(
        '-f', '--force-version', action='store_true',
        help='Change the Series-version on a series to match its branch')
    _add_mark(add)
    _add_allow_unmarked(add)
    _upstream_add(add)

    series_subparsers.add_parser('archive', aliases=ALIASES['archive'])

    auto = series_subparsers.add_parser('autolink',
                                        aliases=ALIASES['autolink'])
    _add_update(auto)
    _add_wait(auto, 0)

    aall = series_subparsers.add_parser('autolink-all')
    aall.add_argument('-a', '--link-all-versions', action='store_true',
                      help='Link all series versions, not just the latest')
    aall.add_argument('-r', '--replace-existing', action='store_true',
                      help='Replace existing links')
    _add_update(aall)

    series_subparsers.add_parser('dec')

    gat = series_subparsers.add_parser('gather', aliases=ALIASES['gather'])
    _add_gather(gat)
    _add_show_comments(gat)
    _add_show_cover_comments(gat)

    sall = series_subparsers.add_parser('gather-all')
    sall.add_argument(
        '-a', '--gather-all-versions', action='store_true',
        help='Gather tags from all series versions, not just the latest')
    _add_gather(sall)
    _add_show_comments(sall)
    _add_show_cover_comments(sall)

    series_subparsers.add_parser('get-link')
    series_subparsers.add_parser('inc')
    series_subparsers.add_parser('ls')

    mar = series_subparsers.add_parser('mark')
    mar.add_argument('-m', '--allow-marked', action='store_true',
                     default=False,
                     help="Don't require commits to be unmarked")

    series_subparsers.add_parser('open', aliases=ALIASES['open'])
    pat = series_subparsers.add_parser(
        'patches', epilog='Show a list of patches and optional details')
    pat.add_argument('-t', '--commit', action='store_true',
                     help='Show the commit and diffstat')
    pat.add_argument('-p', '--patch', action='store_true',
                     help='Show the patch body')

    prog = series_subparsers.add_parser('progress',
                                        aliases=ALIASES['progress'])
    prog.add_argument('-a', '--show-all-versions', action='store_true',
                      help='Show all series versions, not just the latest')
    prog.add_argument('-l', '--list-patches', action='store_true',
                      help='List patch subject and status')

    ren = series_subparsers.add_parser('rename')
    ren.add_argument('-N', '--new-name', help='New name for the series')

    series_subparsers.add_parser('rm')
    series_subparsers.add_parser('rm-version', aliases=ALIASES['rm-version'])

    scan = series_subparsers.add_parser('scan')
    _add_mark(scan)
    _add_allow_unmarked(scan)
    _upstream_add(scan)

    ssend = series_subparsers.add_parser('send')
    add_send_args(ssend)
    ssend.add_argument(
        '--no-autolink', action='store_false', default=True, dest='autolink',
        help='Monitor patchwork after sending so the series can be autolinked')
    _add_wait(ssend, 120)

    setl = series_subparsers.add_parser('set-link')
    _add_update(setl)

    setl.add_argument(
        'link', help='Link to use, i.e. patchwork series number (e.g. 452329)')
    stat = series_subparsers.add_parser('status', aliases=ALIASES['status'])
    _add_show_comments(stat)
    _add_show_cover_comments(stat)

    series_subparsers.add_parser('summary')

    series_subparsers.add_parser('unarchive', aliases=ALIASES['unarchive'])

    unm = series_subparsers.add_parser('unmark')
    _add_allow_unmarked(unm)

    ver = series_subparsers.add_parser(
        'version-change', help='Change a version to a different version')
    ver.add_argument('--new-version', type=int,
                     help='New version number to change this one too')

    return series


def add_send_subparser(subparsers):
    """Add the 'send' subparser

    Args:
        subparsers (argparse action): Subparser parent

    Return:
        ArgumentParser: send subparser
    """
    send = subparsers.add_parser(
        'send', help='Format, check and email patches (default command)')
    send.add_argument(
        '-b', '--branch', type=str,
        help="Branch to process (by default, the current branch)")
    send.add_argument(
        '-n', '--dry-run', action='store_true', dest='dry_run',
        default=False, help="Do a dry run (create but don't email patches)")
    send.add_argument(
        '--cc-cmd', dest='cc_cmd', type=str, action='store',
        default=None, help='Output cc list for patch file (used by git)')
    add_send_args(send)
    send.add_argument('patchfiles', nargs='*')
    return send


def add_status_subparser(subparsers):
    """Add the 'status' subparser

    Args:
        subparsers (argparse action): Subparser parent

    Return:
        ArgumentParser: status subparser
    """
    status = subparsers.add_parser('status', aliases=ALIASES['status'],
                                   help='Check status of patches in patchwork')
    _add_show_comments(status)
    status.add_argument(
        '-d', '--dest-branch', type=str,
        help='Name of branch to create with collected responses')
    status.add_argument('-f', '--force', action='store_true',
                        help='Force overwriting an existing branch')
    status.add_argument('-T', '--single-thread', action='store_true',
                        help='Disable multithreading when reading patchwork')
    return status


def add_upstream_subparser(subparsers):
    """Add the 'status' subparser

    Args:
        subparsers (argparse action): Subparser parent

    Return:
        ArgumentParser: status subparser
    """
    upstream = subparsers.add_parser('upstream', aliases=ALIASES['upstream'],
                                     help='Manage upstream destinations')
    upstream.defaults_cmds = [
        ['add', 'us', 'http://fred'],
        ['delete', 'us'],
    ]
    upstream_subparsers = upstream.add_subparsers(dest='subcmd')
    uadd = upstream_subparsers.add_parser('add')
    uadd.add_argument('remote_name',
                      help="Git remote name used for this upstream, e.g. 'us'")
    uadd.add_argument(
        'url', help='URL to use for this upstream, e.g. '
                    "'https://gitlab.denx.de/u-boot/u-boot.git'")
    udel = upstream_subparsers.add_parser('delete')
    udel.add_argument(
        'remote_name',
        help="Git remote name used for this upstream, e.g. 'us'")
    upstream_subparsers.add_parser('list')
    udef = upstream_subparsers.add_parser('default')
    udef.add_argument('-u', '--unset', action='store_true',
                      help='Unset the default upstream')
    udef.add_argument('remote_name', nargs='?',
                      help="Git remote name used for this upstream, e.g. 'us'")
    return upstream


def setup_parser():
    """Set up command-line parser

    Returns:
        argparse.Parser object
    """
    epilog = '''Create patches from commits in a branch, check them and email
        them as specified by tags you place in the commits. Use -n to do a dry
        run first.'''

    parser = ErrorCatchingArgumentParser(epilog=epilog)
    parser.add_argument(
        '-D', '--debug', action='store_true',
        help='Enabling debugging (provides a full traceback on error)')
    parser.add_argument(
        '-N', '--no-capture', action='store_true',
        help='Disable capturing of console output in tests')
    parser.add_argument('-p', '--project', default=project.detect_project(),
                        help="Project name; affects default option values and "
                        "aliases [default: %(default)s]")
    parser.add_argument('-P', '--patchwork-url',
                        default='https://patchwork.ozlabs.org',
                        help='URL of patchwork server [default: %(default)s]')
    parser.add_argument(
        '-T', '--thread', action='store_true', dest='thread',
        default=False, help='Create patches as a single thread')
    parser.add_argument(
        '-v', '--verbose', action='store_true', dest='verbose', default=False,
        help='Verbose output of errors and warnings')
    parser.add_argument(
        '-X', '--test-preserve-dirs', action='store_true',
        help='Preserve and display test-created directories')
    parser.add_argument(
        '-H', '--full-help', action='store_true', dest='full_help',
        default=False, help='Display the README file')

    subparsers = parser.add_subparsers(dest='cmd')
    add_send_subparser(subparsers)
    patchwork = add_patchwork_subparser(subparsers)
    series = add_series_subparser(subparsers)
    add_status_subparser(subparsers)
    upstream = add_upstream_subparser(subparsers)

    # Only add the 'test' action if the test data files are available.
    if HAS_TESTS:
        test_parser = subparsers.add_parser('test', help='Run tests')
        test_parser.add_argument('testname', type=str, default=None, nargs='?',
                                 help="Specify the test to run")

    parsers = {
        'main': parser,
        'series': series,
        'patchwork': patchwork,
        'upstream': upstream,
        }
    return parsers


def parse_args(argv=None, config_fname=None, parsers=None):
    """Parse command line arguments from sys.argv[]

    Args:
        argv (str or None): Arguments to process, or None to use sys.argv[1:]
        config_fname (str): Config file to read, or None for default, or False
            for an empty config

    Returns:
        tuple containing:
            options: command line options
            args: command lin arguments
    """
    if not parsers:
        parsers = setup_parser()
    parser = parsers['main']

    # Parse options twice: first to get the project and second to handle
    # defaults properly (which depends on project)
    # Use parse_known_args() in case 'cmd' is omitted
    if not argv:
        argv = sys.argv[1:]

    args, rest = parser.parse_known_args(argv)
    if hasattr(args, 'project'):
        settings.Setup(parser, args.project, argv, config_fname)
        args, rest = parser.parse_known_args(argv)

    # If we have a command, it is safe to parse all arguments
    if args.cmd:
        args = parser.parse_args(argv)
    elif not args.full_help:
        # No command, so insert it after the known arguments and before the ones
        # that presumably relate to the 'send' subcommand
        nargs = len(rest)
        argv = argv[:-nargs] + ['send'] + rest
        args = parser.parse_args(argv)

    # Resolve aliases
    for full, aliases in ALIASES.items():
        if args.cmd in aliases:
            args.cmd = full
        if 'subcmd' in args and args.subcmd in aliases:
            args.subcmd = full
    if args.cmd in ['series', 'upstream', 'patchwork'] and not args.subcmd:
        parser.parse_args([args.cmd, '--help'])

    return args

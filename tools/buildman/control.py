# Copyright (c) 2013 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import multiprocessing
import os
import sys

import board
import bsettings
from builder import Builder
import gitutil
import patchstream
import terminal
import toolchain
import command
import subprocess

def GetPlural(count):
    """Returns a plural 's' if count is not 1"""
    return 's' if count != 1 else ''

def GetActionSummary(is_summary, commits, selected, options):
    """Return a string summarising the intended action.

    Returns:
        Summary string.
    """
    if commits:
        count = len(commits)
        count = (count + options.step - 1) / options.step
        commit_str = '%d commit%s' % (count, GetPlural(count))
    else:
        commit_str = 'current source'
    str = '%s %s for %d boards' % (
        'Summary of' if is_summary else 'Building', commit_str,
        len(selected))
    str += ' (%d thread%s, %d job%s per thread)' % (options.threads,
            GetPlural(options.threads), options.jobs, GetPlural(options.jobs))
    return str

def ShowActions(series, why_selected, boards_selected, builder, options):
    """Display a list of actions that we would take, if not a dry run.

    Args:
        series: Series object
        why_selected: Dictionary where each key is a buildman argument
                provided by the user, and the value is the boards brought
                in by that argument. For example, 'arm' might bring in
                400 boards, so in this case the key would be 'arm' and
                the value would be a list of board names.
        boards_selected: Dict of selected boards, key is target name,
                value is Board object
        builder: The builder that will be used to build the commits
        options: Command line options object
    """
    col = terminal.Color()
    print 'Dry run, so not doing much. But I would do this:'
    print
    if series:
        commits = series.commits
    else:
        commits = None
    print GetActionSummary(False, commits, boards_selected,
            options)
    print 'Build directory: %s' % builder.base_dir
    if commits:
        for upto in range(0, len(series.commits), options.step):
            commit = series.commits[upto]
            print '   ', col.Color(col.YELLOW, commit.hash, bright=False),
            print commit.subject
    print
    for arg in why_selected:
        if arg != 'all':
            print arg, ': %d boards' % why_selected[arg]
    print ('Total boards to build for each commit: %d\n' %
            why_selected['all'])

def DoBuildman(options, args):
    """The main control code for buildman

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
    """
    gitutil.Setup()

    bsettings.Setup(options.config_file)
    options.git_dir = os.path.join(options.git, '.git')

    toolchains = toolchain.Toolchains()
    toolchains.Scan(options.list_tool_chains)
    if options.list_tool_chains:
        toolchains.List()
        print
        return

    # Work out how many commits to build. We want to build everything on the
    # branch. We also build the upstream commit as a control so we can see
    # problems introduced by the first commit on the branch.
    col = terminal.Color()
    count = options.count
    if count == -1:
        if not options.branch:
            count = 1
        else:
            count = gitutil.CountCommitsInBranch(options.git_dir,
                                                 options.branch)
            if count is None:
                str = ("Branch '%s' not found or has no upstream" %
                       options.branch)
                sys.exit(col.Color(col.RED, str))
            count += 1   # Build upstream commit also

    if not count:
        str = ("No commits found to process in branch '%s': "
               "set branch's upstream or use -c flag" % options.branch)
        sys.exit(col.Color(col.RED, str))

    # Work out what subset of the boards we are building
    board_file = os.path.join(options.git, 'boards.cfg')
    status = subprocess.call([os.path.join(options.git,
                                           'tools/genboardscfg.py')])
    if status != 0:
        sys.exit("Failed to generate boards.cfg")

    boards = board.Boards()
    boards.ReadBoards(os.path.join(options.git, 'boards.cfg'))
    why_selected = boards.SelectBoards(args)
    selected = boards.GetSelected()
    if not len(selected):
        sys.exit(col.Color(col.RED, 'No matching boards found'))

    # Read the metadata from the commits. First look at the upstream commit,
    # then the ones in the branch. We would like to do something like
    # upstream/master~..branch but that isn't possible if upstream/master is
    # a merge commit (it will list all the commits that form part of the
    # merge)
    if options.branch:
        if count == -1:
            range_expr = gitutil.GetRangeInBranch(options.git_dir,
                                                  options.branch)
            upstream_commit = gitutil.GetUpstream(options.git_dir,
                                                  options.branch)
            series = patchstream.GetMetaDataForList(upstream_commit,
                options.git_dir, 1)

            # Conflicting tags are not a problem for buildman, since it does
            # not use them. For example, Series-version is not useful for
            # buildman. On the other hand conflicting tags will cause an
            # error. So allow later tags to overwrite earlier ones.
            series.allow_overwrite = True
            series = patchstream.GetMetaDataForList(range_expr,
                                              options.git_dir, None, series)
        else:
            # Honour the count
            series = patchstream.GetMetaDataForList(options.branch,
                                                    options.git_dir, count)
    else:
        series = None
        options.verbose = True
        options.show_errors = True

    # By default we have one thread per CPU. But if there are not enough jobs
    # we can have fewer threads and use a high '-j' value for make.
    if not options.threads:
        options.threads = min(multiprocessing.cpu_count(), len(selected))
    if not options.jobs:
        options.jobs = max(1, (multiprocessing.cpu_count() +
                len(selected) - 1) / len(selected))

    if not options.step:
        options.step = len(series.commits) - 1

    gnu_make = command.Output(os.path.join(options.git,
                                           'scripts/show-gnu-make')).rstrip()
    if not gnu_make:
        sys.exit('GNU Make not found')

    # Create a new builder with the selected options
    if options.branch:
        dirname = options.branch
    else:
        dirname = 'current'
    output_dir = os.path.join(options.output_dir, dirname)
    builder = Builder(toolchains, output_dir, options.git_dir,
            options.threads, options.jobs, gnu_make=gnu_make, checkout=True,
            show_unknown=options.show_unknown, step=options.step)
    builder.force_config_on_failure = not options.quick

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        ShowActions(series, why_selected, selected, builder, options)
    else:
        builder.force_build = options.force_build
        builder.force_build_failures = options.force_build_failures
        builder.force_reconfig = options.force_reconfig
        builder.in_tree = options.in_tree

        # Work out which boards to build
        board_selected = boards.GetSelectedDict()

        if series:
            commits = series.commits
        else:
            commits = None

        print GetActionSummary(options.summary, commits, board_selected,
                               options)

        builder.SetDisplayOptions(options.show_errors, options.show_sizes,
                                  options.show_detail, options.show_bloat)
        if options.summary:
            # We can't show function sizes without board details at present
            if options.show_bloat:
                options.show_detail = True
            builder.ShowSummary(commits, board_selected)
        else:
            builder.BuildBoards(commits, board_selected,
                                options.keep_outputs, options.verbose)

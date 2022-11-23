# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2013 The Chromium OS Authors.
#

import multiprocessing
import os
import shutil
import subprocess
import sys

from buildman import boards
from buildman import bsettings
from buildman import cfgutil
from buildman import toolchain
from buildman.builder import Builder
from patman import command
from patman import gitutil
from patman import patchstream
from patman import terminal
from patman import tools
from patman.terminal import tprint

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
        count = (count + options.step - 1) // options.step
        commit_str = '%d commit%s' % (count, GetPlural(count))
    else:
        commit_str = 'current source'
    str = '%s %s for %d boards' % (
        'Summary of' if is_summary else 'Building', commit_str,
        len(selected))
    str += ' (%d thread%s, %d job%s per thread)' % (options.threads,
            GetPlural(options.threads), options.jobs, GetPlural(options.jobs))
    return str

def ShowActions(series, why_selected, boards_selected, builder, options,
                board_warnings):
    """Display a list of actions that we would take, if not a dry run.

    Args:
        series: Series object
        why_selected: Dictionary where each key is a buildman argument
                provided by the user, and the value is the list of boards
                brought in by that argument. For example, 'arm' might bring
                in 400 boards, so in this case the key would be 'arm' and
                the value would be a list of board names.
        boards_selected: Dict of selected boards, key is target name,
                value is Board object
        builder: The builder that will be used to build the commits
        options: Command line options object
        board_warnings: List of warnings obtained from board selected
    """
    col = terminal.Color()
    print('Dry run, so not doing much. But I would do this:')
    print()
    if series:
        commits = series.commits
    else:
        commits = None
    print(GetActionSummary(False, commits, boards_selected,
            options))
    print('Build directory: %s' % builder.base_dir)
    if commits:
        for upto in range(0, len(series.commits), options.step):
            commit = series.commits[upto]
            print('   ', col.build(col.YELLOW, commit.hash[:8], bright=False), end=' ')
            print(commit.subject)
    print()
    for arg in why_selected:
        if arg != 'all':
            print(arg, ': %d boards' % len(why_selected[arg]))
            if options.verbose:
                print('   %s' % ' '.join(why_selected[arg]))
    print(('Total boards to build for each commit: %d\n' %
            len(why_selected['all'])))
    if board_warnings:
        for warning in board_warnings:
            print(col.build(col.YELLOW, warning))

def ShowToolchainPrefix(brds, toolchains):
    """Show information about a the tool chain used by one or more boards

    The function checks that all boards use the same toolchain, then prints
    the correct value for CROSS_COMPILE.

    Args:
        boards: Boards object containing selected boards
        toolchains: Toolchains object containing available toolchains

    Return:
        None on success, string error message otherwise
    """
    board_selected = brds.get_selected_dict()
    tc_set = set()
    for brd in board_selected.values():
        tc_set.add(toolchains.Select(brd.arch))
    if len(tc_set) != 1:
        return 'Supplied boards must share one toolchain'
        return False
    tc = tc_set.pop()
    print(tc.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))
    return None

def get_allow_missing(opt_allow, opt_no_allow, num_selected, has_branch):
    allow_missing = False
    am_setting = bsettings.GetGlobalItemValue('allow-missing')
    if am_setting:
        if am_setting == 'always':
            allow_missing = True
        if 'multiple' in am_setting and num_selected > 1:
            allow_missing = True
        if 'branch' in am_setting and has_branch:
            allow_missing = True

    if opt_allow:
        allow_missing = True
    if opt_no_allow:
        allow_missing = False
    return allow_missing

def DoBuildman(options, args, toolchains=None, make_func=None, brds=None,
               clean_dir=False, test_thread_exceptions=False):
    """The main control code for buildman

    Args:
        options: Command line options object
        args: Command line arguments (list of strings)
        toolchains: Toolchains to use - this should be a Toolchains()
                object. If None, then it will be created and scanned
        make_func: Make function to use for the builder. This is called
                to execute 'make'. If this is None, the normal function
                will be used, which calls the 'make' tool with suitable
                arguments. This setting is useful for tests.
        brds: Boards() object to use, containing a list of available
                boards. If this is None it will be created and scanned.
        clean_dir: Used for tests only, indicates that the existing output_dir
            should be removed before starting the build
        test_thread_exceptions: Uses for tests only, True to make the threads
            raise an exception instead of reporting their result. This simulates
            a failure in the code somewhere
    """
    global builder

    if options.full_help:
        tools.print_full_help(
            os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                         'README.rst'))
        return 0

    gitutil.setup()
    col = terminal.Color()

    options.git_dir = os.path.join(options.git, '.git')

    no_toolchains = toolchains is None
    if no_toolchains:
        toolchains = toolchain.Toolchains(options.override_toolchain)

    if options.fetch_arch:
        if options.fetch_arch == 'list':
            sorted_list = toolchains.ListArchs()
            print(col.build(col.BLUE, 'Available architectures: %s\n' %
                            ' '.join(sorted_list)))
            return 0
        else:
            fetch_arch = options.fetch_arch
            if fetch_arch == 'all':
                fetch_arch = ','.join(toolchains.ListArchs())
                print(col.build(col.CYAN, '\nDownloading toolchains: %s' %
                                fetch_arch))
            for arch in fetch_arch.split(','):
                print()
                ret = toolchains.FetchAndInstall(arch)
                if ret:
                    return ret
            return 0

    if no_toolchains:
        toolchains.GetSettings()
        toolchains.Scan(options.list_tool_chains and options.verbose)
    if options.list_tool_chains:
        toolchains.List()
        print()
        return 0

    if not options.output_dir:
        if options.work_in_output:
            sys.exit(col.build(col.RED, '-w requires that you specify -o'))
        options.output_dir = '..'

    # Work out what subset of the boards we are building
    if not brds:
        if not os.path.exists(options.output_dir):
            os.makedirs(options.output_dir)
        board_file = os.path.join(options.output_dir, 'boards.cfg')

        brds = boards.Boards()
        ok = brds.ensure_board_list(board_file,
                                    options.threads or multiprocessing.cpu_count(),
                                    force=options.regen_board_list,
                                    quiet=not options.verbose)
        if options.regen_board_list:
            return 0 if ok else 2
        brds.read_boards(board_file)

    exclude = []
    if options.exclude:
        for arg in options.exclude:
            exclude += arg.split(',')

    if options.boards:
        requested_boards = []
        for b in options.boards:
            requested_boards += b.split(',')
    else:
        requested_boards = None
    why_selected, board_warnings = brds.select_boards(args, exclude,
                                                      requested_boards)
    selected = brds.get_selected()
    if not len(selected):
        sys.exit(col.build(col.RED, 'No matching boards found'))

    if options.print_prefix:
        err = ShowToolchainPrefix(brds, toolchains)
        if err:
            sys.exit(col.build(col.RED, err))
        return 0

    # Work out how many commits to build. We want to build everything on the
    # branch. We also build the upstream commit as a control so we can see
    # problems introduced by the first commit on the branch.
    count = options.count
    has_range = options.branch and '..' in options.branch
    if count == -1:
        if not options.branch:
            count = 1
        else:
            if has_range:
                count, msg = gitutil.count_commits_in_range(options.git_dir,
                                                         options.branch)
            else:
                count, msg = gitutil.count_commits_in_branch(options.git_dir,
                                                          options.branch)
            if count is None:
                sys.exit(col.build(col.RED, msg))
            elif count == 0:
                sys.exit(col.build(col.RED, "Range '%s' has no commits" %
                                   options.branch))
            if msg:
                print(col.build(col.YELLOW, msg))
            count += 1   # Build upstream commit also

    if not count:
        str = ("No commits found to process in branch '%s': "
               "set branch's upstream or use -c flag" % options.branch)
        sys.exit(col.build(col.RED, str))
    if options.work_in_output:
        if len(selected) != 1:
            sys.exit(col.build(col.RED,
                               '-w can only be used with a single board'))
        if count != 1:
            sys.exit(col.build(col.RED,
                               '-w can only be used with a single commit'))

    # Read the metadata from the commits. First look at the upstream commit,
    # then the ones in the branch. We would like to do something like
    # upstream/master~..branch but that isn't possible if upstream/master is
    # a merge commit (it will list all the commits that form part of the
    # merge)
    # Conflicting tags are not a problem for buildman, since it does not use
    # them. For example, Series-version is not useful for buildman. On the
    # other hand conflicting tags will cause an error. So allow later tags
    # to overwrite earlier ones by setting allow_overwrite=True
    if options.branch:
        if count == -1:
            if has_range:
                range_expr = options.branch
            else:
                range_expr = gitutil.get_range_in_branch(options.git_dir,
                                                      options.branch)
            upstream_commit = gitutil.get_upstream(options.git_dir,
                                                  options.branch)
            series = patchstream.get_metadata_for_list(upstream_commit,
                options.git_dir, 1, series=None, allow_overwrite=True)

            series = patchstream.get_metadata_for_list(range_expr,
                    options.git_dir, None, series, allow_overwrite=True)
        else:
            # Honour the count
            series = patchstream.get_metadata_for_list(options.branch,
                    options.git_dir, count, series=None, allow_overwrite=True)
    else:
        series = None
        if not options.dry_run:
            options.verbose = True
            if not options.summary:
                options.show_errors = True

    # By default we have one thread per CPU. But if there are not enough jobs
    # we can have fewer threads and use a high '-j' value for make.
    if options.threads is None:
        options.threads = min(multiprocessing.cpu_count(), len(selected))
    if not options.jobs:
        options.jobs = max(1, (multiprocessing.cpu_count() +
                len(selected) - 1) // len(selected))

    if not options.step:
        options.step = len(series.commits) - 1

    gnu_make = command.output(os.path.join(options.git,
            'scripts/show-gnu-make'), raise_on_error=False).rstrip()
    if not gnu_make:
        sys.exit('GNU Make not found')

    allow_missing = get_allow_missing(options.allow_missing,
                                      options.no_allow_missing, len(selected),
                                      options.branch)

    # Create a new builder with the selected options.
    output_dir = options.output_dir
    if options.branch:
        dirname = options.branch.replace('/', '_')
        # As a special case allow the board directory to be placed in the
        # output directory itself rather than any subdirectory.
        if not options.no_subdirs:
            output_dir = os.path.join(options.output_dir, dirname)
        if clean_dir and os.path.exists(output_dir):
            shutil.rmtree(output_dir)
    adjust_cfg = cfgutil.convert_list_to_dict(options.adjust_cfg)

    builder = Builder(toolchains, output_dir, options.git_dir,
            options.threads, options.jobs, gnu_make=gnu_make, checkout=True,
            show_unknown=options.show_unknown, step=options.step,
            no_subdirs=options.no_subdirs, full_path=options.full_path,
            verbose_build=options.verbose_build,
            mrproper=options.mrproper,
            per_board_out_dir=options.per_board_out_dir,
            config_only=options.config_only,
            squash_config_y=not options.preserve_config_y,
            warnings_as_errors=options.warnings_as_errors,
            work_in_output=options.work_in_output,
            test_thread_exceptions=test_thread_exceptions,
            adjust_cfg=adjust_cfg,
            allow_missing=allow_missing)
    builder.force_config_on_failure = not options.quick
    if make_func:
        builder.do_make = make_func

    # For a dry run, just show our actions as a sanity check
    if options.dry_run:
        ShowActions(series, why_selected, selected, builder, options,
                    board_warnings)
    else:
        builder.force_build = options.force_build
        builder.force_build_failures = options.force_build_failures
        builder.force_reconfig = options.force_reconfig
        builder.in_tree = options.in_tree

        # Work out which boards to build
        board_selected = brds.get_selected_dict()

        if series:
            commits = series.commits
            # Number the commits for test purposes
            for commit in range(len(commits)):
                commits[commit].sequence = commit
        else:
            commits = None

        if not options.ide:
            tprint(GetActionSummary(options.summary, commits, board_selected,
                                    options))

        # We can't show function sizes without board details at present
        if options.show_bloat:
            options.show_detail = True
        builder.SetDisplayOptions(
            options.show_errors, options.show_sizes, options.show_detail,
            options.show_bloat, options.list_error_boards, options.show_config,
            options.show_environment, options.filter_dtb_warnings,
            options.filter_migration_warnings, options.ide)
        if options.summary:
            builder.ShowSummary(commits, board_selected)
        else:
            fail, warned, excs = builder.BuildBoards(
                commits, board_selected, options.keep_outputs, options.verbose)
            if excs:
                return 102
            elif fail:
                return 100
            elif warned and not options.ignore_warnings:
                return 101
    return 0

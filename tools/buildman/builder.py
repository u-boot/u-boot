# Copyright (c) 2013 The Chromium OS Authors.
#
# Bloat-o-meter code used here Copyright 2004 Matt Mackall <mpm@selenic.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

import collections
import errno
from datetime import datetime, timedelta
import glob
import os
import re
import Queue
import shutil
import string
import sys
import threading
import time

import command
import gitutil
import terminal
import toolchain


"""
Theory of Operation

Please see README for user documentation, and you should be familiar with
that before trying to make sense of this.

Buildman works by keeping the machine as busy as possible, building different
commits for different boards on multiple CPUs at once.

The source repo (self.git_dir) contains all the commits to be built. Each
thread works on a single board at a time. It checks out the first commit,
configures it for that board, then builds it. Then it checks out the next
commit and builds it (typically without re-configuring). When it runs out
of commits, it gets another job from the builder and starts again with that
board.

Clearly the builder threads could work either way - they could check out a
commit and then built it for all boards. Using separate directories for each
commit/board pair they could leave their build product around afterwards
also.

The intent behind building a single board for multiple commits, is to make
use of incremental builds. Since each commit is built incrementally from
the previous one, builds are faster. Reconfiguring for a different board
removes all intermediate object files.

Many threads can be working at once, but each has its own working directory.
When a thread finishes a build, it puts the output files into a result
directory.

The base directory used by buildman is normally '../<branch>', i.e.
a directory higher than the source repository and named after the branch
being built.

Within the base directory, we have one subdirectory for each commit. Within
that is one subdirectory for each board. Within that is the build output for
that commit/board combination.

Buildman also create working directories for each thread, in a .bm-work/
subdirectory in the base dir.

As an example, say we are building branch 'us-net' for boards 'sandbox' and
'seaboard', and say that us-net has two commits. We will have directories
like this:

us-net/             base directory
    01_of_02_g4ed4ebc_net--Add-tftp-speed-/
        sandbox/
            u-boot.bin
        seaboard/
            u-boot.bin
    02_of_02_g4ed4ebc_net--Check-tftp-comp/
        sandbox/
            u-boot.bin
        seaboard/
            u-boot.bin
    .bm-work/
        00/         working directory for thread 0 (contains source checkout)
            build/  build output
        01/         working directory for thread 1
            build/  build output
        ...
u-boot/             source directory
    .git/           repository
"""

# Possible build outcomes
OUTCOME_OK, OUTCOME_WARNING, OUTCOME_ERROR, OUTCOME_UNKNOWN = range(4)

# Translate a commit subject into a valid filename
trans_valid_chars = string.maketrans("/: ", "---")


def Mkdir(dirname):
    """Make a directory if it doesn't already exist.

    Args:
        dirname: Directory to create
    """
    try:
        os.mkdir(dirname)
    except OSError as err:
        if err.errno == errno.EEXIST:
            pass
        else:
            raise

class BuilderJob:
    """Holds information about a job to be performed by a thread

    Members:
        board: Board object to build
        commits: List of commit options to build.
    """
    def __init__(self):
        self.board = None
        self.commits = []


class ResultThread(threading.Thread):
    """This thread processes results from builder threads.

    It simply passes the results on to the builder. There is only one
    result thread, and this helps to serialise the build output.
    """
    def __init__(self, builder):
        """Set up a new result thread

        Args:
            builder: Builder which will be sent each result
        """
        threading.Thread.__init__(self)
        self.builder = builder

    def run(self):
        """Called to start up the result thread.

        We collect the next result job and pass it on to the build.
        """
        while True:
            result = self.builder.out_queue.get()
            self.builder.ProcessResult(result)
            self.builder.out_queue.task_done()


class BuilderThread(threading.Thread):
    """This thread builds U-Boot for a particular board.

    An input queue provides each new job. We run 'make' to build U-Boot
    and then pass the results on to the output queue.

    Members:
        builder: The builder which contains information we might need
        thread_num: Our thread number (0-n-1), used to decide on a
                temporary directory
    """
    def __init__(self, builder, thread_num):
        """Set up a new builder thread"""
        threading.Thread.__init__(self)
        self.builder = builder
        self.thread_num = thread_num

    def Make(self, commit, brd, stage, cwd, *args, **kwargs):
        """Run 'make' on a particular commit and board.

        The source code will already be checked out, so the 'commit'
        argument is only for information.

        Args:
            commit: Commit object that is being built
            brd: Board object that is being built
            stage: Stage of the build. Valid stages are:
                        distclean - can be called to clean source
                        config - called to configure for a board
                        build - the main make invocation - it does the build
            args: A list of arguments to pass to 'make'
            kwargs: A list of keyword arguments to pass to command.RunPipe()

        Returns:
            CommandResult object
        """
        return self.builder.do_make(commit, brd, stage, cwd, *args,
                **kwargs)

    def RunCommit(self, commit_upto, brd, work_dir, do_config, force_build):
        """Build a particular commit.

        If the build is already done, and we are not forcing a build, we skip
        the build and just return the previously-saved results.

        Args:
            commit_upto: Commit number to build (0...n-1)
            brd: Board object to build
            work_dir: Directory to which the source will be checked out
            do_config: True to run a make <board>_config on the source
            force_build: Force a build even if one was previously done

        Returns:
            tuple containing:
                - CommandResult object containing the results of the build
                - boolean indicating whether 'make config' is still needed
        """
        # Create a default result - it will be overwritte by the call to
        # self.Make() below, in the event that we do a build.
        result = command.CommandResult()
        result.return_code = 0
        out_dir = os.path.join(work_dir, 'build')

        # Check if the job was already completed last time
        done_file = self.builder.GetDoneFile(commit_upto, brd.target)
        result.already_done = os.path.exists(done_file)
        if result.already_done and not force_build:
            # Get the return code from that build and use it
            with open(done_file, 'r') as fd:
                result.return_code = int(fd.readline())
            err_file = self.builder.GetErrFile(commit_upto, brd.target)
            if os.path.exists(err_file) and os.stat(err_file).st_size:
                result.stderr = 'bad'
        else:
            # We are going to have to build it. First, get a toolchain
            if not self.toolchain:
                try:
                    self.toolchain = self.builder.toolchains.Select(brd.arch)
                except ValueError as err:
                    result.return_code = 10
                    result.stdout = ''
                    result.stderr = str(err)
                    # TODO(sjg@chromium.org): This gets swallowed, but needs
                    # to be reported.

            if self.toolchain:
                # Checkout the right commit
                if commit_upto is not None:
                    commit = self.builder.commits[commit_upto]
                    if self.builder.checkout:
                        git_dir = os.path.join(work_dir, '.git')
                        gitutil.Checkout(commit.hash, git_dir, work_dir,
                                         force=True)
                else:
                    commit = self.builder.commit # Ick, fix this for BuildCommits()

                # Set up the environment and command line
                env = self.toolchain.MakeEnvironment()
                Mkdir(out_dir)
                args = ['O=build', '-s']
                if self.builder.num_jobs is not None:
                    args.extend(['-j', str(self.builder.num_jobs)])
                config_args = ['%s_config' % brd.target]
                config_out = ''
                args.extend(self.builder.toolchains.GetMakeArguments(brd))

                # If we need to reconfigure, do that now
                if do_config:
                    result = self.Make(commit, brd, 'distclean', work_dir,
                            'distclean', *args, env=env)
                    result = self.Make(commit, brd, 'config', work_dir,
                            *(args + config_args), env=env)
                    config_out = result.combined
                    do_config = False   # No need to configure next time
                if result.return_code == 0:
                    result = self.Make(commit, brd, 'build', work_dir, *args,
                            env=env)
                    result.stdout = config_out + result.stdout
            else:
                result.return_code = 1
                result.stderr = 'No tool chain for %s\n' % brd.arch
            result.already_done = False

        result.toolchain = self.toolchain
        result.brd = brd
        result.commit_upto = commit_upto
        result.out_dir = out_dir
        return result, do_config

    def _WriteResult(self, result, keep_outputs):
        """Write a built result to the output directory.

        Args:
            result: CommandResult object containing result to write
            keep_outputs: True to store the output binaries, False
                to delete them
        """
        # Fatal error
        if result.return_code < 0:
            return

        # Aborted?
        if result.stderr and 'No child processes' in result.stderr:
            return

        if result.already_done:
            return

        # Write the output and stderr
        output_dir = self.builder._GetOutputDir(result.commit_upto)
        Mkdir(output_dir)
        build_dir = self.builder.GetBuildDir(result.commit_upto,
                result.brd.target)
        Mkdir(build_dir)

        outfile = os.path.join(build_dir, 'log')
        with open(outfile, 'w') as fd:
            if result.stdout:
                fd.write(result.stdout)

        errfile = self.builder.GetErrFile(result.commit_upto,
                result.brd.target)
        if result.stderr:
            with open(errfile, 'w') as fd:
                fd.write(result.stderr)
        elif os.path.exists(errfile):
            os.remove(errfile)

        if result.toolchain:
            # Write the build result and toolchain information.
            done_file = self.builder.GetDoneFile(result.commit_upto,
                    result.brd.target)
            with open(done_file, 'w') as fd:
                fd.write('%s' % result.return_code)
            with open(os.path.join(build_dir, 'toolchain'), 'w') as fd:
                print >>fd, 'gcc', result.toolchain.gcc
                print >>fd, 'path', result.toolchain.path
                print >>fd, 'cross', result.toolchain.cross
                print >>fd, 'arch', result.toolchain.arch
                fd.write('%s' % result.return_code)

            with open(os.path.join(build_dir, 'toolchain'), 'w') as fd:
                print >>fd, 'gcc', result.toolchain.gcc
                print >>fd, 'path', result.toolchain.path

            # Write out the image and function size information and an objdump
            env = result.toolchain.MakeEnvironment()
            lines = []
            for fname in ['u-boot', 'spl/u-boot-spl']:
                cmd = ['%snm' % self.toolchain.cross, '--size-sort', fname]
                nm_result = command.RunPipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                if nm_result.stdout:
                    nm = self.builder.GetFuncSizesFile(result.commit_upto,
                                    result.brd.target, fname)
                    with open(nm, 'w') as fd:
                        print >>fd, nm_result.stdout,

                cmd = ['%sobjdump' % self.toolchain.cross, '-h', fname]
                dump_result = command.RunPipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                rodata_size = ''
                if dump_result.stdout:
                    objdump = self.builder.GetObjdumpFile(result.commit_upto,
                                    result.brd.target, fname)
                    with open(objdump, 'w') as fd:
                        print >>fd, dump_result.stdout,
                    for line in dump_result.stdout.splitlines():
                        fields = line.split()
                        if len(fields) > 5 and fields[1] == '.rodata':
                            rodata_size = fields[2]

                cmd = ['%ssize' % self.toolchain.cross, fname]
                size_result = command.RunPipe([cmd], capture=True,
                        capture_stderr=True, cwd=result.out_dir,
                        raise_on_error=False, env=env)
                if size_result.stdout:
                    lines.append(size_result.stdout.splitlines()[1] + ' ' +
                                 rodata_size)

            # Write out the image sizes file. This is similar to the output
            # of binutil's 'size' utility, but it omits the header line and
            # adds an additional hex value at the end of each line for the
            # rodata size
            if len(lines):
                sizes = self.builder.GetSizesFile(result.commit_upto,
                                result.brd.target)
                with open(sizes, 'w') as fd:
                    print >>fd, '\n'.join(lines)

        # Now write the actual build output
        if keep_outputs:
            patterns = ['u-boot', '*.bin', 'u-boot.dtb', '*.map',
                        'include/autoconf.mk', 'spl/u-boot-spl',
                        'spl/u-boot-spl.bin']
            for pattern in patterns:
                file_list = glob.glob(os.path.join(result.out_dir, pattern))
                for fname in file_list:
                    shutil.copy(fname, build_dir)


    def RunJob(self, job):
        """Run a single job

        A job consists of a building a list of commits for a particular board.

        Args:
            job: Job to build
        """
        brd = job.board
        work_dir = self.builder.GetThreadDir(self.thread_num)
        self.toolchain = None
        if job.commits:
            # Run 'make board_config' on the first commit
            do_config = True
            commit_upto  = 0
            force_build = False
            for commit_upto in range(0, len(job.commits), job.step):
                result, request_config = self.RunCommit(commit_upto, brd,
                        work_dir, do_config,
                        force_build or self.builder.force_build)
                failed = result.return_code or result.stderr
                if failed and not do_config:
                    # If our incremental build failed, try building again
                    # with a reconfig.
                    if self.builder.force_config_on_failure:
                        result, request_config = self.RunCommit(commit_upto,
                            brd, work_dir, True, True)
                do_config = request_config

                # If we built that commit, then config is done. But if we got
                # an warning, reconfig next time to force it to build the same
                # files that created warnings this time. Otherwise an
                # incremental build may not build the same file, and we will
                # think that the warning has gone away.
                # We could avoid this by using -Werror everywhere...
                # For errors, the problem doesn't happen, since presumably
                # the build stopped and didn't generate output, so will retry
                # that file next time. So we could detect warnings and deal
                # with them specially here. For now, we just reconfigure if
                # anything goes work.
                # Of course this is substantially slower if there are build
                # errors/warnings (e.g. 2-3x slower even if only 10% of builds
                # have problems).
                if (failed and not result.already_done and not do_config and
                        self.builder.force_config_on_failure):
                    # If this build failed, try the next one with a
                    # reconfigure.
                    # Sometimes if the board_config.h file changes it can mess
                    # with dependencies, and we get:
                    # make: *** No rule to make target `include/autoconf.mk',
                    #     needed by `depend'.
                    do_config = True
                    force_build = True
                else:
                    force_build = False
                    if self.builder.force_config_on_failure:
                        if failed:
                            do_config = True
                    result.commit_upto = commit_upto
                    if result.return_code < 0:
                        raise ValueError('Interrupt')

                # We have the build results, so output the result
                self._WriteResult(result, job.keep_outputs)
                self.builder.out_queue.put(result)
        else:
            # Just build the currently checked-out build
            result = self.RunCommit(None, True)
            result.commit_upto = self.builder.upto
            self.builder.out_queue.put(result)

    def run(self):
        """Our thread's run function

        This thread picks a job from the queue, runs it, and then goes to the
        next job.
        """
        alive = True
        while True:
            job = self.builder.queue.get()
            try:
                if self.builder.active and alive:
                    self.RunJob(job)
            except Exception as err:
                alive = False
                print err
            self.builder.queue.task_done()


class Builder:
    """Class for building U-Boot for a particular commit.

    Public members: (many should ->private)
        active: True if the builder is active and has not been stopped
        already_done: Number of builds already completed
        base_dir: Base directory to use for builder
        checkout: True to check out source, False to skip that step.
            This is used for testing.
        col: terminal.Color() object
        count: Number of commits to build
        do_make: Method to call to invoke Make
        fail: Number of builds that failed due to error
        force_build: Force building even if a build already exists
        force_config_on_failure: If a commit fails for a board, disable
            incremental building for the next commit we build for that
            board, so that we will see all warnings/errors again.
        git_dir: Git directory containing source repository
        last_line_len: Length of the last line we printed (used for erasing
            it with new progress information)
        num_jobs: Number of jobs to run at once (passed to make as -j)
        num_threads: Number of builder threads to run
        out_queue: Queue of results to process
        re_make_err: Compiled regular expression for ignore_lines
        queue: Queue of jobs to run
        threads: List of active threads
        toolchains: Toolchains object to use for building
        upto: Current commit number we are building (0.count-1)
        warned: Number of builds that produced at least one warning

    Private members:
        _base_board_dict: Last-summarised Dict of boards
        _base_err_lines: Last-summarised list of errors
        _build_period_us: Time taken for a single build (float object).
        _complete_delay: Expected delay until completion (timedelta)
        _next_delay_update: Next time we plan to display a progress update
                (datatime)
        _show_unknown: Show unknown boards (those not built) in summary
        _timestamps: List of timestamps for the completion of the last
            last _timestamp_count builds. Each is a datetime object.
        _timestamp_count: Number of timestamps to keep in our list.
        _working_dir: Base working directory containing all threads
    """
    class Outcome:
        """Records a build outcome for a single make invocation

        Public Members:
            rc: Outcome value (OUTCOME_...)
            err_lines: List of error lines or [] if none
            sizes: Dictionary of image size information, keyed by filename
                - Each value is itself a dictionary containing
                    values for 'text', 'data' and 'bss', being the integer
                    size in bytes of each section.
            func_sizes: Dictionary keyed by filename - e.g. 'u-boot'. Each
                    value is itself a dictionary:
                        key: function name
                        value: Size of function in bytes
        """
        def __init__(self, rc, err_lines, sizes, func_sizes):
            self.rc = rc
            self.err_lines = err_lines
            self.sizes = sizes
            self.func_sizes = func_sizes

    def __init__(self, toolchains, base_dir, git_dir, num_threads, num_jobs,
                 checkout=True, show_unknown=True, step=1):
        """Create a new Builder object

        Args:
            toolchains: Toolchains object to use for building
            base_dir: Base directory to use for builder
            git_dir: Git directory containing source repository
            num_threads: Number of builder threads to run
            num_jobs: Number of jobs to run at once (passed to make as -j)
            checkout: True to check out source, False to skip that step.
                This is used for testing.
            show_unknown: Show unknown boards (those not built) in summary
            step: 1 to process every commit, n to process every nth commit
        """
        self.toolchains = toolchains
        self.base_dir = base_dir
        self._working_dir = os.path.join(base_dir, '.bm-work')
        self.threads = []
        self.active = True
        self.do_make = self.Make
        self.checkout = checkout
        self.num_threads = num_threads
        self.num_jobs = num_jobs
        self.already_done = 0
        self.force_build = False
        self.git_dir = git_dir
        self._show_unknown = show_unknown
        self._timestamp_count = 10
        self._build_period_us = None
        self._complete_delay = None
        self._next_delay_update = datetime.now()
        self.force_config_on_failure = True
        self._step = step

        self.col = terminal.Color()

        self.queue = Queue.Queue()
        self.out_queue = Queue.Queue()
        for i in range(self.num_threads):
            t = BuilderThread(self, i)
            t.setDaemon(True)
            t.start()
            self.threads.append(t)

        self.last_line_len = 0
        t = ResultThread(self)
        t.setDaemon(True)
        t.start()
        self.threads.append(t)

        ignore_lines = ['(make.*Waiting for unfinished)', '(Segmentation fault)']
        self.re_make_err = re.compile('|'.join(ignore_lines))

    def __del__(self):
        """Get rid of all threads created by the builder"""
        for t in self.threads:
            del t

    def _AddTimestamp(self):
        """Add a new timestamp to the list and record the build period.

        The build period is the length of time taken to perform a single
        build (one board, one commit).
        """
        now = datetime.now()
        self._timestamps.append(now)
        count = len(self._timestamps)
        delta = self._timestamps[-1] - self._timestamps[0]
        seconds = delta.total_seconds()

        # If we have enough data, estimate build period (time taken for a
        # single build) and therefore completion time.
        if count > 1 and self._next_delay_update < now:
            self._next_delay_update = now + timedelta(seconds=2)
            if seconds > 0:
                self._build_period = float(seconds) / count
                todo = self.count - self.upto
                self._complete_delay = timedelta(microseconds=
                        self._build_period * todo * 1000000)
                # Round it
                self._complete_delay -= timedelta(
                        microseconds=self._complete_delay.microseconds)

        if seconds > 60:
            self._timestamps.popleft()
            count -= 1

    def ClearLine(self, length):
        """Clear any characters on the current line

        Make way for a new line of length 'length', by outputting enough
        spaces to clear out the old line. Then remember the new length for
        next time.

        Args:
            length: Length of new line, in characters
        """
        if length < self.last_line_len:
            print ' ' * (self.last_line_len - length),
            print '\r',
        self.last_line_len = length
        sys.stdout.flush()

    def SelectCommit(self, commit, checkout=True):
        """Checkout the selected commit for this build
        """
        self.commit = commit
        if checkout and self.checkout:
            gitutil.Checkout(commit.hash)

    def Make(self, commit, brd, stage, cwd, *args, **kwargs):
        """Run make

        Args:
            commit: Commit object that is being built
            brd: Board object that is being built
            stage: Stage that we are at (distclean, config, build)
            cwd: Directory where make should be run
            args: Arguments to pass to make
            kwargs: Arguments to pass to command.RunPipe()
        """
        cmd = ['make'] + list(args)
        result = command.RunPipe([cmd], capture=True, capture_stderr=True,
                cwd=cwd, raise_on_error=False, **kwargs)
        return result

    def ProcessResult(self, result):
        """Process the result of a build, showing progress information

        Args:
            result: A CommandResult object
        """
        col = terminal.Color()
        if result:
            target = result.brd.target

            if result.return_code < 0:
                self.active = False
                command.StopAll()
                return

            self.upto += 1
            if result.return_code != 0:
                self.fail += 1
            elif result.stderr:
                self.warned += 1
            if result.already_done:
                self.already_done += 1
        else:
            target = '(starting)'

        # Display separate counts for ok, warned and fail
        ok = self.upto - self.warned - self.fail
        line = '\r' + self.col.Color(self.col.GREEN, '%5d' % ok)
        line += self.col.Color(self.col.YELLOW, '%5d' % self.warned)
        line += self.col.Color(self.col.RED, '%5d' % self.fail)

        name = ' /%-5d  ' % self.count

        # Add our current completion time estimate
        self._AddTimestamp()
        if self._complete_delay:
            name += '%s  : ' % self._complete_delay
        # When building all boards for a commit, we can print a commit
        # progress message.
        if result and result.commit_upto is None:
            name += 'commit %2d/%-3d' % (self.commit_upto + 1,
                    self.commit_count)

        name += target
        print line + name,
        length = 13 + len(name)
        self.ClearLine(length)

    def _GetOutputDir(self, commit_upto):
        """Get the name of the output directory for a commit number

        The output directory is typically .../<branch>/<commit>.

        Args:
            commit_upto: Commit number to use (0..self.count-1)
        """
        commit = self.commits[commit_upto]
        subject = commit.subject.translate(trans_valid_chars)
        commit_dir = ('%02d_of_%02d_g%s_%s' % (commit_upto + 1,
                self.commit_count, commit.hash, subject[:20]))
        output_dir = os.path.join(self.base_dir, commit_dir)
        return output_dir

    def GetBuildDir(self, commit_upto, target):
        """Get the name of the build directory for a commit number

        The build directory is typically .../<branch>/<commit>/<target>.

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        output_dir = self._GetOutputDir(commit_upto)
        return os.path.join(output_dir, target)

    def GetDoneFile(self, commit_upto, target):
        """Get the name of the done file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        return os.path.join(self.GetBuildDir(commit_upto, target), 'done')

    def GetSizesFile(self, commit_upto, target):
        """Get the name of the sizes file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        return os.path.join(self.GetBuildDir(commit_upto, target), 'sizes')

    def GetFuncSizesFile(self, commit_upto, target, elf_fname):
        """Get the name of the funcsizes file for a commit number and ELF file

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
            elf_fname: Filename of elf image
        """
        return os.path.join(self.GetBuildDir(commit_upto, target),
                            '%s.sizes' % elf_fname.replace('/', '-'))

    def GetObjdumpFile(self, commit_upto, target, elf_fname):
        """Get the name of the objdump file for a commit number and ELF file

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
            elf_fname: Filename of elf image
        """
        return os.path.join(self.GetBuildDir(commit_upto, target),
                            '%s.objdump' % elf_fname.replace('/', '-'))

    def GetErrFile(self, commit_upto, target):
        """Get the name of the err file for a commit number

        Args:
            commit_upto: Commit number to use (0..self.count-1)
            target: Target name
        """
        output_dir = self.GetBuildDir(commit_upto, target)
        return os.path.join(output_dir, 'err')

    def FilterErrors(self, lines):
        """Filter out errors in which we have no interest

        We should probably use map().

        Args:
            lines: List of error lines, each a string
        Returns:
            New list with only interesting lines included
        """
        out_lines = []
        for line in lines:
            if not self.re_make_err.search(line):
                out_lines.append(line)
        return out_lines

    def ReadFuncSizes(self, fname, fd):
        """Read function sizes from the output of 'nm'

        Args:
            fd: File containing data to read
            fname: Filename we are reading from (just for errors)

        Returns:
            Dictionary containing size of each function in bytes, indexed by
            function name.
        """
        sym = {}
        for line in fd.readlines():
            try:
                size, type, name = line[:-1].split()
            except:
                print "Invalid line in file '%s': '%s'" % (fname, line[:-1])
                continue
            if type in 'tTdDbB':
                # function names begin with '.' on 64-bit powerpc
                if '.' in name[1:]:
                    name = 'static.' + name.split('.')[0]
                sym[name] = sym.get(name, 0) + int(size, 16)
        return sym

    def GetBuildOutcome(self, commit_upto, target, read_func_sizes):
        """Work out the outcome of a build.

        Args:
            commit_upto: Commit number to check (0..n-1)
            target: Target board to check
            read_func_sizes: True to read function size information

        Returns:
            Outcome object
        """
        done_file = self.GetDoneFile(commit_upto, target)
        sizes_file = self.GetSizesFile(commit_upto, target)
        sizes = {}
        func_sizes = {}
        if os.path.exists(done_file):
            with open(done_file, 'r') as fd:
                return_code = int(fd.readline())
                err_lines = []
                err_file = self.GetErrFile(commit_upto, target)
                if os.path.exists(err_file):
                    with open(err_file, 'r') as fd:
                        err_lines = self.FilterErrors(fd.readlines())

                # Decide whether the build was ok, failed or created warnings
                if return_code:
                    rc = OUTCOME_ERROR
                elif len(err_lines):
                    rc = OUTCOME_WARNING
                else:
                    rc = OUTCOME_OK

                # Convert size information to our simple format
                if os.path.exists(sizes_file):
                    with open(sizes_file, 'r') as fd:
                        for line in fd.readlines():
                            values = line.split()
                            rodata = 0
                            if len(values) > 6:
                                rodata = int(values[6], 16)
                            size_dict = {
                                'all' : int(values[0]) + int(values[1]) +
                                        int(values[2]),
                                'text' : int(values[0]) - rodata,
                                'data' : int(values[1]),
                                'bss' : int(values[2]),
                                'rodata' : rodata,
                            }
                            sizes[values[5]] = size_dict

            if read_func_sizes:
                pattern = self.GetFuncSizesFile(commit_upto, target, '*')
                for fname in glob.glob(pattern):
                    with open(fname, 'r') as fd:
                        dict_name = os.path.basename(fname).replace('.sizes',
                                                                    '')
                        func_sizes[dict_name] = self.ReadFuncSizes(fname, fd)

            return Builder.Outcome(rc, err_lines, sizes, func_sizes)

        return Builder.Outcome(OUTCOME_UNKNOWN, [], {}, {})

    def GetResultSummary(self, boards_selected, commit_upto, read_func_sizes):
        """Calculate a summary of the results of building a commit.

        Args:
            board_selected: Dict containing boards to summarise
            commit_upto: Commit number to summarize (0..self.count-1)
            read_func_sizes: True to read function size information

        Returns:
            Tuple:
                Dict containing boards which passed building this commit.
                    keyed by board.target
                List containing a summary of error/warning lines
        """
        board_dict = {}
        err_lines_summary = []

        for board in boards_selected.itervalues():
            outcome = self.GetBuildOutcome(commit_upto, board.target,
                                           read_func_sizes)
            board_dict[board.target] = outcome
            for err in outcome.err_lines:
                if err and not err.rstrip() in err_lines_summary:
                    err_lines_summary.append(err.rstrip())
        return board_dict, err_lines_summary

    def AddOutcome(self, board_dict, arch_list, changes, char, color):
        """Add an output to our list of outcomes for each architecture

        This simple function adds failing boards (changes) to the
        relevant architecture string, so we can print the results out
        sorted by architecture.

        Args:
             board_dict: Dict containing all boards
             arch_list: Dict keyed by arch name. Value is a string containing
                    a list of board names which failed for that arch.
             changes: List of boards to add to arch_list
             color: terminal.Colour object
        """
        done_arch = {}
        for target in changes:
            if target in board_dict:
                arch = board_dict[target].arch
            else:
                arch = 'unknown'
            str = self.col.Color(color, ' ' + target)
            if not arch in done_arch:
                str = self.col.Color(color, char) + '  ' + str
                done_arch[arch] = True
            if not arch in arch_list:
                arch_list[arch] = str
            else:
                arch_list[arch] += str


    def ColourNum(self, num):
        color = self.col.RED if num > 0 else self.col.GREEN
        if num == 0:
            return '0'
        return self.col.Color(color, str(num))

    def ResetResultSummary(self, board_selected):
        """Reset the results summary ready for use.

        Set up the base board list to be all those selected, and set the
        error lines to empty.

        Following this, calls to PrintResultSummary() will use this
        information to work out what has changed.

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
        """
        self._base_board_dict = {}
        for board in board_selected:
            self._base_board_dict[board] = Builder.Outcome(0, [], [], {})
        self._base_err_lines = []

    def PrintFuncSizeDetail(self, fname, old, new):
        grow, shrink, add, remove, up, down = 0, 0, 0, 0, 0, 0
        delta, common = [], {}

        for a in old:
            if a in new:
                common[a] = 1

        for name in old:
            if name not in common:
                remove += 1
                down += old[name]
                delta.append([-old[name], name])

        for name in new:
            if name not in common:
                add += 1
                up += new[name]
                delta.append([new[name], name])

        for name in common:
                diff = new.get(name, 0) - old.get(name, 0)
                if diff > 0:
                    grow, up = grow + 1, up + diff
                elif diff < 0:
                    shrink, down = shrink + 1, down - diff
                delta.append([diff, name])

        delta.sort()
        delta.reverse()

        args = [add, -remove, grow, -shrink, up, -down, up - down]
        if max(args) == 0:
            return
        args = [self.ColourNum(x) for x in args]
        indent = ' ' * 15
        print ('%s%s: add: %s/%s, grow: %s/%s bytes: %s/%s (%s)' %
               tuple([indent, self.col.Color(self.col.YELLOW, fname)] + args))
        print '%s  %-38s %7s %7s %+7s' % (indent, 'function', 'old', 'new',
                                        'delta')
        for diff, name in delta:
            if diff:
                color = self.col.RED if diff > 0 else self.col.GREEN
                msg = '%s  %-38s %7s %7s %+7d' % (indent, name,
                        old.get(name, '-'), new.get(name,'-'), diff)
                print self.col.Color(color, msg)


    def PrintSizeDetail(self, target_list, show_bloat):
        """Show details size information for each board

        Args:
            target_list: List of targets, each a dict containing:
                    'target': Target name
                    'total_diff': Total difference in bytes across all areas
                    <part_name>: Difference for that part
            show_bloat: Show detail for each function
        """
        targets_by_diff = sorted(target_list, reverse=True,
        key=lambda x: x['_total_diff'])
        for result in targets_by_diff:
            printed_target = False
            for name in sorted(result):
                diff = result[name]
                if name.startswith('_'):
                    continue
                if diff != 0:
                    color = self.col.RED if diff > 0 else self.col.GREEN
                msg = ' %s %+d' % (name, diff)
                if not printed_target:
                    print '%10s  %-15s:' % ('', result['_target']),
                    printed_target = True
                print self.col.Color(color, msg),
            if printed_target:
                print
                if show_bloat:
                    target = result['_target']
                    outcome = result['_outcome']
                    base_outcome = self._base_board_dict[target]
                    for fname in outcome.func_sizes:
                        self.PrintFuncSizeDetail(fname,
                                                 base_outcome.func_sizes[fname],
                                                 outcome.func_sizes[fname])


    def PrintSizeSummary(self, board_selected, board_dict, show_detail,
                         show_bloat):
        """Print a summary of image sizes broken down by section.

        The summary takes the form of one line per architecture. The
        line contains deltas for each of the sections (+ means the section
        got bigger, - means smaller). The nunmbers are the average number
        of bytes that a board in this section increased by.

        For example:
           powerpc: (622 boards)   text -0.0
          arm: (285 boards)   text -0.0
          nds32: (3 boards)   text -8.0

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
            board_dict: Dict containing boards for which we built this
                commit, keyed by board.target. The value is an Outcome object.
            show_detail: Show detail for each board
            show_bloat: Show detail for each function
        """
        arch_list = {}
        arch_count = {}

        # Calculate changes in size for different image parts
        # The previous sizes are in Board.sizes, for each board
        for target in board_dict:
            if target not in board_selected:
                continue
            base_sizes = self._base_board_dict[target].sizes
            outcome = board_dict[target]
            sizes = outcome.sizes

            # Loop through the list of images, creating a dict of size
            # changes for each image/part. We end up with something like
            # {'target' : 'snapper9g45, 'data' : 5, 'u-boot-spl:text' : -4}
            # which means that U-Boot data increased by 5 bytes and SPL
            # text decreased by 4.
            err = {'_target' : target}
            for image in sizes:
                if image in base_sizes:
                    base_image = base_sizes[image]
                    # Loop through the text, data, bss parts
                    for part in sorted(sizes[image]):
                        diff = sizes[image][part] - base_image[part]
                        col = None
                        if diff:
                            if image == 'u-boot':
                                name = part
                            else:
                                name = image + ':' + part
                            err[name] = diff
            arch = board_selected[target].arch
            if not arch in arch_count:
                arch_count[arch] = 1
            else:
                arch_count[arch] += 1
            if not sizes:
                pass    # Only add to our list when we have some stats
            elif not arch in arch_list:
                arch_list[arch] = [err]
            else:
                arch_list[arch].append(err)

        # We now have a list of image size changes sorted by arch
        # Print out a summary of these
        for arch, target_list in arch_list.iteritems():
            # Get total difference for each type
            totals = {}
            for result in target_list:
                total = 0
                for name, diff in result.iteritems():
                    if name.startswith('_'):
                        continue
                    total += diff
                    if name in totals:
                        totals[name] += diff
                    else:
                        totals[name] = diff
                result['_total_diff'] = total
                result['_outcome'] = board_dict[result['_target']]

            count = len(target_list)
            printed_arch = False
            for name in sorted(totals):
                diff = totals[name]
                if diff:
                    # Display the average difference in this name for this
                    # architecture
                    avg_diff = float(diff) / count
                    color = self.col.RED if avg_diff > 0 else self.col.GREEN
                    msg = ' %s %+1.1f' % (name, avg_diff)
                    if not printed_arch:
                        print '%10s: (for %d/%d boards)' % (arch, count,
                                arch_count[arch]),
                        printed_arch = True
                    print self.col.Color(color, msg),

            if printed_arch:
                print
                if show_detail:
                    self.PrintSizeDetail(target_list, show_bloat)


    def PrintResultSummary(self, board_selected, board_dict, err_lines,
                           show_sizes, show_detail, show_bloat):
        """Compare results with the base results and display delta.

        Only boards mentioned in board_selected will be considered. This
        function is intended to be called repeatedly with the results of
        each commit. It therefore shows a 'diff' between what it saw in
        the last call and what it sees now.

        Args:
            board_selected: Dict containing boards to summarise, keyed by
                board.target
            board_dict: Dict containing boards for which we built this
                commit, keyed by board.target. The value is an Outcome object.
            err_lines: A list of errors for this commit, or [] if there is
                none, or we don't want to print errors
            show_sizes: Show image size deltas
            show_detail: Show detail for each board
            show_bloat: Show detail for each function
        """
        better = []     # List of boards fixed since last commit
        worse = []      # List of new broken boards since last commit
        new = []        # List of boards that didn't exist last time
        unknown = []    # List of boards that were not built

        for target in board_dict:
            if target not in board_selected:
                continue

            # If the board was built last time, add its outcome to a list
            if target in self._base_board_dict:
                base_outcome = self._base_board_dict[target].rc
                outcome = board_dict[target]
                if outcome.rc == OUTCOME_UNKNOWN:
                    unknown.append(target)
                elif outcome.rc < base_outcome:
                    better.append(target)
                elif outcome.rc > base_outcome:
                    worse.append(target)
            else:
                new.append(target)

        # Get a list of errors that have appeared, and disappeared
        better_err = []
        worse_err = []
        for line in err_lines:
            if line not in self._base_err_lines:
                worse_err.append('+' + line)
        for line in self._base_err_lines:
            if line not in err_lines:
                better_err.append('-' + line)

        # Display results by arch
        if better or worse or unknown or new or worse_err or better_err:
            arch_list = {}
            self.AddOutcome(board_selected, arch_list, better, '',
                    self.col.GREEN)
            self.AddOutcome(board_selected, arch_list, worse, '+',
                    self.col.RED)
            self.AddOutcome(board_selected, arch_list, new, '*', self.col.BLUE)
            if self._show_unknown:
                self.AddOutcome(board_selected, arch_list, unknown, '?',
                        self.col.MAGENTA)
            for arch, target_list in arch_list.iteritems():
                print '%10s: %s' % (arch, target_list)
            if better_err:
                print self.col.Color(self.col.GREEN, '\n'.join(better_err))
            if worse_err:
                print self.col.Color(self.col.RED, '\n'.join(worse_err))

        if show_sizes:
            self.PrintSizeSummary(board_selected, board_dict, show_detail,
                                  show_bloat)

        # Save our updated information for the next call to this function
        self._base_board_dict = board_dict
        self._base_err_lines = err_lines

        # Get a list of boards that did not get built, if needed
        not_built = []
        for board in board_selected:
            if not board in board_dict:
                not_built.append(board)
        if not_built:
            print "Boards not built (%d): %s" % (len(not_built),
                    ', '.join(not_built))


    def ShowSummary(self, commits, board_selected, show_errors, show_sizes,
                    show_detail, show_bloat):
        """Show a build summary for U-Boot for a given board list.

        Reset the result summary, then repeatedly call GetResultSummary on
        each commit's results, then display the differences we see.

        Args:
            commit: Commit objects to summarise
            board_selected: Dict containing boards to summarise
            show_errors: Show errors that occured
            show_sizes: Show size deltas
            show_detail: Show detail for each board
            show_bloat: Show detail for each function
        """
        self.commit_count = len(commits)
        self.commits = commits
        self.ResetResultSummary(board_selected)

        for commit_upto in range(0, self.commit_count, self._step):
            board_dict, err_lines = self.GetResultSummary(board_selected,
                    commit_upto, read_func_sizes=show_bloat)
            msg = '%02d: %s' % (commit_upto + 1, commits[commit_upto].subject)
            print self.col.Color(self.col.BLUE, msg)
            self.PrintResultSummary(board_selected, board_dict,
                    err_lines if show_errors else [], show_sizes, show_detail,
                    show_bloat)


    def SetupBuild(self, board_selected, commits):
        """Set up ready to start a build.

        Args:
            board_selected: Selected boards to build
            commits: Selected commits to build
        """
        # First work out how many commits we will build
        count = (len(commits) + self._step - 1) / self._step
        self.count = len(board_selected) * count
        self.upto = self.warned = self.fail = 0
        self._timestamps = collections.deque()

    def BuildBoardsForCommit(self, board_selected, keep_outputs):
        """Build all boards for a single commit"""
        self.SetupBuild(board_selected)
        self.count = len(board_selected)
        for brd in board_selected.itervalues():
            job = BuilderJob()
            job.board = brd
            job.commits = None
            job.keep_outputs = keep_outputs
            self.queue.put(brd)

        self.queue.join()
        self.out_queue.join()
        print
        self.ClearLine(0)

    def BuildCommits(self, commits, board_selected, show_errors, keep_outputs):
        """Build all boards for all commits (non-incremental)"""
        self.commit_count = len(commits)

        self.ResetResultSummary(board_selected)
        for self.commit_upto in range(self.commit_count):
            self.SelectCommit(commits[self.commit_upto])
            self.SelectOutputDir()
            Mkdir(self.output_dir)

            self.BuildBoardsForCommit(board_selected, keep_outputs)
            board_dict, err_lines = self.GetResultSummary()
            self.PrintResultSummary(board_selected, board_dict,
                err_lines if show_errors else [])

        if self.already_done:
            print '%d builds already done' % self.already_done

    def GetThreadDir(self, thread_num):
        """Get the directory path to the working dir for a thread.

        Args:
            thread_num: Number of thread to check.
        """
        return os.path.join(self._working_dir, '%02d' % thread_num)

    def _PrepareThread(self, thread_num):
        """Prepare the working directory for a thread.

        This clones or fetches the repo into the thread's work directory.

        Args:
            thread_num: Thread number (0, 1, ...)
        """
        thread_dir = self.GetThreadDir(thread_num)
        Mkdir(thread_dir)
        git_dir = os.path.join(thread_dir, '.git')

        # Clone the repo if it doesn't already exist
        # TODO(sjg@chromium): Perhaps some git hackery to symlink instead, so
        # we have a private index but uses the origin repo's contents?
        if self.git_dir:
            src_dir = os.path.abspath(self.git_dir)
            if os.path.exists(git_dir):
                gitutil.Fetch(git_dir, thread_dir)
            else:
                print 'Cloning repo for thread %d' % thread_num
                gitutil.Clone(src_dir, thread_dir)

    def _PrepareWorkingSpace(self, max_threads):
        """Prepare the working directory for use.

        Set up the git repo for each thread.

        Args:
            max_threads: Maximum number of threads we expect to need.
        """
        Mkdir(self._working_dir)
        for thread in range(max_threads):
            self._PrepareThread(thread)

    def _PrepareOutputSpace(self):
        """Get the output directories ready to receive files.

        We delete any output directories which look like ones we need to
        create. Having left over directories is confusing when the user wants
        to check the output manually.
        """
        dir_list = []
        for commit_upto in range(self.commit_count):
            dir_list.append(self._GetOutputDir(commit_upto))

        for dirname in glob.glob(os.path.join(self.base_dir, '*')):
            if dirname not in dir_list:
                shutil.rmtree(dirname)

    def BuildBoards(self, commits, board_selected, show_errors, keep_outputs):
        """Build all commits for a list of boards

        Args:
            commits: List of commits to be build, each a Commit object
            boards_selected: Dict of selected boards, key is target name,
                    value is Board object
            show_errors: True to show summarised error/warning info
            keep_outputs: True to save build output files
        """
        self.commit_count = len(commits)
        self.commits = commits

        self.ResetResultSummary(board_selected)
        Mkdir(self.base_dir)
        self._PrepareWorkingSpace(min(self.num_threads, len(board_selected)))
        self._PrepareOutputSpace()
        self.SetupBuild(board_selected, commits)
        self.ProcessResult(None)

        # Create jobs to build all commits for each board
        for brd in board_selected.itervalues():
            job = BuilderJob()
            job.board = brd
            job.commits = commits
            job.keep_outputs = keep_outputs
            job.step = self._step
            self.queue.put(job)

        # Wait until all jobs are started
        self.queue.join()

        # Wait until we have processed all output
        self.out_queue.join()
        print
        self.ClearLine(0)

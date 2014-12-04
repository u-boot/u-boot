# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:      GPL-2.0+
#

import errno
import glob
import os
import shutil
import threading

import command
import gitutil

def Mkdir(dirname, parents = False):
    """Make a directory if it doesn't already exist.

    Args:
        dirname: Directory to create
    """
    try:
        if parents:
            os.makedirs(dirname)
        else:
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
                        mrproper - can be called to clean source
                        config - called to configure for a board
                        build - the main make invocation - it does the build
            args: A list of arguments to pass to 'make'
            kwargs: A list of keyword arguments to pass to command.RunPipe()

        Returns:
            CommandResult object
        """
        return self.builder.do_make(commit, brd, stage, cwd, *args,
                **kwargs)

    def RunCommit(self, commit_upto, brd, work_dir, do_config, force_build,
                  force_build_failures):
        """Build a particular commit.

        If the build is already done, and we are not forcing a build, we skip
        the build and just return the previously-saved results.

        Args:
            commit_upto: Commit number to build (0...n-1)
            brd: Board object to build
            work_dir: Directory to which the source will be checked out
            do_config: True to run a make <board>_defconfig on the source
            force_build: Force a build even if one was previously done
            force_build_failures: Force a bulid if the previous result showed
                failure

        Returns:
            tuple containing:
                - CommandResult object containing the results of the build
                - boolean indicating whether 'make config' is still needed
        """
        # Create a default result - it will be overwritte by the call to
        # self.Make() below, in the event that we do a build.
        result = command.CommandResult()
        result.return_code = 0
        if self.builder.in_tree:
            out_dir = work_dir
        else:
            out_dir = os.path.join(work_dir, 'build')

        # Check if the job was already completed last time
        done_file = self.builder.GetDoneFile(commit_upto, brd.target)
        result.already_done = os.path.exists(done_file)
        will_build = (force_build or force_build_failures or
            not result.already_done)
        if result.already_done:
            # Get the return code from that build and use it
            with open(done_file, 'r') as fd:
                result.return_code = int(fd.readline())
            if will_build:
                err_file = self.builder.GetErrFile(commit_upto, brd.target)
                if os.path.exists(err_file) and os.stat(err_file).st_size:
                    result.stderr = 'bad'
                elif not force_build:
                    # The build passed, so no need to build it again
                    will_build = False

        if will_build:
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
                if self.builder.commits:
                    commit = self.builder.commits[commit_upto]
                    if self.builder.checkout:
                        git_dir = os.path.join(work_dir, '.git')
                        gitutil.Checkout(commit.hash, git_dir, work_dir,
                                         force=True)
                else:
                    commit = 'current'

                # Set up the environment and command line
                env = self.toolchain.MakeEnvironment()
                Mkdir(out_dir)
                args = []
                cwd = work_dir
                src_dir = os.path.realpath(work_dir)
                if not self.builder.in_tree:
                    if commit_upto is None:
                        # In this case we are building in the original source
                        # directory (i.e. the current directory where buildman
                        # is invoked. The output directory is set to this
                        # thread's selected work directory.
                        #
                        # Symlinks can confuse U-Boot's Makefile since
                        # we may use '..' in our path, so remove them.
                        work_dir = os.path.realpath(work_dir)
                        args.append('O=%s/build' % work_dir)
                        cwd = None
                        src_dir = os.getcwd()
                    else:
                        args.append('O=build')
                args.append('-s')
                if self.builder.num_jobs is not None:
                    args.extend(['-j', str(self.builder.num_jobs)])
                config_args = ['%s_defconfig' % brd.target]
                config_out = ''
                args.extend(self.builder.toolchains.GetMakeArguments(brd))

                # If we need to reconfigure, do that now
                if do_config:
                    result = self.Make(commit, brd, 'mrproper', cwd,
                            'mrproper', *args, env=env)
                    result = self.Make(commit, brd, 'config', cwd,
                            *(args + config_args), env=env)
                    config_out = result.combined
                    do_config = False   # No need to configure next time
                if result.return_code == 0:
                    result = self.Make(commit, brd, 'build', cwd, *args,
                            env=env)
                result.stderr = result.stderr.replace(src_dir + '/', '')
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
            patterns = ['u-boot', '*.bin', 'u-boot.dtb', '*.map', '*.img',
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
            # Run 'make board_defconfig' on the first commit
            do_config = True
            commit_upto  = 0
            force_build = False
            for commit_upto in range(0, len(job.commits), job.step):
                result, request_config = self.RunCommit(commit_upto, brd,
                        work_dir, do_config,
                        force_build or self.builder.force_build,
                        self.builder.force_build_failures)
                failed = result.return_code or result.stderr
                did_config = do_config
                if failed and not do_config:
                    # If our incremental build failed, try building again
                    # with a reconfig.
                    if self.builder.force_config_on_failure:
                        result, request_config = self.RunCommit(commit_upto,
                            brd, work_dir, True, True, False)
                        did_config = True
                if not self.builder.force_reconfig:
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
                if (failed and not result.already_done and not did_config and
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
            result, request_config = self.RunCommit(None, brd, work_dir, True,
                        True, self.builder.force_build_failures)
            result.commit_upto = 0
            self._WriteResult(result, job.keep_outputs)
            self.builder.out_queue.put(result)

    def run(self):
        """Our thread's run function

        This thread picks a job from the queue, runs it, and then goes to the
        next job.
        """
        alive = True
        while True:
            job = self.builder.queue.get()
            if self.builder.active and alive:
                self.RunJob(job)
            '''
            try:
                if self.builder.active and alive:
                    self.RunJob(job)
            except Exception as err:
                alive = False
                print err
            '''
            self.builder.queue.task_done()

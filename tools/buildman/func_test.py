#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:      GPL-2.0+
#

import os
import shutil
import sys
import tempfile
import unittest

import bsettings
import cmdline
import command
import control
import gitutil
import terminal
import toolchain

settings_data = '''
# Buildman settings file

[toolchain]

[toolchain-alias]

[make-flags]
src=/home/sjg/c/src
chroot=/home/sjg/c/chroot
vboot=USE_STDINT=1 VBOOT_DEBUG=1 MAKEFLAGS_VBOOT=DEBUG=1 CFLAGS_EXTRA_VBOOT=-DUNROLL_LOOPS VBOOT_SOURCE=${src}/platform/vboot_reference
chromeos_coreboot=VBOOT=${chroot}/build/link/usr ${vboot}
chromeos_daisy=VBOOT=${chroot}/build/daisy/usr ${vboot}
chromeos_peach=VBOOT=${chroot}/build/peach_pit/usr ${vboot}
'''

class TestFunctional(unittest.TestCase):
    """Functional test for buildman.

    This aims to test from just below the invocation of buildman (parsing
    of arguments) to 'make' and 'git' invocation. It is not a true
    emd-to-end test, as it mocks git, make and the tool chain. But this
    makes it easier to detect when the builder is doing the wrong thing,
    since in many cases this test code will fail. For example, only a
    very limited subset of 'git' arguments is supported - anything
    unexpected will fail.
    """
    def setUp(self):
        self._base_dir = tempfile.mkdtemp()
        self._git_dir = os.path.join(self._base_dir, 'src')
        self._buildman_pathname = sys.argv[0]
        self._buildman_dir = os.path.dirname(sys.argv[0])
        command.test_result = self._HandleCommand
        self._toolchains = toolchain.Toolchains()
        self._toolchains.Add('gcc', test=False)
        bsettings.Setup(None)
        bsettings.AddFile(settings_data)

    def tearDown(self):
        shutil.rmtree(self._base_dir)

    def _RunBuildman(self, *args):
        return command.RunPipe([[self._buildman_pathname] + list(args)],
                capture=True, capture_stderr=True)

    def _RunControl(self, *args):
        sys.argv = [sys.argv[0]] + list(args)
        options, args = cmdline.ParseArgs()
        return control.DoBuildman(options, args, toolchains=self._toolchains,
                make_func=self._HandleMake)

    def testFullHelp(self):
        command.test_result = None
        result = self._RunBuildman('-H')
        help_file = os.path.join(self._buildman_dir, 'README')
        self.assertEqual(len(result.stdout), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testHelp(self):
        command.test_result = None
        result = self._RunBuildman('-h')
        help_file = os.path.join(self._buildman_dir, 'README')
        self.assertTrue(len(result.stdout) > 1000)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def testGitSetup(self):
        """Test gitutils.Setup(), from outside the module itself"""
        command.test_result = command.CommandResult(return_code=1)
        gitutil.Setup()
        self.assertEqual(gitutil.use_no_decorate, False)

        command.test_result = command.CommandResult(return_code=0)
        gitutil.Setup()
        self.assertEqual(gitutil.use_no_decorate, True)

    def _HandleCommandGitLog(self, args):
        if '-n0' in args:
            return command.CommandResult(return_code=0)

        # Not handled, so abort
        print 'git log', args
        sys.exit(1)

    def _HandleCommandGit(self, in_args):
        """Handle execution of a git command

        This uses a hacked-up parser.

        Args:
            in_args: Arguments after 'git' from the command line
        """
        git_args = []           # Top-level arguments to git itself
        sub_cmd = None          # Git sub-command selected
        args = []               # Arguments to the git sub-command
        for arg in in_args:
            if sub_cmd:
                args.append(arg)
            elif arg[0] == '-':
                git_args.append(arg)
            else:
                sub_cmd = arg
        if sub_cmd == 'config':
            return command.CommandResult(return_code=0)
        elif sub_cmd == 'log':
            return self._HandleCommandGitLog(args)

        # Not handled, so abort
        print 'git', git_args, sub_cmd, args
        sys.exit(1)

    def _HandleCommandNm(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommandObjdump(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommandSize(self, args):
        return command.CommandResult(return_code=0)

    def _HandleCommand(self, **kwargs):
        """Handle a command execution.

        The command is in kwargs['pipe-list'], as a list of pipes, each a
        list of commands. The command should be emulated as required for
        testing purposes.

        Returns:
            A CommandResult object
        """
        pipe_list = kwargs['pipe_list']
        if len(pipe_list) != 1:
            print 'invalid pipe', kwargs
            sys.exit(1)
        cmd = pipe_list[0][0]
        args = pipe_list[0][1:]
        if cmd == 'git':
            return self._HandleCommandGit(args)
        elif cmd == './scripts/show-gnu-make':
            return command.CommandResult(return_code=0, stdout='make')
        elif cmd == 'nm':
            return self._HandleCommandNm(args)
        elif cmd == 'objdump':
            return self._HandleCommandObjdump(args)
        elif cmd == 'size':
            return self._HandleCommandSize(args)

        # Not handled, so abort
        print 'unknown command', kwargs
        sys.exit(1)
        return command.CommandResult(return_code=0)

    def _HandleMake(self, commit, brd, stage, cwd, *args, **kwargs):
        """Handle execution of 'make'

        Args:
            commit: Commit object that is being built
            brd: Board object that is being built
            stage: Stage that we are at (mrproper, config, build)
            cwd: Directory where make should be run
            args: Arguments to pass to make
            kwargs: Arguments to pass to command.RunPipe()
        """
        if stage == 'mrproper':
            return command.CommandResult(return_code=0)
        elif stage == 'config':
            return command.CommandResult(return_code=0,
                    combined='Test configuration complete')
        elif stage == 'build':
            return command.CommandResult(return_code=0)

        # Not handled, so abort
        print 'make', stage
        sys.exit(1)

    def testCurrentSource(self):
        """Very simple test to invoke buildman on the current source"""
        self._RunControl()
        lines = terminal.GetPrintTestLines()
        self.assertTrue(lines[0].text.startswith('Building current source'))

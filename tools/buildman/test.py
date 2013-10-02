#
# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import os
import shutil
import sys
import tempfile
import time
import unittest

# Bring in the patman libraries
our_path = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(our_path, '../patman'))

import board
import bsettings
import builder
import control
import command
import commit
import toolchain

errors = [
    '''main.c: In function 'main_loop':
main.c:260:6: warning: unused variable 'joe' [-Wunused-variable]
''',
    '''main.c: In function 'main_loop':
main.c:295:2: error: 'fred' undeclared (first use in this function)
main.c:295:2: note: each undeclared identifier is reported only once for each function it appears in
make[1]: *** [main.o] Error 1
make: *** [common/libcommon.o] Error 2
Make failed
''',
    '''main.c: In function 'main_loop':
main.c:280:6: warning: unused variable 'mary' [-Wunused-variable]
''',
    '''powerpc-linux-ld: warning: dot moved backwards before `.bss'
powerpc-linux-ld: warning: dot moved backwards before `.bss'
powerpc-linux-ld: u-boot: section .text lma 0xfffc0000 overlaps previous sections
powerpc-linux-ld: u-boot: section .rodata lma 0xfffef3ec overlaps previous sections
powerpc-linux-ld: u-boot: section .reloc lma 0xffffa400 overlaps previous sections
powerpc-linux-ld: u-boot: section .data lma 0xffffcd38 overlaps previous sections
powerpc-linux-ld: u-boot: section .u_boot_cmd lma 0xffffeb40 overlaps previous sections
powerpc-linux-ld: u-boot: section .bootpg lma 0xfffff198 overlaps previous sections
'''
]


# hash, subject, return code, list of errors/warnings
commits = [
    ['1234', 'upstream/master, ok', 0, []],
    ['5678', 'Second commit, a warning', 0, errors[0:1]],
    ['9012', 'Third commit, error', 1, errors[0:2]],
    ['3456', 'Fourth commit, warning', 0, [errors[0], errors[2]]],
    ['7890', 'Fifth commit, link errors', 1, [errors[0], errors[3]]],
    ['abcd', 'Sixth commit, fixes all errors', 0, []]
]

boards = [
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 1', 'board0',  ''],
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 2', 'board1', ''],
    ['Active', 'powerpc', 'powerpc', '', 'Tester', 'PowerPC board 1', 'board2', ''],
    ['Active', 'powerpc', 'mpc5xx', '', 'Tester', 'PowerPC board 2', 'board3', ''],
    ['Active', 'sandbox', 'sandbox', '', 'Tester', 'Sandbox board', 'board4', ''],
]

class Options:
    """Class that holds build options"""
    pass

class TestBuild(unittest.TestCase):
    """Test buildman

    TODO: Write tests for the rest of the functionality
    """
    def setUp(self):
        # Set up commits to build
        self.commits = []
        sequence = 0
        for commit_info in commits:
            comm = commit.Commit(commit_info[0])
            comm.subject = commit_info[1]
            comm.return_code = commit_info[2]
            comm.error_list = commit_info[3]
            comm.sequence = sequence
            sequence += 1
            self.commits.append(comm)

        # Set up boards to build
        self.boards = board.Boards()
        for brd in boards:
            self.boards.AddBoard(board.Board(*brd))
        self.boards.SelectBoards([])

        # Set up the toolchains
        bsettings.Setup()
        self.toolchains = toolchain.Toolchains()
        self.toolchains.Add('arm-linux-gcc', test=False)
        self.toolchains.Add('sparc-linux-gcc', test=False)
        self.toolchains.Add('powerpc-linux-gcc', test=False)
        self.toolchains.Add('gcc', test=False)

    def Make(self, commit, brd, stage, *args, **kwargs):
        result = command.CommandResult()
        boardnum = int(brd.target[-1])
        result.return_code = 0
        result.stderr = ''
        result.stdout = ('This is the test output for board %s, commit %s' %
                (brd.target, commit.hash))
        if boardnum >= 1 and boardnum >= commit.sequence:
            result.return_code = commit.return_code
            result.stderr = ''.join(commit.error_list)
        if stage == 'build':
            target_dir = None
            for arg in args:
                if arg.startswith('O='):
                    target_dir = arg[2:]

            if not os.path.isdir(target_dir):
                os.mkdir(target_dir)
            #time.sleep(.2 + boardnum * .2)

        result.combined = result.stdout + result.stderr
        return result

    def testBasic(self):
        """Test basic builder operation"""
        output_dir = tempfile.mkdtemp()
        if not os.path.isdir(output_dir):
            os.mkdir(output_dir)
        build = builder.Builder(self.toolchains, output_dir, None, 1, 2,
                                checkout=False, show_unknown=False)
        build.do_make = self.Make
        board_selected = self.boards.GetSelectedDict()

        #build.BuildCommits(self.commits, board_selected, False)
        build.BuildBoards(self.commits, board_selected, False, False)
        build.ShowSummary(self.commits, board_selected, True, False,
                          False, False)

    def _testGit(self):
        """Test basic builder operation by building a branch"""
        base_dir = tempfile.mkdtemp()
        if not os.path.isdir(base_dir):
            os.mkdir(base_dir)
        options = Options()
        options.git = os.getcwd()
        options.summary = False
        options.jobs = None
        options.dry_run = False
        #options.git = os.path.join(base_dir, 'repo')
        options.branch = 'test-buildman'
        options.force_build = False
        options.list_tool_chains = False
        options.count = -1
        options.git_dir = None
        options.threads = None
        options.show_unknown = False
        options.quick = False
        options.show_errors = False
        options.keep_outputs = False
        args = ['tegra20']
        control.DoBuildman(options, args)

if __name__ == "__main__":
    unittest.main()

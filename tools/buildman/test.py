# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
#

from filelock import FileLock
import os
import shutil
import sys
import tempfile
import time
import unittest
from unittest.mock import patch

from buildman import board
from buildman import boards
from buildman import bsettings
from buildman import builder
from buildman import cfgutil
from buildman import control
from buildman import toolchain
from patman import commit
from u_boot_pylib import command
from u_boot_pylib import terminal
from u_boot_pylib import test_util
from u_boot_pylib import tools

use_network = True

settings_data = '''
# Buildman settings file

[toolchain]
main: /usr/sbin

[toolchain-alias]
x86: i386 x86_64
'''

settings_data_wrapper = '''
# Buildman settings file

[toolchain]
main: /usr/sbin

[toolchain-wrapper]
wrapper = ccache
'''

settings_data_homedir = '''
# Buildman settings file

[toolchain]
main = ~/mypath

[toolchain-prefix]
x86 = ~/mypath-x86-
'''

migration = '''===================== WARNING ======================
This board does not use CONFIG_DM. CONFIG_DM will be
compulsory starting with the v2020.01 release.
Failure to update may result in board removal.
See doc/develop/driver-model/migration.rst for more info.
====================================================
'''

errors = [
    '''main.c: In function 'main_loop':
main.c:260:6: warning: unused variable 'joe' [-Wunused-variable]
''',
    '''main.c: In function 'main_loop2':
main.c:295:2: error: 'fred' undeclared (first use in this function)
main.c:295:2: note: each undeclared identifier is reported only once for each function it appears in
make[1]: *** [main.o] Error 1
make: *** [common/libcommon.o] Error 2
Make failed
''',
    '''arch/arm/dts/socfpga_arria10_socdk_sdmmc.dtb: Warning \
(avoid_unnecessary_addr_size): /clocks: unnecessary #address-cells/#size-cells \
without "ranges" or child "reg" property
''',
    '''powerpc-linux-ld: warning: dot moved backwards before `.bss'
powerpc-linux-ld: warning: dot moved backwards before `.bss'
powerpc-linux-ld: u-boot: section .text lma 0xfffc0000 overlaps previous sections
powerpc-linux-ld: u-boot: section .rodata lma 0xfffef3ec overlaps previous sections
powerpc-linux-ld: u-boot: section .reloc lma 0xffffa400 overlaps previous sections
powerpc-linux-ld: u-boot: section .data lma 0xffffcd38 overlaps previous sections
powerpc-linux-ld: u-boot: section .u_boot_cmd lma 0xffffeb40 overlaps previous sections
powerpc-linux-ld: u-boot: section .bootpg lma 0xfffff198 overlaps previous sections
''',
   '''In file included from %(basedir)sarch/sandbox/cpu/cpu.c:9:0:
%(basedir)sarch/sandbox/include/asm/state.h:44:0: warning: "xxxx" redefined [enabled by default]
%(basedir)sarch/sandbox/include/asm/state.h:43:0: note: this is the location of the previous definition
%(basedir)sarch/sandbox/cpu/cpu.c: In function 'do_reset':
%(basedir)sarch/sandbox/cpu/cpu.c:27:1: error: unknown type name 'blah'
%(basedir)sarch/sandbox/cpu/cpu.c:28:12: error: expected declaration specifiers or '...' before numeric constant
make[2]: *** [arch/sandbox/cpu/cpu.o] Error 1
make[1]: *** [arch/sandbox/cpu] Error 2
make[1]: *** Waiting for unfinished jobs....
In file included from %(basedir)scommon/board_f.c:55:0:
%(basedir)sarch/sandbox/include/asm/state.h:44:0: warning: "xxxx" redefined [enabled by default]
%(basedir)sarch/sandbox/include/asm/state.h:43:0: note: this is the location of the previous definition
make: *** [sub-make] Error 2
'''
]


# hash, subject, return code, list of errors/warnings
commits = [
    ['1234', 'upstream/master, migration warning', 0, []],
    ['5678', 'Second commit, a warning', 0, errors[0:1]],
    ['9012', 'Third commit, error', 1, errors[0:2]],
    ['3456', 'Fourth commit, warning', 0, [errors[0], errors[2]]],
    ['7890', 'Fifth commit, link errors', 1, [errors[0], errors[3]]],
    ['abcd', 'Sixth commit, fixes all errors', 0, []],
    ['ef01', 'Seventh commit, fix migration, check directory suppression', 1,
     [errors[4]]],
]

BOARDS = [
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 1', 'board0',  ''],
    ['Active', 'arm', 'armv7', '', 'Tester', 'ARM Board 2', 'board1', ''],
    ['Active', 'powerpc', 'powerpc', '', 'Tester', 'PowerPC board 1', 'board2', ''],
    ['Active', 'powerpc', 'mpc83xx', '', 'Tester', 'PowerPC board 2', 'board3', ''],
    ['Active', 'sandbox', 'sandbox', '', 'Tester', 'Sandbox board', 'board4', ''],
]

BASE_DIR = 'base'

OUTCOME_OK, OUTCOME_WARN, OUTCOME_ERR = range(3)

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
            if sequence < 6:
                 comm.error_list += [migration]
            comm.sequence = sequence
            sequence += 1
            self.commits.append(comm)

        # Set up boards to build
        self.brds = boards.Boards()
        for brd in BOARDS:
            self.brds.add_board(board.Board(*brd))
        self.brds.select_boards([])

        # Add some test settings
        bsettings.setup(None)
        bsettings.add_file(settings_data)

        # Set up the toolchains
        self.toolchains = toolchain.Toolchains()
        self.toolchains.Add('arm-linux-gcc', test=False)
        self.toolchains.Add('sparc-linux-gcc', test=False)
        self.toolchains.Add('powerpc-linux-gcc', test=False)
        self.toolchains.Add('/path/to/aarch64-linux-gcc', test=False)
        self.toolchains.Add('gcc', test=False)

        # Avoid sending any output
        terminal.set_print_test_mode()
        self._col = terminal.Color()

        self.base_dir = tempfile.mkdtemp()
        if not os.path.isdir(self.base_dir):
            os.mkdir(self.base_dir)

        self.cur_time = 0
        self.valid_pids = []
        self.finish_time = None
        self.finish_pid = None

    def tearDown(self):
        shutil.rmtree(self.base_dir)

    def Make(self, commit, brd, stage, *args, **kwargs):
        result = command.CommandResult()
        boardnum = int(brd.target[-1])
        result.return_code = 0
        result.stderr = ''
        result.stdout = ('This is the test output for board %s, commit %s' %
                (brd.target, commit.hash))
        if ((boardnum >= 1 and boardnum >= commit.sequence) or
                boardnum == 4 and commit.sequence == 6):
            result.return_code = commit.return_code
            result.stderr = (''.join(commit.error_list)
                % {'basedir' : self.base_dir + '/.bm-work/00/'})
        elif commit.sequence < 6:
            result.stderr = migration

        result.combined = result.stdout + result.stderr
        return result

    def assertSummary(self, text, arch, plus, brds, outcome=OUTCOME_ERR):
        col = self._col
        expected_colour = (col.GREEN if outcome == OUTCOME_OK else
                           col.YELLOW if outcome == OUTCOME_WARN else col.RED)
        expect = '%10s: ' % arch
        # TODO(sjg@chromium.org): If plus is '', we shouldn't need this
        expect += ' ' + col.build(expected_colour, plus)
        expect += '  '
        for brd in brds:
            expect += col.build(expected_colour, ' %s' % brd)
        self.assertEqual(text, expect)

    def _SetupTest(self, echo_lines=False, threads=1, **kwdisplay_args):
        """Set up the test by running a build and summary

        Args:
            echo_lines: True to echo lines to the terminal to aid test
                development
            kwdisplay_args: Dict of arguments to pass to
                Builder.SetDisplayOptions()

        Returns:
            Iterator containing the output lines, each a PrintLine() object
        """
        build = builder.Builder(self.toolchains, self.base_dir, None, threads,
                                2, checkout=False, show_unknown=False)
        build.do_make = self.Make
        board_selected = self.brds.get_selected_dict()

        # Build the boards for the pre-defined commits and warnings/errors
        # associated with each. This calls our Make() to inject the fake output.
        build.build_boards(self.commits, board_selected, keep_outputs=False,
                           verbose=False)
        lines = terminal.get_print_test_lines()
        count = 0
        for line in lines:
            if line.text.strip():
                count += 1

        # We should get two starting messages, an update for every commit built
        # and a summary message
        self.assertEqual(count, len(commits) * len(BOARDS) + 3)
        build.set_display_options(**kwdisplay_args);
        build.show_summary(self.commits, board_selected)
        if echo_lines:
            terminal.echo_print_test_lines()
        return iter(terminal.get_print_test_lines())

    def _CheckOutput(self, lines, list_error_boards=False,
                     filter_dtb_warnings=False,
                     filter_migration_warnings=False):
        """Check for expected output from the build summary

        Args:
            lines: Iterator containing the lines returned from the summary
            list_error_boards: Adjust the check for output produced with the
               --list-error-boards flag
            filter_dtb_warnings: Adjust the check for output produced with the
               --filter-dtb-warnings flag
        """
        def add_line_prefix(prefix, brds, error_str, colour):
            """Add a prefix to each line of a string

            The training \n in error_str is removed before processing

            Args:
                prefix: String prefix to add
                error_str: Error string containing the lines
                colour: Expected colour for the line. Note that the board list,
                    if present, always appears in magenta

            Returns:
                New string where each line has the prefix added
            """
            lines = error_str.strip().splitlines()
            new_lines = []
            for line in lines:
                if brds:
                    expect = self._col.build(colour, prefix + '(')
                    expect += self._col.build(self._col.MAGENTA, brds,
                                              bright=False)
                    expect += self._col.build(colour, ') %s' % line)
                else:
                    expect = self._col.build(colour, prefix + line)
                new_lines.append(expect)
            return '\n'.join(new_lines)

        col = terminal.Color()
        boards01234 = ('board0 board1 board2 board3 board4'
                       if list_error_boards else '')
        boards1234 = 'board1 board2 board3 board4' if list_error_boards else ''
        boards234 = 'board2 board3 board4' if list_error_boards else ''
        boards34 = 'board3 board4' if list_error_boards else ''
        boards4 = 'board4' if list_error_boards else ''

        # Upstream commit: migration warnings only
        self.assertEqual(next(lines).text, '01: %s' % commits[0][1])

        if not filter_migration_warnings:
            self.assertSummary(next(lines).text, 'arm', 'w+',
                               ['board0', 'board1'], outcome=OUTCOME_WARN)
            self.assertSummary(next(lines).text, 'powerpc', 'w+',
                               ['board2', 'board3'], outcome=OUTCOME_WARN)
            self.assertSummary(next(lines).text, 'sandbox', 'w+', ['board4'],
                               outcome=OUTCOME_WARN)

            self.assertEqual(next(lines).text,
                add_line_prefix('+', boards01234, migration, col.RED))

        # Second commit: all archs should fail with warnings
        self.assertEqual(next(lines).text, '02: %s' % commits[1][1])

        if filter_migration_warnings:
            self.assertSummary(next(lines).text, 'arm', 'w+',
                               ['board1'], outcome=OUTCOME_WARN)
            self.assertSummary(next(lines).text, 'powerpc', 'w+',
                               ['board2', 'board3'], outcome=OUTCOME_WARN)
            self.assertSummary(next(lines).text, 'sandbox', 'w+', ['board4'],
                               outcome=OUTCOME_WARN)

        # Second commit: The warnings should be listed
        self.assertEqual(next(lines).text,
            add_line_prefix('w+', boards1234, errors[0], col.YELLOW))

        # Third commit: Still fails
        self.assertEqual(next(lines).text, '03: %s' % commits[2][1])
        if filter_migration_warnings:
            self.assertSummary(next(lines).text, 'arm', '',
                               ['board1'], outcome=OUTCOME_OK)
        self.assertSummary(next(lines).text, 'powerpc', '+',
                           ['board2', 'board3'])
        self.assertSummary(next(lines).text, 'sandbox', '+', ['board4'])

        # Expect a compiler error
        self.assertEqual(next(lines).text,
                         add_line_prefix('+', boards234, errors[1], col.RED))

        # Fourth commit: Compile errors are fixed, just have warning for board3
        self.assertEqual(next(lines).text, '04: %s' % commits[3][1])
        if filter_migration_warnings:
            expect = '%10s: ' % 'powerpc'
            expect += ' ' + col.build(col.GREEN, '')
            expect += '  '
            expect += col.build(col.GREEN, ' %s' % 'board2')
            expect += ' ' + col.build(col.YELLOW, 'w+')
            expect += '  '
            expect += col.build(col.YELLOW, ' %s' % 'board3')
            self.assertEqual(next(lines).text, expect)
        else:
            self.assertSummary(next(lines).text, 'powerpc', 'w+',
                               ['board2', 'board3'], outcome=OUTCOME_WARN)
        self.assertSummary(next(lines).text, 'sandbox', 'w+', ['board4'],
                               outcome=OUTCOME_WARN)

        # Compile error fixed
        self.assertEqual(next(lines).text,
                         add_line_prefix('-', boards234, errors[1], col.GREEN))

        if not filter_dtb_warnings:
            self.assertEqual(
                next(lines).text,
                add_line_prefix('w+', boards34, errors[2], col.YELLOW))

        # Fifth commit
        self.assertEqual(next(lines).text, '05: %s' % commits[4][1])
        if filter_migration_warnings:
            self.assertSummary(next(lines).text, 'powerpc', '', ['board3'],
                               outcome=OUTCOME_OK)
        self.assertSummary(next(lines).text, 'sandbox', '+', ['board4'])

        # The second line of errors[3] is a duplicate, so buildman will drop it
        expect = errors[3].rstrip().split('\n')
        expect = [expect[0]] + expect[2:]
        expect = '\n'.join(expect)
        self.assertEqual(next(lines).text,
                         add_line_prefix('+', boards4, expect, col.RED))

        if not filter_dtb_warnings:
            self.assertEqual(
                next(lines).text,
                add_line_prefix('w-', boards34, errors[2], col.CYAN))

        # Sixth commit
        self.assertEqual(next(lines).text, '06: %s' % commits[5][1])
        if filter_migration_warnings:
            self.assertSummary(next(lines).text, 'sandbox', '', ['board4'],
                               outcome=OUTCOME_OK)
        else:
            self.assertSummary(next(lines).text, 'sandbox', 'w+', ['board4'],
                               outcome=OUTCOME_WARN)

        # The second line of errors[3] is a duplicate, so buildman will drop it
        expect = errors[3].rstrip().split('\n')
        expect = [expect[0]] + expect[2:]
        expect = '\n'.join(expect)
        self.assertEqual(next(lines).text,
                         add_line_prefix('-', boards4, expect, col.GREEN))
        self.assertEqual(next(lines).text,
                         add_line_prefix('w-', boards4, errors[0], col.CYAN))

        # Seventh commit
        self.assertEqual(next(lines).text, '07: %s' % commits[6][1])
        if filter_migration_warnings:
            self.assertSummary(next(lines).text, 'sandbox', '+', ['board4'])
        else:
            self.assertSummary(next(lines).text, 'arm', '', ['board0', 'board1'],
                               outcome=OUTCOME_OK)
            self.assertSummary(next(lines).text, 'powerpc', '',
                               ['board2', 'board3'], outcome=OUTCOME_OK)
            self.assertSummary(next(lines).text, 'sandbox', '+', ['board4'])

        # Pick out the correct error lines
        expect_str = errors[4].rstrip().replace('%(basedir)s', '').split('\n')
        expect = expect_str[3:8] + [expect_str[-1]]
        expect = '\n'.join(expect)
        if not filter_migration_warnings:
            self.assertEqual(
                next(lines).text,
                add_line_prefix('-', boards01234, migration, col.GREEN))

        self.assertEqual(next(lines).text,
                         add_line_prefix('+', boards4, expect, col.RED))

        # Now the warnings lines
        expect = [expect_str[0]] + expect_str[10:12] + [expect_str[9]]
        expect = '\n'.join(expect)
        self.assertEqual(next(lines).text,
                         add_line_prefix('w+', boards4, expect, col.YELLOW))

    def testOutput(self):
        """Test basic builder operation and output

        This does a line-by-line verification of the summary output.
        """
        lines = self._SetupTest(show_errors=True)
        self._CheckOutput(lines, list_error_boards=False,
                          filter_dtb_warnings=False)

    def testErrorBoards(self):
        """Test output with --list-error-boards

        This does a line-by-line verification of the summary output.
        """
        lines = self._SetupTest(show_errors=True, list_error_boards=True)
        self._CheckOutput(lines, list_error_boards=True)

    def testFilterDtb(self):
        """Test output with --filter-dtb-warnings

        This does a line-by-line verification of the summary output.
        """
        lines = self._SetupTest(show_errors=True, filter_dtb_warnings=True)
        self._CheckOutput(lines, filter_dtb_warnings=True)

    def testFilterMigration(self):
        """Test output with --filter-migration-warnings

        This does a line-by-line verification of the summary output.
        """
        lines = self._SetupTest(show_errors=True,
                                filter_migration_warnings=True)
        self._CheckOutput(lines, filter_migration_warnings=True)

    def testSingleThread(self):
        """Test operation without threading"""
        lines = self._SetupTest(show_errors=True, threads=0)
        self._CheckOutput(lines, list_error_boards=False,
                          filter_dtb_warnings=False)

    def _testGit(self):
        """Test basic builder operation by building a branch"""
        options = Options()
        options.git = os.getcwd()
        options.summary = False
        options.jobs = None
        options.dry_run = False
        #options.git = os.path.join(self.base_dir, 'repo')
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
        control.do_buildman(options, args)

    def testBoardSingle(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['sandbox']),
                         ({'all': ['board4'], 'sandbox': ['board4']}, []))

    def testBoardArch(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['arm']),
                         ({'all': ['board0', 'board1'],
                          'arm': ['board0', 'board1']}, []))

    def testBoardArchSingle(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['arm sandbox']),
                         ({'sandbox': ['board4'],
                          'all': ['board0', 'board1', 'board4'],
                          'arm': ['board0', 'board1']}, []))


    def testBoardArchSingleMultiWord(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['arm', 'sandbox']),
                         ({'sandbox': ['board4'],
                          'all': ['board0', 'board1', 'board4'],
                          'arm': ['board0', 'board1']}, []))

    def testBoardSingleAnd(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['Tester & arm']),
                         ({'Tester&arm': ['board0', 'board1'],
                           'all': ['board0', 'board1']}, []))

    def testBoardTwoAnd(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['Tester', '&', 'arm',
                                                   'Tester' '&', 'powerpc',
                                                   'sandbox']),
                         ({'sandbox': ['board4'],
                          'all': ['board0', 'board1', 'board2', 'board3',
                                  'board4'],
                          'Tester&powerpc': ['board2', 'board3'],
                          'Tester&arm': ['board0', 'board1']}, []))

    def testBoardAll(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards([]),
                         ({'all': ['board0', 'board1', 'board2', 'board3',
                                  'board4']}, []))

    def testBoardRegularExpression(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['T.*r&^Po']),
                         ({'all': ['board2', 'board3'],
                          'T.*r&^Po': ['board2', 'board3']}, []))

    def testBoardDuplicate(self):
        """Test single board selection"""
        self.assertEqual(self.brds.select_boards(['sandbox sandbox',
                                                   'sandbox']),
                         ({'all': ['board4'], 'sandbox': ['board4']}, []))
    def CheckDirs(self, build, dirname):
        self.assertEqual('base%s' % dirname, build.get_output_dir(1))
        self.assertEqual('base%s/fred' % dirname,
                         build.get_build_dir(1, 'fred'))
        self.assertEqual('base%s/fred/done' % dirname,
                         build.get_done_file(1, 'fred'))
        self.assertEqual('base%s/fred/u-boot.sizes' % dirname,
                         build.get_func_sizes_file(1, 'fred', 'u-boot'))
        self.assertEqual('base%s/fred/u-boot.objdump' % dirname,
                         build.get_objdump_file(1, 'fred', 'u-boot'))
        self.assertEqual('base%s/fred/err' % dirname,
                         build.get_err_file(1, 'fred'))

    def testOutputDir(self):
        build = builder.Builder(self.toolchains, BASE_DIR, None, 1, 2,
                                checkout=False, show_unknown=False)
        build.commits = self.commits
        build.commit_count = len(self.commits)
        subject = self.commits[1].subject.translate(builder.trans_valid_chars)
        dirname ='/%02d_g%s_%s' % (2, commits[1][0], subject[:20])
        self.CheckDirs(build, dirname)

    def testOutputDirCurrent(self):
        build = builder.Builder(self.toolchains, BASE_DIR, None, 1, 2,
                                checkout=False, show_unknown=False)
        build.commits = None
        build.commit_count = 0
        self.CheckDirs(build, '/current')

    def testOutputDirNoSubdirs(self):
        build = builder.Builder(self.toolchains, BASE_DIR, None, 1, 2,
                                checkout=False, show_unknown=False,
                                no_subdirs=True)
        build.commits = None
        build.commit_count = 0
        self.CheckDirs(build, '')

    def testToolchainAliases(self):
        self.assertTrue(self.toolchains.Select('arm') != None)
        with self.assertRaises(ValueError):
            self.toolchains.Select('no-arch')
        with self.assertRaises(ValueError):
            self.toolchains.Select('x86')

        self.toolchains = toolchain.Toolchains()
        self.toolchains.Add('x86_64-linux-gcc', test=False)
        self.assertTrue(self.toolchains.Select('x86') != None)

        self.toolchains = toolchain.Toolchains()
        self.toolchains.Add('i386-linux-gcc', test=False)
        self.assertTrue(self.toolchains.Select('x86') != None)

    def testToolchainDownload(self):
        """Test that we can download toolchains"""
        if use_network:
            with test_util.capture_sys_output() as (stdout, stderr):
                url = self.toolchains.LocateArchUrl('arm')
            self.assertRegex(url, 'https://www.kernel.org/pub/tools/'
                    'crosstool/files/bin/x86_64/.*/'
                    'x86_64-gcc-.*-nolibc[-_]arm-.*linux-gnueabi.tar.xz')

    def testGetEnvArgs(self):
        """Test the GetEnvArgs() function"""
        tc = self.toolchains.Select('arm')
        self.assertEqual('arm-linux-',
                         tc.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))
        self.assertEqual('', tc.GetEnvArgs(toolchain.VAR_PATH))
        self.assertEqual('arm',
                         tc.GetEnvArgs(toolchain.VAR_ARCH))
        self.assertEqual('', tc.GetEnvArgs(toolchain.VAR_MAKE_ARGS))

        tc = self.toolchains.Select('sandbox')
        self.assertEqual('', tc.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))

        self.toolchains.Add('/path/to/x86_64-linux-gcc', test=False)
        tc = self.toolchains.Select('x86')
        self.assertEqual('/path/to',
                         tc.GetEnvArgs(toolchain.VAR_PATH))
        tc.override_toolchain = 'clang'
        self.assertEqual('HOSTCC=clang CC=clang',
                         tc.GetEnvArgs(toolchain.VAR_MAKE_ARGS))

        # Test config with ccache wrapper
        bsettings.setup(None)
        bsettings.add_file(settings_data_wrapper)

        tc = self.toolchains.Select('arm')
        self.assertEqual('ccache arm-linux-',
                         tc.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))

        tc = self.toolchains.Select('sandbox')
        self.assertEqual('', tc.GetEnvArgs(toolchain.VAR_CROSS_COMPILE))

    def testMakeEnvironment(self):
        """Test the MakeEnvironment function"""
        tc = self.toolchains.Select('arm')
        env = tc.MakeEnvironment(False)
        self.assertEqual(env[b'CROSS_COMPILE'], b'arm-linux-')

        tc = self.toolchains.Select('sandbox')
        env = tc.MakeEnvironment(False)
        self.assertTrue(b'CROSS_COMPILE' not in env)

        # Test config with ccache wrapper
        bsettings.setup(None)
        bsettings.add_file(settings_data_wrapper)

        tc = self.toolchains.Select('arm')
        env = tc.MakeEnvironment(False)
        self.assertEqual(env[b'CROSS_COMPILE'], b'ccache arm-linux-')

        tc = self.toolchains.Select('sandbox')
        env = tc.MakeEnvironment(False)
        self.assertTrue(b'CROSS_COMPILE' not in env)

    def testPrepareOutputSpace(self):
        def _Touch(fname):
            tools.write_file(os.path.join(base_dir, fname), b'')

        base_dir = tempfile.mkdtemp()

        # Add various files that we want removed and left alone
        to_remove = ['01_g0982734987_title', '102_g92bf_title',
                     '01_g2938abd8_title']
        to_leave = ['something_else', '01-something.patch', '01_another']
        for name in to_remove + to_leave:
            _Touch(name)

        build = builder.Builder(self.toolchains, base_dir, None, 1, 2)
        build.commits = self.commits
        build.commit_count = len(commits)
        result = set(build._get_output_space_removals())
        expected = set([os.path.join(base_dir, f) for f in to_remove])
        self.assertEqual(expected, result)

    def test_adjust_cfg_nop(self):
        """check various adjustments of config that are nops"""
        # enable an enabled CONFIG
        self.assertEqual(
            'CONFIG_FRED=y',
            cfgutil.adjust_cfg_line('CONFIG_FRED=y', {'FRED':'FRED'})[0])

        # disable a disabled CONFIG
        self.assertEqual(
            '# CONFIG_FRED is not set',
            cfgutil.adjust_cfg_line(
                '# CONFIG_FRED is not set', {'FRED':'~FRED'})[0])

        # use the adjust_cfg_lines() function
        self.assertEqual(
            ['CONFIG_FRED=y'],
            cfgutil.adjust_cfg_lines(['CONFIG_FRED=y'], {'FRED':'FRED'}))
        self.assertEqual(
            ['# CONFIG_FRED is not set'],
            cfgutil.adjust_cfg_lines(['CONFIG_FRED=y'], {'FRED':'~FRED'}))

        # handling an empty line
        self.assertEqual('#', cfgutil.adjust_cfg_line('#', {'FRED':'~FRED'})[0])

    def test_adjust_cfg(self):
        """check various adjustments of config"""
        # disable a CONFIG
        self.assertEqual(
            '# CONFIG_FRED is not set',
            cfgutil.adjust_cfg_line('CONFIG_FRED=1' , {'FRED':'~FRED'})[0])

        # enable a disabled CONFIG
        self.assertEqual(
            'CONFIG_FRED=y',
            cfgutil.adjust_cfg_line(
                '# CONFIG_FRED is not set', {'FRED':'FRED'})[0])

        # enable a CONFIG that doesn't exist
        self.assertEqual(
            ['CONFIG_FRED=y'],
            cfgutil.adjust_cfg_lines([], {'FRED':'FRED'}))

        # disable a CONFIG that doesn't exist
        self.assertEqual(
            ['# CONFIG_FRED is not set'],
            cfgutil.adjust_cfg_lines([], {'FRED':'~FRED'}))

        # disable a value CONFIG
        self.assertEqual(
            '# CONFIG_FRED is not set',
            cfgutil.adjust_cfg_line('CONFIG_FRED="fred"' , {'FRED':'~FRED'})[0])

        # setting a value CONFIG
        self.assertEqual(
            'CONFIG_FRED="fred"',
            cfgutil.adjust_cfg_line('# CONFIG_FRED is not set' ,
                                    {'FRED':'FRED="fred"'})[0])

        # changing a value CONFIG
        self.assertEqual(
            'CONFIG_FRED="fred"',
            cfgutil.adjust_cfg_line('CONFIG_FRED="ernie"' ,
                                    {'FRED':'FRED="fred"'})[0])

        # setting a value for a CONFIG that doesn't exist
        self.assertEqual(
            ['CONFIG_FRED="fred"'],
            cfgutil.adjust_cfg_lines([], {'FRED':'FRED="fred"'}))

    def test_convert_adjust_cfg_list(self):
        """Check conversion of the list of changes into a dict"""
        self.assertEqual({}, cfgutil.convert_list_to_dict(None))

        expect = {
            'FRED':'FRED',
            'MARY':'~MARY',
            'JOHN':'JOHN=0x123',
            'ALICE':'ALICE="alice"',
            'AMY':'AMY',
            'ABE':'~ABE',
            'MARK':'MARK=0x456',
            'ANNA':'ANNA="anna"',
            }
        actual = cfgutil.convert_list_to_dict(
            ['FRED', '~MARY', 'JOHN=0x123', 'ALICE="alice"',
             'CONFIG_AMY', '~CONFIG_ABE', 'CONFIG_MARK=0x456',
             'CONFIG_ANNA="anna"'])
        self.assertEqual(expect, actual)

    def test_check_cfg_file(self):
        """Test check_cfg_file detects conflicts as expected"""
        # Check failure to disable CONFIG
        result = cfgutil.check_cfg_lines(['CONFIG_FRED=1'], {'FRED':'~FRED'})
        self.assertEqual([['~FRED', 'CONFIG_FRED=1']], result)

        result = cfgutil.check_cfg_lines(
            ['CONFIG_FRED=1', 'CONFIG_MARY="mary"'], {'FRED':'~FRED'})
        self.assertEqual([['~FRED', 'CONFIG_FRED=1']], result)

        result = cfgutil.check_cfg_lines(
            ['CONFIG_FRED=1', 'CONFIG_MARY="mary"'], {'MARY':'~MARY'})
        self.assertEqual([['~MARY', 'CONFIG_MARY="mary"']], result)

        # Check failure to enable CONFIG
        result = cfgutil.check_cfg_lines(
            ['# CONFIG_FRED is not set'], {'FRED':'FRED'})
        self.assertEqual([['FRED', '# CONFIG_FRED is not set']], result)

        # Check failure to set CONFIG value
        result = cfgutil.check_cfg_lines(
            ['# CONFIG_FRED is not set', 'CONFIG_MARY="not"'],
            {'MARY':'MARY="mary"', 'FRED':'FRED'})
        self.assertEqual([
            ['FRED', '# CONFIG_FRED is not set'],
            ['MARY="mary"', 'CONFIG_MARY="not"']], result)

        # Check failure to add CONFIG value
        result = cfgutil.check_cfg_lines([], {'MARY':'MARY="mary"'})
        self.assertEqual([
            ['MARY="mary"', 'Missing expected line: CONFIG_MARY="mary"']], result)

    def get_procs(self):
        running_fname = os.path.join(self.base_dir, control.RUNNING_FNAME)
        items = tools.read_file(running_fname, binary=False).split()
        return [int(x) for x in items]

    def get_time(self):
        return self.cur_time

    def inc_time(self, amount):
        self.cur_time += amount

        # Handle a process exiting
        if self.finish_time == self.cur_time:
            self.valid_pids = [pid for pid in self.valid_pids
                               if pid != self.finish_pid]

    def kill(self, pid, signal):
        if pid not in self.valid_pids:
            raise OSError('Invalid PID')

    def test_process_limit(self):
        """Test wait_for_process_limit() function"""
        tmpdir = self.base_dir

        with (patch('time.time', side_effect=self.get_time),
              patch('time.perf_counter', side_effect=self.get_time),
              patch('time.monotonic', side_effect=self.get_time),
              patch('time.sleep', side_effect=self.inc_time),
              patch('os.kill', side_effect=self.kill)):
            # Grab the process. Since there is no other profcess, this should
            # immediately succeed
            control.wait_for_process_limit(1, tmpdir=tmpdir, pid=1)
            lines = terminal.get_print_test_lines()
            self.assertEqual(0, self.cur_time)
            self.assertEqual('Waiting for other buildman processes...',
                             lines[0].text)
            self.assertEqual(self._col.RED, lines[0].colour)
            self.assertEqual(False, lines[0].newline)
            self.assertEqual(True, lines[0].bright)

            self.assertEqual('done...', lines[1].text)
            self.assertEqual(None, lines[1].colour)
            self.assertEqual(False, lines[1].newline)
            self.assertEqual(True, lines[1].bright)

            self.assertEqual('starting build', lines[2].text)
            self.assertEqual([1], control.read_procs(tmpdir))
            self.assertEqual(None, lines[2].colour)
            self.assertEqual(False, lines[2].newline)
            self.assertEqual(True, lines[2].bright)

            # Try again, with a different PID...this should eventually timeout
            # and start the build anyway
            self.cur_time = 0
            self.valid_pids = [1]
            control.wait_for_process_limit(1, tmpdir=tmpdir, pid=2)
            lines = terminal.get_print_test_lines()
            self.assertEqual('Waiting for other buildman processes...',
                             lines[0].text)
            self.assertEqual('timeout...', lines[1].text)
            self.assertEqual(None, lines[1].colour)
            self.assertEqual(False, lines[1].newline)
            self.assertEqual(True, lines[1].bright)
            self.assertEqual('starting build', lines[2].text)
            self.assertEqual([1, 2], control.read_procs(tmpdir))
            self.assertEqual(control.RUN_WAIT_S, self.cur_time)

            # Check lock-busting
            self.cur_time = 0
            self.valid_pids = [1, 2]
            lock_fname = os.path.join(tmpdir, control.LOCK_FNAME)
            lock = FileLock(lock_fname)
            lock.acquire(timeout=1)
            control.wait_for_process_limit(1, tmpdir=tmpdir, pid=3)
            lines = terminal.get_print_test_lines()
            self.assertEqual('Waiting for other buildman processes...',
                             lines[0].text)
            self.assertEqual('failed to get lock: busting...', lines[1].text)
            self.assertEqual(None, lines[1].colour)
            self.assertEqual(False, lines[1].newline)
            self.assertEqual(True, lines[1].bright)
            self.assertEqual('timeout...', lines[2].text)
            self.assertEqual('starting build', lines[3].text)
            self.assertEqual([1, 2, 3], control.read_procs(tmpdir))
            self.assertEqual(control.RUN_WAIT_S, self.cur_time)
            lock.release()

            # Check handling of dead processes. Here we have PID 2 as a running
            # process, even though the PID file contains 1, 2 and 3. So we can
            # add one more PID, to make 2 and 4
            self.cur_time = 0
            self.valid_pids = [2]
            control.wait_for_process_limit(2, tmpdir=tmpdir, pid=4)
            lines = terminal.get_print_test_lines()
            self.assertEqual('Waiting for other buildman processes...',
                             lines[0].text)
            self.assertEqual('done...', lines[1].text)
            self.assertEqual('starting build', lines[2].text)
            self.assertEqual([2, 4], control.read_procs(tmpdir))
            self.assertEqual(0, self.cur_time)

            # Try again, with PID 2 quitting at time 50. This allows the new
            # build to start
            self.cur_time = 0
            self.valid_pids = [2, 4]
            self.finish_pid = 2
            self.finish_time = 50
            control.wait_for_process_limit(2, tmpdir=tmpdir, pid=5)
            lines = terminal.get_print_test_lines()
            self.assertEqual('Waiting for other buildman processes...',
                             lines[0].text)
            self.assertEqual('done...', lines[1].text)
            self.assertEqual('starting build', lines[2].text)
            self.assertEqual([4, 5], control.read_procs(tmpdir))
            self.assertEqual(self.finish_time, self.cur_time)

    def call_make_environment(self, tchn, full_path, in_env=None):
        """Call Toolchain.MakeEnvironment() and process the result

        Args:
            tchn (Toolchain): Toolchain to use
            full_path (bool): True to return the full path in CROSS_COMPILE
                rather than adding it to the PATH variable
            in_env (dict): Input environment to use, None to use current env

        Returns:
            tuple:
                dict: Changes that MakeEnvironment has made to the environment
                    key: Environment variable that was changed
                    value: New value (for PATH this only includes components
                        which were added)
                str: Full value of the new PATH variable
        """
        env = tchn.MakeEnvironment(full_path, env=in_env)

        # Get the original environment
        orig_env = dict(os.environb if in_env is None else in_env)
        orig_path = orig_env[b'PATH'].split(b':')

        # Find new variables
        diff = dict((k, env[k]) for k in env if orig_env.get(k) != env[k])

        # Find new / different path components
        diff_path = None
        new_path = None
        if b'PATH' in diff:
            new_path = diff[b'PATH'].split(b':')
            diff_paths = [p for p in new_path if p not in orig_path]
            diff_path = b':'.join(p for p in new_path if p not in orig_path)
            if diff_path:
                diff[b'PATH'] = diff_path
            else:
                del diff[b'PATH']
        return diff, new_path

    def test_toolchain_env(self):
        """Test PATH and other environment settings for toolchains"""
        # Use a toolchain which has a path, so that full_path makes a difference
        tchn = self.toolchains.Select('aarch64')

        # Normal cases
        diff = self.call_make_environment(tchn, full_path=False)[0]
        self.assertEqual(
            {b'CROSS_COMPILE': b'aarch64-linux-', b'LC_ALL': b'C',
             b'PATH': b'/path/to'}, diff)

        diff = self.call_make_environment(tchn, full_path=True)[0]
        self.assertEqual(
            {b'CROSS_COMPILE': b'/path/to/aarch64-linux-', b'LC_ALL': b'C'},
            diff)

        # When overriding the toolchain, only LC_ALL should be set
        tchn.override_toolchain = True
        diff = self.call_make_environment(tchn, full_path=True)[0]
        self.assertEqual({b'LC_ALL': b'C'}, diff)

        # Test that virtualenv is handled correctly
        tchn.override_toolchain = False
        sys.prefix = '/some/venv'
        env = dict(os.environb)
        env[b'PATH'] = b'/some/venv/bin:other/things'
        tchn.path = '/my/path'
        diff, diff_path = self.call_make_environment(tchn, False, env)

        self.assertIn(b'PATH', diff)
        self.assertEqual([b'/some/venv/bin', b'/my/path', b'other/things'],
                         diff_path)
        self.assertEqual(
            {b'CROSS_COMPILE': b'aarch64-linux-', b'LC_ALL': b'C',
             b'PATH': b'/my/path'}, diff)

        # Handle a toolchain wrapper
        tchn.path = ''
        bsettings.add_section('toolchain-wrapper')
        bsettings.set_item('toolchain-wrapper', 'my-wrapper', 'fred')
        diff = self.call_make_environment(tchn, full_path=True)[0]
        self.assertEqual(
            {b'CROSS_COMPILE': b'fred aarch64-linux-', b'LC_ALL': b'C'}, diff)

    def test_skip_dtc(self):
        """Test skipping building the dtc tool"""
        old_path = os.getenv('PATH')
        try:
            os.environ['PATH'] = self.base_dir

            # Check a missing tool
            with self.assertRaises(ValueError) as exc:
                builder.Builder(self.toolchains, self.base_dir, None, 0, 2,
                                dtc_skip=True)
            self.assertIn('Cannot find dtc', str(exc.exception))

            # Create a fake tool to use
            dtc = os.path.join(self.base_dir, 'dtc')
            tools.write_file(dtc, b'xx')
            os.chmod(dtc, 0o777)

            build = builder.Builder(self.toolchains, self.base_dir, None, 0, 2,
                                    dtc_skip=True)
            toolchain = self.toolchains.Select('arm')
            env = build.make_environment(toolchain)
            self.assertIn(b'DTC', env)

            # Try the normal case, i.e. not skipping the dtc build
            build = builder.Builder(self.toolchains, self.base_dir, None, 0, 2)
            toolchain = self.toolchains.Select('arm')
            env = build.make_environment(toolchain)
            self.assertNotIn(b'DTC', env)
        finally:
            os.environ['PATH'] = old_path

    def testHomedir(self):
        """Test using ~ in a toolchain or toolchain-prefix section"""
        # Add some test settings
        bsettings.setup(None)
        bsettings.add_file(settings_data_homedir)

        # Set up the toolchains
        home = os.path.expanduser('~')
        toolchains = toolchain.Toolchains()
        toolchains.GetSettings()
        self.assertEqual([f'{home}/mypath'], toolchains.paths)

        # Check scanning
        with test_util.capture_sys_output() as (stdout, _):
            toolchains.Scan(verbose=True, raise_on_error=False)
        lines = iter(stdout.getvalue().splitlines() + ['##done'])
        self.assertEqual('Scanning for tool chains', next(lines))
        self.assertEqual(f"   - scanning prefix '{home}/mypath-x86-'",
                         next(lines))
        self.assertEqual(
            f"Error: No tool chain found for prefix '{home}/mypath-x86-gcc'",
            next(lines))
        self.assertEqual(f"   - scanning path '{home}/mypath'", next(lines))
        self.assertEqual(f"      - looking in '{home}/mypath/.'", next(lines))
        self.assertEqual(f"      - looking in '{home}/mypath/bin'", next(lines))
        self.assertEqual(f"      - looking in '{home}/mypath/usr/bin'",
                         next(lines))
        self.assertEqual('##done', next(lines))

        # Check adding a toolchain
        with test_util.capture_sys_output() as (stdout, _):
            toolchains.Add('~/aarch64-linux-gcc', test=True, verbose=True)
        lines = iter(stdout.getvalue().splitlines() + ['##done'])
        self.assertEqual('Tool chain test:  BAD', next(lines))
        self.assertEqual(f'Command: {home}/aarch64-linux-gcc --version',
                         next(lines))
        self.assertEqual('', next(lines))
        self.assertEqual('', next(lines))
        self.assertEqual('##done', next(lines))


if __name__ == "__main__":
    unittest.main()

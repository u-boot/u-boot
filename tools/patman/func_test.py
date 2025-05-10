# -*- coding: utf-8 -*-
# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2017 Google, Inc
#

"""Functional tests for checking that patman behaves correctly"""

import asyncio
import contextlib
import os
import pathlib
import re
import shutil
import sys
import unittest

import pygit2

from u_boot_pylib import command
from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tools

from patman.commit import Commit
from patman import control
from patman import patchstream
from patman.patchstream import PatchStream
from patman import patchwork
from patman import send
from patman.series import Series
from patman import status
from patman.test_common import TestCommon

PATMAN_DIR = pathlib.Path(__file__).parent
TEST_DATA_DIR = PATMAN_DIR / 'test/'


@contextlib.contextmanager
def directory_excursion(directory):
    """Change directory to `directory` for a limited to the context block."""
    current = os.getcwd()
    try:
        os.chdir(directory)
        yield
    finally:
        os.chdir(current)


class TestFunctional(unittest.TestCase, TestCommon):
    """Functional tests for checking that patman behaves correctly"""
    fred = 'Fred Bloggs <f.bloggs@napier.net>'
    joe = 'Joe Bloggs <joe@napierwallies.co.nz>'
    mary = 'Mary Bloggs <mary@napierwallies.co.nz>'
    commits = None
    patches = None

    def setUp(self):
        TestCommon.setUp(self)
        self.repo = None
        self._patman_pathname = sys.argv[0]
        self._patman_dir = os.path.dirname(os.path.realpath(sys.argv[0]))

    def tearDown(self):
        TestCommon.tearDown(self)

    @staticmethod
    def _get_path(fname):
        """Get the path to a test file

        Args:
            fname (str): Filename to obtain

        Returns:
            str: Full path to file in the test directory
        """
        return TEST_DATA_DIR / fname

    @classmethod
    def _get_text(cls, fname):
        """Read a file as text

        Args:
            fname (str): Filename to read

        Returns:
            str: Contents of file
        """
        return open(cls._get_path(fname), encoding='utf-8').read()

    @classmethod
    def _get_patch_name(cls, subject):
        """Get the filename of a patch given its subject

        Args:
            subject (str): Patch subject

        Returns:
            str: Filename for that patch
        """
        fname = re.sub('[ :]', '-', subject)
        return fname.replace('--', '-')

    def _create_patches_for_test(self, series):
        """Create patch files for use by tests

        This copies patch files from the test directory as needed by the series

        Args:
            series (Series): Series containing commits to convert

        Returns:
            tuple:
                str: Cover-letter filename, or None if none
                fname_list: list of str, each a patch filename
        """
        cover_fname = None
        fname_list = []
        for i, commit in enumerate(series.commits):
            clean_subject = self._get_patch_name(commit.subject)
            src_fname = '%04d-%s.patch' % (i + 1, clean_subject[:52])
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self._get_path(src_fname), fname)
            fname_list.append(fname)
        if series.get('cover'):
            src_fname = '0000-cover-letter.patch'
            cover_fname = os.path.join(self.tmpdir, src_fname)
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self._get_path(src_fname), fname)

        return cover_fname, fname_list

    def test_basic(self):
        """Tests the basic flow of patman

        This creates a series from some hard-coded patches build from a simple
        tree with the following metadata in the top commit:

            Series-to: u-boot
            Series-prefix: RFC
            Series-postfix: some-branch
            Series-cc: Stefan Brüns <stefan.bruens@rwth-aachen.de>
            Cover-letter-cc: Lord Mëlchett <clergy@palace.gov>
            Series-version: 3
            Patch-cc: fred
            Series-process-log: sort, uniq
            Series-changes: 4
            - Some changes
            - Multi
              line
              change

            Commit-changes: 2
            - Changes only for this commit

            Cover-changes: 4
            - Some notes for the cover letter

            Cover-letter:
            test: A test patch series
            This is a test of how the cover
            letter
            works
            END

        and this in the first commit:

            Commit-changes: 2
            - second revision change

            Series-notes:
            some notes
            about some things
            from the first commit
            END

            Commit-notes:
            Some notes about
            the first commit
            END

        with the following commands:

           git log -n2 --reverse >/path/to/tools/patman/test/test01.txt
           git format-patch --subject-prefix RFC --cover-letter HEAD~2
           mv 00* /path/to/tools/patman/test

        It checks these aspects:
            - git log can be processed by patchstream
            - emailing patches uses the correct command
            - CC file has information on each commit
            - cover letter has the expected text and subject
            - each patch has the correct subject
            - dry-run information prints out correctly
            - unicode is handled correctly
            - Series-to, Series-cc, Series-prefix, Series-postfix, Cover-letter
            - Cover-letter-cc, Series-version, Series-changes, Series-notes
            - Commit-notes
        """
        process_tags = True
        ignore_bad_tags = False
        stefan = (b'Stefan Br\xc3\xbcns <stefan.bruens@rwth-aachen.de>'
                  .decode('utf-8'))
        rick = 'Richard III <richard@palace.gov>'
        mel = b'Lord M\xc3\xablchett <clergy@palace.gov>'.decode('utf-8')
        add_maintainers = [stefan, rick]
        dry_run = True
        in_reply_to = mel
        count = 2
        alias = {
            'fdt': ['simon'],
            'u-boot': ['u-boot@lists.denx.de'],
            'simon': [self.leb],
            'fred': [self.fred],
            'joe': [self.joe],
        }

        text = self._get_text('test01.txt')
        series = patchstream.get_metadata_for_test(text)
        series.base_commit = Commit('1a44532')
        series.branch = 'mybranch'
        cover_fname, args = self._create_patches_for_test(series)
        get_maintainer_script = str(pathlib.Path(__file__).parent.parent.parent
                                    / 'get_maintainer.pl') + ' --norolestats'
        with terminal.capture() as out:
            patchstream.fix_patches(series, args)
            if cover_fname and series.get('cover'):
                patchstream.insert_cover_letter(cover_fname, series, count)
            series.DoChecks()
            cc_file = series.MakeCcFile(process_tags, cover_fname,
                                        not ignore_bad_tags, add_maintainers,
                                        None, get_maintainer_script, alias)
            cmd = gitutil.email_patches(
                series, cover_fname, args, dry_run, not ignore_bad_tags,
                cc_file, alias, in_reply_to=in_reply_to, thread=None)
            series.ShowActions(args, cmd, process_tags, alias)
        cc_lines = tools.read_file(cc_file, binary=False).splitlines()
        os.remove(cc_file)

        itr = iter(out[0].getvalue().splitlines())
        self.assertEqual('Cleaned %s patches' % len(series.commits),
                         next(itr))
        self.assertEqual('Change log missing for v2', next(itr))
        self.assertEqual('Change log missing for v3', next(itr))
        self.assertEqual('Change log for unknown version v4', next(itr))
        self.assertEqual("Alias 'pci' not found", next(itr))
        while next(itr) != 'Cc processing complete':
            pass
        self.assertIn('Dry run', next(itr))
        self.assertEqual('', next(itr))
        self.assertIn('Send a total of %d patches' % count, next(itr))
        prev = next(itr)
        for i in range(len(series.commits)):
            self.assertEqual('   %s' % args[i], prev)
            while True:
                prev = next(itr)
                if 'Cc:' not in prev:
                    break
        self.assertEqual('To:	  u-boot@lists.denx.de', prev)
        self.assertEqual('Cc:	  %s' % stefan, next(itr))
        self.assertEqual('Version:  3', next(itr))
        self.assertEqual('Prefix:\t  RFC', next(itr))
        self.assertEqual('Postfix:\t  some-branch', next(itr))
        self.assertEqual('Cover: 4 lines', next(itr))
        self.assertEqual('      Cc:  %s' % self.fred, next(itr))
        self.assertEqual('      Cc:  %s' % self.joe, next(itr))
        self.assertEqual('      Cc:  %s' % self.leb,
                         next(itr))
        self.assertEqual('      Cc:  %s' % mel, next(itr))
        self.assertEqual('      Cc:  %s' % rick, next(itr))
        expected = ('Git command: git send-email --annotate '
                    '--in-reply-to="%s" --to u-boot@lists.denx.de '
                    '--cc "%s" --cc-cmd "%s send --cc-cmd %s" %s %s'
                    % (in_reply_to, stefan, sys.argv[0], cc_file, cover_fname,
                       ' '.join(args)))
        self.assertEqual(expected, next(itr))

        self.assertEqual(('%s %s\0%s' % (args[0], rick, stefan)), cc_lines[0])
        self.assertEqual(
            '%s %s\0%s\0%s\0%s\0%s' % (args[1], self.fred, self.joe, self.leb,
                                       rick, stefan),
            cc_lines[1])

        expected = '''
This is a test of how the cover
letter
works

some notes
about some things
from the first commit

Changes in v4:
- Multi
  line
  change
- Some changes
- Some notes for the cover letter
- fdt: Correct cast for sandbox in fdtdec_setup_mem_size_base()

Simon Glass (2):
  pci: Correct cast for sandbox
  fdt: Correct cast for sandbox in fdtdec_setup_mem_size_base()

 cmd/pci.c                   | 3 ++-
 fs/fat/fat.c                | 1 +
 lib/efi_loader/efi_memory.c | 1 +
 lib/fdtdec.c                | 3 ++-
 4 files changed, 6 insertions(+), 2 deletions(-)

--\x20
2.7.4

base-commit: 1a44532
branch: mybranch
'''
        lines = tools.read_file(cover_fname, binary=False).splitlines()
        self.assertEqual(
            'Subject: [RFC PATCH some-branch v3 0/2] test: A test patch series',
            lines[3])
        self.assertEqual(expected.splitlines(), lines[7:])

        for i, fname in enumerate(args):
            lines = tools.read_file(fname, binary=False).splitlines()
            subject = [line for line in lines if line.startswith('Subject')]
            self.assertEqual('Subject: [RFC %d/%d]' % (i + 1, count),
                             subject[0][:18])

            # Check that we got our commit notes
            start = 0
            expected = ''

            if i == 0:
                start = 17
                expected = '''---
Some notes about
the first commit

(no changes since v2)

Changes in v2:
- second revision change'''
            elif i == 1:
                start = 17
                expected = '''---

Changes in v4:
- Multi
  line
  change
- New
- Some changes

Changes in v2:
- Changes only for this commit'''

            if expected:
                expected = expected.splitlines()
                self.assertEqual(expected, lines[start:(start+len(expected))])

    def test_base_commit(self):
        """Test adding a base commit with no cover letter"""
        orig_text = self._get_text('test01.txt')
        pos = orig_text.index(
            'commit 5ab48490f03051875ab13d288a4bf32b507d76fd')
        text = orig_text[:pos]
        series = patchstream.get_metadata_for_test(text)
        series.base_commit = Commit('1a44532')
        series.branch = 'mybranch'
        cover_fname, args = self._create_patches_for_test(series)
        self.assertFalse(cover_fname)
        with terminal.capture() as out:
            patchstream.fix_patches(series, args, insert_base_commit=True)
        self.assertEqual('Cleaned 1 patch\n', out[0].getvalue())
        lines = tools.read_file(args[0], binary=False).splitlines()
        pos = lines.index('-- ')

        # We expect these lines at the end:
        # -- (with trailing space)
        # 2.7.4
        # (empty)
        # base-commit: xxx
        # branch: xxx
        self.assertEqual('base-commit: 1a44532', lines[pos + 3])
        self.assertEqual('branch: mybranch', lines[pos + 4])

    def test_branch(self):
        """Test creating patches from a branch"""
        repo = self.make_git_tree()
        target = repo.lookup_reference('refs/heads/first')
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        self.repo.checkout(target, strategy=pygit2.GIT_CHECKOUT_FORCE)
        control.setup()
        orig_dir = os.getcwd()
        try:
            os.chdir(self.tmpdir)

            # Check that it can detect the current branch
            self.assertEqual(2, gitutil.count_commits_to_branch(None))
            col = terminal.Color()
            with terminal.capture() as _:
                _, cover_fname, patch_files = send.prepare_patches(
                    col, branch=None, count=-1, start=0, end=0,
                    ignore_binary=False, signoff=True)
            self.assertIsNone(cover_fname)
            self.assertEqual(2, len(patch_files))

            # Check that it can detect a different branch
            self.assertEqual(3, gitutil.count_commits_to_branch('second'))
            with terminal.capture() as _:
                _, cover_fname, patch_files = send.prepare_patches(
                    col, branch='second', count=-1, start=0, end=0,
                    ignore_binary=False, signoff=True)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(3, len(patch_files))

            cover = tools.read_file(cover_fname, binary=False)
            lines = cover.splitlines()[-2:]
            base = repo.lookup_reference('refs/heads/base').target
            self.assertEqual(f'base-commit: {base}', lines[0])
            self.assertEqual('branch: second', lines[1])

            # Make sure that the base-commit is not present when it is in the
            # cover letter
            for fname in patch_files:
                self.assertNotIn(b'base-commit:', tools.read_file(fname))

            # Check that it can skip patches at the end
            with terminal.capture() as _:
                _, cover_fname, patch_files = send.prepare_patches(
                    col, branch='second', count=-1, start=0, end=1,
                    ignore_binary=False, signoff=True)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(2, len(patch_files))

            cover = tools.read_file(cover_fname, binary=False)
            lines = cover.splitlines()[-2:]
            base2 = repo.lookup_reference('refs/heads/second')
            ref = base2.peel(pygit2.GIT_OBJ_COMMIT).parents[0].parents[0].id
            self.assertEqual(f'base-commit: {ref}', lines[0])
            self.assertEqual('branch: second', lines[1])
        finally:
            os.chdir(orig_dir)

    def test_custom_get_maintainer_script(self):
        """Validate that a custom get_maintainer script gets used."""
        self.make_git_tree()
        with directory_excursion(self.tmpdir):
            # Setup git.
            os.environ['GIT_CONFIG_GLOBAL'] = '/dev/null'
            os.environ['GIT_CONFIG_SYSTEM'] = '/dev/null'
            tools.run('git', 'config', 'user.name', 'Dummy')
            tools.run('git', 'config', 'user.email', 'dumdum@dummy.com')
            tools.run('git', 'branch', 'upstream')
            tools.run('git', 'branch', '--set-upstream-to=upstream')

            # Setup patman configuration.
            tools.write_file('.patman', '[settings]\n'
                             'get_maintainer_script: dummy-script.sh\n'
                             'check_patch: False\n'
                             'add_maintainers: True\n', binary=False)
            tools.write_file('dummy-script.sh',
                             '#!/usr/bin/env python3\n'
                             'print("hello@there.com")\n', binary=False)
            os.chmod('dummy-script.sh', 0x555)
            tools.run('git', 'add', '.')
            tools.run('git', 'commit', '-m', 'new commit')

            # Finally, do the test
            with terminal.capture():
                output = tools.run(PATMAN_DIR / 'patman', '--dry-run')
                # Assert the email address is part of the dry-run
                # output.
                self.assertIn('hello@there.com', output)

    def test_tags(self):
        """Test collection of tags in a patchstream"""
        text = '''This is a patch

Signed-off-by: Terminator
Reviewed-by: %s
Reviewed-by: %s
Tested-by: %s
''' % (self.joe, self.mary, self.leb)
        pstrm = PatchStream.process_text(text)
        self.assertEqual(pstrm.commit.rtags, {
            'Reviewed-by': {self.joe, self.mary},
            'Tested-by': {self.leb}})

    def test_invalid_tag(self):
        """Test invalid tag in a patchstream"""
        text = '''This is a patch

Serie-version: 2
'''
        with self.assertRaises(ValueError) as exc:
            PatchStream.process_text(text)
        self.assertEqual("Line 3: Invalid tag = 'Serie-version: 2'",
                         str(exc.exception))

    def test_missing_end(self):
        """Test a missing END tag"""
        text = '''This is a patch

Cover-letter:
This is the title
missing END after this line
Signed-off-by: Fred
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Missing 'END' in section 'cover'"],
                         pstrm.commit.warn)

    def test_missing_blank_line(self):
        """Test a missing blank line after a tag"""
        text = '''This is a patch

Series-changes: 2
- First line of changes
- Missing blank line after this line
Signed-off-by: Fred
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Missing 'blank line' in section 'Series-changes'"],
                         pstrm.commit.warn)

    def test_invalid_commit_tag(self):
        """Test an invalid Commit-xxx tag"""
        text = '''This is a patch

Commit-fred: testing
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Line 3: Ignoring Commit-fred"], pstrm.commit.warn)

    def test_self_test(self):
        """Test a tested by tag by this user"""
        test_line = 'Tested-by: %s@napier.com' % os.getenv('USER')
        text = '''This is a patch

%s
''' % test_line
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Ignoring '%s'" % test_line], pstrm.commit.warn)

    def test_space_before_tab(self):
        """Test a space before a tab"""
        text = '''This is a patch

+ \tSomething
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Line 3/0 has space before tab"], pstrm.commit.warn)

    def test_lines_after_test(self):
        """Test detecting lines after TEST= line"""
        text = '''This is a patch

TEST=sometest
more lines
here
'''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(["Found 2 lines after TEST="], pstrm.commit.warn)

    def test_blank_line_at_end(self):
        """Test detecting a blank line at the end of a file"""
        text = '''This is a patch

diff --git a/lib/fdtdec.c b/lib/fdtdec.c
index c072e54..942244f 100644
--- a/lib/fdtdec.c
+++ b/lib/fdtdec.c
@@ -1200,7 +1200,8 @@ int fdtdec_setup_mem_size_base(void)
 \t}

 \tgd->ram_size = (phys_size_t)(res.end - res.start + 1);
-	debug("%s: Initial DRAM size %llx\n", __func__, (u64)gd->ram_size);
+	debug("%s: Initial DRAM size %llx\n", __func__,
+	      (unsigned long long)gd->ram_size);
+
diff --git a/lib/efi_loader/efi_memory.c b/lib/efi_loader/efi_memory.c

--
2.7.4

 '''
        pstrm = PatchStream.process_text(text)
        self.assertEqual(
            ["Found possible blank line(s) at end of file 'lib/fdtdec.c'"],
            pstrm.commit.warn)

    def test_no_upstream(self):
        """Test CountCommitsToBranch when there is no upstream"""
        repo = self.make_git_tree()
        target = repo.lookup_reference('refs/heads/base')
        # pylint doesn't seem to find this
        # pylint: disable=E1101
        self.repo.checkout(target, strategy=pygit2.GIT_CHECKOUT_FORCE)

        # Check that it can detect the current branch
        orig_dir = os.getcwd()
        try:
            os.chdir(self.gitdir)
            with self.assertRaises(ValueError) as exc:
                gitutil.count_commits_to_branch(None)
            self.assertIn(
                "Failed to determine upstream: fatal: no upstream configured for branch 'base'",
                str(exc.exception))
        finally:
            os.chdir(orig_dir)

    def run_patman(self, *args):
        """Run patman using the provided arguments

        This runs the patman executable from scratch, as opposed to calling
        the control.do_patman() function.

        Args:
            args (list of str): Arguments to pass (excluding argv[0])

        Return:
            CommandResult: Result of execution
        """
        all_args = [self._patman_pathname] + list(args)
        return command.run_one(*all_args, capture=True, capture_stderr=True)

    def test_full_help(self):
        """Test getting full help"""
        command.TEST_RESULT = None
        result = self.run_patman('-H')
        help_file = os.path.join(self._patman_dir, 'README.rst')
        # Remove possible extraneous strings
        extra = '::::::::::::::\n' + help_file + '\n::::::::::::::\n'
        gothelp = result.stdout.replace(extra, '')
        self.assertEqual(len(gothelp), os.path.getsize(help_file))
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    def test_help(self):
        """Test getting help with commands and arguments"""
        command.TEST_RESULT = None
        result = self.run_patman('-h')
        self.assertTrue(len(result.stdout) > 1000)
        self.assertEqual(0, len(result.stderr))
        self.assertEqual(0, result.return_code)

    @staticmethod
    def _fake_patchwork(subpath):
        """Fake Patchwork server for the function below

        This handles accessing a series, providing a list consisting of a
        single patch

        Args:
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': [
                    {'id': '1', 'name': 'Some patch'}]}
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_status_mismatch(self):
        """Test Patchwork patches not matching the series"""
        pwork = patchwork.Patchwork.for_testing(self._fake_patchwork)
        with terminal.capture() as (_, err):
            loop = asyncio.get_event_loop()
            _, patches = loop.run_until_complete(status.check_status(1234,
                                                                     pwork))
            status.check_patch_count(0, len(patches))
        self.assertIn('Warning: Patchwork reports 1 patches, series has 0',
                      err.getvalue())

    def test_status_read_patch(self):
        """Test handling a single patch in Patchwork"""
        pwork = patchwork.Patchwork.for_testing(self._fake_patchwork)
        loop = asyncio.get_event_loop()
        _, patches = loop.run_until_complete(status.check_status(1234, pwork))
        self.assertEqual(1, len(patches))
        patch = patches[0]
        self.assertEqual('1', patch.id)
        self.assertEqual('Some patch', patch.raw_subject)

    def test_parse_subject(self):
        """Test parsing of the patch subject"""
        patch = patchwork.Patch('1')

        # Simple patch not in a series
        patch.parse_subject('Testing')
        self.assertEqual('Testing', patch.raw_subject)
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(1, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # First patch in a series
        patch.parse_subject('[1/2] Testing')
        self.assertEqual('[1/2] Testing', patch.raw_subject)
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(2, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # Second patch in a series
        patch.parse_subject('[2/2] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(2, patch.seq)
        self.assertEqual(2, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual(None, patch.version)

        # With PATCH prefix
        patch.parse_subject('[PATCH,2/5] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(2, patch.seq)
        self.assertEqual(5, patch.count)
        self.assertEqual('PATCH', patch.prefix)
        self.assertEqual(None, patch.version)

        # RFC patch
        patch.parse_subject('[RFC,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual('RFC', patch.prefix)
        self.assertEqual(None, patch.version)

        # Version patch
        patch.parse_subject('[v2,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual(None, patch.prefix)
        self.assertEqual('v2', patch.version)

        # All fields
        patch.parse_subject('[RESEND,v2,3/7] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(3, patch.seq)
        self.assertEqual(7, patch.count)
        self.assertEqual('RESEND', patch.prefix)
        self.assertEqual('v2', patch.version)

        # RFC only
        patch.parse_subject('[RESEND] Testing')
        self.assertEqual('Testing', patch.subject)
        self.assertEqual(1, patch.seq)
        self.assertEqual(1, patch.count)
        self.assertEqual('RESEND', patch.prefix)
        self.assertEqual(None, patch.version)

    def test_compare_series(self):
        """Test operation of compare_with_series()"""
        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'
        commit3 = Commit('3456')
        commit3.subject = 'Subject 2'

        patch1 = patchwork.Patch('1')
        patch1.subject = 'Subject 1'
        patch2 = patchwork.Patch('2')
        patch2.subject = 'Subject 2'
        patch3 = patchwork.Patch('3')
        patch3.subject = 'Subject 2'

        series = Series()
        series.commits = [commit1]
        patches = [patch1]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])

        series.commits = [commit1]
        patches = [patch1, patch2]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find commit for patch 2 ('Subject 2')"],
                         warnings)

        series.commits = [commit1, commit2]
        patches = [patch1]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find patch for commit 2 ('Subject 2')"],
                         warnings)

        series.commits = [commit1, commit2, commit3]
        patches = [patch1, patch2]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(2, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(patch2, patch_for_commit[1])
        self.assertEqual(1, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Cannot find patch for commit 3 ('Subject 2')",
                          "Multiple commits match patch 2 ('Subject 2'):\n"
                          '   Subject 2\n   Subject 2'],
                         warnings)

        series.commits = [commit1, commit2]
        patches = [patch1, patch2, patch3]
        patch_for_commit, commit_for_patch, warnings = (
            status.compare_with_series(series, patches))
        self.assertEqual(1, len(patch_for_commit))
        self.assertEqual(patch1, patch_for_commit[0])
        self.assertEqual(2, len(commit_for_patch))
        self.assertEqual(commit1, commit_for_patch[0])
        self.assertEqual(["Multiple patches match commit 2 ('Subject 2'):\n"
                          '   Subject 2\n   Subject 2',
                          "Cannot find commit for patch 3 ('Subject 2')"],
                         warnings)

    def _fake_patchwork2(self, subpath):
        """Fake Patchwork server for the function below

        This handles accessing series, patches and comments, providing the data
        in self.patches to the caller

        Args:
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        re_patch = re.match(r'patches/(\d*)/$', subpath)
        re_comments = re.match(r'patches/(\d*)/comments/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': self.patches}
        elif re_patch:
            patch_num = int(re_patch.group(1))
            patch = self.patches[patch_num - 1]
            return patch
        elif re_comments:
            patch_num = int(re_comments.group(1))
            patch = self.patches[patch_num - 1]
            return patch.comments
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_find_new_responses(self):
        """Test operation of find_new_responses()"""
        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'

        patch1 = patchwork.Patch('1')
        patch1.parse_subject('[1/2] Subject 1')
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'content': 'Reviewed-by: %s\n' % self.joe}

        patch1.comments = [comment1a]

        patch2 = patchwork.Patch('2')
        patch2.parse_subject('[2/2] Subject 2')
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {'content': 'Reviewed-by: %s' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up commits and patch for use by the fake
        # Rest API function _fake_patchwork2(). It calls various functions in
        # the status module after setting up tags in the commits, checking that
        # things behaves as expected
        self.commits = [commit1, commit2]
        self.patches = [patch1, patch2]

        # Check that the tags are picked up on the first patch
        new_rtags, _ = status.process_reviews(patch1.content, patch1.comments,
                                              commit1.rtags)
        self.assertEqual(new_rtags, {'Reviewed-by': {self.joe}})

        # Now the second patch
        new_rtags, _ = status.process_reviews(patch2.content, patch2.comments,
                                              commit2.rtags)
        self.assertEqual(new_rtags, {
            'Reviewed-by': {self.mary, self.fred},
            'Tested-by': {self.leb}})

        # Now add some tags to the commit, which means they should not appear as
        # 'new' tags when scanning comments
        commit1.rtags = {'Reviewed-by': {self.joe}}
        new_rtags, _ = status.process_reviews(patch1.content, patch1.comments,
                                              commit1.rtags)
        self.assertEqual(new_rtags, {})

        # For the second commit, add Ed and Fred, so only Mary should be left
        commit2.rtags = {
            'Tested-by': {self.leb},
            'Reviewed-by': {self.fred}}
        new_rtags, _ = status.process_reviews(patch2.content, patch2.comments,
                                              commit2.rtags)
        self.assertEqual(new_rtags, {'Reviewed-by': {self.mary}})

        # Check that the output patches expectations:
        #   1 Subject 1
        #     Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 Subject 2
        #     Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        #     Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        # 1 new response available in patchwork

        series = Series()
        series.commits = [commit1, commit2]
        terminal.set_print_test_mode()
        pwork = patchwork.Patchwork.for_testing(self._fake_patchwork2)
        status.check_and_show_status(series, '1234', None, None, False, False,
                                     False, pwork)
        itr = iter(terminal.get_print_test_lines())
        col = terminal.Color()
        self.assertEqual(terminal.PrintLine('  1 Subject 1', col.YELLOW),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('    Reviewed-by: ', col.GREEN, newline=False,
                               bright=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.joe, col.WHITE, bright=False),
                         next(itr))

        self.assertEqual(terminal.PrintLine('  2 Subject 2', col.YELLOW),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('    Reviewed-by: ', col.GREEN, newline=False,
                               bright=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.fred, col.WHITE,
                                            bright=False), next(itr))
        self.assertEqual(
            terminal.PrintLine('    Tested-by: ', col.GREEN, newline=False,
                               bright=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.leb, col.WHITE, bright=False),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.mary, col.WHITE),
                         next(itr))
        self.assertEqual(terminal.PrintLine(
            '1 new response available in patchwork (use -d to write them to a new branch)',
            None), next(itr))

    def _fake_patchwork3(self, subpath):
        """Fake Patchwork server for the function below

        This handles accessing series, patches and comments, providing the data
        in self.patches to the caller

        Args:
            subpath (str): URL subpath to use
        """
        re_series = re.match(r'series/(\d*)/$', subpath)
        re_patch = re.match(r'patches/(\d*)/$', subpath)
        re_comments = re.match(r'patches/(\d*)/comments/$', subpath)
        if re_series:
            series_num = re_series.group(1)
            if series_num == '1234':
                return {'patches': self.patches}
        elif re_patch:
            patch_num = int(re_patch.group(1))
            patch = self.patches[patch_num - 1]
            return patch
        elif re_comments:
            patch_num = int(re_comments.group(1))
            patch = self.patches[patch_num - 1]
            return patch.comments
        raise ValueError('Fake Patchwork does not understand: %s' % subpath)

    def test_create_branch(self):
        """Test operation of create_branch()"""
        repo = self.make_git_tree()
        branch = 'first'
        dest_branch = 'first2'
        count = 2
        gitdir = self.gitdir

        # Set up the test git tree. We use branch 'first' which has two commits
        # in it
        series = patchstream.get_metadata_for_list(branch, gitdir, count)
        self.assertEqual(2, len(series.commits))

        patch1 = patchwork.Patch('1')
        patch1.parse_subject('[1/2] %s' % series.commits[0].subject)
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'content': 'Reviewed-by: %s\n' % self.joe}

        patch1.comments = [comment1a]

        patch2 = patchwork.Patch('2')
        patch2.parse_subject('[2/2] %s' % series.commits[1].subject)
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {
            'content': 'Reviewed-by: %s' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up patches for use by the fake Rest API
        # function _fake_patchwork3(). The fake patch comments above should
        # result in new review tags that are collected and added to the commits
        # created in the destination branch.
        self.patches = [patch1, patch2]
        count = 2

        # Expected output:
        #   1 i2c: I2C things
        #   + Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 spi: SPI fixes
        #   + Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        #   + Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        # 4 new responses available in patchwork
        # 4 responses added from patchwork into new branch 'first2'
        # <unittest.result.TestResult run=8 errors=0 failures=0>

        terminal.set_print_test_mode()
        pwork = patchwork.Patchwork.for_testing(self._fake_patchwork3)
        status.check_and_show_status(
            series, '1234', branch, dest_branch, False, False, False, pwork,
            repo)
        lines = terminal.get_print_test_lines()
        self.assertEqual(12, len(lines))
        self.assertEqual(
            "4 responses added from patchwork into new branch 'first2'",
            lines[11].text)

        # Check that the destination branch has the new tags
        new_series = patchstream.get_metadata_for_list(dest_branch, gitdir,
                                                       count)
        self.assertEqual(
            {'Reviewed-by': {self.joe}},
            new_series.commits[0].rtags)
        self.assertEqual(
            {'Tested-by': {self.leb},
             'Reviewed-by': {self.fred, self.mary}},
            new_series.commits[1].rtags)

        # Now check the actual test of the first commit message. We expect to
        # see the new tags immediately below the old ones.
        stdout = patchstream.get_list(dest_branch, count=count, git_dir=gitdir)
        itr = iter([line.strip() for line in stdout.splitlines()
                    if '-by:' in line])

        # First patch should have the review tag
        self.assertEqual('Reviewed-by: %s' % self.joe, next(itr))

        # Second patch should have the sign-off then the tested-by and two
        # reviewed-by tags
        self.assertEqual('Signed-off-by: %s' % self.leb, next(itr))
        self.assertEqual('Reviewed-by: %s' % self.fred, next(itr))
        self.assertEqual('Reviewed-by: %s' % self.mary, next(itr))
        self.assertEqual('Tested-by: %s' % self.leb, next(itr))

    def test_parse_snippets(self):
        """Test parsing of review snippets"""
        text = '''Hi Fred,

This is a comment from someone.

Something else

On some recent date, Fred wrote:
> This is why I wrote the patch
> so here it is

Now a comment about the commit message
A little more to say

Even more

> diff --git a/file.c b/file.c
> Some more code
> Code line 2
> Code line 3
> Code line 4
> Code line 5
> Code line 6
> Code line 7
> Code line 8
> Code line 9

And another comment

> @@ -153,8 +143,13 @@ def check_patch(fname, show_types=False):
>  further down on the file
>  and more code
> +Addition here
> +Another addition here
>  codey
>  more codey

and another thing in same file

> @@ -253,8 +243,13 @@
>  with no function context

one more thing

> diff --git a/tools/patman/main.py b/tools/patman/main.py
> +line of code
now a very long comment in a different file
line2
line3
line4
line5
line6
line7
line8
'''
        pstrm = PatchStream.process_text(text, True)
        self.assertEqual([], pstrm.commit.warn)

        # We expect to the filename and up to 5 lines of code context before
        # each comment. The 'On xxx wrote:' bit should be removed.
        self.assertEqual(
            [['Hi Fred,',
              'This is a comment from someone.',
              'Something else'],
             ['> This is why I wrote the patch',
              '> so here it is',
              'Now a comment about the commit message',
              'A little more to say', 'Even more'],
             ['> File: file.c', '> Code line 5', '> Code line 6',
              '> Code line 7', '> Code line 8', '> Code line 9',
              'And another comment'],
             ['> File: file.c',
              '> Line: 153 / 143: def check_patch(fname, show_types=False):',
              '>  and more code', '> +Addition here',
              '> +Another addition here', '>  codey', '>  more codey',
              'and another thing in same file'],
             ['> File: file.c', '> Line: 253 / 243',
              '>  with no function context', 'one more thing'],
             ['> File: tools/patman/main.py', '> +line of code',
              'now a very long comment in a different file',
              'line2', 'line3', 'line4', 'line5', 'line6', 'line7', 'line8']],
            pstrm.snippets)

    def test_review_snippets(self):
        """Test showing of review snippets"""
        def _to_submitter(who):
            m_who = re.match('(.*) <(.*)>', who)
            return {
                'name': m_who.group(1),
                'email': m_who.group(2)
                }

        commit1 = Commit('abcd')
        commit1.subject = 'Subject 1'
        commit2 = Commit('ef12')
        commit2.subject = 'Subject 2'

        patch1 = patchwork.Patch('1')
        patch1.parse_subject('[1/2] Subject 1')
        patch1.name = patch1.raw_subject
        patch1.content = 'This is my patch content'
        comment1a = {'submitter': _to_submitter(self.joe),
                     'content': '''Hi Fred,

On some date Fred wrote:

> diff --git a/file.c b/file.c
> Some code
> and more code

Here is my comment above the above...


Reviewed-by: %s
''' % self.joe}

        patch1.comments = [comment1a]

        patch2 = patchwork.Patch('2')
        patch2.parse_subject('[2/2] Subject 2')
        patch2.name = patch2.raw_subject
        patch2.content = 'Some other patch content'
        comment2a = {
            'content': 'Reviewed-by: %s\nTested-by: %s\n' %
                       (self.mary, self.leb)}
        comment2b = {'submitter': _to_submitter(self.fred),
                     'content': '''Hi Fred,

On some date Fred wrote:

> diff --git a/tools/patman/commit.py b/tools/patman/commit.py
> @@ -41,6 +41,9 @@ class Commit:
>          self.rtags = collections.defaultdict(set)
>          self.warn = []
>
> +    def __str__(self):
> +        return self.subject
> +
>      def add_change(self, version, info):
>          """Add a new change line to the change list for a version.
>
A comment

Reviewed-by: %s
''' % self.fred}
        patch2.comments = [comment2a, comment2b]

        # This test works by setting up commits and patch for use by the fake
        # Rest API function _fake_patchwork2(). It calls various functions in
        # the status module after setting up tags in the commits, checking that
        # things behaves as expected
        self.commits = [commit1, commit2]
        self.patches = [patch1, patch2]

        # Check that the output patches expectations:
        #   1 Subject 1
        #     Reviewed-by: Joe Bloggs <joe@napierwallies.co.nz>
        #   2 Subject 2
        #     Tested-by: Lord Edmund Blackaddër <weasel@blackadder.org>
        #     Reviewed-by: Fred Bloggs <f.bloggs@napier.net>
        #   + Reviewed-by: Mary Bloggs <mary@napierwallies.co.nz>
        # 1 new response available in patchwork

        series = Series()
        series.commits = [commit1, commit2]
        terminal.set_print_test_mode()
        pwork = patchwork.Patchwork.for_testing(self._fake_patchwork2)
        status.check_and_show_status(
            series, '1234', None, None, False, True, False, pwork)
        itr = iter(terminal.get_print_test_lines())
        col = terminal.Color()
        self.assertEqual(terminal.PrintLine('  1 Subject 1', col.YELLOW),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.joe, col.WHITE), next(itr))

        self.assertEqual(terminal.PrintLine('Review: %s' % self.joe, col.RED),
                         next(itr))
        self.assertEqual(terminal.PrintLine('    Hi Fred,', None), next(itr))
        self.assertEqual(terminal.PrintLine('', None), next(itr))
        self.assertEqual(terminal.PrintLine('    > File: file.c', col.MAGENTA),
                         next(itr))
        self.assertEqual(terminal.PrintLine('    > Some code', col.MAGENTA),
                         next(itr))
        self.assertEqual(terminal.PrintLine('    > and more code',
                                            col.MAGENTA),
                         next(itr))
        self.assertEqual(terminal.PrintLine(
            '    Here is my comment above the above...', None), next(itr))
        self.assertEqual(terminal.PrintLine('', None), next(itr))

        self.assertEqual(terminal.PrintLine('  2 Subject 2', col.YELLOW),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.fred, col.WHITE),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('  + Reviewed-by: ', col.GREEN, newline=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.mary, col.WHITE),
                         next(itr))
        self.assertEqual(
            terminal.PrintLine('  + Tested-by: ', col.GREEN, newline=False),
            next(itr))
        self.assertEqual(terminal.PrintLine(self.leb, col.WHITE),
                         next(itr))

        self.assertEqual(terminal.PrintLine('Review: %s' % self.fred, col.RED),
                         next(itr))
        self.assertEqual(terminal.PrintLine('    Hi Fred,', None), next(itr))
        self.assertEqual(terminal.PrintLine('', None), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    > File: tools/patman/commit.py', col.MAGENTA), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    > Line: 41 / 41: class Commit:', col.MAGENTA), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    > +        return self.subject', col.MAGENTA), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    > +', col.MAGENTA), next(itr))
        self.assertEqual(
            terminal.PrintLine(
                '    >      def add_change(self, version, info):',
                col.MAGENTA),
            next(itr))
        self.assertEqual(terminal.PrintLine(
            '    >          """Add a new change line to the change list for a version.',
            col.MAGENTA), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    >', col.MAGENTA), next(itr))
        self.assertEqual(terminal.PrintLine(
            '    A comment', None), next(itr))
        self.assertEqual(terminal.PrintLine('', None), next(itr))

        self.assertEqual(terminal.PrintLine(
            '4 new responses available in patchwork (use -d to write them to a new branch)',
            None), next(itr))

    def test_insert_tags(self):
        """Test inserting of review tags"""
        msg = '''first line
second line.'''
        tags = [
            'Reviewed-by: Bin Meng <bmeng.cn@gmail.com>',
            'Tested-by: Bin Meng <bmeng.cn@gmail.com>'
            ]
        signoff = 'Signed-off-by: Simon Glass <sjg@chromium.com>'
        tag_str = '\n'.join(tags)

        new_msg = patchstream.insert_tags(msg, tags)
        self.assertEqual(msg + '\n\n' + tag_str, new_msg)

        new_msg = patchstream.insert_tags(msg + '\n', tags)
        self.assertEqual(msg + '\n\n' + tag_str, new_msg)

        msg += '\n\n' + signoff
        new_msg = patchstream.insert_tags(msg, tags)
        self.assertEqual(msg + '\n' + tag_str, new_msg)

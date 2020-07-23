# -*- coding: utf-8 -*-
# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2017 Google, Inc
#

import contextlib
import os
import re
import shutil
import sys
import tempfile
import unittest

from io import StringIO

from patman import control
from patman import gitutil
from patman import patchstream
from patman import settings
from patman import terminal
from patman import tools
from patman.test_util import capture_sys_output

try:
    import pygit2
    HAVE_PYGIT2= True
except ModuleNotFoundError:
    HAVE_PYGIT2 = False


@contextlib.contextmanager
def capture():
    oldout,olderr = sys.stdout, sys.stderr
    try:
        out=[StringIO(), StringIO()]
        sys.stdout,sys.stderr = out
        yield out
    finally:
        sys.stdout,sys.stderr = oldout, olderr
        out[0] = out[0].getvalue()
        out[1] = out[1].getvalue()


class TestFunctional(unittest.TestCase):
    def setUp(self):
        self.tmpdir = tempfile.mkdtemp(prefix='patman.')
        self.gitdir = os.path.join(self.tmpdir, 'git')
        self.repo = None

    def tearDown(self):
        shutil.rmtree(self.tmpdir)

    @staticmethod
    def GetPath(fname):
        return os.path.join(os.path.dirname(os.path.realpath(sys.argv[0])),
                            'test', fname)

    @classmethod
    def GetText(self, fname):
        return open(self.GetPath(fname), encoding='utf-8').read()

    @classmethod
    def GetPatchName(self, subject):
        fname = re.sub('[ :]', '-', subject)
        return fname.replace('--', '-')

    def CreatePatchesForTest(self, series):
        cover_fname = None
        fname_list = []
        for i, commit in enumerate(series.commits):
            clean_subject = self.GetPatchName(commit.subject)
            src_fname = '%04d-%s.patch' % (i + 1, clean_subject[:52])
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self.GetPath(src_fname), fname)
            fname_list.append(fname)
        if series.get('cover'):
            src_fname = '0000-cover-letter.patch'
            cover_fname = os.path.join(self.tmpdir, src_fname)
            fname = os.path.join(self.tmpdir, src_fname)
            shutil.copy(self.GetPath(src_fname), fname)

        return cover_fname, fname_list

    def testBasic(self):
        """Tests the basic flow of patman

        This creates a series from some hard-coded patches build from a simple
        tree with the following metadata in the top commit:

            Series-to: u-boot
            Series-prefix: RFC
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
            - Series-to, Series-cc, Series-prefix, Cover-letter
            - Cover-letter-cc, Series-version, Series-changes, Series-notes
            - Commit-notes
        """
        process_tags = True
        ignore_bad_tags = True
        stefan = b'Stefan Br\xc3\xbcns <stefan.bruens@rwth-aachen.de>'.decode('utf-8')
        rick = 'Richard III <richard@palace.gov>'
        mel = b'Lord M\xc3\xablchett <clergy@palace.gov>'.decode('utf-8')
        ed = b'Lond Edmund Blackadd\xc3\xabr <weasel@blackadder.org'.decode('utf-8')
        fred = 'Fred Bloggs <f.bloggs@napier.net>'
        add_maintainers = [stefan, rick]
        dry_run = True
        in_reply_to = mel
        count = 2
        settings.alias = {
                'fdt': ['simon'],
                'u-boot': ['u-boot@lists.denx.de'],
                'simon': [ed],
                'fred': [fred],
        }

        text = self.GetText('test01.txt')
        series = patchstream.GetMetaDataForTest(text)
        cover_fname, args = self.CreatePatchesForTest(series)
        with capture() as out:
            patchstream.FixPatches(series, args)
            if cover_fname and series.get('cover'):
                patchstream.InsertCoverLetter(cover_fname, series, count)
            series.DoChecks()
            cc_file = series.MakeCcFile(process_tags, cover_fname,
                                        not ignore_bad_tags, add_maintainers,
                                        None)
            cmd = gitutil.EmailPatches(series, cover_fname, args,
                    dry_run, not ignore_bad_tags, cc_file,
                    in_reply_to=in_reply_to, thread=None)
            series.ShowActions(args, cmd, process_tags)
        cc_lines = open(cc_file, encoding='utf-8').read().splitlines()
        os.remove(cc_file)

        lines = out[0].splitlines()
        self.assertEqual('Cleaned %s patches' % len(series.commits), lines[0])
        self.assertEqual('Change log missing for v2', lines[1])
        self.assertEqual('Change log missing for v3', lines[2])
        self.assertEqual('Change log for unknown version v4', lines[3])
        self.assertEqual("Alias 'pci' not found", lines[4])
        self.assertIn('Dry run', lines[5])
        self.assertIn('Send a total of %d patches' % count, lines[7])
        line = 8
        for i, commit in enumerate(series.commits):
            self.assertEqual('   %s' % args[i], lines[line + 0])
            line += 1
            while 'Cc:' in lines[line]:
                line += 1
        self.assertEqual('To:	  u-boot@lists.denx.de', lines[line])
        self.assertEqual('Cc:	  %s' % tools.FromUnicode(stefan),
                         lines[line + 1])
        self.assertEqual('Version:  3', lines[line + 2])
        self.assertEqual('Prefix:\t  RFC', lines[line + 3])
        self.assertEqual('Cover: 4 lines', lines[line + 4])
        line += 5
        self.assertEqual('      Cc:  %s' % fred, lines[line + 0])
        self.assertEqual('      Cc:  %s' % tools.FromUnicode(ed),
                         lines[line + 1])
        self.assertEqual('      Cc:  %s' % tools.FromUnicode(mel),
                         lines[line + 2])
        self.assertEqual('      Cc:  %s' % rick, lines[line + 3])
        expected = ('Git command: git send-email --annotate '
                    '--in-reply-to="%s" --to "u-boot@lists.denx.de" '
                    '--cc "%s" --cc-cmd "%s --cc-cmd %s" %s %s'
                    % (in_reply_to, stefan, sys.argv[0], cc_file, cover_fname,
                       ' '.join(args)))
        line += 4
        self.assertEqual(expected, tools.ToUnicode(lines[line]))

        self.assertEqual(('%s %s\0%s' % (args[0], rick, stefan)),
                         tools.ToUnicode(cc_lines[0]))
        self.assertEqual(('%s %s\0%s\0%s\0%s' % (args[1], fred, ed, rick,
                                     stefan)), tools.ToUnicode(cc_lines[1]))

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

'''
        lines = open(cover_fname, encoding='utf-8').read().splitlines()
        self.assertEqual(
                'Subject: [RFC PATCH v3 0/2] test: A test patch series',
                lines[3])
        self.assertEqual(expected.splitlines(), lines[7:])

        for i, fname in enumerate(args):
            lines = open(fname, encoding='utf-8').read().splitlines()
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
- Some changes

Changes in v2:
- Changes only for this commit'''

            if expected:
                expected = expected.splitlines()
                self.assertEqual(expected, lines[start:(start+len(expected))])

    def make_commit_with_file(self, subject, body, fname, text):
        """Create a file and add it to the git repo with a new commit

        Args:
            subject (str): Subject for the commit
            body (str): Body text of the commit
            fname (str): Filename of file to create
            text (str): Text to put into the file
        """
        path = os.path.join(self.gitdir, fname)
        tools.WriteFile(path, text, binary=False)
        index = self.repo.index
        index.add(fname)
        author =  pygit2.Signature('Test user', 'test@email.com')
        committer = author
        tree = index.write_tree()
        message = subject + '\n' + body
        self.repo.create_commit('HEAD', author, committer, message, tree,
                                [self.repo.head.target])

    def make_git_tree(self):
        """Make a simple git tree suitable for testing

        It has three branches:
            'base' has two commits: PCI, main
            'first' has base as upstream and two more commits: I2C, SPI
            'second' has base as upstream and three more: video, serial, bootm

        Returns:
            pygit2 repository
        """
        repo = pygit2.init_repository(self.gitdir)
        self.repo = repo
        new_tree = repo.TreeBuilder().write()

        author = pygit2.Signature('Test user', 'test@email.com')
        committer = author
        commit = repo.create_commit('HEAD', author, committer,
                                         'Created master', new_tree, [])

        self.make_commit_with_file('Initial commit', '''
Add a README

''', 'README', '''This is the README file
describing this project
in very little detail''')

        self.make_commit_with_file('pci: PCI implementation', '''
Here is a basic PCI implementation

''', 'pci.c', '''This is a file
it has some contents
and some more things''')
        self.make_commit_with_file('main: Main program', '''
Hello here is the second commit.
''', 'main.c', '''This is the main file
there is very little here
but we can always add more later
if we want to

Series-to: u-boot
Series-cc: Barry Crump <bcrump@whataroa.nz>
''')
        base_target = repo.revparse_single('HEAD')
        self.make_commit_with_file('i2c: I2C things', '''
This has some stuff to do with I2C
''', 'i2c.c', '''And this is the file contents
with some I2C-related things in it''')
        self.make_commit_with_file('spi: SPI fixes', '''
SPI needs some fixes
and here they are
''', 'spi.c', '''Some fixes for SPI in this
file to make SPI work
better than before''')
        first_target = repo.revparse_single('HEAD')

        target = repo.revparse_single('HEAD~2')
        repo.reset(target.oid, pygit2.GIT_CHECKOUT_FORCE)
        self.make_commit_with_file('video: Some video improvements', '''
Fix up the video so that
it looks more purple. Purple is
a very nice colour.
''', 'video.c', '''More purple here
Purple and purple
Even more purple
Could not be any more purple''')
        self.make_commit_with_file('serial: Add a serial driver', '''
Here is the serial driver
for my chip.

Cover-letter:
Series for my board
This series implements support
for my glorious board.
END
''', 'serial.c', '''The code for the
serial driver is here''')
        self.make_commit_with_file('bootm: Make it boot', '''
This makes my board boot
with a fix to the bootm
command
''', 'bootm.c', '''Fix up the bootm
command to make the code as
complicated as possible''')
        second_target = repo.revparse_single('HEAD')

        repo.branches.local.create('first', first_target)
        repo.config.set_multivar('branch.first.remote', '', '.')
        repo.config.set_multivar('branch.first.merge', '', 'refs/heads/base')

        repo.branches.local.create('second', second_target)
        repo.config.set_multivar('branch.second.remote', '', '.')
        repo.config.set_multivar('branch.second.merge', '', 'refs/heads/base')

        repo.branches.local.create('base', base_target)
        return repo

    @unittest.skipIf(not HAVE_PYGIT2, 'Missing python3-pygit2')
    def testBranch(self):
        """Test creating patches from a branch"""
        repo = self.make_git_tree()
        target = repo.lookup_reference('refs/heads/first')
        self.repo.checkout(target, strategy=pygit2.GIT_CHECKOUT_FORCE)
        control.setup()
        try:
            orig_dir = os.getcwd()
            os.chdir(self.gitdir)

            # Check that it can detect the current branch
            self.assertEqual(2, gitutil.CountCommitsToBranch(None))
            col = terminal.Color()
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch=None, count=-1, start=0, end=0,
                    ignore_binary=False)
            self.assertIsNone(cover_fname)
            self.assertEqual(2, len(patch_files))

            # Check that it can detect a different branch
            self.assertEqual(3, gitutil.CountCommitsToBranch('second'))
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch='second', count=-1, start=0, end=0,
                    ignore_binary=False)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(3, len(patch_files))

            # Check that it can skip patches at the end
            with capture_sys_output() as _:
                _, cover_fname, patch_files = control.prepare_patches(
                    col, branch='second', count=-1, start=0, end=1,
                    ignore_binary=False)
            self.assertIsNotNone(cover_fname)
            self.assertEqual(2, len(patch_files))
        finally:
            os.chdir(orig_dir)

# SPDX-License-Identifier: GPL-2.0+

# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Functional tests for checking that patman behaves correctly"""

import asyncio
from datetime import datetime
import os
import re
import unittest
from unittest import mock

import pygit2

from u_boot_pylib import cros_subprocess
from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tools
from patman import cmdline
from patman import control
from patman import cser_helper
from patman import cseries
from patman.database import Pcommit
from patman import database
from patman import patchstream
from patman.patchwork import Patchwork
from patman.test_common import TestCommon

HASH_RE = r'[0-9a-f]+'
#pylint: disable=protected-access

class Namespace:
    """Simple namespace for use instead of argparse in tests"""
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)


class TestCseries(unittest.TestCase, TestCommon):
    """Test cases for the Cseries class

    In some cases there are tests for both direct Cseries calls and for
    accessing the feature via the cmdline. It is possible to do this with mocks
    but it is a bit painful to catch all cases that way. The approach here is
    to create a check_...() function which yields back to the test routines to
    make the call or run the command. The check_...() function typically yields
    a Cseries while it is working and False when it is done, allowing the test
    to check that everything is finished.

    Some subcommands don't have command tests, if it would be duplicative. Some
    tests avoid using the check_...() function and just write the test out
    twice, if it would be too confusing to use a coroutine.

    Note the -N flag which sort-of disables capturing of output, although in
    fact it is still captured, just output at the end. When debugging the code
    you may need to temporarily comment out the 'with terminal.capture()'
    parts.
    """
    def setUp(self):
        TestCommon.setUp(self)
        self.autolink_extra = None
        self.loop = asyncio.get_event_loop()
        self.cser = None

    def tearDown(self):
        TestCommon.tearDown(self)

    class _Stage:
        def __init__(self, name):
            self.name = name

        def __enter__(self):
            if not terminal.USE_CAPTURE:
                print(f"--- starting '{self.name}'")

        def __exit__(self, exc_type, exc_val, exc_tb):
            if not terminal.USE_CAPTURE:
                print(f"--- finished '{self.name}'\n")

    def stage(self, name):
        """Context manager to count requests across a range of patchwork calls

        Args:
            name (str): Stage name

        Return:
            _Stage: contect object

        Usage:
            with self.stage('name'):
                ...do things

            Note that the output only appears if the -N flag is used
        """
        return self._Stage(name)

    def assert_finished(self, itr):
        """Assert that an iterator is finished

        Args:
            itr (iter): Iterator to check
        """
        self.assertFalse(list(itr))

    def test_database_setup(self):
        """Check setting up of the series database"""
        cser = cseries.Cseries(self.tmpdir)
        with terminal.capture() as (_, err):
            cser.open_database()
        self.assertEqual(f'Creating new database {self.tmpdir}/.patman.db',
                         err.getvalue().strip())
        res = cser.db.execute("SELECT name FROM series")
        self.assertTrue(res)
        cser.close_database()

    def get_database(self):
        """Open the database and silence the warning output

        Return:
            Cseries: Resulting Cseries object
        """
        cser = cseries.Cseries(self.tmpdir, terminal.COLOR_NEVER)
        with terminal.capture() as _:
            cser.open_database()
        self.cser = cser
        return cser

    def get_cser(self):
        """Set up a git tree and database

        Return:
            Cseries: object
        """
        self.make_git_tree()
        return self.get_database()

    def db_close(self):
        """Close the database if open"""
        if self.cser and self.cser.db.cur:
            self.cser.close_database()
            return True
        return False

    def db_open(self):
        """Open the database if closed"""
        if self.cser and not self.cser.db.cur:
            self.cser.open_database()

    def run_args(self, *argv, expect_ret=0, pwork=None, cser=None):
        """Run patman with the given arguments

        Args:
            argv (list of str): List of arguments, excluding 'patman'
            expect_ret (int): Expected return code, used to check errors
            pwork (Patchwork): Patchwork object to use when executing the
                command, or None to create one
            cser (Cseries): Cseries object to use when executing the command,
                or None to create one
        """
        was_open = self.db_close()
        args = cmdline.parse_args(['-D'] + list(argv), config_fname=False)
        exit_code = control.do_patman(args, self.tmpdir, pwork, cser)
        self.assertEqual(expect_ret, exit_code)
        if was_open:
            self.db_open()

    def test_series_add(self):
        """Test adding a new cseries"""
        cser = self.get_cser()
        self.assertFalse(cser.db.series_get_dict())

        with terminal.capture() as (out, _):
            cser.add('first', 'my description', allow_unmarked=True)
        lines = out.getvalue().strip().splitlines()
        self.assertEqual(
            "Adding series 'first' v1: mark False allow_unmarked True",
            lines[0])
        self.assertEqual("Added series 'first' v1 (2 commits)", lines[1])
        self.assertEqual(2, len(lines))

        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        self.assertEqual('first', slist['first'].name)
        self.assertEqual('my description', slist['first'].desc)

        svlist = cser.get_ser_ver_list()
        self.assertEqual(1, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(1, svlist[0].version)

        pclist = cser.get_pcommit_dict()
        self.assertEqual(2, len(pclist))
        self.assertIn(1, pclist)
        self.assertEqual(
            Pcommit(1, 0, 'i2c: I2C things', 1, None, None, None, None),
            pclist[1])
        self.assertEqual(
            Pcommit(2, 1, 'spi: SPI fixes', 1, None, None, None, None),
            pclist[2])

    def test_series_not_checked_out(self):
        """Test adding a new cseries when a different one is checked out"""
        cser = self.get_cser()
        self.assertFalse(cser.db.series_get_dict())

        with terminal.capture() as (out, _):
            cser.add('second', allow_unmarked=True)
        lines = out.getvalue().strip().splitlines()
        self.assertEqual(
            "Adding series 'second' v1: mark False allow_unmarked True",
            lines[0])
        self.assertEqual("Added series 'second' v1 (3 commits)", lines[1])
        self.assertEqual(2, len(lines))

    def test_series_add_manual(self):
        """Test adding a new cseries with a version number"""
        cser = self.get_cser()
        self.assertFalse(cser.db.series_get_dict())

        repo = pygit2.init_repository(self.gitdir)
        first_target = repo.revparse_single('first')
        repo.branches.local.create('first2', first_target)
        repo.config.set_multivar('branch.first2.remote', '', '.')
        repo.config.set_multivar('branch.first2.merge', '', 'refs/heads/base')

        with terminal.capture() as (out, _):
            cser.add('first2', 'description', allow_unmarked=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Adding series 'first' v2: mark False allow_unmarked True",
            lines[0])
        self.assertEqual("Added series 'first' v2 (2 commits)", lines[1])
        self.assertEqual(2, len(lines))

        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        self.assertEqual('first', slist['first'].name)

        # We should have just one entry, with version 2
        svlist = cser.get_ser_ver_list()
        self.assertEqual(1, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(2, svlist[0].version)

    def add_first2(self, checkout):
        """Add a new first2 branch, a copy of first"""
        repo = pygit2.init_repository(self.gitdir)
        first_target = repo.revparse_single('first')
        repo.branches.local.create('first2', first_target)
        repo.config.set_multivar('branch.first2.remote', '', '.')
        repo.config.set_multivar('branch.first2.merge', '', 'refs/heads/base')

        if checkout:
            target = repo.lookup_reference('refs/heads/first2')
            repo.checkout(target, strategy=pygit2.enums.CheckoutStrategy.FORCE)

    def test_series_add_different(self):
        """Test adding a different version of a series from that checked out"""
        cser = self.get_cser()

        self.add_first2(True)

        # Add first2 initially
        with terminal.capture() as (out, _):
            cser.add(None, 'description', allow_unmarked=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Adding series 'first' v2: mark False allow_unmarked True",
            lines[0])
        self.assertEqual("Added series 'first' v2 (2 commits)", lines[1])
        self.assertEqual(2, len(lines))

        # Now add first: it should be added as a new version
        with terminal.capture() as (out, _):
            cser.add('first', 'description', allow_unmarked=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Adding series 'first' v1: mark False allow_unmarked True",
            lines[0])
        self.assertEqual(
            "Added v1 to existing series 'first' (2 commits)", lines[1])
        self.assertEqual(2, len(lines))

        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        self.assertEqual('first', slist['first'].name)

        # We should have two entries, one of each version
        svlist = cser.get_ser_ver_list()
        self.assertEqual(2, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(2, svlist[0].version)

        self.assertEqual(2, svlist[1].idnum)
        self.assertEqual(1, svlist[1].series_id)
        self.assertEqual(1, svlist[1].version)

    def test_series_add_dup(self):
        """Test adding a series twice"""
        cser = self.get_cser()
        with terminal.capture() as (out, _):
            cser.add(None, 'description', allow_unmarked=True)

        with terminal.capture() as (out, _):
            cser.add(None, 'description', allow_unmarked=True)
        self.assertIn("Series 'first' v1 already exists",
                      out.getvalue().strip())

        self.add_first2(False)

        with terminal.capture() as (out, _):
            cser.add('first2', 'description', allow_unmarked=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Added v2 to existing series 'first' (2 commits)", lines[1])

    def test_series_add_dup_reverse(self):
        """Test adding a series twice, v2 then v1"""
        cser = self.get_cser()
        self.add_first2(True)
        with terminal.capture() as (out, _):
            cser.add(None, 'description', allow_unmarked=True)
        self.assertIn("Added series 'first' v2", out.getvalue().strip())

        with terminal.capture() as (out, _):
            cser.add('first', 'description', allow_unmarked=True)
        self.assertIn("Added v1 to existing series 'first'",
                      out.getvalue().strip())

    def test_series_add_dup_reverse_cmdline(self):
        """Test adding a series twice, v2 then v1"""
        cser = self.get_cser()
        self.add_first2(True)
        with terminal.capture() as (out, _):
            self.run_args('series', 'add', '-M', '-D', 'description',
                          pwork=True)
        self.assertIn("Added series 'first' v2 (2 commits)",
                      out.getvalue().strip())

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', 'add', '-M',
                          '-D', 'description', pwork=True)
            cser.add('first', 'description', allow_unmarked=True)
        self.assertIn("Added v1 to existing series 'first'",
                      out.getvalue().strip())

    def test_series_add_skip_version(self):
        """Test adding a series which is v4 but has no earlier version"""
        cser = self.get_cser()
        with terminal.capture() as (out, _):
            cser.add('third4', 'The glorious third series', mark=False,
                     allow_unmarked=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Adding series 'third' v4: mark False allow_unmarked True",
            lines[0])
        self.assertEqual("Added series 'third' v4 (4 commits)", lines[1])
        self.assertEqual(2, len(lines))

        sdict = cser.db.series_get_dict()
        self.assertIn('third', sdict)
        chk = sdict['third']
        self.assertEqual('third', chk['name'])
        self.assertEqual('The glorious third series', chk['desc'])

        svid = cser.get_series_svid(chk['idnum'], 4)
        self.assertEqual(4, len(cser.get_pcommit_dict(svid)))

        # Remove the series and add it again with just two commits
        with terminal.capture():
            cser.remove('third4')

        with terminal.capture() as (out, _):
            cser.add('third4', 'The glorious third series', mark=False,
                     allow_unmarked=True, end='third4~2')
        lines = out.getvalue().splitlines()
        self.assertEqual(
            "Adding series 'third' v4: mark False allow_unmarked True",
            lines[0])
        self.assertRegex(
            lines[1],
            'Ending before .* main: Change to the main program')
        self.assertEqual("Added series 'third' v4 (2 commits)", lines[2])

        sdict = cser.db.series_get_dict()
        self.assertIn('third', sdict)
        chk = sdict['third']
        self.assertEqual('third', chk['name'])
        self.assertEqual('The glorious third series', chk['desc'])

        svid = cser.get_series_svid(chk['idnum'], 4)
        self.assertEqual(2, len(cser.get_pcommit_dict(svid)))

    def test_series_add_wrong_version(self):
        """Test adding a series with an incorrect branch name or version

        This updates branch 'first' to have version 2, then tries to add it.
        """
        cser = self.get_cser()
        self.assertFalse(cser.db.series_get_dict())

        with terminal.capture():
            _, ser, max_vers, _ = cser.prep_series('first')
            cser.update_series('first', ser, max_vers, None, False,
                                add_vers=2)

        with self.assertRaises(ValueError) as exc:
            with terminal.capture():
                cser.add('first', 'my description', allow_unmarked=True)
        self.assertEqual(
            "Series name 'first' suggests version 1 but Series-version tag "
            'indicates 2 (see --force-version)', str(exc.exception))

        # Now try again with --force-version which should force version 1
        with terminal.capture() as (out, _):
            cser.add('first', 'my description', allow_unmarked=True,
                     force_version=True)
        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            "Adding series 'first' v1: mark False allow_unmarked True",
            next(itr))
        self.assertRegex(
            next(itr), 'Checking out upstream commit refs/heads/base: .*')
        self.assertEqual(
            "Processing 2 commits from branch 'first'", next(itr))
        self.assertRegex(next(itr),
                         f'-        {HASH_RE} as {HASH_RE} i2c: I2C things')
        self.assertRegex(next(itr),
                         f'- rm v1: {HASH_RE} as {HASH_RE} spi: SPI fixes')
        self.assertRegex(next(itr),
                         f'Updating branch first from {HASH_RE} to {HASH_RE}')
        self.assertEqual("Added series 'first' v1 (2 commits)", next(itr))
        try:
            self.assertEqual('extra line', next(itr))
        except StopIteration:
            pass

        # Since this is v1 the Series-version tag should have been removed
        series = patchstream.get_metadata('first', 0, 2, git_dir=self.gitdir)
        self.assertNotIn('version', series)

    def _fake_patchwork_cser(self, subpath):
        """Fake Patchwork server for the function below

        This handles accessing various things used by the tests below. It has
        hard-coded data, about from self.autolink_extra which can be adjusted
        by the test.

        Args:
            subpath (str): URL subpath to use
        """
        # Get a list of projects
        if subpath == 'projects/':
            return [
                {'id': self.PROJ_ID, 'name': 'U-Boot',
                 'link_name': self.PROJ_LINK_NAME},
                {'id': 9, 'name': 'other', 'link_name': 'other'}
            ]

        # Search for series by their cover-letter name
        re_search = re.match(r'series/\?project=(\d+)&q=.*$', subpath)
        if re_search:
            result = [
                {'id': 56, 'name': 'contains first name', 'version': 1},
                {'id': 43, 'name': 'has first in it', 'version': 1},
                {'id': 1234, 'name': 'first series', 'version': 1},
                {'id': self.SERIES_ID_SECOND_V1, 'name': self.TITLE_SECOND,
                 'version': 1},
                {'id': self.SERIES_ID_SECOND_V2, 'name': self.TITLE_SECOND,
                 'version': 2},
                {'id': 12345, 'name': 'i2c: I2C things', 'version': 1},
            ]
            if self.autolink_extra:
                result += [self.autolink_extra]
            return result

        # Read information about a series, given its link (patchwork series ID)
        m_series = re.match(r'series/(\d+)/$', subpath)
        series_id = int(m_series.group(1)) if m_series else ''
        if series_id:
            if series_id == self.SERIES_ID_SECOND_V1:
                # series 'second'
                return {
                    'patches': [
                        {'id': '10',
                         'name': '[PATCH,1/3] video: Some video improvements',
                         'content': ''},
                        {'id': '11',
                         'name': '[PATCH,2/3] serial: Add a serial driver',
                         'content': ''},
                        {'id': '12', 'name': '[PATCH,3/3] bootm: Make it boot',
                         'content': ''},
                    ],
                    'cover_letter': {
                        'id': 39,
                        'name': 'The name of the cover letter',
                    }
                }
            if series_id == self.SERIES_ID_SECOND_V2:
                # series 'second2'
                return {
                    'patches': [
                        {'id': '110',
                         'name':
                             '[PATCH,v2,1/3] video: Some video improvements',
                         'content': ''},
                        {'id': '111',
                         'name': '[PATCH,v2,2/3] serial: Add a serial driver',
                         'content': ''},
                        {'id': '112',
                         'name': '[PATCH,v2,3/3] bootm: Make it boot',
                         'content': ''},
                    ],
                    'cover_letter': {
                        'id': 139,
                        'name': 'The name of the cover letter',
                    }
                }
            if series_id == self.SERIES_ID_FIRST_V3:
                # series 'first3'
                return {
                    'patches': [
                        {'id': 20, 'name': '[PATCH,v3,1/2] i2c: I2C things',
                         'content': ''},
                        {'id': 21, 'name': '[PATCH,v3,2/2] spi: SPI fixes',
                         'content': ''},
                    ],
                    'cover_letter': {
                        'id': 29,
                        'name': 'Cover letter for first',
                    }
                }
            if series_id == 123:
                return {
                    'patches': [
                        {'id': 20, 'name': '[PATCH,1/2] i2c: I2C things',
                         'content': ''},
                        {'id': 21, 'name': '[PATCH,2/2] spi: SPI fixes',
                         'content': ''},
                    ],
                }
            if series_id == 1234:
                return {
                    'patches': [
                        {'id': 20, 'name': '[PATCH,v2,1/2] i2c: I2C things',
                         'content': ''},
                        {'id': 21, 'name': '[PATCH,v2,2/2] spi: SPI fixes',
                         'content': ''},
                    ],
                }
            raise ValueError(f'Fake Patchwork unknown series_id: {series_id}')

        # Read patch status
        m_pat = re.search(r'patches/(\d*)/$', subpath)
        patch_id = int(m_pat.group(1)) if m_pat else ''
        if patch_id:
            if patch_id in [10, 110]:
                return {'state': 'accepted',
                        'content':
                            'Reviewed-by: Fred Bloggs <fred@bloggs.com>'}
            if patch_id in [11, 111]:
                return {'state': 'changes-requested', 'content': ''}
            if patch_id in [12, 112]:
                return {'state': 'rejected',
                        'content': "I don't like this at all, sorry"}
            if patch_id == 20:
                return {'state': 'awaiting-upstream', 'content': ''}
            if patch_id == 21:
                return {'state': 'not-applicable', 'content': ''}
            raise ValueError(f'Fake Patchwork unknown patch_id: {patch_id}')

        # Read comments a from patch
        m_comm = re.search(r'patches/(\d*)/comments/', subpath)
        patch_id = int(m_comm.group(1)) if m_comm else ''
        if patch_id:
            if patch_id in [10, 110]:
                return [
                    {'id': 1, 'content': ''},
                    {'id': 2,
                     'content':
                         '''On some date Mary Smith <msmith@wibble.com> wrote:
> This was my original patch
> which is being quoted

I like the approach here and I would love to see more of it.

Reviewed-by: Fred Bloggs <fred@bloggs.com>
''',
                     'submitter': {
                         'name': 'Fred Bloggs',
                         'email': 'fred@bloggs.com',
                         }
                     },
                ]
            if patch_id in [11, 111]:
                return []
            if patch_id in [12, 112]:
                return [
                    {'id': 4, 'content': ''},
                    {'id': 5, 'content': ''},
                    {'id': 6, 'content': ''},
                ]
            if patch_id == 20:
                return [
                    {'id': 7, 'content':
                     '''On some date Alex Miller <alex@country.org> wrote:

> Sometimes we need to create a patch.
> This is one of those times

Tested-by: Mary Smith <msmith@wibble.com>   # yak
'''},
                    {'id': 8, 'content': ''},
                ]
            if patch_id == 21:
                return []
            raise ValueError(
                f'Fake Patchwork does not understand patch_id {patch_id}: '
                f'{subpath}')

        # Read comments from a cover letter
        m_cover_id = re.search(r'covers/(\d*)/comments/', subpath)
        cover_id = int(m_cover_id.group(1)) if m_cover_id else ''
        if cover_id:
            if cover_id in [39, 139]:
                return [
                    {'content': 'some comment',
                        'submitter': {
                            'name': 'A user',
                            'email': 'user@user.com',
                          },
                        'date': 'Sun 13 Apr 14:06:02 MDT 2025',
                     },
                    {'content': 'another comment',
                        'submitter': {
                            'name': 'Ghenkis Khan',
                            'email': 'gk@eurasia.gov',
                        },
                     'date': 'Sun 13 Apr 13:06:02 MDT 2025',
                     },
                ]
            if cover_id == 29:
                return []

            raise ValueError(f'Fake Patchwork unknown cover_id: {cover_id}')

        raise ValueError(f'Fake Patchwork does not understand: {subpath}')

    def setup_second(self, do_sync=True):
        """Set up the 'second' series synced with the fake patchwork

        Args:
            do_sync (bool): True to sync the series

        Return: tuple:
            Cseries: New Cseries object
            pwork: Patchwork object
        """
        with self.stage('setup second'):
            cser = self.get_cser()
            pwork = Patchwork.for_testing(self._fake_patchwork_cser)
            pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)

            with terminal.capture() as (out, _):
                cser.add('first', '', allow_unmarked=True)
                cser.add('second', allow_unmarked=True)

            series = patchstream.get_metadata_for_list('second', self.gitdir,
                                                       3)
            self.assertEqual('456', series.links)

            with terminal.capture() as (out, _):
                cser.increment('second')

            series = patchstream.get_metadata_for_list('second', self.gitdir,
                                                       3)
            self.assertEqual('456', series.links)

            series = patchstream.get_metadata_for_list('second2', self.gitdir,
                                                       3)
            self.assertEqual('1:456', series.links)

            if do_sync:
                with terminal.capture() as (out, _):
                    cser.link_auto(pwork, 'second', 2, True)
                with terminal.capture() as (out, _):
                    cser.gather(pwork, 'second', 2, False, True, False)
                lines = out.getvalue().splitlines()
                self.assertEqual(
                    "Updating series 'second' version 2 from link '457'",
                    lines[0])
                self.assertEqual(
                    '3 patches and cover letter updated (8 requests)',
                    lines[1])
                self.assertEqual(2, len(lines))

        return cser, pwork

    def test_series_add_no_cover(self):
        """Test patchwork when adding a series which has no cover letter"""
        cser = self.get_cser()
        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)

        with terminal.capture() as (out, _):
            cser.add('first', 'my name for this', mark=False,
                     allow_unmarked=True)
        self.assertIn("Added series 'first' v1 (2 commits)", out.getvalue())

        with terminal.capture() as (out, _):
            cser.link_auto(pwork, 'first', 1, True)
        self.assertIn("Setting link for series 'first' v1 to 12345",
                      out.getvalue())

    def test_series_list(self):
        """Test listing cseries"""
        self.setup_second()

        self.db_close()
        args = Namespace(subcmd='ls')
        with terminal.capture() as (out, _):
            control.do_series(args, test_db=self.tmpdir, pwork=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(5, len(lines))
        self.assertEqual(
            'Name             Description                               '
            'Accepted  Versions', lines[0])
        self.assertTrue(lines[1].startswith('--'))
        self.assertEqual(
            'first                                                      '
            '     -/2  1', lines[2])
        self.assertEqual(
            'second           Series for my board                       '
            '     1/3  1 2', lines[3])
        self.assertTrue(lines[4].startswith('--'))

    def test_do_series_add(self):
        """Add a new cseries"""
        self.make_git_tree()
        args = Namespace(subcmd='add', desc='my-description', series='first',
                         mark=False, allow_unmarked=True, upstream=None,
                         dry_run=False)
        with terminal.capture() as (out, _):
            control.do_series(args, test_db=self.tmpdir, pwork=True)

        cser = self.get_database()
        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        ser = slist.get('first')
        self.assertTrue(ser)
        self.assertEqual('first', ser.name)
        self.assertEqual('my-description', ser.desc)

        self.db_close()
        args.subcmd = 'ls'
        with terminal.capture() as (out, _):
            control.do_series(args, test_db=self.tmpdir, pwork=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(4, len(lines))
        self.assertTrue(lines[1].startswith('--'))
        self.assertEqual(
            'first            my-description                                 '
            '-/2  1', lines[2])

    def test_do_series_add_cmdline(self):
        """Add a new cseries using the cmdline"""
        self.make_git_tree()
        with terminal.capture():
            self.run_args('series', '-s', 'first', 'add', '-M',
                          '-D', 'my-description', pwork=True)

        cser = self.get_database()
        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        ser = slist.get('first')
        self.assertTrue(ser)
        self.assertEqual('first', ser.name)
        self.assertEqual('my-description', ser.desc)

    def test_do_series_add_auto(self):
        """Add a new cseries without any arguments"""
        self.make_git_tree()

        # Use the 'second' branch, which has a cover letter
        gitutil.checkout('second', self.gitdir, work_tree=self.tmpdir,
                         force=True)
        args = Namespace(subcmd='add', series=None, mark=False,
                         allow_unmarked=True, upstream=None, dry_run=False,
                         desc=None)
        with terminal.capture():
            control.do_series(args, test_db=self.tmpdir, pwork=True)

        cser = self.get_database()
        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))
        ser = slist.get('second')
        self.assertTrue(ser)
        self.assertEqual('second', ser.name)
        self.assertEqual('Series for my board', ser.desc)
        cser.close_database()

    def _check_inc(self, out):
        """Check output from an 'increment' operation

        Args:
            out (StringIO): Text to check
        """
        itr = iter(out.getvalue().splitlines())

        self.assertEqual("Increment 'first' v1: 2 patches", next(itr))
        self.assertRegex(next(itr), 'Checking out upstream commit .*')
        self.assertEqual("Processing 2 commits from branch 'first2'",
                         next(itr))
        self.assertRegex(next(itr),
                         f'-         {HASH_RE} as {HASH_RE} i2c: I2C things')
        self.assertRegex(next(itr),
                         f'- add v2: {HASH_RE} as {HASH_RE} spi: SPI fixes')
        self.assertRegex(
            next(itr), f'Updating branch first2 from {HASH_RE} to {HASH_RE}')
        self.assertEqual('Added new branch first2', next(itr))
        return itr

    def test_series_link(self):
        """Test adding a patchwork link to a cseries"""
        cser = self.get_cser()

        repo = pygit2.init_repository(self.gitdir)
        first = repo.lookup_branch('first').peel(
            pygit2.enums.ObjectType.COMMIT).oid
        base = repo.lookup_branch('base').peel(
            pygit2.enums.ObjectType.COMMIT).oid

        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)

        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        with self.assertRaises(ValueError) as exc:
            cser.link_set('first', 2, '1234', True)
        self.assertEqual("Series 'first' does not have a version 2",
                         str(exc.exception))

        self.assertEqual('first', gitutil.get_branch(self.gitdir))
        with terminal.capture() as (out, _):
            cser.increment('first')
        self.assertTrue(repo.lookup_branch('first2'))

        with terminal.capture() as (out, _):
            cser.link_set('first', 2, '2345', True)

        lines = out.getvalue().splitlines()
        self.assertEqual(6, len(lines))
        self.assertRegex(
            lines[0], 'Checking out upstream commit refs/heads/base: .*')
        self.assertEqual("Processing 2 commits from branch 'first2'",
                         lines[1])
        self.assertRegex(
            lines[2],
            f'-                        {HASH_RE} as {HASH_RE} i2c: I2C things')
        self.assertRegex(
            lines[3],
            f"- add v2 links '2:2345': {HASH_RE} as {HASH_RE} spi: SPI fixes")
        self.assertRegex(
            lines[4], f'Updating branch first2 from {HASH_RE} to {HASH_RE}')
        self.assertEqual("Setting link for series 'first' v2 to 2345",
                         lines[5])

        self.assertEqual('2345', cser.link_get('first', 2))

        series = patchstream.get_metadata_for_list('first2', self.gitdir, 2)
        self.assertEqual('2:2345', series.links)

        self.assertEqual('first2', gitutil.get_branch(self.gitdir))

        # Check the original series was left alone
        self.assertEqual(
            first, repo.lookup_branch('first').peel(
                pygit2.enums.ObjectType.COMMIT).oid)
        count = 2
        series1 = patchstream.get_metadata_for_list('first', self.gitdir,
                                                    count)
        self.assertFalse('links' in series1)
        self.assertFalse('version' in series1)

        # Check that base is left alone
        self.assertEqual(
            base, repo.lookup_branch('base').peel(
                pygit2.enums.ObjectType.COMMIT).oid)
        series1 = patchstream.get_metadata_for_list('base', self.gitdir, count)
        self.assertFalse('links' in series1)
        self.assertFalse('version' in series1)

        # Check out second and try to update first
        gitutil.checkout('second', self.gitdir, work_tree=self.tmpdir,
                         force=True)
        with terminal.capture():
            cser.link_set('first', 1, '16', True)

        # Overwrite the link
        with terminal.capture():
            cser.link_set('first', 1, '17', True)

        series2 = patchstream.get_metadata_for_list('first', self.gitdir,
                                                    count)
        self.assertEqual('1:17', series2.links)

    def test_series_link_cmdline(self):
        """Test adding a patchwork link to a cseries using the cmdline"""
        cser = self.get_cser()

        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)

        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', '-V', '4', 'set-link', '-u',
                          '1234', expect_ret=1, pwork=True)
        self.assertIn("Series 'first' does not have a version 4",
                      out.getvalue())

        with self.assertRaises(ValueError) as exc:
            cser.link_get('first', 4)
        self.assertEqual("Series 'first' does not have a version 4",
                         str(exc.exception))

        with terminal.capture() as (out, _):
            cser.increment('first')

        with self.assertRaises(ValueError) as exc:
            cser.link_get('first', 4)
        self.assertEqual("Series 'first' does not have a version 4",
                         str(exc.exception))

        with terminal.capture() as (out, _):
            cser.increment('first')
            cser.increment('first')

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', '-V', '4', 'set-link', '-u',
                          '1234', pwork=True)
        lines = out.getvalue().splitlines()
        self.assertRegex(
            lines[-3],
            f"- add v4 links '4:1234': {HASH_RE} as {HASH_RE} spi: SPI fixes")
        self.assertEqual("Setting link for series 'first' v4 to 1234",
                         lines[-1])

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', '-V', '4', 'get-link',
                          pwork=True)
        self.assertIn('1234', out.getvalue())

        series = patchstream.get_metadata_for_list('first4', self.gitdir, 1)
        self.assertEqual('4:1234', series.links)

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', '-V', '5', 'get-link',
                          expect_ret=1, pwork=True)

        self.assertIn("Series 'first' does not have a version 5",
                      out.getvalue())

        # Checkout 'first' and try to get the link from 'first4'
        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first4', 'get-link', pwork=True)
        self.assertIn('1234', out.getvalue())

        # This should get the link for 'first'
        with terminal.capture() as (out, _):
            self.run_args('series', 'get-link', pwork=True)
        self.assertIn('None', out.getvalue())

        # Checkout 'first4' again; this should get the link for 'first4'
        gitutil.checkout('first4', self.gitdir, work_tree=self.tmpdir,
                         force=True)

        with terminal.capture() as (out, _):
            self.run_args('series', 'get-link', pwork=True)
        self.assertIn('1234', out.getvalue())

    def test_series_link_auto_version(self):
        """Test finding the patchwork link for a cseries automatically"""
        cser = self.get_cser()

        with terminal.capture() as (out, _):
            cser.add('second', allow_unmarked=True)

        # Make sure that the link is there
        count = 3
        series = patchstream.get_metadata('second', 0, count,
                                          git_dir=self.gitdir)
        self.assertEqual(f'{self.SERIES_ID_SECOND_V1}', series.links)

        # Set link with detected version
        with terminal.capture() as (out, _):
            cser.link_set('second', None, f'{self.SERIES_ID_SECOND_V1}', True)
        self.assertEqual(
            "Setting link for series 'second' v1 to 456",
            out.getvalue().splitlines()[-1])

        # Make sure that the link was set
        series = patchstream.get_metadata('second', 0, count,
                                          git_dir=self.gitdir)
        self.assertEqual(f'1:{self.SERIES_ID_SECOND_V1}', series.links)

        with terminal.capture():
            cser.increment('second')

        # Make sure that the new series gets the same link
        series = patchstream.get_metadata('second2', 0, 3,
                                          git_dir=self.gitdir)

        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)
        self.assertFalse(cser.project_get())
        cser.project_set(pwork, 'U-Boot', quiet=True)

        self.assertEqual(
            (self.SERIES_ID_SECOND_V1, None, 'second', 1,
             'Series for my board'),
            cser.link_search(pwork, 'second', 1))

        with terminal.capture():
            cser.increment('second')

        self.assertEqual((457, None, 'second', 2, 'Series for my board'),
                         cser.link_search(pwork, 'second', 2))

    def test_series_link_auto_name(self):
        """Test finding the patchwork link for a cseries with auto name"""
        cser = self.get_cser()

        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        # Set link with detected name
        with self.assertRaises(ValueError) as exc:
            cser.link_set(None, 2, '2345', True)
        self.assertEqual(
            "Series 'first' does not have a version 2", str(exc.exception))

        with terminal.capture():
            cser.increment('first')

        with terminal.capture() as (out, _):
            cser.link_set(None, 2, '2345', True)
        self.assertEqual(
                "Setting link for series 'first' v2 to 2345",
                out.getvalue().splitlines()[-1])

        svlist = cser.get_ser_ver_list()
        self.assertEqual(2, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(1, svlist[0].version)
        self.assertIsNone(svlist[0].link)

        self.assertEqual(2, svlist[1].idnum)
        self.assertEqual(1, svlist[1].series_id)
        self.assertEqual(2, svlist[1].version)
        self.assertEqual('2345', svlist[1].link)

    def test_series_link_auto_name_version(self):
        """Find patchwork link for a cseries with auto name + version"""
        cser = self.get_cser()

        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        # Set link with detected name and version
        with terminal.capture() as (out, _):
            cser.link_set(None, None, '1234', True)
        self.assertEqual(
                "Setting link for series 'first' v1 to 1234",
                out.getvalue().splitlines()[-1])

        with terminal.capture():
            cser.increment('first')

        with terminal.capture() as (out, _):
            cser.link_set(None, None, '2345', True)
        self.assertEqual(
                "Setting link for series 'first' v2 to 2345",
                out.getvalue().splitlines()[-1])

        svlist = cser.get_ser_ver_list()
        self.assertEqual(2, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(1, svlist[0].version)
        self.assertEqual('1234', svlist[0].link)

        self.assertEqual(2, svlist[1].idnum)
        self.assertEqual(1, svlist[1].series_id)
        self.assertEqual(2, svlist[1].version)
        self.assertEqual('2345', svlist[1].link)

    def test_series_link_missing(self):
        """Test finding patchwork link for a cseries but it is missing"""
        cser = self.get_cser()

        with terminal.capture():
            cser.add('second', allow_unmarked=True)

        with terminal.capture():
            cser.increment('second')
            cser.increment('second')

        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)
        self.assertFalse(cser.project_get())
        cser.project_set(pwork, 'U-Boot', quiet=True)

        self.assertEqual(
            (self.SERIES_ID_SECOND_V1, None, 'second', 1,
             'Series for my board'),
            cser.link_search(pwork, 'second', 1))
        self.assertEqual((457, None, 'second', 2, 'Series for my board'),
                         cser.link_search(pwork, 'second', 2))
        res = cser.link_search(pwork, 'second', 3)
        self.assertEqual(
            (None,
             [{'id': self.SERIES_ID_SECOND_V1, 'name': 'Series for my board',
               'version': 1},
              {'id': 457, 'name': 'Series for my board', 'version': 2}],
             'second', 3, 'Series for my board'),
            res)

    def check_series_autolink(self):
        """Common code for autolink tests"""
        cser = self.get_cser()

        with self.stage('setup'):
            pwork = Patchwork.for_testing(self._fake_patchwork_cser)
            pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)
            self.assertFalse(cser.project_get())
            cser.project_set(pwork, 'U-Boot', quiet=True)

            with terminal.capture():
                cser.add('first', '', allow_unmarked=True)
                cser.add('second', allow_unmarked=True)

        with self.stage('autolink unset'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            self.assertEqual(
                "Setting link for series 'second' v1 to "
                f'{self.SERIES_ID_SECOND_V1}',
                out.getvalue().splitlines()[-1])

        svlist = cser.get_ser_ver_list()
        self.assertEqual(2, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(1, svlist[0].version)
        self.assertEqual(2, svlist[1].idnum)
        self.assertEqual(2, svlist[1].series_id)
        self.assertEqual(1, svlist[1].version)
        self.assertEqual(str(self.SERIES_ID_SECOND_V1), svlist[1].link)
        yield None

    def test_series_autolink(self):
        """Test linking a cseries to its patchwork series by description"""
        cor = self.check_series_autolink()
        cser, pwork = next(cor)

        with self.assertRaises(ValueError) as exc:
            cser.link_auto(pwork, 'first', None, True)
        self.assertIn("Series 'first' has an empty description",
                      str(exc.exception))

        # autolink unset
        cser.link_auto(pwork, 'second', None, True)

        self.assertFalse(next(cor))
        cor.close()

    def test_series_autolink_cmdline(self):
        """Test linking to patchwork series by description on cmdline"""
        cor = self.check_series_autolink()
        _, pwork = next(cor)

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', 'autolink', expect_ret=1,
                          pwork=pwork)
        self.assertEqual(
            "patman: ValueError: Series 'first' has an empty description",
            out.getvalue().strip())

        # autolink unset
        self.run_args('series', '-s', 'second', 'autolink', '-u', pwork=pwork)

        self.assertFalse(next(cor))
        cor.close()

    def _autolink_setup(self):
        """Set things up for autolink tests

        Return: tuple:
            Cseries object
            Patchwork object
        """
        cser = self.get_cser()

        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)
        self.assertFalse(cser.project_get())
        cser.project_set(pwork, 'U-Boot', quiet=True)

        with terminal.capture():
            cser.add('first', 'first series', allow_unmarked=True)
            cser.add('second', allow_unmarked=True)
            cser.increment('first')
        return cser, pwork

    def test_series_link_auto_all(self):
        """Test linking all cseries to their patchwork series by description"""
        cser, pwork = self._autolink_setup()
        with terminal.capture() as (out, _):
            summary = cser.link_auto_all(pwork, update_commit=True,
                                         link_all_versions=True,
                                         replace_existing=False, dry_run=True,
                                         show_summary=False)
        self.assertEqual(3, len(summary))
        items = iter(summary.values())
        linked = next(items)
        self.assertEqual(
            ('first', 1, None, 'first series', 'linked:1234'), linked)
        self.assertEqual(
            ('first', 2, None, 'first series', 'not found'), next(items))
        self.assertEqual(
            ('second', 1, f'{self.SERIES_ID_SECOND_V1}', 'Series for my board',
             f'already:{self.SERIES_ID_SECOND_V1}'),
            next(items))
        self.assertEqual('Dry run completed', out.getvalue().splitlines()[-1])

        # A second dry run should do exactly the same thing
        with terminal.capture() as (out2, _):
            summary2 = cser.link_auto_all(pwork, update_commit=True,
                                          link_all_versions=True,
                                          replace_existing=False, dry_run=True,
                                          show_summary=False)
        self.assertEqual(out.getvalue(), out2.getvalue())
        self.assertEqual(summary, summary2)

        # Now do it for real
        with terminal.capture():
            summary = cser.link_auto_all(pwork, update_commit=True,
                                         link_all_versions=True,
                                         replace_existing=False, dry_run=False,
                                         show_summary=False)

        # Check the link was updated
        pdict = cser.get_ser_ver_dict()
        svid = list(summary)[0]
        self.assertEqual('1234', pdict[svid].link)

        series = patchstream.get_metadata_for_list('first', self.gitdir, 2)
        self.assertEqual('1:1234', series.links)

    def test_series_autolink_latest(self):
        """Test linking the lastest versions"""
        cser, pwork = self._autolink_setup()
        with terminal.capture():
            summary = cser.link_auto_all(pwork, update_commit=True,
                                         link_all_versions=False,
                                         replace_existing=False, dry_run=False,
                                         show_summary=False)
        self.assertEqual(2, len(summary))
        items = iter(summary.values())
        self.assertEqual(
            ('first', 2, None, 'first series', 'not found'), next(items))
        self.assertEqual(
            ('second', 1, f'{self.SERIES_ID_SECOND_V1}', 'Series for my board',
             f'already:{self.SERIES_ID_SECOND_V1}'),
            next(items))

    def test_series_autolink_no_update(self):
        """Test linking the lastest versions without updating commits"""
        cser, pwork = self._autolink_setup()
        with terminal.capture():
            cser.link_auto_all(pwork, update_commit=False,
                               link_all_versions=True, replace_existing=False,
                               dry_run=False,
                               show_summary=False)

        series = patchstream.get_metadata_for_list('first', self.gitdir, 2)
        self.assertNotIn('links', series)

    def test_series_autolink_replace(self):
        """Test linking the lastest versions without updating commits"""
        cser, pwork = self._autolink_setup()
        with terminal.capture():
            summary = cser.link_auto_all(pwork, update_commit=True,
                                         link_all_versions=True,
                                         replace_existing=True, dry_run=False,
                                         show_summary=False)
        self.assertEqual(3, len(summary))
        items = iter(summary.values())
        linked = next(items)
        self.assertEqual(
            ('first', 1, None, 'first series', 'linked:1234'), linked)
        self.assertEqual(
            ('first', 2, None, 'first series', 'not found'), next(items))
        self.assertEqual(
            ('second', 1, f'{self.SERIES_ID_SECOND_V1}', 'Series for my board',
             f'linked:{self.SERIES_ID_SECOND_V1}'),
            next(items))

    def test_series_autolink_extra(self):
        """Test command-line operation

        This just uses mocks for now since we can rely on the direct tests for
        the actual operation.
        """
        _, pwork = self._autolink_setup()
        with (mock.patch.object(cseries.Cseries, 'link_auto_all',
                                return_value=None) as method):
            self.run_args('series', 'autolink-all', pwork=True)
        method.assert_called_once_with(True, update_commit=False,
                                       link_all_versions=False,
                                       replace_existing=False, dry_run=False,
                                       show_summary=True)

        with (mock.patch.object(cseries.Cseries, 'link_auto_all',
                                return_value=None) as method):
            self.run_args('series', 'autolink-all', '-a', pwork=True)
        method.assert_called_once_with(True, update_commit=False,
                                       link_all_versions=True,
                                       replace_existing=False, dry_run=False,
                                       show_summary=True)

        with (mock.patch.object(cseries.Cseries, 'link_auto_all',
                                return_value=None) as method):
            self.run_args('series', 'autolink-all', '-a', '-r', pwork=True)
        method.assert_called_once_with(True, update_commit=False,
                                       link_all_versions=True,
                                       replace_existing=True, dry_run=False,
                                       show_summary=True)

        with (mock.patch.object(cseries.Cseries, 'link_auto_all',
                                return_value=None) as method):
            self.run_args('series', '-n', 'autolink-all', '-r', pwork=True)
        method.assert_called_once_with(True, update_commit=False,
                                       link_all_versions=False,
                                       replace_existing=True, dry_run=True,
                                       show_summary=True)

        with (mock.patch.object(cseries.Cseries, 'link_auto_all',
                                return_value=None) as method):
            self.run_args('series', 'autolink-all', '-u', pwork=True)
        method.assert_called_once_with(True, update_commit=True,
                                       link_all_versions=False,
                                       replace_existing=False, dry_run=False,
                                       show_summary=True)

        # Now do a real one to check the patchwork handling and output
        with terminal.capture() as (out, _):
            self.run_args('series', 'autolink-all', '-a', pwork=pwork)
        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            '1 series linked, 1 already linked, 1 not found (3 requests)',
            next(itr))
        self.assertEqual('', next(itr))
        self.assertEqual(
            'Name             Version  Description                            '
            '   Result', next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assertEqual(
            'first                  1  first series                           '
            '   linked:1234', next(itr))
        self.assertEqual(
            'first                  2  first series                           '
            '   not found', next(itr))
        self.assertEqual(
            'second                 1  Series for my board                   '
            f'    already:{self.SERIES_ID_SECOND_V1}',
            next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assert_finished(itr)

    def check_series_archive(self):
        """Coroutine to run the archive test"""
        cser = self.get_cser()
        with self.stage('setup'):
            with terminal.capture():
                cser.add('first', '', allow_unmarked=True)

            # Check the series is visible in the list
            slist = cser.db.series_get_dict()
            self.assertEqual(1, len(slist))
            self.assertEqual('first', slist['first'].name)

            # Add a second branch
            with terminal.capture():
                cser.increment('first')

        cser.fake_now = datetime(24, 9, 14)
        repo = pygit2.init_repository(self.gitdir)
        with self.stage('archive'):
            expected_commit1 = repo.revparse_single('first')
            expected_commit2 = repo.revparse_single('first2')
            expected_tag1 = 'first-14sep24'
            expected_tag2 = 'first2-14sep24'

            # Archive it and make sure it is invisible
            yield cser
            slist = cser.db.series_get_dict()
            self.assertFalse(slist)

            # ...unless we include archived items
            slist = cser.db.series_get_dict(include_archived=True)
            self.assertEqual(1, len(slist))
            first = slist['first']
            self.assertEqual('first', first.name)

            # Make sure the branches have been tagged
            svlist = cser.db.ser_ver_get_for_series(first.idnum)
            self.assertEqual(expected_tag1, svlist[0].archive_tag)
            self.assertEqual(expected_tag2, svlist[1].archive_tag)

            # Check that the tags were created and point to old branch commits
            target1 = repo.revparse_single(expected_tag1)
            self.assertEqual(expected_commit1, target1.get_object())
            target2 = repo.revparse_single(expected_tag2)
            self.assertEqual(expected_commit2, target2.get_object())

            # The branches should be deleted
            self.assertFalse('first' in repo.branches)
            self.assertFalse('first2' in repo.branches)

        with self.stage('unarchive'):
            # or we unarchive it
            yield cser
            slist = cser.db.series_get_dict()
            self.assertEqual(1, len(slist))

            # Make sure the branches have been restored
            branch1 = repo.branches['first']
            branch2 = repo.branches['first2']
            self.assertEqual(expected_commit1.oid, branch1.target)
            self.assertEqual(expected_commit2.oid, branch2.target)

            # Make sure the tags were deleted
            try:
                target1 = repo.revparse_single(expected_tag1)
                self.fail('target1 is still present')
            except KeyError:
                pass
            try:
                target1 = repo.revparse_single(expected_tag2)
                self.fail('target2 is still present')
            except KeyError:
                pass

            # Make sure the tag information has been removed
            svlist = cser.db.ser_ver_get_for_series(first.idnum)
            self.assertFalse(svlist[0].archive_tag)
            self.assertFalse(svlist[1].archive_tag)

        yield False

    def test_series_archive(self):
        """Test marking a series as archived"""
        cor = self.check_series_archive()
        cser = next(cor)

        # Archive it and make sure it is invisible
        cser.archive('first')
        cser = next(cor)
        cser.unarchive('first')
        self.assertFalse(next(cor))
        cor.close()

    def test_series_archive_cmdline(self):
        """Test marking a series as archived with cmdline"""
        cor = self.check_series_archive()
        cser = next(cor)

        # Archive it and make sure it is invisible
        self.run_args('series', '-s', 'first', 'archive', pwork=True,
                      cser=cser)
        next(cor)
        self.run_args('series', '-s', 'first', 'unarchive', pwork=True,
                      cser=cser)
        self.assertFalse(next(cor))
        cor.close()

    def check_series_inc(self):
        """Coroutine to run the increment test"""
        cser = self.get_cser()

        with self.stage('setup'):
            gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                             force=True)
            with terminal.capture() as (out, _):
                cser.add('first', '', allow_unmarked=True)

        with self.stage('increment'):
            with terminal.capture() as (out, _):
                yield cser
            self._check_inc(out)

            slist = cser.db.series_get_dict()
            self.assertEqual(1, len(slist))

            svlist = cser.get_ser_ver_list()
            self.assertEqual(2, len(svlist))
            self.assertEqual(1, svlist[0].idnum)
            self.assertEqual(1, svlist[0].series_id)
            self.assertEqual(1, svlist[0].version)

            self.assertEqual(2, svlist[1].idnum)
            self.assertEqual(1, svlist[1].series_id)
            self.assertEqual(2, svlist[1].version)

            series = patchstream.get_metadata_for_list('first2', self.gitdir,
                                                       1)
            self.assertEqual('2', series.version)

            series = patchstream.get_metadata_for_list('first', self.gitdir, 1)
            self.assertNotIn('version', series)

            self.assertEqual('first2', gitutil.get_branch(self.gitdir))
        yield None

    def test_series_inc(self):
        """Test incrementing the version"""
        cor = self.check_series_inc()
        cser = next(cor)

        cser.increment('first')
        self.assertFalse(next(cor))

        cor.close()

    def test_series_inc_cmdline(self):
        """Test incrementing the version with cmdline"""
        cor = self.check_series_inc()
        next(cor)

        self.run_args('series', '-s', 'first', 'inc', pwork=True)
        self.assertFalse(next(cor))
        cor.close()

    def test_series_inc_no_upstream(self):
        """Increment a series which has no upstream branch"""
        cser = self.get_cser()

        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)
        with terminal.capture():
            cser.add('first', '', allow_unmarked=True)

        repo = pygit2.init_repository(self.gitdir)
        upstream = repo.lookup_branch('base')
        upstream.delete()
        with terminal.capture():
            cser.increment('first')

        slist = cser.db.series_get_dict()
        self.assertEqual(1, len(slist))

    def test_series_inc_dryrun(self):
        """Test incrementing the version with cmdline"""
        cser = self.get_cser()

        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)
        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        with terminal.capture() as (out, _):
            cser.increment('first', dry_run=True)
        itr = self._check_inc(out)
        self.assertEqual('Dry run completed', next(itr))

        # Make sure that nothing was added
        svlist = cser.get_ser_ver_list()
        self.assertEqual(1, len(svlist))
        self.assertEqual(1, svlist[0].idnum)
        self.assertEqual(1, svlist[0].series_id)
        self.assertEqual(1, svlist[0].version)

        # We should still be on the same branch
        self.assertEqual('first', gitutil.get_branch(self.gitdir))

    def test_series_dec(self):
        """Test decrementing the version"""
        cser = self.get_cser()

        gitutil.checkout('first', self.gitdir, work_tree=self.tmpdir,
                         force=True)
        with terminal.capture() as (out, _):
            cser.add('first', '', allow_unmarked=True)

        pclist = cser.get_pcommit_dict()
        self.assertEqual(2, len(pclist))

        # Try decrementing when there is only one version
        with self.assertRaises(ValueError) as exc:
            cser.decrement('first')
        self.assertEqual("Series 'first' only has one version",
                         str(exc.exception))

        # Add a version; now there should be two
        with terminal.capture() as (out, _):
            cser.increment('first')
        svdict = cser.get_ser_ver_dict()
        self.assertEqual(2, len(svdict))

        pclist = cser.get_pcommit_dict()
        self.assertEqual(4, len(pclist))

        # Remove version two, using dry run (i.e. no effect)
        with terminal.capture() as (out, _):
            cser.decrement('first', dry_run=True)
        svdict = cser.get_ser_ver_dict()
        self.assertEqual(2, len(svdict))

        repo = pygit2.init_repository(self.gitdir)
        branch = repo.lookup_branch('first2')
        self.assertTrue(branch)
        branch_oid = branch.peel(pygit2.enums.ObjectType.COMMIT).oid

        pclist = cser.get_pcommit_dict()
        self.assertEqual(4, len(pclist))

        # Now remove version two for real
        with terminal.capture() as (out, _):
            cser.decrement('first')
        lines = out.getvalue().splitlines()
        self.assertEqual(2, len(lines))
        self.assertEqual("Removing series 'first' v2", lines[0])
        self.assertEqual(
            f"Deleted branch 'first2' {str(branch_oid)[:10]}", lines[1])

        svdict = cser.get_ser_ver_dict()
        self.assertEqual(1, len(svdict))

        pclist = cser.get_pcommit_dict()
        self.assertEqual(2, len(pclist))

        branch = repo.lookup_branch('first2')
        self.assertFalse(branch)

        # Removing the only version should not be allowed
        with self.assertRaises(ValueError) as exc:
            cser.decrement('first', dry_run=True)
        self.assertEqual("Series 'first' only has one version",
                         str(exc.exception))

    def test_upstream_add(self):
        """Test adding an upsream"""
        cser = self.get_cser()

        cser.upstream_add('us', 'https://one')
        ulist = cser.get_upstream_dict()
        self.assertEqual(1, len(ulist))
        self.assertEqual(('https://one', None), ulist['us'])

        cser.upstream_add('ci', 'git@two')
        ulist = cser.get_upstream_dict()
        self.assertEqual(2, len(ulist))
        self.assertEqual(('https://one', None), ulist['us'])
        self.assertEqual(('git@two', None), ulist['ci'])

        # Try to add a duplicate
        with self.assertRaises(ValueError) as exc:
            cser.upstream_add('ci', 'git@three')
        self.assertEqual("Upstream 'ci' already exists", str(exc.exception))

        with terminal.capture() as (out, _):
            cser.upstream_list()
        lines = out.getvalue().splitlines()
        self.assertEqual(2, len(lines))
        self.assertEqual('us                       https://one', lines[0])
        self.assertEqual('ci                       git@two', lines[1])

    def test_upstream_add_cmdline(self):
        """Test adding an upsream with cmdline"""
        with terminal.capture():
            self.run_args('upstream', 'add', 'us', 'https://one')

        with terminal.capture() as (out, _):
            self.run_args('upstream', 'list')
        lines = out.getvalue().splitlines()
        self.assertEqual(1, len(lines))
        self.assertEqual('us                       https://one', lines[0])

    def test_upstream_default(self):
        """Operation of the default upstream"""
        cser = self.get_cser()

        with self.assertRaises(ValueError) as exc:
            cser.upstream_set_default('us')
        self.assertEqual("No such upstream 'us'", str(exc.exception))

        cser.upstream_add('us', 'https://one')
        cser.upstream_add('ci', 'git@two')

        self.assertIsNone(cser.upstream_get_default())

        cser.upstream_set_default('us')
        self.assertEqual('us', cser.upstream_get_default())

        cser.upstream_set_default('us')

        cser.upstream_set_default('ci')
        self.assertEqual('ci', cser.upstream_get_default())

        with terminal.capture() as (out, _):
            cser.upstream_list()
        lines = out.getvalue().splitlines()
        self.assertEqual(2, len(lines))
        self.assertEqual('us                       https://one', lines[0])
        self.assertEqual('ci              default  git@two', lines[1])

        cser.upstream_set_default(None)
        self.assertIsNone(cser.upstream_get_default())

    def test_upstream_default_cmdline(self):
        """Operation of the default upstream on cmdline"""
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default', 'us', expect_ret=1)
        self.assertEqual("patman: ValueError: No such upstream 'us'",
                         out.getvalue().strip().splitlines()[-1])

        self.run_args('upstream', 'add', 'us', 'https://one')
        self.run_args('upstream', 'add', 'ci', 'git@two')

        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default')
        self.assertEqual('unset', out.getvalue().strip())

        self.run_args('upstream', 'default', 'us')
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default')
        self.assertEqual('us', out.getvalue().strip())

        self.run_args('upstream', 'default', 'ci')
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default')
        self.assertEqual('ci', out.getvalue().strip())

        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default', '--unset')
        self.assertFalse(out.getvalue().strip())

        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default')
        self.assertEqual('unset', out.getvalue().strip())

    def test_upstream_delete(self):
        """Test operation of the default upstream"""
        cser = self.get_cser()

        with self.assertRaises(ValueError) as exc:
            cser.upstream_delete('us')
        self.assertEqual("No such upstream 'us'", str(exc.exception))

        cser.upstream_add('us', 'https://one')
        cser.upstream_add('ci', 'git@two')

        cser.upstream_set_default('us')
        cser.upstream_delete('us')
        self.assertIsNone(cser.upstream_get_default())

        cser.upstream_delete('ci')
        ulist = cser.get_upstream_dict()
        self.assertFalse(ulist)

    def test_upstream_delete_cmdline(self):
        """Test deleting an upstream"""
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'delete', 'us', expect_ret=1)
        self.assertEqual("patman: ValueError: No such upstream 'us'",
                         out.getvalue().strip().splitlines()[-1])

        self.run_args('us', 'add', 'us', 'https://one')
        self.run_args('us', 'add', 'ci', 'git@two')

        self.run_args('upstream', 'default', 'us')
        self.run_args('upstream', 'delete', 'us')
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'default', 'us', expect_ret=1)
        self.assertEqual("patman: ValueError: No such upstream 'us'",
                         out.getvalue().strip())

        self.run_args('upstream', 'delete', 'ci')
        with terminal.capture() as (out, _):
            self.run_args('upstream', 'list')
        self.assertFalse(out.getvalue().strip())

    def test_series_add_mark(self):
        """Test marking a cseries with Change-Id fields"""
        cser = self.get_cser()

        with terminal.capture():
            cser.add('first', '', mark=True)

        pcdict = cser.get_pcommit_dict()

        series = patchstream.get_metadata('first', 0, 2, git_dir=self.gitdir)
        self.assertEqual(2, len(series.commits))
        self.assertIn(1, pcdict)
        self.assertEqual(1, pcdict[1].idnum)
        self.assertEqual('i2c: I2C things', pcdict[1].subject)
        self.assertEqual(1, pcdict[1].svid)
        self.assertEqual(series.commits[0].change_id, pcdict[1].change_id)

        self.assertIn(2, pcdict)
        self.assertEqual(2, pcdict[2].idnum)
        self.assertEqual('spi: SPI fixes', pcdict[2].subject)
        self.assertEqual(1, pcdict[2].svid)
        self.assertEqual(series.commits[1].change_id, pcdict[2].change_id)

    def test_series_add_mark_fail(self):
        """Test marking a cseries when the tree is dirty"""
        cser = self.get_cser()

        tools.write_file(os.path.join(self.tmpdir, 'fname'), b'123')
        with terminal.capture():
            cser.add('first', '', mark=True)

        tools.write_file(os.path.join(self.tmpdir, 'i2c.c'), b'123')
        with self.assertRaises(ValueError) as exc:
            with terminal.capture():
                cser.add('first', '', mark=True)
        self.assertEqual(
            "Modified files exist: use 'git status' to check: [' M i2c.c']",
            str(exc.exception))

    def test_series_add_mark_dry_run(self):
        """Test marking a cseries with Change-Id fields"""
        cser = self.get_cser()

        with terminal.capture() as (out, _):
            cser.add('first', '', mark=True, dry_run=True)
        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            "Adding series 'first' v1: mark True allow_unmarked False",
            next(itr))
        self.assertRegex(
            next(itr), 'Checking out upstream commit refs/heads/base: .*')
        self.assertEqual("Processing 2 commits from branch 'first'",
                         next(itr))
        self.assertRegex(
            next(itr), f'- marked: {HASH_RE} as {HASH_RE} i2c: I2C things')
        self.assertRegex(
            next(itr), f'- marked: {HASH_RE} as {HASH_RE} spi: SPI fixes')
        self.assertRegex(
            next(itr), f'Updating branch first from {HASH_RE} to {HASH_RE}')
        self.assertEqual("Added series 'first' v1 (2 commits)",
                         next(itr))
        self.assertEqual('Dry run completed', next(itr))

        # Doing another dry run should produce the same result
        with terminal.capture() as (out2, _):
            cser.add('first', '', mark=True, dry_run=True)
        self.assertEqual(out.getvalue(), out2.getvalue())

        tools.write_file(os.path.join(self.tmpdir, 'i2c.c'), b'123')
        with terminal.capture() as (out, _):
            with self.assertRaises(ValueError) as exc:
                cser.add('first', '', mark=True, dry_run=True)
        self.assertEqual(
            "Modified files exist: use 'git status' to check: [' M i2c.c']",
            str(exc.exception))

        pcdict = cser.get_pcommit_dict()
        self.assertFalse(pcdict)

    def test_series_add_mark_cmdline(self):
        """Test marking a cseries with Change-Id fields using the cmdline"""
        cser = self.get_cser()

        with terminal.capture():
            self.run_args('series', '-s', 'first', 'add', '-m',
                          '-D', 'my-description', pwork=True)

        pcdict = cser.get_pcommit_dict()
        self.assertTrue(pcdict[1].change_id)
        self.assertTrue(pcdict[2].change_id)

    def test_series_add_unmarked_cmdline(self):
        """Test adding an unmarked cseries using the command line"""
        cser = self.get_cser()

        with terminal.capture():
            self.run_args('series', '-s', 'first', 'add', '-M',
                          '-D', 'my-description', pwork=True)

        pcdict = cser.get_pcommit_dict()
        self.assertFalse(pcdict[1].change_id)
        self.assertFalse(pcdict[2].change_id)

    def test_series_add_unmarked_bad_cmdline(self):
        """Test failure to add an unmarked cseries using a bad command line"""
        self.get_cser()

        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', 'add',
                          '-D', 'my-description', expect_ret=1, pwork=True)
        last_line = out.getvalue().splitlines()[-2]
        self.assertEqual(
            'patman: ValueError: 2 commit(s) are unmarked; '
            'please use -m or -M', last_line)

    def check_series_unmark(self):
        """Checker for unmarking tests"""
        cser = self.get_cser()
        with self.stage('unmarked commits'):
            yield cser

        with self.stage('mark commits'):
            with terminal.capture() as (out, _):
                yield cser

        with self.stage('unmark: dry run'):
            with terminal.capture() as (out, _):
                yield cser

        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            "Unmarking series 'first': allow_unmarked False",
            next(itr))
        self.assertRegex(
            next(itr), 'Checking out upstream commit refs/heads/base: .*')
        self.assertEqual("Processing 2 commits from branch 'first'",
                         next(itr))
        self.assertRegex(
            next(itr),
            f'- unmarked: {HASH_RE} as {HASH_RE} i2c: I2C things')
        self.assertRegex(
            next(itr),
            f'- unmarked: {HASH_RE} as {HASH_RE} spi: SPI fixes')
        self.assertRegex(
            next(itr), f'Updating branch first from {HASH_RE} to {HASH_RE}')
        self.assertEqual('Dry run completed', next(itr))

        with self.stage('unmark'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertIn('- unmarked', out.getvalue())

        with self.stage('unmark: allow unmarked'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertIn('- no mark', out.getvalue())

        yield None

    def test_series_unmark(self):
        """Test unmarking a cseries, i.e. removing Change-Id fields"""
        cor = self.check_series_unmark()
        cser = next(cor)

        # check the allow_unmarked flag
        with terminal.capture():
            with self.assertRaises(ValueError) as exc:
                cser.unmark('first', dry_run=True)
        self.assertEqual('Unmarked commits 2/2', str(exc.exception))

        # mark commits
        cser = next(cor)
        cser.add('first', '', mark=True)

        # unmark: dry run
        cser = next(cor)
        cser.unmark('first', dry_run=True)

        # unmark
        cser = next(cor)
        cser.unmark('first')

        # unmark: allow unmarked
        cser = next(cor)
        cser.unmark('first', allow_unmarked=True)

        self.assertFalse(next(cor))

    def test_series_unmark_cmdline(self):
        """Test the unmark command"""
        cor = self.check_series_unmark()
        next(cor)

        # check the allow_unmarked flag
        with terminal.capture() as (out, _):
            self.run_args('series', 'unmark', expect_ret=1, pwork=True)
        self.assertIn('Unmarked commits 2/2', out.getvalue())

        # mark commits
        next(cor)
        self.run_args('series', '-s', 'first', 'add',  '-D', '', '--mark',
                      pwork=True)

        # unmark: dry run
        next(cor)
        self.run_args('series', '-s', 'first', '-n', 'unmark', pwork=True)

        # unmark
        next(cor)
        self.run_args('series', '-s', 'first', 'unmark', pwork=True)

        # unmark: allow unmarked
        next(cor)
        self.run_args('series', '-s', 'first', 'unmark', '--allow-unmarked',
                      pwork=True)

        self.assertFalse(next(cor))

    def test_series_unmark_middle(self):
        """Test unmarking with Change-Id fields not last in the commit"""
        cser = self.get_cser()
        with terminal.capture():
            cser.add('first', '', allow_unmarked=True)

        # Add some change IDs in the middle of the commit message
        with terminal.capture():
            name, ser, _, _ = cser.prep_series('first')
            old_msgs = []
            for vals in cser.process_series(name, ser):
                old_msgs.append(vals.msg)
                lines = vals.msg.splitlines()
                change_id = cser.make_change_id(vals.commit)
                extra = [f'{cser_helper.CHANGE_ID_TAG}: {change_id}']
                vals.msg = '\n'.join(lines[:2] + extra + lines[2:]) + '\n'

        with terminal.capture():
            cser.unmark('first')

        # We should get back the original commit message
        series = patchstream.get_metadata('first', 0, 2, git_dir=self.gitdir)
        self.assertEqual(old_msgs[0], series.commits[0].msg)
        self.assertEqual(old_msgs[1], series.commits[1].msg)

    def check_series_mark(self):
        """Checker for marking tests"""
        cser = self.get_cser()
        yield cser

        # Start with a dry run, which should do nothing
        with self.stage('dry run'):
            with terminal.capture():
                yield cser

            series = patchstream.get_metadata_for_list('first', self.gitdir, 2)
            self.assertEqual(2, len(series.commits))
            self.assertFalse(series.commits[0].change_id)
            self.assertFalse(series.commits[1].change_id)

        # Now do a real run
        with self.stage('real run'):
            with terminal.capture():
                yield cser

            series = patchstream.get_metadata_for_list('first', self.gitdir, 2)
            self.assertEqual(2, len(series.commits))
            self.assertTrue(series.commits[0].change_id)
            self.assertTrue(series.commits[1].change_id)

        # Try to mark again, which should fail
        with self.stage('mark twice'):
            with terminal.capture():
                with self.assertRaises(ValueError) as exc:
                    cser.mark('first', dry_run=False)
            self.assertEqual('Marked commits 2/2', str(exc.exception))

        # Use the --marked flag to make it succeed
        with self.stage('mark twice with --marked'):
            with terminal.capture():
                yield cser
            self.assertEqual('Marked commits 2/2', str(exc.exception))

            series2 = patchstream.get_metadata_for_list('first', self.gitdir,
                                                        2)
            self.assertEqual(2, len(series2.commits))
            self.assertEqual(series.commits[0].change_id,
                             series2.commits[0].change_id)
            self.assertEqual(series.commits[1].change_id,
                             series2.commits[1].change_id)

        yield None

    def test_series_mark(self):
        """Test marking a cseries, i.e. adding Change-Id fields"""
        cor = self.check_series_mark()
        cser = next(cor)

        # Start with a dry run, which should do nothing
        cser = next(cor)
        cser.mark('first', dry_run=True)

        # Now do a real run
        cser = next(cor)
        cser.mark('first', dry_run=False)

        # Try to mark again, which should fail
        with terminal.capture():
            with self.assertRaises(ValueError) as exc:
                cser.mark('first', dry_run=False)
        self.assertEqual('Marked commits 2/2', str(exc.exception))

        # Use the --allow-marked flag to make it succeed
        cser = next(cor)
        cser.mark('first', allow_marked=True, dry_run=False)

        self.assertFalse(next(cor))

    def test_series_mark_cmdline(self):
        """Test marking a cseries, i.e. adding Change-Id fields"""
        cor = self.check_series_mark()
        next(cor)

        # Start with a dry run, which should do nothing
        next(cor)
        self.run_args('series', '-n', '-s', 'first', 'mark', pwork=True)

        # Now do a real run
        next(cor)
        self.run_args('series', '-s', 'first', 'mark', pwork=True)

        # Try to mark again, which should fail
        with terminal.capture() as (out, _):
            self.run_args('series', '-s', 'first', 'mark', expect_ret=1,
                          pwork=True)
        self.assertIn('Marked commits 2/2', out.getvalue())

        # Use the --allow-marked flag to make it succeed
        next(cor)
        self.run_args('series', '-s', 'first', 'mark', '--allow-marked',
                      pwork=True)
        self.assertFalse(next(cor))

    def test_series_remove(self):
        """Test removing a series"""
        cser = self.get_cser()

        with self.stage('remove non-existent series'):
            with self.assertRaises(ValueError) as exc:
                cser.remove('first')
            self.assertEqual("No such series 'first'", str(exc.exception))

        with self.stage('add'):
            with terminal.capture() as (out, _):
                cser.add('first', '', mark=True)
            self.assertTrue(cser.db.series_get_dict())
            pclist = cser.get_pcommit_dict()
            self.assertEqual(2, len(pclist))

        with self.stage('remove'):
            with terminal.capture() as (out, _):
                cser.remove('first')
            self.assertEqual("Removed series 'first'", out.getvalue().strip())
            self.assertFalse(cser.db.series_get_dict())

            pclist = cser.get_pcommit_dict()
            self.assertFalse(len(pclist))

    def test_series_remove_cmdline(self):
        """Test removing a series using the command line"""
        cser = self.get_cser()

        with self.stage('remove non-existent series'):
            with terminal.capture() as (out, _):
                self.run_args('series', '-s', 'first', 'rm', expect_ret=1,
                              pwork=True)
            self.assertEqual("patman: ValueError: No such series 'first'",
                             out.getvalue().strip())

        with self.stage('add'):
            with terminal.capture() as (out, _):
                cser.add('first', '', mark=True)
            self.assertTrue(cser.db.series_get_dict())

        with self.stage('remove'):
            with terminal.capture() as (out, _):
                cser.remove('first')
            self.assertEqual("Removed series 'first'", out.getvalue().strip())
            self.assertFalse(cser.db.series_get_dict())

    def check_series_remove_multiple(self):
        """Check for removing a series with more than one version"""
        cser = self.get_cser()

        with self.stage('setup'):
            self.add_first2(True)

            with terminal.capture() as (out, _):
                cser.add(None, '', mark=True)
                cser.add('first', '', mark=True)
            self.assertTrue(cser.db.series_get_dict())
            pclist = cser.get_pcommit_dict()
            self.assertEqual(4, len(pclist))

        # Do a dry-run removal
        with self.stage('dry run'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertEqual("Removed version 1 from series 'first'\n"
                             'Dry run completed', out.getvalue().strip())
            self.assertEqual({'first'}, cser.db.series_get_dict().keys())

            svlist = cser.get_ser_ver_list()
            self.assertEqual(2, len(svlist))
            self.assertEqual(1, svlist[0].idnum)
            self.assertEqual(1, svlist[0].series_id)
            self.assertEqual(2, svlist[0].version)

            self.assertEqual(2, svlist[1].idnum)
            self.assertEqual(1, svlist[1].series_id)
            self.assertEqual(1, svlist[1].version)

        # Now remove for real
        with self.stage('real'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertEqual("Removed version 1 from series 'first'",
                             out.getvalue().strip())
            self.assertEqual({'first'}, cser.db.series_get_dict().keys())
            plist = cser.get_ser_ver_list()
            self.assertEqual(1, len(plist))
            pclist = cser.get_pcommit_dict()
            self.assertEqual(2, len(pclist))

        with self.stage('remove only version'):
            yield cser
            self.assertEqual({'first'}, cser.db.series_get_dict().keys())

            svlist = cser.get_ser_ver_list()
            self.assertEqual(1, len(svlist))
            self.assertEqual(1, svlist[0].idnum)
            self.assertEqual(1, svlist[0].series_id)
            self.assertEqual(2, svlist[0].version)

        with self.stage('remove series (dry run'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertEqual("Removed series 'first'\nDry run completed",
                             out.getvalue().strip())
            self.assertTrue(cser.db.series_get_dict())
            self.assertTrue(cser.get_ser_ver_list())

        with self.stage('remove series'):
            with terminal.capture() as (out, _):
                yield cser
            self.assertEqual("Removed series 'first'", out.getvalue().strip())
            self.assertFalse(cser.db.series_get_dict())
            self.assertFalse(cser.get_ser_ver_list())

        yield False

    def test_series_remove_multiple(self):
        """Test removing a series with more than one version"""
        cor = self.check_series_remove_multiple()
        cser = next(cor)

        # Do a dry-run removal
        cser.version_remove('first', 1, dry_run=True)
        cser = next(cor)

        # Now remove for real
        cser.version_remove('first', 1)
        cser = next(cor)

        # Remove only version
        with self.assertRaises(ValueError) as exc:
            cser.version_remove('first', 2, dry_run=True)
        self.assertEqual(
            "Series 'first' only has one version: remove the series",
            str(exc.exception))
        cser = next(cor)

        # Remove series (dry run)
        cser.remove('first', dry_run=True)
        cser = next(cor)

        # Remove series (real)
        cser.remove('first')

        self.assertFalse(next(cor))
        cor.close()

    def test_series_remove_multiple_cmdline(self):
        """Test removing a series with more than one version on cmdline"""
        cor = self.check_series_remove_multiple()
        next(cor)

        # Do a dry-run removal
        self.run_args('series', '-n', '-s', 'first', '-V', '1', 'rm-version',
                      pwork=True)
        next(cor)

        # Now remove for real
        self.run_args('series', '-s', 'first', '-V', '1', 'rm-version',
                      pwork=True)
        next(cor)

        # Remove only version
        with terminal.capture() as (out, _):
            self.run_args('series', '-n', '-s', 'first', '-V', '2',
                          'rm-version', expect_ret=1, pwork=True)
        self.assertIn(
            "Series 'first' only has one version: remove the series",
            out.getvalue().strip())
        next(cor)

        # Remove series (dry run)
        self.run_args('series', '-n', '-s', 'first', 'rm', pwork=True)
        next(cor)

        # Remove series (real)
        self.run_args('series', '-s', 'first', 'rm', pwork=True)

        self.assertFalse(next(cor))
        cor.close()

    def test_patchwork_set_project(self):
        """Test setting the project ID"""
        cser = self.get_cser()
        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        with terminal.capture() as (out, _):
            cser.project_set(pwork, 'U-Boot')
        self.assertEqual(
            f"Project 'U-Boot' patchwork-ID {self.PROJ_ID} link-name uboot",
            out.getvalue().strip())

    def test_patchwork_project_get(self):
        """Test setting the project ID"""
        cser = self.get_cser()
        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        self.assertFalse(cser.project_get())
        with terminal.capture() as (out, _):
            cser.project_set(pwork, 'U-Boot')
        self.assertEqual(
            f"Project 'U-Boot' patchwork-ID {self.PROJ_ID} link-name uboot",
            out.getvalue().strip())

        name, pwid, link_name = cser.project_get()
        self.assertEqual('U-Boot', name)
        self.assertEqual(self.PROJ_ID, pwid)
        self.assertEqual('uboot', link_name)

    def test_patchwork_project_get_cmdline(self):
        """Test setting the project ID"""
        cser = self.get_cser()

        self.assertFalse(cser.project_get())

        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        with terminal.capture() as (out, _):
            self.run_args('-P', 'https://url', 'patchwork', 'set-project',
                          'U-Boot', pwork=pwork)
        self.assertEqual(
            f"Project 'U-Boot' patchwork-ID {self.PROJ_ID} link-name uboot",
            out.getvalue().strip())

        name, pwid, link_name = cser.project_get()
        self.assertEqual('U-Boot', name)
        self.assertEqual(6, pwid)
        self.assertEqual('uboot', link_name)

        with terminal.capture() as (out, _):
            self.run_args('-P', 'https://url', 'patchwork', 'get-project')
        self.assertEqual(
            f"Project 'U-Boot' patchwork-ID {self.PROJ_ID} link-name uboot",
            out.getvalue().strip())

    def check_series_list_patches(self):
        """Test listing the patches for a series"""
        cser = self.get_cser()

        with self.stage('setup'):
            with terminal.capture() as (out, _):
                cser.add(None, '', allow_unmarked=True)
                cser.add('second', allow_unmarked=True)
                target = self.repo.lookup_reference('refs/heads/second')
                self.repo.checkout(
                    target, strategy=pygit2.enums.CheckoutStrategy.FORCE)
                cser.increment('second')

        with self.stage('list first'):
            with terminal.capture() as (out, _):
                yield cser
            itr = iter(out.getvalue().splitlines())
            self.assertEqual("Branch 'first' (total 2): 2:unknown", next(itr))
            self.assertIn('PatchId', next(itr))
            self.assertRegex(next(itr), r'  0 .* i2c: I2C things')
            self.assertRegex(next(itr), r'  1 .* spi: SPI fixes')

        with self.stage('list second2'):
            with terminal.capture() as (out, _):
                yield cser
            itr = iter(out.getvalue().splitlines())
            self.assertEqual(
                "Branch 'second2' (total 3): 3:unknown", next(itr))
            self.assertIn('PatchId', next(itr))
            self.assertRegex(
                next(itr), '  0 .* video: Some video improvements')
            self.assertRegex(next(itr), '  1 .* serial: Add a serial driver')
            self.assertRegex(next(itr), '  2 .* bootm: Make it boot')

        yield None

    def test_series_list_patches(self):
        """Test listing the patches for a series"""
        cor = self.check_series_list_patches()
        cser = next(cor)

        # list first
        cser.list_patches('first', 1)
        cser = next(cor)

        # list second2
        cser.list_patches('second2', 2)
        self.assertFalse(next(cor))
        cor.close()

    def test_series_list_patches_cmdline(self):
        """Test listing the patches for a series using the cmdline"""
        cor = self.check_series_list_patches()
        next(cor)

        # list first
        self.run_args('series',  '-s', 'first', 'patches', pwork=True)
        next(cor)

        # list second2
        self.run_args('series',  '-s', 'second', '-V', '2', 'patches',
                      pwork=True)
        self.assertFalse(next(cor))
        cor.close()

    def test_series_list_patches_detail(self):
        """Test listing the patches for a series"""
        cser = self.get_cser()
        with terminal.capture():
            cser.add(None, '', allow_unmarked=True)
            cser.add('second', allow_unmarked=True)
            target = self.repo.lookup_reference('refs/heads/second')
            self.repo.checkout(
                target, strategy=pygit2.enums.CheckoutStrategy.FORCE)
            cser.increment('second')

        with terminal.capture() as (out, _):
            cser.list_patches('first', 1, show_commit=True)
        expect = r'''Branch 'first' (total 2): 2:unknown
Seq State      Com PatchId Commit     Subject
  0 unknown      -         .* i2c: I2C things

commit .*
Author: Test user <test@email.com>
Date:   .*

    i2c: I2C things

    This has some stuff to do with I2C

 i2c.c | 2 ++
 1 file changed, 2 insertions(+)


  1 unknown      -         .* spi: SPI fixes

commit .*
Author: Test user <test@email.com>
Date:   .*

    spi: SPI fixes

    SPI needs some fixes
    and here they are

    Signed-off-by: Lord Edmund Blackaddër <weasel@blackadder.org>

    Series-to: u-boot
    Commit-notes:
    title of the series
    This is the cover letter for the series
    with various details
    END

 spi.c | 3 +++
 1 file changed, 3 insertions(+)
'''
        itr = iter(out.getvalue().splitlines())
        for seq, eline in enumerate(expect.splitlines()):
            line = next(itr).rstrip()
            if '*' in eline:
                self.assertRegex(line, eline, f'line {seq + 1}')
            else:
                self.assertEqual(eline, line, f'line {seq + 1}')

        # Show just the patch; this should exclude the commit message
        with terminal.capture() as (out, _):
            cser.list_patches('first', 1, show_patch=True)
        chk = out.getvalue()
        self.assertIn('SPI fixes', chk)                 # subject
        self.assertNotIn('SPI needs some fixes', chk)   # commit body
        self.assertIn('make SPI work', chk)             # patch body

        # Show both
        with terminal.capture() as (out, _):
            cser.list_patches('first', 1, show_commit=True, show_patch=True)
        chk = out.getvalue()
        self.assertIn('SPI fixes', chk)                 # subject
        self.assertIn('SPI needs some fixes', chk)   # commit body
        self.assertIn('make SPI work', chk)             # patch body

    def check_series_gather(self):
        """Checker for gathering tags for a series"""
        cser = self.get_cser()
        with self.stage('setup'):
            pwork = Patchwork.for_testing(self._fake_patchwork_cser)
            self.assertFalse(cser.project_get())
            cser.project_set(pwork, 'U-Boot', quiet=True)

            with terminal.capture() as (out, _):
                cser.add('second', 'description', allow_unmarked=True)

            ser = cser.get_series_by_name('second')
            pwid = cser.get_series_svid(ser.idnum, 1)

        # First do a dry run
        with self.stage('gather: dry run'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            lines = out.getvalue().splitlines()
            self.assertEqual(
                f"Updating series 'second' version 1 from link "
                f"'{self.SERIES_ID_SECOND_V1}'",
                lines[0])
            self.assertEqual('3 patches updated (7 requests)', lines[1])
            self.assertEqual('Dry run completed', lines[2])
            self.assertEqual(3, len(lines))

            pwc = cser.get_pcommit_dict(pwid)
            self.assertIsNone(pwc[0].state)
            self.assertIsNone(pwc[1].state)
            self.assertIsNone(pwc[2].state)

        # Now try it again, gathering tags
        with self.stage('gather: dry run'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            lines = out.getvalue().splitlines()
            itr = iter(lines)
            self.assertEqual(
                f"Updating series 'second' version 1 from link "
                f"'{self.SERIES_ID_SECOND_V1}'",
                next(itr))
            self.assertEqual('  1 video: Some video improvements', next(itr))
            self.assertEqual('  + Reviewed-by: Fred Bloggs <fred@bloggs.com>',
                             next(itr))
            self.assertEqual('  2 serial: Add a serial driver', next(itr))
            self.assertEqual('  3 bootm: Make it boot', next(itr))

            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual("Processing 3 commits from branch 'second'",
                             next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:       {HASH_RE} as {HASH_RE} '
                'video: Some video improvements')
            self.assertRegex(
                next(itr),
                f"- upd links '1:456': {HASH_RE} as {HASH_RE} "
                'serial: Add a serial driver')
            self.assertRegex(
                next(itr),
                f'-                    {HASH_RE} as {HASH_RE} '
                'bootm: Make it boot')
            self.assertRegex(
                next(itr),
                f'Updating branch second from {HASH_RE} to {HASH_RE}')
            self.assertEqual('3 patches updated (7 requests)', next(itr))
            self.assertEqual('Dry run completed', next(itr))
            self.assert_finished(itr)

            # Make sure that no tags were added to the branch
            series = patchstream.get_metadata_for_list('second', self.gitdir,
                                                       3)
            for cmt in series.commits:
                self.assertFalse(cmt.rtags,
                                 'Commit {cmt.subject} rtags {cmt.rtags}')

        # Now do it for real
        with self.stage('gather: real'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            lines2 = out.getvalue().splitlines()
            self.assertEqual(lines2, lines[:-1])

            # Make sure that the tags were added to the branch
            series = patchstream.get_metadata_for_list('second', self.gitdir,
                                                       3)
            self.assertEqual(
                {'Reviewed-by': {'Fred Bloggs <fred@bloggs.com>'}},
                series.commits[0].rtags)
            self.assertFalse(series.commits[1].rtags)
            self.assertFalse(series.commits[2].rtags)

            # Make sure the status was updated
            pwc = cser.get_pcommit_dict(pwid)
            self.assertEqual('accepted', pwc[0].state)
            self.assertEqual('changes-requested', pwc[1].state)
            self.assertEqual('rejected', pwc[2].state)

        yield None

    def test_series_gather(self):
        """Test gathering tags for a series"""
        cor = self.check_series_gather()
        cser, pwork = next(cor)

        # sync (dry_run)
        cser.gather(pwork, 'second', None, False, False, False, dry_run=True)
        cser, pwork = next(cor)

        # gather (dry_run)
        cser.gather(pwork, 'second', None, False, False, True, dry_run=True)
        cser, pwork = next(cor)

        # gather (real)
        cser.gather(pwork, 'second', None, False, False, True)

        self.assertFalse(next(cor))

    def test_series_gather_cmdline(self):
        """Test gathering tags for a series with cmdline"""
        cor = self.check_series_gather()
        _, pwork = next(cor)

        # sync (dry_run)
        self.run_args(
            'series', '-n', '-s', 'second', 'gather', '-G', pwork=pwork)

        # gather (dry_run)
        _, pwork = next(cor)
        self.run_args('series', '-n', '-s', 'second', 'gather', pwork=pwork)

        # gather (real)
        _, pwork = next(cor)
        self.run_args('series', '-s', 'second', 'gather', pwork=pwork)

        self.assertFalse(next(cor))

    def check_series_gather_all(self):
        """Gather all series at once"""
        with self.stage('setup'):
            cser, pwork = self.setup_second(False)

            with terminal.capture():
                cser.add('first', 'description', allow_unmarked=True)
                cser.increment('first')
                cser.increment('first')
                cser.link_set('first', 1, '123', True)
                cser.link_set('first', 2, '1234', True)
                cser.link_set('first', 3, f'{self.SERIES_ID_FIRST_V3}', True)
                cser.link_auto(pwork, 'second', 2, True)

        with self.stage('no options'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            self.assertEqual(
                "Syncing 'first' v3\n"
                "Syncing 'second' v2\n"
                '\n'
                '5 patches and 2 cover letters updated, 0 missing links '
                '(14 requests)\n'
                'Dry run completed',
                out.getvalue().strip())

        with self.stage('gather'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            lines = out.getvalue().splitlines()
            itr = iter(lines)
            self.assertEqual("Syncing 'first' v3", next(itr))
            self.assertEqual('  1 i2c: I2C things', next(itr))
            self.assertEqual(
                '  + Tested-by: Mary Smith <msmith@wibble.com>   # yak',
                next(itr))
            self.assertEqual('  2 spi: SPI fixes', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 2 commits from branch 'first3'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:      {HASH_RE} as {HASH_RE} i2c: I2C things')
            self.assertRegex(
                next(itr),
                f"- upd links '3:31': {HASH_RE} as {HASH_RE} spi: SPI fixes")
            self.assertRegex(
                next(itr),
                f'Updating branch first3 from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))

            self.assertEqual("Syncing 'second' v2", next(itr))
            self.assertEqual('  1 video: Some video improvements', next(itr))
            self.assertEqual(
                '  + Reviewed-by: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual('  2 serial: Add a serial driver', next(itr))
            self.assertEqual('  3 bootm: Make it boot', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 3 commits from branch 'second2'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:             {HASH_RE} as {HASH_RE} '
                'video: Some video improvements')
            self.assertRegex(
                next(itr),
                f"- upd links '2:457 1:456': {HASH_RE} as {HASH_RE} "
                'serial: Add a serial driver')
            self.assertRegex(
                next(itr),
                f'-                          {HASH_RE} as {HASH_RE} '
                'bootm: Make it boot')
            self.assertRegex(
                next(itr),
                f'Updating branch second2 from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))
            self.assertEqual(
                '5 patches and 2 cover letters updated, 0 missing links '
                '(14 requests)',
                next(itr))
            self.assertEqual('Dry run completed', next(itr))
            self.assert_finished(itr)

        with self.stage('gather, patch comments,!dry_run'):
            with terminal.capture() as (out, _):
                yield cser, pwork
            lines = out.getvalue().splitlines()
            itr = iter(lines)
            self.assertEqual("Syncing 'first' v1", next(itr))
            self.assertEqual('  1 i2c: I2C things', next(itr))
            self.assertEqual(
                '  + Tested-by: Mary Smith <msmith@wibble.com>   # yak',
                next(itr))
            self.assertEqual('  2 spi: SPI fixes', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 2 commits from branch 'first'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:       {HASH_RE} as {HASH_RE} i2c: I2C things')
            self.assertRegex(
                next(itr),
                f"- upd links '1:123': {HASH_RE} as {HASH_RE} spi: SPI fixes")
            self.assertRegex(
                next(itr),
                f'Updating branch first from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))

            self.assertEqual("Syncing 'first' v2", next(itr))
            self.assertEqual('  1 i2c: I2C things', next(itr))
            self.assertEqual(
                '  + Tested-by: Mary Smith <msmith@wibble.com>   # yak',
                next(itr))
            self.assertEqual('  2 spi: SPI fixes', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 2 commits from branch 'first2'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:        {HASH_RE} as {HASH_RE} '
                'i2c: I2C things')
            self.assertRegex(
                next(itr),
                f"- upd links '2:1234': {HASH_RE} as {HASH_RE} spi: SPI fixes")
            self.assertRegex(
                next(itr),
                f'Updating branch first2 from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))
            self.assertEqual("Syncing 'first' v3", next(itr))
            self.assertEqual('  1 i2c: I2C things', next(itr))
            self.assertEqual(
                '  + Tested-by: Mary Smith <msmith@wibble.com>   # yak',
                next(itr))
            self.assertEqual('  2 spi: SPI fixes', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 2 commits from branch 'first3'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:      {HASH_RE} as {HASH_RE} i2c: I2C things')
            self.assertRegex(
                next(itr),
                f"- upd links '3:31': {HASH_RE} as {HASH_RE} spi: SPI fixes")
            self.assertRegex(
                next(itr),
                f'Updating branch first3 from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))

            self.assertEqual("Syncing 'second' v1", next(itr))
            self.assertEqual('  1 video: Some video improvements', next(itr))
            self.assertEqual(
                '  + Reviewed-by: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual(
                'Review: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual('    > This was my original patch', next(itr))
            self.assertEqual('    > which is being quoted', next(itr))
            self.assertEqual(
                '    I like the approach here and I would love to see more '
                'of it.', next(itr))
            self.assertEqual('', next(itr))
            self.assertEqual('  2 serial: Add a serial driver', next(itr))
            self.assertEqual('  3 bootm: Make it boot', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 3 commits from branch 'second'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:       {HASH_RE} as {HASH_RE} '
                'video: Some video improvements')
            self.assertRegex(
                next(itr),
                f"- upd links '1:456': {HASH_RE} as {HASH_RE} "
                'serial: Add a serial driver')
            self.assertRegex(
                next(itr),
                f'-                    {HASH_RE} as {HASH_RE} '
                'bootm: Make it boot')
            self.assertRegex(
                next(itr),
                f'Updating branch second from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))

            self.assertEqual("Syncing 'second' v2", next(itr))
            self.assertEqual('  1 video: Some video improvements', next(itr))
            self.assertEqual(
                '  + Reviewed-by: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual(
                'Review: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual('    > This was my original patch', next(itr))
            self.assertEqual('    > which is being quoted', next(itr))
            self.assertEqual(
                '    I like the approach here and I would love to see more '
                'of it.', next(itr))
            self.assertEqual('', next(itr))
            self.assertEqual('  2 serial: Add a serial driver', next(itr))
            self.assertEqual('  3 bootm: Make it boot', next(itr))
            self.assertRegex(
                next(itr), 'Checking out upstream commit refs/heads/base: .*')
            self.assertEqual(
                "Processing 3 commits from branch 'second2'", next(itr))
            self.assertRegex(
                next(itr),
                f'- added 1 tag:             {HASH_RE} as {HASH_RE} '
                'video: Some video improvements')
            self.assertRegex(
                next(itr),
                f"- upd links '2:457 1:456': {HASH_RE} as {HASH_RE} "
                'serial: Add a serial driver')
            self.assertRegex(
                next(itr),
                f'-                          {HASH_RE} as {HASH_RE} '
                'bootm: Make it boot')
            self.assertRegex(
                next(itr),
                f'Updating branch second2 from {HASH_RE} to {HASH_RE}')
            self.assertEqual('', next(itr))
            self.assertEqual(
                '12 patches and 3 cover letters updated, 0 missing links '
                '(32 requests)', next(itr))
            self.assert_finished(itr)

        yield None

    def test_series_gather_all(self):
        """Gather all series at once"""
        cor = self.check_series_gather_all()
        cser, pwork = next(cor)

        # no options
        cser.gather_all(pwork, False, True, False, False, dry_run=True)
        cser, pwork = next(cor)

        # gather
        cser.gather_all(pwork, False, False, False, True, dry_run=True)
        cser, pwork = next(cor)

        # gather, patch comments, !dry_run
        cser.gather_all(pwork, True, False, True, True)

        self.assertFalse(next(cor))

    def test_series_gather_all_cmdline(self):
        """Sync all series at once using cmdline"""
        cor = self.check_series_gather_all()
        _, pwork = next(cor)

        # no options
        self.run_args('series', '-n', '-s', 'second', 'gather-all', '-G',
                      pwork=pwork)
        _, pwork = next(cor)

        # gather
        self.run_args('series', '-n', '-s', 'second', 'gather-all',
                      pwork=pwork)
        _, pwork = next(cor)

        # gather, patch comments, !dry_run
        self.run_args('series',  '-s', 'second', 'gather-all', '-a', '-c',
                      pwork=pwork)

        self.assertFalse(next(cor))

    def _check_second(self, itr, show_all):
        """Check output from a 'progress' command

        Args:
            itr (Iterator): Contains the output lines to check
            show_all (bool): True if all versions are being shown, not just
                latest
        """
        self.assertEqual('second: Series for my board (versions: 1 2)',
                         next(itr))
        if show_all:
            self.assertEqual("Branch 'second' (total 3): 3:unknown",
                             next(itr))
            self.assertIn('PatchId', next(itr))
            self.assertRegex(
                next(itr),
                '  0 unknown      -         .* video: Some video improvements')
            self.assertRegex(
                next(itr),
                '  1 unknown      -         .* serial: Add a serial driver')
            self.assertRegex(
                next(itr),
                '  2 unknown      -         .* bootm: Make it boot')
            self.assertEqual('', next(itr))
        self.assertEqual(
            "Branch 'second2' (total 3): 1:accepted 1:changes 1:rejected",
            next(itr))
        self.assertIn('PatchId', next(itr))
        self.assertEqual(
            'Cov              2     139            '
            'The name of the cover letter', next(itr))
        self.assertRegex(
            next(itr),
            '  0 accepted     2     110 .* video: Some video improvements')
        self.assertRegex(
            next(itr),
            '  1 changes            111 .* serial: Add a serial driver')
        self.assertRegex(
            next(itr),
            '  2 rejected     3     112 .* bootm: Make it boot')

    def test_series_progress(self):
        """Test showing progress for a cseries"""
        self.setup_second()
        self.db_close()

        with self.stage('latest versions'):
            args = Namespace(subcmd='progress', series='second',
                             show_all_versions=False, list_patches=True)
            with terminal.capture() as (out, _):
                control.do_series(args, test_db=self.tmpdir, pwork=True)
            lines = iter(out.getvalue().splitlines())
            self._check_second(lines, False)

        with self.stage('all versions'):
            args.show_all_versions = True
            with terminal.capture() as (out, _):
                control.do_series(args, test_db=self.tmpdir, pwork=True)
            lines = iter(out.getvalue().splitlines())
            self._check_second(lines, True)

    def _check_first(self, itr):
        """Check output from the progress command

        Args:
            itr (Iterator): Contains the output lines to check
        """
        self.assertEqual('first:  (versions: 1)', next(itr))
        self.assertEqual("Branch 'first' (total 2): 2:unknown", next(itr))
        self.assertIn('PatchId', next(itr))
        self.assertRegex(
            next(itr),
            '  0 unknown      -        .* i2c: I2C things')
        self.assertRegex(
            next(itr),
            '  1 unknown      -        .* spi: SPI fixes')
        self.assertEqual('', next(itr))

    def test_series_progress_all(self):
        """Test showing progress for all cseries"""
        self.setup_second()
        self.db_close()

        with self.stage('progress with patches'):
            args = Namespace(subcmd='progress', series=None,
                             show_all_versions=False, list_patches=True)
            with terminal.capture() as (out, _):
                control.do_series(args, test_db=self.tmpdir, pwork=True)
            lines = iter(out.getvalue().splitlines())
            self._check_first(lines)
            self._check_second(lines, False)

        with self.stage('all versions'):
            args.show_all_versions = True
            with terminal.capture() as (out, _):
                control.do_series(args, test_db=self.tmpdir, pwork=True)
            lines = iter(out.getvalue().splitlines())
            self._check_first(lines)
            self._check_second(lines, True)

    def test_series_progress_no_patches(self):
        """Test showing progress for all cseries without patches"""
        self.setup_second()

        with terminal.capture() as (out, _):
            self.run_args('series', 'progress', pwork=True)
        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            'Name             Description                               '
            'Count  Status', next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assertEqual(
            'first                                                      '
            '    2  2:unknown', next(itr))
        self.assertEqual(
            'second2          The name of the cover letter              '
            '    3  1:accepted 1:changes 1:rejected', next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assertEqual(
            ['2', 'series', '5', '2:unknown', '1:accepted', '1:changes',
             '1:rejected'],
            next(itr).split())
        self.assert_finished(itr)

    def test_series_progress_all_no_patches(self):
        """Test showing progress for all cseries versions without patches"""
        self.setup_second()

        with terminal.capture() as (out, _):
            self.run_args('series', 'progress', '--show-all-versions',
                          pwork=True)
        itr = iter(out.getvalue().splitlines())
        self.assertEqual(
            'Name             Description                               '
            'Count  Status', next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assertEqual(
            'first                                                      '
            '    2  2:unknown', next(itr))
        self.assertEqual(
            'second           Series for my board                       '
            '    3  3:unknown', next(itr))
        self.assertEqual(
            'second2          The name of the cover letter              '
            '    3  1:accepted 1:changes 1:rejected', next(itr))
        self.assertTrue(next(itr).startswith('--'))
        self.assertEqual(
            ['3', 'series', '8', '5:unknown', '1:accepted', '1:changes',
             '1:rejected'],
            next(itr).split())
        self.assert_finished(itr)

    def test_series_summary(self):
        """Test showing a summary of series status"""
        self.setup_second()

        self.db_close()
        args = Namespace(subcmd='summary', series=None)
        with terminal.capture() as (out, _):
            control.do_series(args, test_db=self.tmpdir, pwork=True)
        lines = out.getvalue().splitlines()
        self.assertEqual(
            'Name               Status  Description',
            lines[0])
        self.assertEqual(
            '-----------------  ------  ------------------------------',
            lines[1])
        self.assertEqual('first          -/2  ', lines[2])
        self.assertEqual('second         1/3  Series for my board', lines[3])

    def test_series_open(self):
        """Test opening a series in a web browser"""
        cser = self.get_cser()
        pwork = Patchwork.for_testing(self._fake_patchwork_cser)
        self.assertFalse(cser.project_get())
        pwork.project_set(self.PROJ_ID, self.PROJ_LINK_NAME)

        with terminal.capture():
            cser.add('second', allow_unmarked=True)
            cser.increment('second')
            cser.link_auto(pwork, 'second', 2, True)
            cser.gather(pwork, 'second', 2, False, False, False)

        with mock.patch.object(cros_subprocess.Popen, '__init__',
                               return_value=None) as method:
            with terminal.capture() as (out, _):
                cser.open(pwork, 'second2', 2)

        url = ('https://patchwork.ozlabs.org/project/uboot/list/?series=457'
               '&state=*&archive=both')
        method.assert_called_once_with(['xdg-open', url])
        self.assertEqual(f'Opening {url}', out.getvalue().strip())

    def test_name_version(self):
        """Test handling of series names and versions"""
        cser = self.get_cser()
        repo = self.repo

        self.assertEqual(('fred', None),
                         cser_helper.split_name_version('fred'))
        self.assertEqual(('mary', 2), cser_helper.split_name_version('mary2'))

        ser, version = cser._parse_series_and_version(None, None)
        self.assertEqual('first', ser.name)
        self.assertEqual(1, version)

        ser, version = cser._parse_series_and_version('first', None)
        self.assertEqual('first', ser.name)
        self.assertEqual(1, version)

        ser, version = cser._parse_series_and_version('first', 2)
        self.assertEqual('first', ser.name)
        self.assertEqual(2, version)

        with self.assertRaises(ValueError) as exc:
            cser._parse_series_and_version('123', 2)
        self.assertEqual(
            "Series name '123' cannot be a number, use '<name><version>'",
            str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            cser._parse_series_and_version('first', 100)
        self.assertEqual("Version 100 exceeds 99", str(exc.exception))

        with terminal.capture() as (_, err):
            cser._parse_series_and_version('mary3', 4)
        self.assertIn('Version mismatch: -V has 4 but branch name indicates 3',
                      err.getvalue())

        ser, version = cser._parse_series_and_version('mary', 4)
        self.assertEqual('mary', ser.name)
        self.assertEqual(4, version)

        # Move off the branch and check for a sensible error
        commit = repo.revparse_single('first~')
        repo.checkout_tree(commit)
        repo.set_head(commit.oid)

        with self.assertRaises(ValueError) as exc:
            cser._parse_series_and_version(None, None)
        self.assertEqual('No branch detected: please use -s <series>',
                         str(exc.exception))

    def test_name_version_extra(self):
        """More tests for some corner cases"""
        cser, _ = self.setup_second()
        target = self.repo.lookup_reference('refs/heads/second2')
        self.repo.checkout(
            target, strategy=pygit2.enums.CheckoutStrategy.FORCE)

        ser, version = cser._parse_series_and_version(None, None)
        self.assertEqual('second', ser.name)
        self.assertEqual(2, version)

        ser, version = cser._parse_series_and_version('second2', None)
        self.assertEqual('second', ser.name)
        self.assertEqual(2, version)

    def test_migrate(self):
        """Test migration to later schema versions"""
        db = database.Database(f'{self.tmpdir}/.patman.db')
        with terminal.capture() as (out, err):
            db.open_it()
        self.assertEqual(
            f'Creating new database {self.tmpdir}/.patman.db',
            err.getvalue().strip())

        self.assertEqual(0, db.get_schema_version())

        for version in range(1, database.LATEST + 1):
            with terminal.capture() as (out, _):
                db.migrate_to(version)
            self.assertTrue(os.path.exists(
                f'{self.tmpdir}/.patman.dbold.v{version - 1}'))
            self.assertEqual(f'Update database to v{version}',
                             out.getvalue().strip())
            self.assertEqual(version, db.get_schema_version())
        self.assertEqual(4, database.LATEST)

    def test_series_scan(self):
        """Test scanning a series for updates"""
        cser, _ = self.setup_second()
        target = self.repo.lookup_reference('refs/heads/second2')
        self.repo.checkout(
            target, strategy=pygit2.enums.CheckoutStrategy.FORCE)

        # Add a new commit
        self.repo = pygit2.init_repository(self.gitdir)
        self.make_commit_with_file(
            'wip: Try out a new thing', 'Just checking', 'wibble.c',
            '''changes to wibble''')
        target = self.repo.revparse_single('HEAD')
        self.repo.reset(target.oid, pygit2.enums.ResetMode.HARD)

        # name = gitutil.get_branch(self.gitdir)
        # upstream_name = gitutil.get_upstream(self.gitdir, name)
        name, ser, version, _ = cser.prep_series(None)

        # We now have 4 commits numbered 0 (second~3) to 3 (the one we just
        # added). Drop commit 1 (the 'serial' one) from the branch
        cser._filter_commits(name, ser, 1)
        svid = cser.get_ser_ver(ser.idnum, version).idnum
        old_pcdict = cser.get_pcommit_dict(svid).values()

        expect = '''Syncing series 'second2' v2: mark False allow_unmarked True
    0 video: Some video improvements
-   1 serial: Add a serial driver
    1 bootm: Make it boot
+   2 Just checking
'''
        with terminal.capture() as (out, _):
            self.run_args('series', '-n', 'scan', '-M', pwork=True)
        self.assertEqual(expect + 'Dry run completed\n', out.getvalue())

        new_pcdict = cser.get_pcommit_dict(svid).values()
        self.assertEqual(list(old_pcdict), list(new_pcdict))

        with terminal.capture() as (out, _):
            self.run_args('series', 'scan', '-M', pwork=True)
        self.assertEqual(expect, out.getvalue())

        new_pcdict = cser.get_pcommit_dict(svid).values()
        self.assertEqual(len(old_pcdict), len(new_pcdict))
        chk = list(new_pcdict)
        self.assertNotEqual(list(old_pcdict), list(new_pcdict))
        self.assertEqual('video: Some video improvements', chk[0].subject)
        self.assertEqual('bootm: Make it boot', chk[1].subject)
        self.assertEqual('Just checking', chk[2].subject)

    def test_series_send(self):
        """Test sending a series"""
        cser, pwork = self.setup_second()

        # Create a third version
        with terminal.capture():
            cser.increment('second')
        series = patchstream.get_metadata_for_list('second3', self.gitdir, 3)
        self.assertEqual('2:457 1:456', series.links)
        self.assertEqual('3', series.version)

        with terminal.capture() as (out, err):
            self.run_args('series', '-n', '-s', 'second3', 'send',
                          '--no-autolink', pwork=pwork)
        self.assertIn('Send a total of 3 patches with a cover letter',
                      out.getvalue())
        self.assertIn(
            'video.c:1: warning: Missing or malformed SPDX-License-Identifier '
            'tag in line 1', err.getvalue())
        self.assertIn(
            '<patch>:19: warning: added, moved or deleted file(s), does '
            'MAINTAINERS need updating?', err.getvalue())
        self.assertIn('bootm.c:1: check: Avoid CamelCase: <Fix>',
                      err.getvalue())
        self.assertIn(
            'Cc:  Anatolij Gustschin <agust@denx.de>', out.getvalue())

        self.assertTrue(os.path.exists(os.path.join(
            self.tmpdir, '0001-video-Some-video-improvements.patch')))
        self.assertTrue(os.path.exists(os.path.join(
            self.tmpdir, '0002-serial-Add-a-serial-driver.patch')))
        self.assertTrue(os.path.exists(os.path.join(
            self.tmpdir, '0003-bootm-Make-it-boot.patch')))

    def test_series_send_and_link(self):
        """Test sending a series and then adding its link to the database"""
        def h_sleep(time_s):
            if cser.get_time() > 25:
                self.autolink_extra = {'id': 500,
                                       'name': 'Series for my board',
                                       'version': 3}
            cser.inc_fake_time(time_s)

        cser, pwork = self.setup_second()

        # Create a third version
        with terminal.capture():
            cser.increment('second')
        series = patchstream.get_metadata_for_list('second3', self.gitdir, 3)
        self.assertEqual('2:457 1:456', series.links)
        self.assertEqual('3', series.version)

        with terminal.capture():
            self.run_args('series', '-n', 'send', pwork=pwork)

        cser.set_fake_time(h_sleep)
        with terminal.capture() as (out, _):
            cser.link_auto(pwork, 'second3', 3, True, 50)
        itr = iter(out.getvalue().splitlines())
        for i in range(7):
            self.assertEqual(
                "Possible matches for 'second' v3 desc 'Series for my board':",
                next(itr), f'failed at i={i}')
            self.assertEqual('  Link  Version  Description', next(itr))
            self.assertEqual('   456        1  Series for my board', next(itr))
            self.assertEqual('   457        2  Series for my board', next(itr))
            self.assertEqual('Sleeping for 5 seconds', next(itr))
        self.assertEqual('Link completed after 35 seconds', next(itr))
        self.assertRegex(
            next(itr), 'Checking out upstream commit refs/heads/base: .*')
        self.assertEqual(
            "Processing 3 commits from branch 'second3'", next(itr))
        self.assertRegex(
            next(itr),
            f'-                                {HASH_RE} as {HASH_RE} '
            'video: Some video improvements')
        self.assertRegex(
            next(itr),
            f"- add links '3:500 2:457 1:456': {HASH_RE} as {HASH_RE} "
            'serial: Add a serial driver')
        self.assertRegex(
            next(itr),
            f'- add v3:                        {HASH_RE} as {HASH_RE} '
            'bootm: Make it boot')
        self.assertRegex(
            next(itr),
            f'Updating branch second3 from {HASH_RE} to {HASH_RE}')
        self.assertEqual(
            "Setting link for series 'second' v3 to 500", next(itr))

    def _check_status(self, out, has_comments, has_cover_comments):
        """Check output from the status command

        Args:
            itr (Iterator): Contains the output lines to check
        """
        itr = iter(out.getvalue().splitlines())
        if has_cover_comments:
            self.assertEqual('Cov The name of the cover letter', next(itr))
            self.assertEqual(
                'From: A user <user@user.com>: Sun 13 Apr 14:06:02 MDT 2025',
                next(itr))
            self.assertEqual('some comment', next(itr))
            self.assertEqual('', next(itr))

            self.assertEqual(
                'From: Ghenkis Khan <gk@eurasia.gov>: Sun 13 Apr 13:06:02 '
                'MDT 2025',
                next(itr))
            self.assertEqual('another comment', next(itr))
            self.assertEqual('', next(itr))

        self.assertEqual('  1 video: Some video improvements', next(itr))
        self.assertEqual('  + Reviewed-by: Fred Bloggs <fred@bloggs.com>',
                         next(itr))
        if has_comments:
            self.assertEqual(
                'Review: Fred Bloggs <fred@bloggs.com>', next(itr))
            self.assertEqual('    > This was my original patch', next(itr))
            self.assertEqual('    > which is being quoted', next(itr))
            self.assertEqual(
                '    I like the approach here and I would love to see more '
                'of it.', next(itr))
            self.assertEqual('', next(itr))

        self.assertEqual('  2 serial: Add a serial driver', next(itr))
        self.assertEqual('  3 bootm: Make it boot', next(itr))
        self.assertEqual(
            '1 new response available in patchwork (use -d to write them to '
            'a new branch)', next(itr))

    def test_series_status(self):
        """Test getting the status of a series, including comments"""
        cser, pwork = self.setup_second()

        # Use single threading for easy debugging, but the multithreaded
        # version should produce the same output
        with self.stage('status second2: single-threaded'):
            with terminal.capture() as (out, _):
                cser.status(pwork, 'second', 2, False)
            self._check_status(out, False, False)
            self.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self.loop)

        with self.stage('status second2 (normal)'):
            with terminal.capture() as (out2, _):
                cser.status(pwork, 'second', 2, False)
            self.assertEqual(out.getvalue(), out2.getvalue())
            self._check_status(out, False, False)

        with self.stage('with comments'):
            with terminal.capture() as (out, _):
                cser.status(pwork, 'second', 2, show_comments=True)
            self._check_status(out, True, False)

        with self.stage('with comments and cover comments'):
            with terminal.capture() as (out, _):
                cser.status(pwork, 'second', 2, show_comments=True,
                            show_cover_comments=True)
            self._check_status(out, True, True)

    def test_series_status_cmdline(self):
        """Test getting the status of a series, including comments"""
        cser, pwork = self.setup_second()

        with self.stage('status second2'):
            with terminal.capture() as (out, _):
                self.run_args('series', '-s', 'second', '-V', '2', 'status',
                              pwork=pwork)
            self._check_status(out, False, False)

        with self.stage('status second2 (normal)'):
            with terminal.capture() as (out, _):
                cser.status(pwork, 'second', 2, show_comments=True)
            self._check_status(out, True, False)

        with self.stage('with comments and cover comments'):
            with terminal.capture() as (out, _):
                cser.status(pwork, 'second', 2, show_comments=True,
                                   show_cover_comments=True)
            self._check_status(out, True, True)

    def test_series_no_subcmd(self):
        """Test handling of things without a subcommand"""
        parsers = cmdline.setup_parser()
        parsers['series'].catch_error = True
        with terminal.capture() as (out, _):
            cmdline.parse_args(['series'], parsers=parsers)
        self.assertIn('usage: patman series', out.getvalue())

        parsers['patchwork'].catch_error = True
        with terminal.capture() as (out, _):
            cmdline.parse_args(['patchwork'], parsers=parsers)
        self.assertIn('usage: patman patchwork', out.getvalue())

        parsers['upstream'].catch_error = True
        with terminal.capture() as (out, _):
            cmdline.parse_args(['upstream'], parsers=parsers)
        self.assertIn('usage: patman upstream', out.getvalue())

    def check_series_rename(self):
        """Check renaming a series"""
        cser = self.get_cser()
        with self.stage('setup'):
            with terminal.capture() as (out, _):
                cser.add('first', 'my name', allow_unmarked=True)

            # Remember the old series
            old = cser.get_series_by_name('first')

            self.assertEqual('first', gitutil.get_branch(self.gitdir))
            with terminal.capture() as (out, _):
                cser.increment('first')
            self.assertEqual('first2', gitutil.get_branch(self.gitdir))

            with terminal.capture() as (out, _):
                cser.increment('first')
            self.assertEqual('first3', gitutil.get_branch(self.gitdir))

        # Do the dry run
        with self.stage('rename - dry run'):
            with terminal.capture() as (out, _):
                yield cser
            lines = out.getvalue().splitlines()
            itr = iter(lines)
            self.assertEqual("Renaming branch 'first' to 'newname'", next(itr))
            self.assertEqual(
                "Renaming branch 'first2' to 'newname2'", next(itr))
            self.assertEqual(
                "Renaming branch 'first3' to 'newname3'", next(itr))
            self.assertEqual("Renamed series 'first' to 'newname'", next(itr))
            self.assertEqual("Dry run completed", next(itr))
            self.assert_finished(itr)

            # Check nothing changed
            self.assertEqual('first3', gitutil.get_branch(self.gitdir))
            sdict = cser.db.series_get_dict()
            self.assertIn('first', sdict)

        # Now do it for real
        with self.stage('rename - real'):
            with terminal.capture() as (out2, _):
                yield cser
            lines2 = out2.getvalue().splitlines()
            self.assertEqual(lines[:-1], lines2)

            self.assertEqual('newname3', gitutil.get_branch(self.gitdir))

            # Check the series ID did not change
            ser = cser.get_series_by_name('newname')
            self.assertEqual(old.idnum, ser.idnum)

        yield None

    def test_series_rename(self):
        """Test renaming of a series"""
        cor = self.check_series_rename()
        cser = next(cor)

        # Rename (dry run)
        cser.rename('first', 'newname', dry_run=True)
        cser = next(cor)

        # Rename (real)
        cser.rename('first', 'newname')
        self.assertFalse(next(cor))

    def test_series_rename_cmdline(self):
        """Test renaming of a series with the cmdline"""
        cor = self.check_series_rename()
        next(cor)

        # Rename (dry run)
        self.run_args('series', '-n', '-s', 'first', 'rename', '-N', 'newname',
                      pwork=True)
        next(cor)

        # Rename (real)
        self.run_args('series', '-s', 'first', 'rename', '-N', 'newname',
                      pwork=True)

        self.assertFalse(next(cor))

    def test_series_rename_bad(self):
        """Test renaming when it is not allowed"""
        cser = self.get_cser()
        with terminal.capture():
            cser.add('first', 'my name', allow_unmarked=True)
            cser.increment('first')
            cser.increment('first')

        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'first')
        self.assertEqual("Cannot rename series 'first' to itself",
                         str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            cser.rename('first2', 'newname')
        self.assertEqual(
            "Invalid series name 'first2': did you use the branch name?",
            str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'newname2')
        self.assertEqual(
            "Invalid series name 'newname2': did you use the branch name?",
            str(exc.exception))

        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'second')
        self.assertEqual("Cannot rename: branches exist: second",
                         str(exc.exception))

        with terminal.capture():
            cser.add('second', 'another name', allow_unmarked=True)
            cser.increment('second')

        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'second')
        self.assertEqual("Cannot rename: series 'second' already exists",
                         str(exc.exception))

        # Rename second2 so that it gets in the way of the rename
        gitutil.rename_branch('second2', 'newname2', self.gitdir)
        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'newname')
        self.assertEqual("Cannot rename: branches exist: newname2",
                         str(exc.exception))

        # Rename first3 and make sure it stops the rename
        gitutil.rename_branch('first3', 'tempbranch', self.gitdir)
        with self.assertRaises(ValueError) as exc:
            cser.rename('first', 'newname')
        self.assertEqual(
            "Cannot rename: branches missing: first3: branches exist: "
            'newname2', str(exc.exception))

    def test_version_change(self):
        """Test changing a version of a series to a different version number"""
        cser = self.get_cser()

        with self.stage('setup'):
            with terminal.capture():
                cser.add('first', 'my description', allow_unmarked=True)

        with self.stage('non-existent version'):
            # Check changing a non-existent version
            with self.assertRaises(ValueError) as exc:
                cser.version_change('first', 2, 3, dry_run=True)
            self.assertEqual("Series 'first' does not have a version 2",
                             str(exc.exception))

        with self.stage('new version missing'):
            with self.assertRaises(ValueError) as exc:
                cser.version_change('first', None, None, dry_run=True)
            self.assertEqual("Please provide a new version number",
                             str(exc.exception))

        # Change v1 to v2 (dry run)
        with self.stage('v1 -> 2 dry run'):
            with terminal.capture():
                self.assertTrue(gitutil.check_branch('first', self.gitdir))
                cser.version_change('first', 1, 3, dry_run=True)
                self.assertTrue(gitutil.check_branch('first', self.gitdir))
                self.assertFalse(gitutil.check_branch('first3', self.gitdir))

                # Check that nothing actually happened
                series = patchstream.get_metadata('first', 0, 2,
                                                  git_dir=self.gitdir)
                self.assertNotIn('version', series)

                svlist = cser.get_ser_ver_list()
                self.assertEqual(1, len(svlist))
                item = svlist[0]
                self.assertEqual(1, item.version)

        with self.stage('increment twice'):
            # Increment so that we get first3
            with terminal.capture():
                cser.increment('first')
                cser.increment('first')

        with self.stage('existing version'):
            # Check changing to an existing version
            with self.assertRaises(ValueError) as exc:
                cser.version_change('first', 1, 3, dry_run=True)
            self.assertEqual("Series 'first' already has a v3: 1 2 3",
                             str(exc.exception))

        # Change v1 to v4 (for real)
        with self.stage('v1 -> 4'):
            with terminal.capture():
                self.assertTrue(gitutil.check_branch('first', self.gitdir))
                cser.version_change('first', 1, 4)
                self.assertTrue(gitutil.check_branch('first', self.gitdir))
                self.assertTrue(gitutil.check_branch('first4', self.gitdir))

                series = patchstream.get_metadata('first4', 0, 2,
                                                  git_dir=self.gitdir)
                self.assertIn('version', series)
                self.assertEqual('4', series.version)

                svdict = cser.get_ser_ver_dict()
                self.assertEqual(3, len(svdict))
                item = svdict[item.idnum]
                self.assertEqual(4, item.version)

        with self.stage('increment'):
            # Now try to increment first again
            with terminal.capture():
                cser.increment('first')

                ser = cser.get_series_by_name('first')
                self.assertIn(5, cser._get_version_list(ser.idnum))

    def test_version_change_cmdline(self):
        """Check changing a version on the cmdline"""
        self.get_cser()
        with (mock.patch.object(cseries.Cseries, 'version_change',
                                return_value=None) as method):
            self.run_args('series', '-s', 'first', 'version-change',
                          pwork=True)
        method.assert_called_once_with('first', None, None, dry_run=False)

        with (mock.patch.object(cseries.Cseries, 'version_change',
                                return_value=None) as method):
            self.run_args('series', '-s', 'first', 'version-change',
                          '--new-version', '3', pwork=True)
        method.assert_called_once_with('first', None, 3, dry_run=False)

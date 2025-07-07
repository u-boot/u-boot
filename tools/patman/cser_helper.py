# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Helper functions for handling the 'series' subcommand
"""

import asyncio
from collections import OrderedDict, defaultdict, namedtuple
from datetime import datetime
import hashlib
import os
import re
import sys
import time
from types import SimpleNamespace

import aiohttp
import pygit2
from pygit2.enums import CheckoutStrategy

from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tout

from patman import patchstream
from patman.database import Database, Pcommit, SerVer
from patman import patchwork
from patman.series import Series
from patman import status


# Tag to use for Change IDs
CHANGE_ID_TAG = 'Change-Id'

# Length of hash to display
HASH_LEN = 10

# Shorter version of some states, to save horizontal space
SHORTEN_STATE = {
    'handled-elsewhere': 'elsewhere',
    'awaiting-upstream': 'awaiting',
    'not-applicable': 'n/a',
    'changes-requested': 'changes',
}

# Summary info returned from Cseries.link_auto_all()
AUTOLINK = namedtuple('autolink', 'name,version,link,desc,result')


def oid(oid_val):
    """Convert a hash string into a shortened hash

    The number of hex digits git uses for showing hashes depends on the size of
    the repo. For the purposes of showing hashes to the user in lists, we use a
    fixed value for now

    Args:
        str or Pygit2.oid: Hash value to shorten

    Return:
        str: Shortened hash
    """
    return str(oid_val)[:HASH_LEN]


def split_name_version(in_name):
    """Split a branch name into its series name and its version

    For example:
        'series' returns ('series', 1)
        'series3' returns ('series', 3)
    Args:
        in_name (str): Name to parse

    Return:
        tuple:
            str: series name
            int: series version, or None if there is none in in_name
    """
    m_ver = re.match(r'([^0-9]*)(\d*)', in_name)
    version = None
    if m_ver:
        name = m_ver.group(1)
        if m_ver.group(2):
            version = int(m_ver.group(2))
    else:
        name = in_name
    return name, version


class CseriesHelper:
    """Helper functions for Cseries

    This class handles database read/write as well as operations in a git
    directory to update series information.
    """
    def __init__(self, topdir=None, colour=terminal.COLOR_IF_TERMINAL):
        """Set up a new CseriesHelper

        Args:
            topdir (str): Top-level directory of the repo
            colour (terminal.enum): Whether to enable ANSI colour or not

        Properties:
            gitdir (str): Git directory (typically topdir + '/.git')
            db (Database): Database handler
            col (terminal.Colour): Colour object
            _fake_time (float): Holds the current fake time for tests, in
                seconds
            _fake_sleep (func): Function provided by a test; called to fake a
                'time.sleep()' call and take whatever action it wants to take.
                The only argument is the (Float) time to sleep for; it returns
                nothing
            loop (asyncio event loop): Loop used for Patchwork operations
        """
        self.topdir = topdir
        self.gitdir = None
        self.db = None
        self.col = terminal.Color(colour)
        self._fake_time = None
        self._fake_sleep = None
        self.fake_now = None
        self.loop = asyncio.get_event_loop()

    def open_database(self):
        """Open the database ready for use"""
        if not self.topdir:
            self.topdir = gitutil.get_top_level()
            if not self.topdir:
                raise ValueError('No git repo detected in current directory')
        self.gitdir = os.path.join(self.topdir, '.git')
        fname = f'{self.topdir}/.patman.db'

        # For the first instance, start it up with the expected schema
        self.db, is_new = Database.get_instance(fname)
        if is_new:
            self.db.start()
        else:
            # If a previous test has already checked the schema, just open it
            self.db.open_it()

    def close_database(self):
        """Close the database"""
        if self.db:
            self.db.close()

    def commit(self):
        """Commit changes to the database"""
        self.db.commit()

    def rollback(self):
        """Roll back changes to the database"""
        self.db.rollback()

    def set_fake_time(self, fake_sleep):
        """Setup the fake timer

        Args:
            fake_sleep (func(float)): Function to call to fake a sleep
        """
        self._fake_time = 0
        self._fake_sleep = fake_sleep

    def inc_fake_time(self, inc_s):
        """Increment the fake time

        Args:
            inc_s (float): Amount to increment the fake time by
        """
        self._fake_time += inc_s

    def get_time(self):
        """Get the current time, fake or real

        This function should always be used to read the time so that faking the
        time works correctly in tests.

        Return:
            float: Fake time, if time is being faked, else real time
        """
        if self._fake_time is not None:
            return self._fake_time
        return time.monotonic()

    def sleep(self, time_s):
        """Sleep for a while

        This function should always be used to sleep so that faking the time
        works correctly in tests.

        Args:
            time_s (float): Amount of seconds to sleep for
        """
        print(f'Sleeping for {time_s} seconds')
        if self._fake_time is not None:
            self._fake_sleep(time_s)
        else:
            time.sleep(time_s)

    def get_now(self):
        """Get the time now

        This function should always be used to read the datetime, so that
        faking the time works correctly in tests

        Return:
            DateTime object
        """
        if self.fake_now:
            return self.fake_now
        return datetime.now()

    def get_ser_ver_list(self):
        """Get a list of patchwork entries from the database

        Return:
            list of SER_VER
        """
        return self.db.ser_ver_get_list()

    def get_ser_ver_dict(self):
        """Get a dict of patchwork entries from the database

        Return: dict contain all records:
            key (int): ser_ver id
            value (SER_VER): Information about one ser_ver record
        """
        svlist = self.get_ser_ver_list()
        svdict = {}
        for sver in svlist:
            svdict[sver.idnum] = sver
        return svdict

    def get_upstream_dict(self):
        """Get a list of upstream entries from the database

        Return:
            OrderedDict:
                key (str): upstream name
                value (str): url
        """
        return self.db.upstream_get_dict()

    def get_pcommit_dict(self, find_svid=None):
        """Get a dict of pcommits entries from the database

        Args:
            find_svid (int): If not None, finds the records associated with a
                particular series and version

        Return:
            OrderedDict:
                key (int): record ID if find_svid is None, else seq
                value (PCOMMIT): record data
        """
        pcdict = OrderedDict()
        for rec in self.db.pcommit_get_list(find_svid):
            if find_svid is not None:
                pcdict[rec.seq] = rec
            else:
                pcdict[rec.idnum] = rec
        return pcdict

    def _get_series_info(self, idnum):
        """Get information for a series from the database

        Args:
            idnum (int): Series ID to look up

        Return: tuple:
            str: Series name
            str: Series description

        Raises:
            ValueError: Series is not found
        """
        return self.db.series_get_info(idnum)

    def prep_series(self, name, end=None):
        """Prepare to work with a series

        Args:
            name (str): Branch name with version appended, e.g. 'fix2'
            end (str or None): Commit to end at, e.g. 'my_branch~16'. Only
                commits up to that are processed. None to process commits up to
                the upstream branch

        Return: tuple:
            str: Series name, e.g. 'fix'
            Series: Collected series information, including name
            int: Version number, e.g. 2
            str: Message to show
        """
        ser, version = self._parse_series_and_version(name, None)
        if not name:
            name = self._get_branch_name(ser.name, version)

        # First check we have a branch with this name
        if not gitutil.check_branch(name, git_dir=self.gitdir):
            raise ValueError(f"No branch named '{name}'")

        count = gitutil.count_commits_to_branch(name, self.gitdir, end)
        if not count:
            raise ValueError('Cannot detect branch automatically: '
                             'Perhaps use -U <upstream-commit> ?')

        series = patchstream.get_metadata(name, 0, count, git_dir=self.gitdir)
        self._copy_db_fields_to(series, ser)
        msg = None
        if end:
            repo = pygit2.init_repository(self.gitdir)
            target = repo.revparse_single(end)
            first_line = target.message.splitlines()[0]
            msg = f'Ending before {oid(target.id)} {first_line}'

        return name, series, version, msg

    def _copy_db_fields_to(self, series, in_series):
        """Copy over fields used by Cseries from one series to another

        This copes desc, idnum and name

        Args:
            series (Series): Series to copy to
            in_series (Series): Series to copy from
        """
        series.desc = in_series.desc
        series.idnum = in_series.idnum
        series.name = in_series.name

    def _handle_mark(self, branch_name, in_series, version, mark,
                     allow_unmarked, force_version, dry_run):
        """Handle marking a series, checking for unmarked commits, etc.

        Args:
            branch_name (str): Name of branch to sync, or None for current one
            in_series (Series): Series object
            version (int): branch version, e.g. 2 for 'mychange2'
            mark (bool): True to mark each commit with a change ID
            allow_unmarked (str): True to not require each commit to be marked
            force_version (bool): True if ignore a Series-version tag that
                doesn't match its branch name
            dry_run (bool): True to do a dry run

        Returns:
            Series: New series object, if the series was marked;
                copy_db_fields_to() is used to copy fields over

        Raises:
            ValueError: Series being unmarked when it should be marked, etc.
        """
        series = in_series
        if 'version' in series and int(series.version) != version:
            msg = (f"Series name '{branch_name}' suggests version {version} "
                   f"but Series-version tag indicates {series.version}")
            if not force_version:
                raise ValueError(msg + ' (see --force-version)')

            tout.warning(msg)
            tout.warning(f'Updating Series-version tag to version {version}')
            self.update_series(branch_name, series, int(series.version),
                                new_name=None, dry_run=dry_run,
                                add_vers=version)

            # Collect the commits again, as the hashes have changed
            series = patchstream.get_metadata(branch_name, 0,
                                              len(series.commits),
                                              git_dir=self.gitdir)
            self._copy_db_fields_to(series, in_series)

        if mark:
            add_oid = self._mark_series(branch_name, series, dry_run=dry_run)

            # Collect the commits again, as the hashes have changed
            series = patchstream.get_metadata(add_oid, 0, len(series.commits),
                                              git_dir=self.gitdir)
            self._copy_db_fields_to(series, in_series)

        bad_count = 0
        for commit in series.commits:
            if not commit.change_id:
                bad_count += 1
        if bad_count and not allow_unmarked:
            raise ValueError(
                f'{bad_count} commit(s) are unmarked; please use -m or -M')

        return series

    def _add_series_commits(self, series, svid):
        """Add a commits from a series into the database

        Args:
            series (Series): Series containing commits to add
            svid (int): ser_ver-table ID to use for each commit
        """
        to_add = [Pcommit(None, seq, commit.subject, None, commit.change_id,
                          None, None, None)
                  for seq, commit in enumerate(series.commits)]

        self.db.pcommit_add_list(svid, to_add)

    def get_series_by_name(self, name, include_archived=False):
        """Get a Series object from the database by name

        Args:
            name (str): Name of series to get
            include_archived (bool): True to search in archives series

        Return:
            Series: Object containing series info, or None if none
        """
        idnum = self.db.series_find_by_name(name, include_archived)
        if not idnum:
            return None
        name, desc = self.db.series_get_info(idnum)

        return Series.from_fields(idnum, name, desc)

    def _get_branch_name(self, name, version):
        """Get the branch name for a particular version

        Args:
            name (str): Base name of branch
            version (int): Version number to use
        """
        return name + (f'{version}' if version > 1 else '')

    def _ensure_version(self, ser, version):
        """Ensure that a version exists in a series

        Args:
            ser (Series): Series information, with idnum and name used here
            version (int): Version to check

        Returns:
            list of int: List of versions
        """
        versions = self._get_version_list(ser.idnum)
        if version not in versions:
            raise ValueError(
                f"Series '{ser.name}' does not have a version {version}")
        return versions

    def _set_link(self, ser_id, name, version, link, update_commit,
                  dry_run=False):
        """Add / update a series-links link for a series

        Args:
            ser_id (int): Series ID number
            name (str): Series name (used to find the branch)
            version (int): Version number (used to update the database)
            link (str): Patchwork link-string for the series
            update_commit (bool): True to update the current commit with the
                link
            dry_run (bool): True to do a dry run

        Return:
            bool: True if the database was update, False if the ser_id or
                version was not found
        """
        if update_commit:
            branch_name = self._get_branch_name(name, version)
            _, ser, max_vers, _ = self.prep_series(branch_name)
            self.update_series(branch_name, ser, max_vers, add_vers=version,
                                dry_run=dry_run, add_link=link)
        if link is None:
            link = ''
        updated = 1 if self.db.ser_ver_set_link(ser_id, version, link) else 0
        if dry_run:
            self.rollback()
        else:
            self.commit()

        return updated

    def _get_autolink_dict(self, sdict, link_all_versions):
        """Get a dict of ser_vers to fetch, along with their patchwork links

        Note that this returns items that already have links, as well as those
        without links

        Args:
            sdict:
                key: series ID
                value: Series with idnum, name and desc filled out
            link_all_versions (bool): True to sync all versions of a series,
                False to sync only the latest version

        Return: tuple:
            dict:
                key (int): svid
                value (tuple):
                   int: series ID
                   str: series name
                   int: series version
                   str: patchwork link for the series, or None if none
                   desc: cover-letter name / series description
        """
        svdict = self.get_ser_ver_dict()
        to_fetch = {}

        if link_all_versions:
            for svinfo in self.get_ser_ver_list():
                ser = sdict[svinfo.series_id]

                pwc = self.get_pcommit_dict(svinfo.idnum)
                count = len(pwc)
                branch = self._join_name_version(ser.name, svinfo.version)
                series = patchstream.get_metadata(branch, 0, count,
                                                  git_dir=self.gitdir)
                self._copy_db_fields_to(series, ser)

                to_fetch[svinfo.idnum] = (svinfo.series_id, series.name,
                                          svinfo.version, svinfo.link, series)
        else:
            # Find the maximum version for each series
            max_vers = self._series_all_max_versions()

            # Get a list of links to fetch
            for svid, ser_id, version in max_vers:
                svinfo = svdict[svid]
                ser = sdict[ser_id]

                pwc = self.get_pcommit_dict(svid)
                count = len(pwc)
                branch = self._join_name_version(ser.name, version)
                series = patchstream.get_metadata(branch, 0, count,
                                                  git_dir=self.gitdir)
                self._copy_db_fields_to(series, ser)

                to_fetch[svid] = (ser_id, series.name, version, svinfo.link,
                                  series)
        return to_fetch

    def _get_version_list(self, idnum):
        """Get a list of the versions available for a series

        Args:
            idnum (int): ID of series to look up

        Return:
            str: List of versions
        """
        if idnum is None:
            raise ValueError('Unknown series idnum')
        return self.db.series_get_version_list(idnum)

    def _join_name_version(self, in_name, version):
        """Convert a series name plus a version into a branch name

        For example:
            ('series', 1) returns 'series'
            ('series', 3) returns 'series3'

        Args:
            in_name (str): Series name
            version (int): Version number

        Return:
            str: associated branch name
        """
        if version == 1:
            return in_name
        return f'{in_name}{version}'

    def _parse_series(self, name, include_archived=False):
        """Parse the name of a series, or detect it from the current branch

        Args:
            name (str or None): name of series
            include_archived (bool): True to search in archives series

        Return:
            Series: New object with the name set; idnum is also set if the
                series exists in the database
        """
        if not name:
            name = gitutil.get_branch(self.gitdir)
        name, _ = split_name_version(name)
        ser = self.get_series_by_name(name, include_archived)
        if not ser:
            ser = Series()
            ser.name = name
        return ser

    def _parse_series_and_version(self, in_name, in_version):
        """Parse name and version of a series, or detect from current branch

        Figures out the name from in_name, or if that is None, from the current
            branch.

        Uses the version in_version, or if that is None, uses the int at the
        end of the name (e.g. 'series' is version 1, 'series4' is version 4)

        Args:
            in_name (str or None): name of series
            in_version (str or None): version of series

        Return:
            tuple:
                Series: New object with the name set; idnum is also set if the
                    series exists in the database
                int: Series version-number detected from the name
                    (e.g. 'fred' is version 1, 'fred2' is version 2)
        """
        name = in_name
        if not name:
            name = gitutil.get_branch(self.gitdir)
            if not name:
                raise ValueError('No branch detected: please use -s <series>')
        name, version = split_name_version(name)
        if not name:
            raise ValueError(f"Series name '{in_name}' cannot be a number, "
                             f"use '<name><version>'")
        if in_version:
            if version and version != in_version:
                tout.warning(
                    f"Version mismatch: -V has {in_version} but branch name "
                    f'indicates {version}')
            version = in_version
        if not version:
            version = 1
        if version > 99:
            raise ValueError(f"Version {version} exceeds 99")
        ser = self.get_series_by_name(name)
        if not ser:
            ser = Series()
            ser.name = name
        return ser, version

    def _series_get_version_stats(self, idnum, vers):
        """Get the stats for a series

        Args:
            idnum (int): ID number of series to process
            vers (int): Version number to process

        Return:
            tuple:
                str: Status string, '<accepted>/<count>'
                OrderedDict:
                    key (int): record ID if find_svid is None, else seq
                    value (PCOMMIT): record data
        """
        svid, link = self._get_series_svid_link(idnum, vers)
        pwc = self.get_pcommit_dict(svid)
        count = len(pwc.values())
        if link:
            accepted = 0
            for pcm in pwc.values():
                accepted += pcm.state == 'accepted'
        else:
            accepted = '-'
        return f'{accepted}/{count}', pwc

    def get_series_svid(self, series_id, version):
        """Get the patchwork ID of a series version

        Args:
            series_id (int): id of the series to look up
            version (int): version number to look up

        Return:
            str: link found

        Raises:
            ValueError: No matching series found
        """
        return self._get_series_svid_link(series_id, version)[0]

    def _get_series_svid_link(self, series_id, version):
        """Get the patchwork ID of a series version

        Args:
            series_id (int): series ID to look up
            version (int): version number to look up

        Return:
            tuple:
                int: record id
                str: link
        """
        recs = self.get_ser_ver(series_id, version)
        return recs.idnum, recs.link

    def get_ser_ver(self, series_id, version):
        """Get the patchwork details for a series version

        Args:
            series_id (int): series ID to look up
            version (int): version number to look up

        Return:
            SER_VER: Requested information

        Raises:
            ValueError: There is no matching idnum/version
        """
        return self.db.ser_ver_get_for_series(series_id, version)

    def _prepare_process(self, name, count, new_name=None, quiet=False):
        """Get ready to process all commits in a branch

        Args:
            name (str): Name of the branch to process
            count (int): Number of commits
            new_name (str or None): New name, if a new branch is to be created
            quiet (bool): True to avoid output (used for testing)

        Return: tuple:
            pygit2.repo: Repo to use
            pygit2.oid: Upstream commit, onto which commits should be added
            Pygit2.branch: Original branch, for later use
            str: (Possibly new) name of branch to process
            list of Commit: commits to process, in order
            pygit2.Reference: Original head before processing started
        """
        upstream_guess = gitutil.get_upstream(self.gitdir, name)[0]

        tout.debug(f"_process_series name '{name}' new_name '{new_name}' "
                   f"upstream_guess '{upstream_guess}'")
        dirty = gitutil.check_dirty(self.gitdir, self.topdir)
        if dirty:
            raise ValueError(
                f"Modified files exist: use 'git status' to check: "
                f'{dirty[:5]}')
        repo = pygit2.init_repository(self.gitdir)

        commit = None
        upstream_name = None
        if upstream_guess:
            try:
                upstream = repo.lookup_reference(upstream_guess)
                upstream_name = upstream.name
                commit = upstream.peel(pygit2.enums.ObjectType.COMMIT)
            except KeyError:
                pass
            except pygit2.repository.InvalidSpecError as exc:
                print(f"Error '{exc}'")
        if not upstream_name:
            upstream_name = f'{name}~{count}'
            commit = repo.revparse_single(upstream_name)

        branch = repo.lookup_branch(name)
        if not quiet:
            tout.info(
                f'Checking out upstream commit {upstream_name}: '
                f'{oid(commit.oid)}')

        old_head = repo.head
        if old_head.shorthand == name:
            old_head = None
        else:
            old_head = repo.head

        if new_name:
            name = new_name
        repo.set_head(commit.oid)

        commits = []
        cmt = repo.get(branch.target)
        for _ in range(count):
            commits.append(cmt)
            cmt = cmt.parents[0]

        return (repo, repo.head, branch, name, commit, list(reversed(commits)),
                old_head)

    def _pick_commit(self, repo, cmt):
        """Apply a commit to the source tree, without committing it

        _prepare_process() must be called before starting to pick commits

        This function must be called before _finish_commit()

        Note that this uses a cherry-pick method, creating a new tree_id each
        time, so can make source-code changes

        Args:
            repo (pygit2.repo): Repo to use
            cmt (Commit): Commit to apply

        Return: tuple:
            tree_id (pygit2.oid): Oid of index with source-changes applied
            commit (pygit2.oid): Old commit being cherry-picked
        """
        tout.detail(f"- adding {oid(cmt.hash)} {cmt}")
        repo.cherrypick(cmt.hash)
        if repo.index.conflicts:
            raise ValueError('Conflicts detected')

        tree_id = repo.index.write_tree()
        cherry = repo.get(cmt.hash)
        tout.detail(f"cherry {oid(cherry.oid)}")
        return tree_id, cherry

    def _finish_commit(self, repo, tree_id, commit, cur, msg=None):
        """Complete a commit

        This must be called after _pick_commit().

        Args:
            repo (pygit2.repo): Repo to use
            tree_id (pygit2.oid): Oid of index with source-changes applied; if
                None then the existing commit.tree_id is used
            commit (pygit2.oid): Old commit being cherry-picked
            cur (pygit2.reference): Reference to parent to use for the commit
            msg (str): Commit subject and message; None to use commit.message
        """
        if msg is None:
            msg = commit.message
        if not tree_id:
            tree_id = commit.tree_id
        repo.create_commit('HEAD', commit.author, commit.committer,
                           msg, tree_id, [cur.target])
        return repo.head

    def _finish_process(self, repo, branch, name, cur, old_head, new_name=None,
                        switch=False, dry_run=False, quiet=False):
        """Finish processing commits

        Args:
            repo (pygit2.repo): Repo to use
            branch (pygit2.branch): Branch returned by _prepare_process()
            name (str): Name of the branch to process
            new_name (str or None): New name, if a new branch is being created
            switch (bool): True to switch to the new branch after processing;
                otherwise HEAD remains at the original branch, as amended
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards
            quiet (bool): True to avoid output (used for testing)

        Return:
            pygit2.reference: Final commit after everything is completed
        """
        repo.state_cleanup()

        # Update the branch
        target = repo.revparse_single('HEAD')
        if not quiet:
            tout.info(f'Updating branch {name} from {oid(branch.target)} to '
                      f'{str(target.oid)[:HASH_LEN]}')
        if dry_run:
            if new_name:
                repo.head.set_target(branch.target)
            else:
                branch_oid = branch.peel(pygit2.enums.ObjectType.COMMIT).oid
                repo.head.set_target(branch_oid)
            repo.head.set_target(branch.target)
            repo.set_head(branch.name)
        else:
            if new_name:
                new_branch = repo.branches.create(new_name, target)
                if branch.upstream:
                    new_branch.upstream = branch.upstream
                branch = new_branch
            else:
                branch.set_target(cur.target)
            repo.set_head(branch.name)
        if old_head:
            if not switch:
                repo.set_head(old_head.name)
        return target

    def make_change_id(self, commit):
        """Make a Change ID for a commit

        This is similar to the gerrit script:
        git var GIT_COMMITTER_IDENT ; echo "$refhash" ; cat "README"; }
            | git hash-object --stdin)

        Args:
            commit (pygit2.commit): Commit to process

        Return:
            Change ID in hex format
        """
        sig = commit.committer
        val = hashlib.sha1()
        to_hash = f'{sig.name} <{sig.email}> {sig.time} {sig.offset}'
        val.update(to_hash.encode('utf-8'))
        val.update(str(commit.tree_id).encode('utf-8'))
        val.update(commit.message.encode('utf-8'))
        return val.hexdigest()

    def _filter_commits(self, name, series, seq_to_drop):
        """Filter commits to drop one

        This function rebases the current branch, dropping a single commit,
        thus changing the resulting code in the tree.

        Args:
            name (str): Name of the branch to process
            series (Series): Series object
            seq_to_drop (int): Commit sequence to drop; commits are numbered
                from 0, which is the one after the upstream branch, to
                count - 1
        """
        count = len(series.commits)
        (repo, cur, branch, name, commit, _, _) = self._prepare_process(
            name, count, quiet=True)
        repo.checkout_tree(commit, strategy=CheckoutStrategy.FORCE |
                           CheckoutStrategy.RECREATE_MISSING)
        repo.set_head(commit.oid)
        for seq, cmt in enumerate(series.commits):
            if seq != seq_to_drop:
                tree_id, cherry = self._pick_commit(repo, cmt)
                cur = self._finish_commit(repo, tree_id, cherry, cur)
        self._finish_process(repo, branch, name, cur, None, quiet=True)

    def process_series(self, name, series, new_name=None, switch=False,
                       dry_run=False):
        """Rewrite a series commit messages, leaving code alone

        This uses a 'vals' namespace to pass things to the controlling
        function.

        Each time _process_series() yields, it sets up:
            commit (Commit): The pygit2 commit that is being processed
            msg (str): Commit message, which can be modified
            info (str): Initially empty; the controlling function can add a
                short message here which will be shown to the user
            final (bool): True if this is the last commit to apply
            seq (int): Current sequence number in the commits to apply (0,,n-1)

            It also sets git HEAD at the commit before this commit being
            processed

        The function can change msg and info, e.g. to add or remove tags from
        the commit.

        Args:
            name (str): Name of the branch to process
            series (Series): Series object
            new_name (str or None): New name, if a new branch is to be created
            switch (bool): True to switch to the new branch after processing;
                otherwise HEAD remains at the original branch, as amended
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards

        Return:
            pygit.oid: oid of the new branch
        """
        count = len(series.commits)
        repo, cur, branch, name, _, commits, old_head = self._prepare_process(
            name, count, new_name)
        vals = SimpleNamespace()
        vals.final = False
        tout.info(f"Processing {count} commits from branch '{name}'")

        # Record the message lines
        lines = []
        for seq, cmt in enumerate(series.commits):
            commit = commits[seq]
            vals.commit = commit
            vals.msg = commit.message
            vals.info = ''
            vals.final = seq == len(series.commits) - 1
            vals.seq = seq
            yield vals

            cur = self._finish_commit(repo, None, commit, cur, vals.msg)
            lines.append([vals.info.strip(),
                          f'{oid(cmt.hash)} as {oid(cur.target)} {cmt}'])

        max_len = max(len(info) for info, rest in lines) + 1
        for info, rest in lines:
            if info:
                info += ':'
            tout.info(f'- {info.ljust(max_len)} {rest}')
        target = self._finish_process(repo, branch, name, cur, old_head,
                                      new_name, switch, dry_run)
        vals.oid = target.oid

    def _mark_series(self, name, series, dry_run=False):
        """Mark a series with Change-Id tags

        Args:
            name (str): Name of the series to mark
            series (Series): Series object
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards

        Return:
            pygit.oid: oid of the new branch
        """
        vals = None
        for vals in self.process_series(name, series, dry_run=dry_run):
            if CHANGE_ID_TAG not in vals.msg:
                change_id = self.make_change_id(vals.commit)
                vals.msg = vals.msg + f'\n{CHANGE_ID_TAG}: {change_id}'
                tout.detail("   - adding mark")
                vals.info = 'marked'
            else:
                vals.info = 'has mark'

        return vals.oid

    def update_series(self, branch_name, series, max_vers, new_name=None,
                       dry_run=False, add_vers=None, add_link=None,
                       add_rtags=None, switch=False):
        """Rewrite a series to update the Series-version/Series-links lines

        This updates the series in git; it does not update the database

        Args:
            branch_name (str): Name of the branch to process
            series (Series): Series object
            max_vers (int): Version number of the series being updated
            new_name (str or None): New name, if a new branch is to be created
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards
            add_vers (int or None): Version number to add to the series, if any
            add_link (str or None): Link to add to the series, if any
            add_rtags (list of dict): List of review tags to add, one item for
                    each commit, each a dict:
                key: Response tag (e.g. 'Reviewed-by')
                value: Set of people who gave that response, each a name/email
                    string
            switch (bool): True to switch to the new branch after processing;
                otherwise HEAD remains at the original branch, as amended

        Return:
            pygit.oid: oid of the new branch
        """
        def _do_version():
            if add_vers:
                if add_vers == 1:
                    vals.info += f'rm v{add_vers} '
                else:
                    vals.info += f'add v{add_vers} '
                    out.append(f'Series-version: {add_vers}')

        def _do_links(new_links):
            if add_link:
                if 'add' not in vals.info:
                    vals.info += 'add '
                vals.info += f"links '{new_links}' "
            else:
                vals.info += f"upd links '{new_links}' "
            out.append(f'Series-links: {new_links}')

        added_version = False
        added_link = False
        for vals in self.process_series(branch_name, series, new_name, switch,
                                         dry_run):
            out = []
            for line in vals.msg.splitlines():
                m_ver = re.match('Series-version:(.*)', line)
                m_links = re.match('Series-links:(.*)', line)
                if m_ver and add_vers:
                    if ('version' in series and
                            int(series.version) != max_vers):
                        tout.warning(
                            f'Branch {branch_name}: Series-version tag '
                            f'{series.version} does not match expected '
                            f'version {max_vers}')
                    _do_version()
                    added_version = True
                elif m_links:
                    links = series.get_links(m_links.group(1), max_vers)
                    if add_link:
                        links[max_vers] = add_link
                    _do_links(series.build_links(links))
                    added_link = True
                else:
                    out.append(line)
            if vals.final:
                if not added_version and add_vers and add_vers > 1:
                    _do_version()
                if not added_link and add_link:
                    _do_links(f'{max_vers}:{add_link}')

            vals.msg = '\n'.join(out) + '\n'
            if add_rtags and add_rtags[vals.seq]:
                lines = []
                for tag, people in add_rtags[vals.seq].items():
                    for who in people:
                        lines.append(f'{tag}: {who}')
                vals.msg = patchstream.insert_tags(vals.msg.rstrip(),
                                                   sorted(lines))
                vals.info += (f'added {len(lines)} '
                              f"tag{'' if len(lines) == 1 else 's'}")

    def _build_col(self, state, prefix='', base_str=None):
        """Build a patch-state string with colour

        Args:
            state (str): State to colourise (also indicates the colour to use)
            prefix (str): Prefix string to also colourise
            base_str (str or None): String to show instead of state, or None to
                show state

        Return:
            str: String with ANSI colour characters
        """
        bright = True
        if state == 'accepted':
            col = self.col.GREEN
        elif state == 'awaiting-upstream':
            bright = False
            col = self.col.GREEN
        elif state in ['changes-requested']:
            col = self.col.CYAN
        elif state in ['rejected', 'deferred', 'not-applicable', 'superseded',
                       'handled-elsewhere']:
            col = self.col.RED
        elif not state:
            state = 'unknown'
            col = self.col.MAGENTA
        else:
            # under-review, rfc, needs-review-ack
            col = self.col.WHITE
        out = base_str or SHORTEN_STATE.get(state, state)
        pad = ' ' * (10 - len(out))
        col_state = self.col.build(col, prefix + out, bright)
        return col_state, pad

    def _get_patches(self, series, version):
        """Get a Series object containing the patches in a series

        Args:
            series (str): Name of series to use, or None to use current branch
            version (int): Version number, or None to detect from name

        Return: tuple:
            str: Name of branch, e.g. 'mary2'
            Series: Series object containing the commits and idnum, desc, name
            int: Version number of series, e.g. 2
            OrderedDict:
                key (int): record ID if find_svid is None, else seq
                value (PCOMMIT): record data
            str: series name (for this version)
            str: patchwork link
            str: cover_id
            int: cover_num_comments
        """
        ser, version = self._parse_series_and_version(series, version)
        if not ser.idnum:
            raise ValueError(f"Unknown series '{series}'")
        self._ensure_version(ser, version)
        svinfo = self.get_ser_ver(ser.idnum, version)
        pwc = self.get_pcommit_dict(svinfo.idnum)

        count = len(pwc)
        branch = self._join_name_version(ser.name, version)
        series = patchstream.get_metadata(branch, 0, count,
                                          git_dir=self.gitdir)
        self._copy_db_fields_to(series, ser)

        return (branch, series, version, pwc, svinfo.name, svinfo.link,
                svinfo.cover_id, svinfo.cover_num_comments)

    def _list_patches(self, branch, pwc, series, desc, cover_id, num_comments,
                      show_commit, show_patch, list_patches, state_totals):
        """List patches along with optional status info

        Args:
            branch (str): Branch name        if self.show_progress
            pwc (dict): pcommit records:
                key (int): seq
                value (PCOMMIT): Record from database
            series (Series): Series to show, or None to just use the database
            desc (str): Series title
            cover_id (int): Cover-letter ID
            num_comments (int): The number of comments on the cover letter
            show_commit (bool): True to show the commit and diffstate
            show_patch (bool): True to show the patch
            list_patches (bool): True to list all patches for each series,
                False to just show the series summary on a single line
            state_totals (dict): Holds totals for each state across all patches
                key (str): state name
                value (int): Number of patches in that state

        Return:
            bool: True if OK, False if any commit subjects don't match their
                patchwork subjects
        """
        lines = []
        states = defaultdict(int)
        count = len(pwc)
        ok = True
        for seq, item in enumerate(pwc.values()):
            if series:
                cmt = series.commits[seq]
                if cmt.subject != item.subject:
                    ok = False

            col_state, pad = self._build_col(item.state)
            patch_id = item.patch_id if item.patch_id else ''
            if item.num_comments:
                comments = str(item.num_comments)
            elif item.num_comments is None:
                comments = '-'
            else:
                comments = ''

            if show_commit or show_patch:
                subject = self.col.build(self.col.BLACK, item.subject,
                                         bright=False, back=self.col.YELLOW)
            else:
                subject = item.subject

            line = (f'{seq:3} {col_state}{pad} {comments.rjust(3)} '
                    f'{patch_id:7} {oid(cmt.hash)} {subject}')
            lines.append(line)
            states[item.state] += 1
        out = ''
        for state, freq in states.items():
            out += ' ' + self._build_col(state, f'{freq}:')[0]
            state_totals[state] += freq
        name = ''
        if not list_patches:
            name = desc or series.desc
            name = self.col.build(self.col.YELLOW, name[:41].ljust(41))
            if not ok:
                out = '*' + out[1:]
            print(f"{branch:16} {name} {len(pwc):5} {out}")
            return ok
        print(f"Branch '{branch}' (total {len(pwc)}):{out}{name}")

        print(self.col.build(
            self.col.MAGENTA,
            f"Seq State      Com PatchId {'Commit'.ljust(HASH_LEN)} Subject"))

        comments = '' if num_comments is None else str(num_comments)
        if desc or comments or cover_id:
            cov = 'Cov' if cover_id else ''
            print(self.col.build(
                self.col.WHITE,
                f"{cov:14} {comments.rjust(3)} {cover_id or '':7}            "
                f'{desc or series.desc}',
                bright=False))
        for seq in range(count):
            line = lines[seq]
            print(line)
            if show_commit or show_patch:
                print()
                cmt = series.commits[seq] if series else ''
                msg = gitutil.show_commit(
                    cmt.hash, show_commit, True, show_patch,
                    colour=self.col.enabled(), git_dir=self.gitdir)
                sys.stdout.write(msg)
                if seq != count - 1:
                    print()
                    print()

        return ok

    def _find_matched_commit(self, commits, pcm):
        """Find a commit in a list of possible matches

        Args:
            commits (dict of Commit): Possible matches
                key (int): sequence number of patch (from 0)
                value (Commit): Commit object
            pcm (PCOMMIT): Patch to check

        Return:
            int: Sequence number of matching commit, or None if not found
        """
        for seq, cmt in commits.items():
            tout.debug(f"- match subject: '{cmt.subject}'")
            if pcm.subject == cmt.subject:
                return seq
        return None

    def _find_matched_patch(self, patches, cmt):
        """Find a patch in a list of possible matches

        Args:
            patches: dict of ossible matches
                key (int): sequence number of patch
                value (PCOMMIT): patch
            cmt (Commit): Commit to check

        Return:
            int: Sequence number of matching patch, or None if not found
        """
        for seq, pcm in patches.items():
            tout.debug(f"- match subject: '{pcm.subject}'")
            if cmt.subject == pcm.subject:
                return seq
        return None

    def _sync_one(self, svid, series_name, version, show_comments,
                  show_cover_comments, gather_tags, cover, patches, dry_run):
        """Sync one series to the database

        Args:
            svid (int): Ser/ver ID
            cover (dict or None): Cover letter from patchwork, with keys:
                id (int): Cover-letter ID in patchwork
                num_comments (int): Number of comments
                name (str): Cover-letter name
            patches (list of Patch): Patches in the series
        """
        pwc = self.get_pcommit_dict(svid)
        if gather_tags:
            count = len(pwc)
            branch = self._join_name_version(series_name, version)
            series = patchstream.get_metadata(branch, 0, count,
                                              git_dir=self.gitdir)

            _, new_rtag_list = status.do_show_status(
                series, cover, patches, show_comments, show_cover_comments,
                self.col, warnings_on_stderr=False)
            self.update_series(branch, series, version, None, dry_run,
                                add_rtags=new_rtag_list)

        updated = 0
        for seq, item in enumerate(pwc.values()):
            if seq >= len(patches):
                continue
            patch = patches[seq]
            if patch.id:
                if self.db.pcommit_update(
                    Pcommit(item.idnum, seq, None, None, None, patch.state,
                            patch.id, len(patch.comments))):
                    updated += 1
        if cover:
            info = SerVer(svid, None, None, None, cover.id,
                           cover.num_comments, cover.name, None)
        else:
            info = SerVer(svid, None, None, None, None, None, patches[0].name,
                           None)
        self.db.ser_ver_set_info(info)

        return updated, 1 if cover else 0

    async def _gather(self, pwork, link, show_cover_comments):
        """Sync the series status from patchwork

        Creates a new client sesion and calls _sync()

        Args:
            pwork (Patchwork): Patchwork object to use
            link (str): Patchwork link for the series
            show_cover_comments (bool): True to show the comments on the cover
                letter

        Return: tuple:
            COVER object, or None if none or not read_cover_comments
            list of PATCH objects
        """
        async with aiohttp.ClientSession() as client:
            return await pwork.series_get_state(client, link, True,
                                                show_cover_comments)

    def _get_fetch_dict(self, sync_all_versions):
        """Get a dict of ser_vers to fetch, along with their patchwork links

        Args:
            sync_all_versions (bool): True to sync all versions of a series,
                False to sync only the latest version

        Return: tuple:
            dict: things to fetch
                key (int): svid
                value (str): patchwork link for the series
            int: number of series which are missing a link
        """
        missing = 0
        svdict = self.get_ser_ver_dict()
        sdict = self.db.series_get_dict_by_id()
        to_fetch = {}

        if sync_all_versions:
            for svinfo in self.get_ser_ver_list():
                ser_ver = svdict[svinfo.idnum]
                if svinfo.link:
                    to_fetch[svinfo.idnum] = patchwork.STATE_REQ(
                        svinfo.link, svinfo.series_id,
                        sdict[svinfo.series_id].name, svinfo.version, False,
                        False)
                else:
                    missing += 1
        else:
            # Find the maximum version for each series
            max_vers = self._series_all_max_versions()

            # Get a list of links to fetch
            for svid, series_id, version in max_vers:
                ser_ver = svdict[svid]
                if series_id not in sdict:
                    # skip archived item
                    continue
                if ser_ver.link:
                    to_fetch[svid] = patchwork.STATE_REQ(
                        ser_ver.link, series_id, sdict[series_id].name,
                        version, False, False)
                else:
                    missing += 1

        # order by series name, version
        ordered = OrderedDict()
        for svid in sorted(
                to_fetch,
                key=lambda k: (to_fetch[k].series_name, to_fetch[k].version)):
            sync = to_fetch[svid]
            ordered[svid] = sync

        return ordered, missing

    async def _sync_all(self, client, pwork, to_fetch):
        """Sync all series status from patchwork

        Args:
            pwork (Patchwork): Patchwork object to use
            sync_all_versions (bool): True to sync all versions of a series,
                False to sync only the latest version
            gather_tags (bool): True to gather review/test tags

        Return: list of tuple:
            COVER object, or None if none or not read_cover_comments
            list of PATCH objects
        """
        with pwork.collect_stats() as stats:
            tasks = [pwork.series_get_state(client, sync.link, True, True)
                     for sync in to_fetch.values() if sync.link]
            result = await asyncio.gather(*tasks)
        return result, stats.request_count

    async def _do_series_sync_all(self, pwork, to_fetch):
        async with aiohttp.ClientSession() as client:
            return await self._sync_all(client, pwork, to_fetch)

    def _progress_one(self, ser, show_all_versions, list_patches,
                      state_totals):
        """Show progress information for all versions in a series

        Args:
            ser (Series): Series to use
            show_all_versions (bool): True to show all versions of a series,
                False to show only the final version
            list_patches (bool): True to list all patches for each series,
                False to just show the series summary on a single line
            state_totals (dict): Holds totals for each state across all patches
                key (str): state name
                value (int): Number of patches in that state

        Return: tuple
            int: Number of series shown
            int: Number of patches shown
            int: Number of version which need a 'scan'
        """
        max_vers = self._series_max_version(ser.idnum)
        name, desc = self._get_series_info(ser.idnum)
        coloured = self.col.build(self.col.BLACK, desc, bright=False,
                                  back=self.col.YELLOW)
        versions = self._get_version_list(ser.idnum)
        vstr = list(map(str, versions))

        if list_patches:
            print(f"{name}: {coloured} (versions: {' '.join(vstr)})")
        add_blank_line = False
        total_series = 0
        total_patches = 0
        need_scan = 0
        for ver in versions:
            if not show_all_versions and ver != max_vers:
                continue
            if add_blank_line:
                print()
            _, pwc = self._series_get_version_stats(ser.idnum, ver)
            count = len(pwc)
            branch = self._join_name_version(ser.name, ver)
            series = patchstream.get_metadata(branch, 0, count,
                                              git_dir=self.gitdir)
            svinfo = self.get_ser_ver(ser.idnum, ver)
            self._copy_db_fields_to(series, ser)

            ok = self._list_patches(
                branch, pwc, series, svinfo.name, svinfo.cover_id,
                svinfo.cover_num_comments, False, False, list_patches,
                state_totals)
            if not ok:
                need_scan += 1
            add_blank_line = list_patches
            total_series += 1
            total_patches += count
        return total_series, total_patches, need_scan

    def _summary_one(self, ser):
        """Show summary information for the latest version in a series

        Args:
            series (str): Name of series to use, or None to show progress for
                all series
        """
        max_vers = self._series_max_version(ser.idnum)
        name, desc = self._get_series_info(ser.idnum)
        stats, pwc = self._series_get_version_stats(ser.idnum, max_vers)
        states = {x.state for x in pwc.values()}
        state = 'accepted'
        for val in ['awaiting-upstream', 'changes-requested', 'rejected',
                    'deferred', 'not-applicable', 'superseded',
                    'handled-elsewhere']:
            if val in states:
                state = val
        state_str, pad = self._build_col(state, base_str=name)
        print(f"{state_str}{pad}  {stats.rjust(6)}  {desc}")

    def _series_max_version(self, idnum):
        """Find the latest version of a series

        Args:
            idnum (int): Series ID to look up

        Return:
            int: maximum version
        """
        return self.db.series_get_max_version(idnum)

    def _series_all_max_versions(self):
        """Find the latest version of all series

        Return: list of:
            int: ser_ver ID
            int: series ID
            int: Maximum version
        """
        return self.db.series_get_all_max_versions()

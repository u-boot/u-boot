# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Simon Glass <sjg@chromium.org>
#
"""Handles the patman database

This uses sqlite3 with a local file.

To adjsut the schema, increment LATEST, create a migrate_to_v<x>() function
and write some code in migrate_to() to call it.
"""

from collections import namedtuple, OrderedDict
import os
import sqlite3

from u_boot_pylib import tools
from u_boot_pylib import tout
from patman.series import Series

# Schema version (version 0 means there is no database yet)
LATEST = 4

# Information about a series/version record
SerVer = namedtuple(
    'SER_VER',
    'idnum,series_id,version,link,cover_id,cover_num_comments,name,'
    'archive_tag')

# Record from the pcommit table:
# idnum (int): record ID
# seq (int): Patch sequence in series (0 is first)
# subject (str): patch subject
# svid (int): ID of series/version record in ser_ver table
# change_id (str): Change-ID value
# state (str): Current status in patchwork
# patch_id (int): Patchwork's patch ID for this patch
# num_comments (int): Number of comments attached to the commit
Pcommit = namedtuple(
    'PCOMMIT',
    'idnum,seq,subject,svid,change_id,state,patch_id,num_comments')


class Database:
    """Database of information used by patman"""

    # dict of databases:
    #   key: filename
    #   value: Database object
    instances = {}

    def __init__(self, db_path):
        """Set up a new database object

        Args:
            db_path (str): Path to the database
        """
        if db_path in Database.instances:
            # Two connections to the database can cause:
            # sqlite3.OperationalError: database is locked
            raise ValueError(f"There is already a database for '{db_path}'")
        self.con = None
        self.cur = None
        self.db_path = db_path
        self.is_open = False
        Database.instances[db_path] = self

    @staticmethod
    def get_instance(db_path):
        """Get the database instance for a path

        This is provides to ensure that different callers can obtain the
        same database object when accessing the same database file.

        Args:
            db_path (str): Path to the database

        Return:
            Database: Database instance, which is created if necessary
        """
        db = Database.instances.get(db_path)
        if db:
            return db, False
        return Database(db_path), True

    def start(self):
        """Open the database read for use, migrate to latest schema"""
        self.open_it()
        self.migrate_to(LATEST)

    def open_it(self):
        """Open the database, creating it if necessary"""
        if self.is_open:
            raise ValueError('Already open')
        if not os.path.exists(self.db_path):
            tout.warning(f'Creating new database {self.db_path}')
        self.con = sqlite3.connect(self.db_path)
        self.cur = self.con.cursor()
        self.is_open = True

    def close(self):
        """Close the database"""
        if not self.is_open:
            raise ValueError('Already closed')
        self.con.close()
        self.cur = None
        self.con = None
        self.is_open = False

    def create_v1(self):
        """Create a database with the v1 schema"""
        self.cur.execute(
            'CREATE TABLE series (id INTEGER PRIMARY KEY AUTOINCREMENT,'
            'name UNIQUE, desc, archived BIT)')

        # Provides a series_id/version pair, which is used to refer to a
        # particular series version sent to patchwork. This stores the link
        # to patchwork
        self.cur.execute(
            'CREATE TABLE ser_ver (id INTEGER PRIMARY KEY AUTOINCREMENT,'
            'series_id INTEGER, version INTEGER, link,'
            'FOREIGN KEY (series_id) REFERENCES series (id))')

        self.cur.execute(
            'CREATE TABLE upstream (name UNIQUE, url, is_default BIT)')

        # change_id is the Change-Id
        # patch_id is the ID of the patch on the patchwork server
        self.cur.execute(
            'CREATE TABLE pcommit (id INTEGER PRIMARY KEY AUTOINCREMENT,'
            'svid INTEGER, seq INTEGER, subject, patch_id INTEGER, '
            'change_id, state, num_comments INTEGER, '
            'FOREIGN KEY (svid) REFERENCES ser_ver (id))')

        self.cur.execute(
            'CREATE TABLE settings (name UNIQUE, proj_id INT, link_name)')

    def _migrate_to_v2(self):
        """Add a schema_version table"""
        self.cur.execute('CREATE TABLE schema_version (version INTEGER)')

    def _migrate_to_v3(self):
        """Store the number of cover-letter comments in the schema"""
        self.cur.execute('ALTER TABLE ser_ver ADD COLUMN cover_id')
        self.cur.execute('ALTER TABLE ser_ver ADD COLUMN cover_num_comments '
                         'INTEGER')
        self.cur.execute('ALTER TABLE ser_ver ADD COLUMN name')

    def _migrate_to_v4(self):
        """Add an archive tag for each ser_ver"""
        self.cur.execute('ALTER TABLE ser_ver ADD COLUMN archive_tag')

    def migrate_to(self, dest_version):
        """Migrate the database to the selected version

        Args:
            dest_version (int): Version to migrate to
        """
        while True:
            version = self.get_schema_version()
            if version == dest_version:
                break

            self.close()
            tools.write_file(f'{self.db_path}old.v{version}',
                             tools.read_file(self.db_path))

            version += 1
            tout.info(f'Update database to v{version}')
            self.open_it()
            if version == 1:
                self.create_v1()
            elif version == 2:
                self._migrate_to_v2()
            elif version == 3:
                self._migrate_to_v3()
            elif version == 4:
                self._migrate_to_v4()

            # Save the new version if we have a schema_version table
            if version > 1:
                self.cur.execute('DELETE FROM schema_version')
                self.cur.execute(
                    'INSERT INTO schema_version (version) VALUES (?)',
                    (version,))
            self.commit()

    def get_schema_version(self):
        """Get the version of the database's schema

        Return:
            int: Database version, 0 means there is no data; anything less than
                LATEST means the schema is out of date and must be updated
        """
        # If there is no database at all, assume v0
        version = 0
        try:
            self.cur.execute('SELECT name FROM series')
        except sqlite3.OperationalError:
            return 0

        # If there is no schema, assume v1
        try:
            self.cur.execute('SELECT version FROM schema_version')
            version = self.cur.fetchone()[0]
        except sqlite3.OperationalError:
            return 1
        return version

    def execute(self, query, parameters=()):
        """Execute a database query

        Args:
            query (str): Query string
            parameters (list of values): Parameters to pass

        Return:

        """
        return self.cur.execute(query, parameters)

    def commit(self):
        """Commit changes to the database"""
        self.con.commit()

    def rollback(self):
        """Roll back changes to the database"""
        self.con.rollback()

    def lastrowid(self):
        """Get the last row-ID reported by the database

        Return:
            int: Value for lastrowid
        """
        return self.cur.lastrowid

    def rowcount(self):
        """Get the row-count reported by the database

        Return:
            int: Value for rowcount
        """
        return self.cur.rowcount

    def _get_series_list(self, include_archived):
        """Get a list of Series objects from the database

        Args:
            include_archived (bool): True to include archives series

        Return:
            list of Series
        """
        res = self.execute(
            'SELECT id, name, desc FROM series ' +
            ('WHERE archived = 0' if not include_archived else ''))
        return [Series.from_fields(idnum=idnum, name=name, desc=desc)
                for idnum, name, desc in res.fetchall()]

    # series functions

    def series_get_dict_by_id(self, include_archived=False):
        """Get a dict of Series objects from the database

        Args:
            include_archived (bool): True to include archives series

        Return:
            OrderedDict:
                key: series ID
                value: Series with idnum, name and desc filled out
        """
        sdict = OrderedDict()
        for ser in self._get_series_list(include_archived):
            sdict[ser.idnum] = ser
        return sdict

    def series_find_by_name(self, name, include_archived=False):
        """Find a series and return its details

        Args:
            name (str): Name to search for
            include_archived (bool): True to include archives series

        Returns:
            idnum, or None if not found
        """
        res = self.execute(
            'SELECT id FROM series WHERE name = ?' +
            ('AND archived = 0' if not include_archived else ''), (name,))
        recs = res.fetchall()

        # This shouldn't happen
        assert len(recs) <= 1, 'Expected one match, but multiple found'

        if len(recs) != 1:
            return None
        return recs[0][0]

    def series_get_info(self, idnum):
        """Get information for a series from the database

        Args:
            idnum (int): Series ID to look up

        Return: tuple:
            str: Series name
            str: Series description

        Raises:
            ValueError: Series is not found
        """
        res = self.execute('SELECT name, desc FROM series WHERE id = ?',
                           (idnum,))
        recs = res.fetchall()
        if len(recs) != 1:
            raise ValueError(f'No series found (id {idnum} len {len(recs)})')
        return recs[0]

    def series_get_dict(self, include_archived=False):
        """Get a dict of Series objects from the database

        Args:
            include_archived (bool): True to include archives series

        Return:
            OrderedDict:
                key: series name
                value: Series with idnum, name and desc filled out
        """
        sdict = OrderedDict()
        for ser in self._get_series_list(include_archived):
            sdict[ser.name] = ser
        return sdict

    def series_get_version_list(self, series_idnum):
        """Get a list of the versions available for a series

        Args:
            series_idnum (int): ID of series to look up

        Return:
            str: List of versions, which may be empty if the series is in the
                process of being added
        """
        res = self.execute('SELECT version FROM ser_ver WHERE series_id = ?',
                           (series_idnum,))
        return [x[0] for x in res.fetchall()]

    def series_get_max_version(self, series_idnum):
        """Get the highest version number available for a series

        Args:
            series_idnum (int): ID of series to look up

        Return:
            int: Maximum version number
        """
        res = self.execute(
            'SELECT MAX(version) FROM ser_ver WHERE series_id = ?',
            (series_idnum,))
        return res.fetchall()[0][0]

    def series_get_all_max_versions(self):
        """Find the latest version of all series

        Return: list of:
            int: ser_ver ID
            int: series ID
            int: Maximum version
        """
        res = self.execute(
            'SELECT id, series_id, MAX(version) FROM ser_ver '
            'GROUP BY series_id')
        return res.fetchall()

    def series_add(self, name, desc):
        """Add a new series record

        The new record is set to not archived

        Args:
            name (str): Series name
            desc (str): Series description

        Return:
            int: ID num of the new series record
        """
        self.execute(
            'INSERT INTO series (name, desc, archived) '
            f"VALUES ('{name}', '{desc}', 0)")
        return self.lastrowid()

    def series_remove(self, idnum):
        """Remove a series from the database

        The series must exist

        Args:
            idnum (int): ID num of series to remove
        """
        self.execute('DELETE FROM series WHERE id = ?', (idnum,))
        assert self.rowcount() == 1

    def series_remove_by_name(self, name):
        """Remove a series from the database

        Args:
            name (str): Name of series to remove

        Raises:
            ValueError: Series does not exist (database is rolled back)
        """
        self.execute('DELETE FROM series WHERE name = ?', (name,))
        if self.rowcount() != 1:
            self.rollback()
            raise ValueError(f"No such series '{name}'")

    def series_set_archived(self, series_idnum, archived):
        """Update archive flag for a series

        Args:
            series_idnum (int): ID num of the series
            archived (bool): Whether to mark the series as archived or
                unarchived
        """
        self.execute(
            'UPDATE series SET archived = ? WHERE id = ?',
            (archived, series_idnum))

    def series_set_name(self, series_idnum, name):
        """Update name for a series

        Args:
            series_idnum (int): ID num of the series
            name (str): new name to use
        """
        self.execute(
            'UPDATE series SET name = ? WHERE id = ?', (name, series_idnum))

    # ser_ver functions

    def ser_ver_get_link(self, series_idnum, version):
        """Get the link for a series version

        Args:
            series_idnum (int): ID num of the series
            version (int): Version number to search for

        Return:
            str: Patchwork link as a string, e.g. '12325', or None if none

        Raises:
            ValueError: Multiple matches are found
        """
        res = self.execute(
            'SELECT link FROM ser_ver WHERE '
            f"series_id = {series_idnum} AND version = '{version}'")
        recs = res.fetchall()
        if not recs:
            return None
        if len(recs) > 1:
            raise ValueError('Expected one match, but multiple matches found')
        return recs[0][0]

    def ser_ver_set_link(self, series_idnum, version, link):
        """Set the link for a series version

        Args:
            series_idnum (int): ID num of the series
            version (int): Version number to search for
            link (str): Patchwork link for the ser_ver

        Return:
            bool: True if the record was found and updated, else False
        """
        if link is None:
            link = ''
        self.execute(
            'UPDATE ser_ver SET link = ? WHERE series_id = ? AND version = ?',
            (str(link), series_idnum, version))
        return self.rowcount() != 0

    def ser_ver_set_info(self, info):
        """Set the info for a series version

        Args:
            info (SER_VER): Info to set. Only two options are supported:
                1: svid,cover_id,cover_num_comments,name
                2: svid,name

        Return:
            bool: True if the record was found and updated, else False
        """
        assert info.idnum is not None
        if info.cover_id:
            assert info.series_id is None
            self.execute(
                'UPDATE ser_ver SET cover_id = ?, cover_num_comments = ?, '
                'name = ? WHERE id = ?',
                (info.cover_id, info.cover_num_comments, info.name,
                 info.idnum))
        else:
            assert not info.cover_id
            assert not info.cover_num_comments
            assert not info.series_id
            assert not info.version
            assert not info.link
            self.execute('UPDATE ser_ver SET name = ? WHERE id = ?',
                         (info.name, info.idnum))

        return self.rowcount() != 0

    def ser_ver_set_version(self, svid, version):
        """Sets the version for a ser_ver record

        Args:
            svid (int): Record ID to update
            version (int): Version number to add

        Raises:
            ValueError: svid was not found
        """
        self.execute(
            'UPDATE ser_ver SET version = ? WHERE id = ?', (version, svid))
        if self.rowcount() != 1:
            raise ValueError(f'No ser_ver updated (svid {svid})')

    def ser_ver_set_archive_tag(self, svid, tag):
        """Sets the archive tag for a ser_ver record

        Args:
            svid (int): Record ID to update
            tag (tag): Tag to add

        Raises:
            ValueError: svid was not found
        """
        self.execute(
            'UPDATE ser_ver SET archive_tag = ? WHERE id = ?', (tag, svid))
        if self.rowcount() != 1:
            raise ValueError(f'No ser_ver updated (svid {svid})')

    def ser_ver_add(self, series_idnum, version, link=None):
        """Add a new ser_ver record

        Args:
            series_idnum (int): ID num of the series which is getting a new
                version
            version (int): Version number to add
            link (str): Patchwork link, or None if not known

        Return:
            int: ID num of the new ser_ver record
        """
        self.execute(
            'INSERT INTO ser_ver (series_id, version, link) VALUES (?, ?, ?)',
            (series_idnum, version, link))
        return self.lastrowid()

    def ser_ver_get_for_series(self, series_idnum, version=None):
        """Get a list of ser_ver records for a given series ID

        Args:
            series_idnum (int): ID num of the series to search
            version (int): Version number to search for, or None for all

        Return:
            SER_VER: Requested information

        Raises:
            ValueError: There is no matching idnum/version
        """
        base = ('SELECT id, series_id, version, link, cover_id, '
                'cover_num_comments, name, archive_tag FROM ser_ver '
                'WHERE series_id = ?')
        if version:
            res = self.execute(base + ' AND version = ?',
                               (series_idnum, version))
        else:
            res = self.execute(base, (series_idnum,))
        recs = res.fetchall()
        if not recs:
            raise ValueError(
                f'No matching series for id {series_idnum} version {version}')
        if version:
            return SerVer(*recs[0])
        return [SerVer(*x) for x in recs]

    def ser_ver_get_ids_for_series(self, series_idnum, version=None):
        """Get a list of ser_ver records for a given series ID

        Args:
            series_idnum (int): ID num of the series to search
            version (int): Version number to search for, or None for all

        Return:
            list of int: List of svids for the matching records
        """
        if version:
            res = self.execute(
                'SELECT id FROM ser_ver WHERE series_id = ? AND version = ?',
                (series_idnum, version))
        else:
            res = self.execute(
                'SELECT id FROM ser_ver WHERE series_id = ?', (series_idnum,))
        return list(res.fetchall()[0])

    def ser_ver_get_list(self):
        """Get a list of patchwork entries from the database

        Return:
            list of SER_VER
        """
        res = self.execute(
            'SELECT id, series_id, version, link, cover_id, '
            'cover_num_comments, name, archive_tag FROM ser_ver')
        items = res.fetchall()
        return [SerVer(*x) for x in items]

    def ser_ver_remove(self, series_idnum, version=None, remove_pcommits=True,
                       remove_series=True):
        """Delete a ser_ver record

        Removes the record which has the given series ID num and version

        Args:
            series_idnum (int): ID num of the series
            version (int): Version number, or None to remove all versions
            remove_pcommits (bool): True to remove associated pcommits too
            remove_series (bool): True to remove the series if versions is None
        """
        if remove_pcommits:
            # Figure out svids to delete
            svids = self.ser_ver_get_ids_for_series(series_idnum, version)

            self.pcommit_delete_list(svids)

        if version:
            self.execute(
                'DELETE FROM ser_ver WHERE series_id = ? AND version = ?',
                (series_idnum, version))
        else:
            self.execute(
                'DELETE FROM ser_ver WHERE series_id = ?',
                (series_idnum,))
        if not version and remove_series:
            self.series_remove(series_idnum)

    # pcommit functions

    def pcommit_get_list(self, find_svid=None):
        """Get a dict of pcommits entries from the database

        Args:
            find_svid (int): If not None, finds the records associated with a
                particular series and version; otherwise returns all records

        Return:
            list of PCOMMIT: pcommit records
        """
        query = ('SELECT id, seq, subject, svid, change_id, state, patch_id, '
                 'num_comments FROM pcommit')
        if find_svid is not None:
            query += f' WHERE svid = {find_svid}'
        res = self.execute(query)
        return [Pcommit(*rec) for rec in res.fetchall()]

    def pcommit_add_list(self, svid, pcommits):
        """Add records to the pcommit table

        Args:
            svid (int): ser_ver ID num
            pcommits (list of PCOMMIT): Only seq, subject, change_id are
                uses; svid comes from the argument passed in and the others
                are assumed to be obtained from patchwork later
        """
        for pcm in pcommits:
            self.execute(
                'INSERT INTO pcommit (svid, seq, subject, change_id) VALUES '
                '(?, ?, ?, ?)', (svid, pcm.seq, pcm.subject, pcm.change_id))

    def pcommit_delete(self, svid):
        """Delete pcommit records for a given ser_ver ID

        Args_:
            svid (int): ser_ver ID num of records to delete
        """
        self.execute('DELETE FROM pcommit WHERE svid = ?', (svid,))

    def pcommit_delete_list(self, svid_list):
        """Delete pcommit records for a given set of ser_ver IDs

        Args_:
            svid (list int): ser_ver ID nums of records to delete
        """
        vals = ', '.join([str(x) for x in svid_list])
        self.execute('DELETE FROM pcommit WHERE svid IN (?)', (vals,))

    def pcommit_update(self, pcm):
        """Update a pcommit record

        Args:
            pcm (PCOMMIT): Information to write; only the idnum, state,
                patch_id and num_comments are used

        Return:
            True if the data was written
        """
        self.execute(
            'UPDATE pcommit SET '
            'patch_id = ?, state = ?, num_comments = ? WHERE id = ?',
            (pcm.patch_id, pcm.state, pcm.num_comments, pcm.idnum))
        return self.rowcount() > 0

    # upstream functions

    def upstream_add(self, name, url):
        """Add a new upstream record

        Args:
            name (str): Name of the tree
            url (str): URL for the tree

        Raises:
            ValueError if the name already exists in the database
        """
        try:
            self.execute(
                'INSERT INTO upstream (name, url) VALUES (?, ?)', (name, url))
        except sqlite3.IntegrityError as exc:
            if 'UNIQUE constraint failed: upstream.name' in str(exc):
                raise ValueError(f"Upstream '{name}' already exists") from exc

    def upstream_set_default(self, name):
        """Mark (only) the given upstream as the default

        Args:
            name (str): Name of the upstream remote to set as default, or None

        Raises:
            ValueError if more than one name matches (should not happen);
                database is rolled back
        """
        self.execute("UPDATE upstream SET is_default = 0")
        if name is not None:
            self.execute(
                'UPDATE upstream SET is_default = 1 WHERE name = ?', (name,))
            if self.rowcount() != 1:
                self.rollback()
                raise ValueError(f"No such upstream '{name}'")

    def upstream_get_default(self):
        """Get the name of the default upstream

        Return:
            str: Default-upstream name, or None if there is no default
        """
        res = self.execute(
            "SELECT name FROM upstream WHERE is_default = 1")
        recs = res.fetchall()
        if len(recs) != 1:
            return None
        return recs[0][0]

    def upstream_delete(self, name):
        """Delete an upstream target

        Args:
            name (str): Name of the upstream remote to delete

        Raises:
            ValueError: Upstream does not exist (database is rolled back)
        """
        self.execute(f"DELETE FROM upstream WHERE name = '{name}'")
        if self.rowcount() != 1:
            self.rollback()
            raise ValueError(f"No such upstream '{name}'")

    def upstream_get_dict(self):
        """Get a list of upstream entries from the database

        Return:
            OrderedDict:
                key (str): upstream name
                value (str): url
        """
        res = self.execute('SELECT name, url, is_default FROM upstream')
        udict = OrderedDict()
        for name, url, is_default in res.fetchall():
            udict[name] = url, is_default
        return udict

    # settings functions

    def settings_update(self, name, proj_id, link_name):
        """Set the patchwork settings of the project

        Args:
            name (str): Name of the project to use in patchwork
            proj_id (int): Project ID for the project
            link_name (str): Link name for the project
        """
        self.execute('DELETE FROM settings')
        self.execute(
                'INSERT INTO settings (name, proj_id, link_name) '
                'VALUES (?, ?, ?)', (name, proj_id, link_name))

    def settings_get(self):
        """Get the patchwork settings of the project

        Returns:
            tuple or None if there are no settings:
                name (str): Project name, e.g. 'U-Boot'
                proj_id (int): Patchworks project ID for this project
                link_name (str): Patchwork's link-name for the project
        """
        res = self.execute("SELECT name, proj_id, link_name FROM settings")
        recs = res.fetchall()
        if len(recs) != 1:
            return None
        return recs[0]

# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2025 Google LLC
#
"""Handles the 'series' subcommand
"""

import asyncio
from collections import OrderedDict, defaultdict

import pygit2

from u_boot_pylib import cros_subprocess
from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tout

from patman import patchstream
from patman import cser_helper
from patman.cser_helper import AUTOLINK, oid
from patman import send
from patman import status


class Cseries(cser_helper.CseriesHelper):
    """Database with information about series

    This class handles database read/write as well as operations in a git
    directory to update series information.
    """
    def __init__(self, topdir=None, colour=terminal.COLOR_IF_TERMINAL):
        """Set up a new Cseries

        Args:
            topdir (str): Top-level directory of the repo
            colour (terminal.enum): Whether to enable ANSI colour or not
        """
        super().__init__(topdir, colour)

    def add(self, branch_name, desc=None, mark=False, allow_unmarked=False,
            end=None, force_version=False, dry_run=False):
        """Add a series (or new version of a series) to the database

        Args:
            branch_name (str): Name of branch to sync, or None for current one
            desc (str): Description to use, or None to use the series subject
            mark (str): True to mark each commit with a change ID
            allow_unmarked (str): True to not require each commit to be marked
            end (str): Add only commits up to but exclu
            force_version (bool): True if ignore a Series-version tag that
                doesn't match its branch name
            dry_run (bool): True to do a dry run
        """
        name, ser, version, msg = self.prep_series(branch_name, end)
        tout.info(f"Adding series '{ser.name}' v{version}: mark {mark} "
                  f'allow_unmarked {allow_unmarked}')
        if msg:
            tout.info(msg)
        if desc is None:
            if not ser.cover:
                raise ValueError(f"Branch '{name}' has no cover letter - "
                                 'please provide description')
            desc = ser['cover'][0]

        ser = self._handle_mark(name, ser, version, mark, allow_unmarked,
                                force_version, dry_run)
        link = ser.get_link_for_version(version)

        msg = 'Added'
        added = False
        series_id = self.db.series_find_by_name(ser.name)
        if not series_id:
            series_id = self.db.series_add(ser.name, desc)
            added = True
            msg += f" series '{ser.name}'"

        if version not in self._get_version_list(series_id):
            svid = self.db.ser_ver_add(series_id, version, link)
            msg += f" v{version}"
            if not added:
                msg += f" to existing series '{ser.name}'"
            added = True

            self._add_series_commits(ser, svid)
            count = len(ser.commits)
            msg += f" ({count} commit{'s' if count > 1 else ''})"
        if not added:
            tout.info(f"Series '{ser.name}' v{version} already exists")
            msg = None
        elif not dry_run:
            self.commit()
        else:
            self.rollback()
            series_id = None
        ser.desc = desc
        ser.idnum = series_id

        if msg:
            tout.info(msg)
        if dry_run:
            tout.info('Dry run completed')

    def decrement(self, series, dry_run=False):
        """Decrement a series to the previous version and delete the branch

        Args:
            series (str): Name of series to use, or None to use current branch
            dry_run (bool): True to do a dry run
        """
        ser = self._parse_series(series)
        if not ser.idnum:
            raise ValueError(f"Series '{ser.name}' not found in database")

        max_vers = self._series_max_version(ser.idnum)
        if max_vers < 2:
            raise ValueError(f"Series '{ser.name}' only has one version")

        tout.info(f"Removing series '{ser.name}' v{max_vers}")

        new_max = max_vers - 1

        repo = pygit2.init_repository(self.gitdir)
        if not dry_run:
            name = self._get_branch_name(ser.name, new_max)
            branch = repo.lookup_branch(name)
            try:
                repo.checkout(branch)
            except pygit2.errors.GitError:
                tout.warning(f"Failed to checkout branch {name}")
                raise

            del_name = f'{ser.name}{max_vers}'
            del_branch = repo.lookup_branch(del_name)
            branch_oid = del_branch.peel(pygit2.enums.ObjectType.COMMIT).oid
            del_branch.delete()
            print(f"Deleted branch '{del_name}' {oid(branch_oid)}")

        self.db.ser_ver_remove(ser.idnum, max_vers)
        if not dry_run:
            self.commit()
        else:
            self.rollback()

    def increment(self, series_name, dry_run=False):
        """Increment a series to the next version and create a new branch

        Args:
            series_name (str): Name of series to use, or None to use current
                branch
            dry_run (bool): True to do a dry run
        """
        ser = self._parse_series(series_name)
        if not ser.idnum:
            raise ValueError(f"Series '{ser.name}' not found in database")

        max_vers = self._series_max_version(ser.idnum)

        branch_name = self._get_branch_name(ser.name, max_vers)
        on_branch = gitutil.get_branch(self.gitdir) == branch_name
        svid = self.get_series_svid(ser.idnum, max_vers)
        pwc = self.get_pcommit_dict(svid)
        count = len(pwc.values())
        series = patchstream.get_metadata(branch_name, 0, count,
                                          git_dir=self.gitdir)
        tout.info(f"Increment '{ser.name}' v{max_vers}: {count} patches")

        # Create a new branch
        vers = max_vers + 1
        new_name = self._join_name_version(ser.name, vers)

        self.update_series(branch_name, series, max_vers, new_name, dry_run,
                            add_vers=vers, switch=on_branch)

        old_svid = self.get_series_svid(ser.idnum, max_vers)
        pcd = self.get_pcommit_dict(old_svid)

        svid = self.db.ser_ver_add(ser.idnum, vers)
        self.db.pcommit_add_list(svid, pcd.values())
        if not dry_run:
            self.commit()
        else:
            self.rollback()

        # repo.head.set_target(amended)
        tout.info(f'Added new branch {new_name}')
        if dry_run:
            tout.info('Dry run completed')

    def link_set(self, series_name, version, link, update_commit):
        """Add / update a series-links link for a series

        Args:
            series_name (str): Name of series to use, or None to use current
                branch
            version (int): Version number, or None to detect from name
            link (str): Patchwork link-string for the series
            update_commit (bool): True to update the current commit with the
                link
        """
        ser, version = self._parse_series_and_version(series_name, version)
        self._ensure_version(ser, version)

        self._set_link(ser.idnum, ser.name, version, link, update_commit)
        self.commit()
        tout.info(f"Setting link for series '{ser.name}' v{version} to {link}")

    def link_get(self, series, version):
        """Get the patchwork link for a version of a series

        Args:
            series (str): Name of series to use, or None to use current branch
            version (int): Version number or None for current

        Return:
            str: Patchwork link as a string, e.g. '12325'
        """
        ser, version = self._parse_series_and_version(series, version)
        self._ensure_version(ser, version)
        return self.db.ser_ver_get_link(ser.idnum, version)

    def link_search(self, pwork, series, version):
        """Search patch for the link for a series

        Returns either the single match, or None, in which case the second part
        of the tuple is filled in

        Args:
            pwork (Patchwork): Patchwork object to use
            series (str): Series name to search for, or None for current series
                that is checked out
            version (int): Version to search for, or None for current version
                detected from branch name

        Returns:
            tuple:
                int: ID of the series found, or None
                list of possible matches, or None, each a dict:
                    'id': series ID
                    'name': series name
                str: series name
                int: series version
                str: series description
        """
        _, ser, version, _, _, _, _, _ = self._get_patches(series, version)

        if not ser.desc:
            raise ValueError(f"Series '{ser.name}' has an empty description")

        pws, options = self.loop.run_until_complete(pwork.find_series(
            ser, version))
        return pws, options, ser.name, version, ser.desc

    def link_auto(self, pwork, series, version, update_commit, wait_s=0):
        """Automatically find a series link by looking in patchwork

        Args:
            pwork (Patchwork): Patchwork object to use
            series (str): Series name to search for, or None for current series
                that is checked out
            version (int): Version to search for, or None for current version
                detected from branch name
            update_commit (bool): True to update the current commit with the
                link
            wait_s (int): Number of seconds to wait for the autolink to succeed
        """
        start = self.get_time()
        stop = start + wait_s
        sleep_time = 5
        while True:
            pws, options, name, version, desc = self.link_search(
                pwork, series, version)
            if pws:
                if wait_s:
                    tout.info('Link completed after '
                              f'{self.get_time() - start} seconds')
                break

            print(f"Possible matches for '{name}' v{version} desc '{desc}':")
            print('  Link  Version  Description')
            for opt in options:
                print(f"{opt['id']:6}  {opt['version']:7}  {opt['name']}")
            if not wait_s or self.get_time() > stop:
                delay = f' after {wait_s} seconds' if wait_s else ''
                raise ValueError(f"Cannot find series '{desc}{delay}'")

            self.sleep(sleep_time)

        self.link_set(name, version, pws, update_commit)

    def link_auto_all(self, pwork, update_commit, link_all_versions,
                      replace_existing, dry_run, show_summary=True):
        """Automatically find a series link by looking in patchwork

        Args:
            pwork (Patchwork): Patchwork object to use
            update_commit (bool): True to update the current commit with the
                link
            link_all_versions (bool): True to sync all versions of a series,
                False to sync only the latest version
            replace_existing (bool): True to sync a series even if it already
                has a link
            dry_run (bool): True to do a dry run
            show_summary (bool): True to show a summary of how things went

        Return:
            OrderedDict of summary info:
                key (int): ser_ver ID
                value (AUTOLINK): result of autolinking on this ser_ver
        """
        sdict = self.db.series_get_dict_by_id()
        all_ser_vers = self._get_autolink_dict(sdict, link_all_versions)

        # Get rid of things without a description
        valid = {}
        state = {}
        no_desc = 0
        not_found = 0
        updated = 0
        failed = 0
        already = 0
        for svid, (ser_id, name, version, link, desc) in all_ser_vers.items():
            if link and not replace_existing:
                state[svid] = f'already:{link}'
                already += 1
            elif desc:
                valid[svid] = ser_id, version, link, desc
            else:
                no_desc += 1
                state[svid] = 'missing description'

        results, requests = self.loop.run_until_complete(
            pwork.find_series_list(valid))

        for svid, ser_id, link, _ in results:
            if link:
                version = all_ser_vers[svid][2]
                if self._set_link(ser_id, sdict[ser_id].name, version,
                                  link, update_commit, dry_run=dry_run):
                    updated += 1
                    state[svid] = f'linked:{link}'
                else:
                    failed += 1
                    state[svid] = 'failed'
            else:
                not_found += 1
                state[svid] = 'not found'

        # Create a summary sorted by name and version
        summary = OrderedDict()
        for svid in sorted(all_ser_vers, key=lambda k: all_ser_vers[k][1:2]):
            _, name, version, link, ser = all_ser_vers[svid]
            summary[svid] = AUTOLINK(name, version, link, ser.desc,
                                     state[svid])

        if show_summary:
            msg = f'{updated} series linked'
            if already:
                msg += f', {already} already linked'
            if not_found:
                msg += f', {not_found} not found'
            if no_desc:
                msg += f', {no_desc} missing description'
            if failed:
                msg += f', {failed} updated failed'
            tout.info(msg + f' ({requests} requests)')

            tout.info('')
            tout.info(f"{'Name':15}  Version  {'Description':40}  Result")
            border = f"{'-' * 15}  -------  {'-' * 40}  {'-' * 15}"
            tout.info(border)
            for name, version, link, desc, state in summary.values():
                bright = True
                if state.startswith('already'):
                    col = self.col.GREEN
                    bright = False
                elif state.startswith('linked'):
                    col = self.col.MAGENTA
                else:
                    col = self.col.RED
                col_state = self.col.build(col, state, bright)
                tout.info(f"{name:16.16} {version:7}  {desc or '':40.40}  "
                          f'{col_state}')
            tout.info(border)
        if dry_run:
            tout.info('Dry run completed')

        return summary

    def series_list(self):
        """List all series

        Lines all series along with their description, number of patches
        accepted and  the available versions
        """
        sdict = self.db.series_get_dict()
        print(f"{'Name':15}  {'Description':40}  Accepted  Versions")
        border = f"{'-' * 15}  {'-' * 40}  --------  {'-' * 15}"
        print(border)
        for name in sorted(sdict):
            ser = sdict[name]
            versions = self._get_version_list(ser.idnum)
            stat = self._series_get_version_stats(
                ser.idnum, self._series_max_version(ser.idnum))[0]

            vlist = ' '.join([str(ver) for ver in sorted(versions)])

            print(f'{name:16.16} {ser.desc:41.41} {stat.rjust(8)}  {vlist}')
        print(border)

    def list_patches(self, series, version, show_commit=False,
                     show_patch=False):
        """List patches in a series

        Args:
            series (str): Name of series to use, or None to use current branch
            version (int): Version number, or None to detect from name
            show_commit (bool): True to show the commit and diffstate
            show_patch (bool): True to show the patch
        """
        branch, series, version, pwc, name, _, cover_id, num_comments = (
            self._get_patches(series, version))
        with terminal.pager():
            state_totals = defaultdict(int)
            self._list_patches(branch, pwc, series, name, cover_id,
                               num_comments, show_commit, show_patch, True,
                               state_totals)

    def mark(self, in_name, allow_marked=False, dry_run=False):
        """Add Change-Id tags to a series

        Args:
            in_name (str): Name of the series to unmark
            allow_marked (bool): Allow commits to be (already) marked
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards

        Return:
            pygit.oid: oid of the new branch
        """
        name, ser, _, _ = self.prep_series(in_name)
        tout.info(f"Marking series '{name}': allow_marked {allow_marked}")

        if not allow_marked:
            bad = []
            for cmt in ser.commits:
                if cmt.change_id:
                    bad.append(cmt)
            if bad:
                print(f'{len(bad)} commit(s) already have marks')
                for cmt in bad:
                    print(f' - {oid(cmt.hash)} {cmt.subject}')
                raise ValueError(
                    f'Marked commits {len(bad)}/{len(ser.commits)}')
        new_oid = self._mark_series(in_name, ser, dry_run=dry_run)

        if dry_run:
            tout.info('Dry run completed')
        return new_oid

    def unmark(self, name, allow_unmarked=False, dry_run=False):
        """Remove Change-Id tags from a series

        Args:
            name (str): Name of the series to unmark
            allow_unmarked (bool): Allow commits to be (already) unmarked
            dry_run (bool): True to do a dry run, restoring the original tree
                afterwards

        Return:
            pygit.oid: oid of the new branch
        """
        name, ser, _, _ = self.prep_series(name)
        tout.info(
            f"Unmarking series '{name}': allow_unmarked {allow_unmarked}")

        if not allow_unmarked:
            bad = []
            for cmt in ser.commits:
                if not cmt.change_id:
                    bad.append(cmt)
            if bad:
                print(f'{len(bad)} commit(s) are missing marks')
                for cmt in bad:
                    print(f' - {oid(cmt.hash)} {cmt.subject}')
                raise ValueError(
                    f'Unmarked commits {len(bad)}/{len(ser.commits)}')
        vals = None
        for vals in self.process_series(name, ser, dry_run=dry_run):
            if cser_helper.CHANGE_ID_TAG in vals.msg:
                lines = vals.msg.splitlines()
                updated = [line for line in lines
                           if not line.startswith(cser_helper.CHANGE_ID_TAG)]
                vals.msg = '\n'.join(updated)

                tout.detail("   - removing mark")
                vals.info = 'unmarked'
            else:
                vals.info = 'no mark'

        if dry_run:
            tout.info('Dry run completed')
        return vals.oid

    def open(self, pwork, name, version):
        """Open the patchwork page for a series

        Args:
            pwork (Patchwork): Patchwork object to use
            name (str): Name of series to open
            version (str): Version number to open
        """
        ser, version = self._parse_series_and_version(name, version)
        link = self.link_get(ser.name, version)
        pwork.url = 'https://patchwork.ozlabs.org'
        url = self.loop.run_until_complete(pwork.get_series_url(link))
        print(f'Opening {url}')

        # With Firefox, GTK produces lots of warnings, so suppress them
        # Gtk-Message: 06:48:20.692: Failed to load module "xapp-gtk3-module"
        # Gtk-Message: 06:48:20.692: Not loading module "atk-bridge": The
        # functionality is provided by GTK natively. Please try to not load it.
        # Gtk-Message: 06:48:20.692: Failed to load module "appmenu-gtk-module"
        # Gtk-Message: 06:48:20.692: Failed to load module "appmenu-gtk-module"
        # [262145, Main Thread] WARNING: GTK+ module /snap/firefox/5987/
        #  gnome-platform/usr/lib/gtk-2.0/modules/libcanberra-gtk-module.so
        #  cannot be loaded.
        # GTK+ 2.x symbols detected. Using GTK+ 2.x and GTK+ 3 in the same
        #  process #  is not supported.: 'glib warning', file /build/firefox/
        #  parts/firefox/build/toolkit/xre/nsSigHandlers.cpp:201
        #
        # (firefox_firefox:262145): Gtk-WARNING **: 06:48:20.728: GTK+ module
        #  /snap/firefox/5987/gnome-platform/usr/lib/gtk-2.0/modules/
        #  libcanberra-gtk-module.so cannot be loaded.
        # GTK+ 2.x symbols detected. Using GTK+ 2.x and GTK+ 3 in the same
        #  process is not supported.
        # Gtk-Message: 06:48:20.728: Failed to load module
        #  "canberra-gtk-module"
        # [262145, Main Thread] WARNING: GTK+ module /snap/firefox/5987/
        #  gnome-platform/usr/lib/gtk-2.0/modules/libcanberra-gtk-module.so
        #  cannot be loaded.
        # GTK+ 2.x symbols detected. Using GTK+ 2.x and GTK+ 3 in the same
        #  process is not supported.: 'glib warning', file /build/firefox/
        #  parts/firefox/build/toolkit/xre/nsSigHandlers.cpp:201
        #
        # (firefox_firefox:262145): Gtk-WARNING **: 06:48:20.729: GTK+ module
        #   /snap/firefox/5987/gnome-platform/usr/lib/gtk-2.0/modules/
        #   libcanberra-gtk-module.so cannot be loaded.
        # GTK+ 2.x symbols detected. Using GTK+ 2.x and GTK+ 3 in the same
        #  process is not supported.
        # Gtk-Message: 06:48:20.729: Failed to load module
        #  "canberra-gtk-module"
        # ATTENTION: default value of option mesa_glthread overridden by
        # environment.
        cros_subprocess.Popen(['xdg-open', url])

    def progress(self, series, show_all_versions, list_patches):
        """Show progress information for all versions in a series

        Args:
            series (str): Name of series to use, or None to show progress for
                all series
            show_all_versions (bool): True to show all versions of a series,
                False to show only the final version
            list_patches (bool): True to list all patches for each series,
                False to just show the series summary on a single line
        """
        with terminal.pager():
            state_totals = defaultdict(int)
            if series is not None:
                _, _, need_scan = self._progress_one(
                    self._parse_series(series), show_all_versions,
                    list_patches, state_totals)
                if need_scan:
                    tout.warning(
                        'Inconsistent commit-subject: Please use '
                        "'patman series -s <branch> scan' to resolve this")
                return

            total_patches = 0
            total_series = 0
            sdict = self.db.series_get_dict()
            border = None
            total_need_scan = 0
            if not list_patches:
                print(self.col.build(
                    self.col.MAGENTA,
                    f"{'Name':16} {'Description':41} Count  {'Status'}"))
                border = f"{'-' * 15}  {'-' * 40}  -----  {'-' * 15}"
                print(border)
            for name in sorted(sdict):
                ser = sdict[name]
                num_series, num_patches, need_scan = self._progress_one(
                    ser, show_all_versions, list_patches, state_totals)
                total_need_scan += need_scan
                if list_patches:
                    print()
                total_series += num_series
                total_patches += num_patches
            if not list_patches:
                print(border)
                total = f'{total_series} series'
                out = ''
                for state, freq in state_totals.items():
                    out += ' ' + self._build_col(state, f'{freq}:')[0]
                if total_need_scan:
                    out = '*' + out[1:]

                print(f"{total:15}  {'':40}  {total_patches:5} {out}")
                if total_need_scan:
                    tout.info(
                        f'Series marked * ({total_need_scan}) have commit '
                        'subjects which mismatch their patches and need to be '
                        'scanned')

    def project_set(self, pwork, name, quiet=False):
        """Set the name of the project

        Args:
            pwork (Patchwork): Patchwork object to use
            name (str): Name of the project to use in patchwork
            quiet (bool): True to skip writing the message
        """
        res = self.loop.run_until_complete(pwork.get_projects())
        proj_id = None
        link_name = None
        for proj in res:
            if proj['name'] == name:
                proj_id = proj['id']
                link_name = proj['link_name']
        if not proj_id:
            raise ValueError(f"Unknown project name '{name}'")
        self.db.settings_update(name, proj_id, link_name)
        self.commit()
        if not quiet:
            tout.info(f"Project '{name}' patchwork-ID {proj_id} "
                      f'link-name {link_name}')

    def project_get(self):
        """Get the details of the project

        Returns:
            tuple or None if there are no settings:
                name (str): Project name, e.g. 'U-Boot'
                proj_id (int): Patchworks project ID for this project
                link_name (str): Patchwork's link-name for the project
        """
        return self.db.settings_get()

    def remove(self, name, dry_run=False):
        """Remove a series from the database

        Args:
            name (str): Name of series to remove, or None to use current one
            dry_run (bool): True to do a dry run
        """
        ser = self._parse_series(name)
        name = ser.name
        if not ser.idnum:
            raise ValueError(f"No such series '{name}'")

        self.db.ser_ver_remove(ser.idnum, None)
        if not dry_run:
            self.commit()
        else:
            self.rollback()

        self.commit()
        tout.info(f"Removed series '{name}'")
        if dry_run:
            tout.info('Dry run completed')

    def rename(self, series, name, dry_run=False):
        """Rename a series

        Renames a series and changes the name of any branches which match
        versions present in the database

        Args:
            series (str): Name of series to use, or None to use current branch
            name (str): new name to use (must not include version number)
            dry_run (bool): True to do a dry run
        """
        old_ser, _ = self._parse_series_and_version(series, None)
        if not old_ser.idnum:
            raise ValueError(f"Series '{old_ser.name}' not found in database")
        if old_ser.name != series:
            raise ValueError(f"Invalid series name '{series}': "
                             'did you use the branch name?')
        chk, _ = cser_helper.split_name_version(name)
        if chk != name:
            raise ValueError(
                f"Invalid series name '{name}': did you use the branch name?")
        if chk == old_ser.name:
            raise ValueError(
                f"Cannot rename series '{old_ser.name}' to itself")
        if self.get_series_by_name(name):
            raise ValueError(f"Cannot rename: series '{name}' already exists")

        versions = self._get_version_list(old_ser.idnum)
        missing = []
        exists = []
        todo = {}
        for ver in versions:
            ok = True
            old_branch = self._get_branch_name(old_ser.name, ver)
            if not gitutil.check_branch(old_branch, self.gitdir):
                missing.append(old_branch)
                ok = False

            branch = self._get_branch_name(name, ver)
            if gitutil.check_branch(branch, self.gitdir):
                exists.append(branch)
                ok = False

            if ok:
                todo[ver] = [old_branch, branch]

        if missing or exists:
            msg = 'Cannot rename'
            if missing:
                msg += f": branches missing: {', '.join(missing)}"
            if exists:
                msg += f": branches exist: {', '.join(exists)}"
            raise ValueError(msg)

        for old_branch, branch in todo.values():
            tout.info(f"Renaming branch '{old_branch}' to '{branch}'")
            if not dry_run:
                gitutil.rename_branch(old_branch, branch, self.gitdir)

        # Change the series name; nothing needs to change in ser_ver
        self.db.series_set_name(old_ser.idnum, name)

        if not dry_run:
            self.commit()
        else:
            self.rollback()

        tout.info(f"Renamed series '{series}' to '{name}'")
        if dry_run:
            tout.info('Dry run completed')

    def scan(self, branch_name, mark=False, allow_unmarked=False, end=None,
             dry_run=False):
        """Scan a branch and make updates to the database if it has changed

        Args:
            branch_name (str): Name of branch to sync, or None for current one
            mark (str): True to mark each commit with a change ID
            allow_unmarked (str): True to not require each commit to be marked
            end (str): Add only commits up to but exclu
            dry_run (bool): True to do a dry run
        """
        def _show_item(oper, seq, subject):
            col = None
            if oper == '+':
                col = self.col.GREEN
            elif oper == '-':
                col = self.col.RED
            out = self.col.build(col, subject) if col else subject
            tout.info(f'{oper} {seq:3} {out}')

        name, ser, version, msg = self.prep_series(branch_name, end)
        svid = self.get_ser_ver(ser.idnum, version).idnum
        pcdict = self.get_pcommit_dict(svid)

        tout.info(
            f"Syncing series '{name}' v{version}: mark {mark} "
            f'allow_unmarked {allow_unmarked}')
        if msg:
            tout.info(msg)

        ser = self._handle_mark(name, ser, version, mark, allow_unmarked,
                                False, dry_run)

        # First check for new patches that are not in the database
        to_add = dict(enumerate(ser.commits))
        for pcm in pcdict.values():
            tout.debug(f'pcm {pcm.subject}')
            i = self._find_matched_commit(to_add, pcm)
            if i is not None:
                del to_add[i]

        # Now check for patches in the database that are not in the branch
        to_remove = dict(enumerate(pcdict.values()))
        for cmt in ser.commits:
            tout.debug(f'cmt {cmt.subject}')
            i = self._find_matched_patch(to_remove, cmt)
            if i is not None:
                del to_remove[i]

        for seq, cmt in enumerate(ser.commits):
            if seq in to_remove:
                _show_item('-', seq, to_remove[seq].subject)
                del to_remove[seq]
            if seq in to_add:
                _show_item('+', seq, to_add[seq].subject)
                del to_add[seq]
            else:
                _show_item(' ', seq, cmt.subject)
        seq = len(ser.commits)
        for cmt in to_add.items():
            _show_item('+', seq, cmt.subject)
            seq += 1
        for seq, pcm in to_remove.items():
            _show_item('+', seq, pcm.subject)

        self.db.pcommit_delete(svid)
        self._add_series_commits(ser, svid)
        if not dry_run:
            self.commit()
        else:
            self.rollback()
            tout.info('Dry run completed')

    def send(self, pwork, name, autolink, autolink_wait, args):
        """Send out a series

        Args:
            pwork (Patchwork): Patchwork object to use
            name (str): Series name to search for, or None for current series
                that is checked out
            autolink (bool): True to auto-link the series after sending
            args (argparse.Namespace): 'send' arguments provided
            autolink_wait (int): Number of seconds to wait for the autolink to
                succeed
        """
        ser, version = self._parse_series_and_version(name, None)
        if not ser.idnum:
            raise ValueError(f"Series '{ser.name}' not found in database")

        args.branch = self._get_branch_name(ser.name, version)
        likely_sent = send.send(args, git_dir=self.gitdir, cwd=self.topdir)

        if likely_sent and autolink:
            print(f'Autolinking with Patchwork ({autolink_wait} seconds)')
            self.link_auto(pwork, name, version, True, wait_s=autolink_wait)

    def archive(self, series):
        """Archive a series

        Args:
            series (str): Name of series to use, or None to use current branch
        """
        ser = self._parse_series(series, include_archived=True)
        if not ser.idnum:
            raise ValueError(f"Series '{ser.name}' not found in database")

        svlist = self.db.ser_ver_get_for_series(ser.idnum)

        # Figure out the tags we will create
        tag_info = {}
        now = self.get_now()
        now_str = now.strftime('%d%b%y').lower()
        for svi in svlist:
            name = self._get_branch_name(ser.name, svi.version)
            if not gitutil.check_branch(name, git_dir=self.gitdir):
                raise ValueError(f"No branch named '{name}'")
            tag_info[svi.version] = [svi.idnum, name, f'{name}-{now_str}']

        # Create the tags
        repo = pygit2.init_repository(self.gitdir)
        for _, (idnum, name, tag_name) in tag_info.items():
            commit = repo.revparse_single(name)
            repo.create_tag(tag_name, commit.hex,
                            pygit2.enums.ObjectType.COMMIT,
                            commit.author, commit.message)

        # Update the database
        for idnum, name, tag_name in tag_info.values():
            self.db.ser_ver_set_archive_tag(idnum, tag_name)

        # Delete the branches
        for idnum, name, tag_name in tag_info.values():
            # Detach HEAD from the branch if pointing to this branch
            commit = repo.revparse_single(name)
            if repo.head.target == commit.oid:
                repo.set_head(commit.oid)

            repo.branches.delete(name)

        self.db.series_set_archived(ser.idnum, True)
        self.commit()

    def unarchive(self, series):
        """Unarchive a series

        Args:
            series (str): Name of series to use, or None to use current branch
        """
        ser = self._parse_series(series, include_archived=True)
        if not ser.idnum:
            raise ValueError(f"Series '{ser.name}' not found in database")
        self.db.series_set_archived(ser.idnum, False)

        svlist = self.db.ser_ver_get_for_series(ser.idnum)

        # Collect the tags
        repo = pygit2.init_repository(self.gitdir)
        tag_info = {}
        for svi in svlist:
            name = self._get_branch_name(ser.name, svi.version)
            target = repo.revparse_single(svi.archive_tag)
            tag_info[svi.idnum] = name, svi.archive_tag, target

        # Make sure the branches don't exist
        for name, tag_name, tag in tag_info.values():
            if name in repo.branches:
                raise ValueError(
                    f"Cannot restore branch '{name}': already exists")

        # Recreate the branches
        for name, tag_name, tag in tag_info.values():
            target = repo.get(tag.target)
            repo.branches.create(name, target)

        # Delete the tags
        for name, tag_name, tag in tag_info.values():
            repo.references.delete(f'refs/tags/{tag_name}')

        # Update the database
        for idnum, (name, tag_name, tag) in tag_info.items():
            self.db.ser_ver_set_archive_tag(idnum, None)

        self.commit()

    def status(self, pwork, series, version, show_comments,
               show_cover_comments=False):
        """Show the series status from patchwork

        Args:
            pwork (Patchwork): Patchwork object to use
            series (str): Name of series to use, or None to use current branch
            version (int): Version number, or None to detect from name
            show_comments (bool): Show all comments on each patch
            show_cover_comments (bool): Show all comments on the cover letter
        """
        branch, series, version, _, _, link, _, _ = self._get_patches(
            series, version)
        if not link:
            raise ValueError(
                f"Series '{series.name}' v{version} has no patchwork link: "
                f"Try 'patman series -s {branch} autolink'")
        status.check_and_show_status(
            series, link, branch, None, False, show_comments,
            show_cover_comments, pwork, self.gitdir)

    def summary(self, series):
        """Show summary information for all series

        Args:
            series (str): Name of series to use
        """
        print(f"{'Name':17}  Status  Description")
        print(f"{'-' * 17}  {'-' * 6}  {'-' * 30}")
        if series is not None:
            self._summary_one(self._parse_series(series))
            return

        sdict = self.db.series_get_dict()
        for ser in sdict.values():
            self._summary_one(ser)

    def gather(self, pwork, series, version, show_comments,
               show_cover_comments, gather_tags, dry_run=False):
        """Gather any new tags from Patchwork, optionally showing comments

        Args:
            pwork (Patchwork): Patchwork object to use
            series (str): Name of series to use, or None to use current branch
            version (int): Version number, or None to detect from name
            show_comments (bool): True to show the comments on each patch
            show_cover_comments (bool): True to show the comments on the cover
                letter
            gather_tags (bool): True to gather review/test tags
            dry_run (bool): True to do a dry run (database is not updated)
        """
        ser, version = self._parse_series_and_version(series, version)
        self._ensure_version(ser, version)
        svid, link = self._get_series_svid_link(ser.idnum, version)
        if not link:
            raise ValueError(
                "No patchwork link is available: use 'patman series autolink'")
        tout.info(
            f"Updating series '{ser.name}' version {version} "
            f"from link '{link}'")

        loop = asyncio.get_event_loop()
        with pwork.collect_stats() as stats:
            cover, patches = loop.run_until_complete(self._gather(
                pwork, link, show_cover_comments))

        with terminal.pager():
            updated, updated_cover = self._sync_one(
                svid, ser.name, version, show_comments, show_cover_comments,
                gather_tags, cover, patches, dry_run)
            tout.info(f"{updated} patch{'es' if updated != 1 else ''}"
                      f"{' and cover letter' if updated_cover else ''} "
                      f'updated ({stats.request_count} requests)')

            if not dry_run:
                self.commit()
            else:
                self.rollback()
                tout.info('Dry run completed')

    def gather_all(self, pwork, show_comments, show_cover_comments,
                   sync_all_versions, gather_tags, dry_run=False):
        to_fetch, missing = self._get_fetch_dict(sync_all_versions)

        loop = asyncio.get_event_loop()
        result, requests = loop.run_until_complete(self._do_series_sync_all(
                pwork, to_fetch))

        with terminal.pager():
            tot_updated = 0
            tot_cover = 0
            add_newline = False
            for (svid, sync), (cover, patches) in zip(to_fetch.items(),
                                                      result):
                if add_newline:
                    tout.info('')
                tout.info(f"Syncing '{sync.series_name}' v{sync.version}")
                updated, updated_cover = self._sync_one(
                    svid, sync.series_name, sync.version, show_comments,
                    show_cover_comments, gather_tags, cover, patches, dry_run)
                tot_updated += updated
                tot_cover += updated_cover
                add_newline = gather_tags

            tout.info('')
            tout.info(
                f"{tot_updated} patch{'es' if tot_updated != 1 else ''} and "
                f"{tot_cover} cover letter{'s' if tot_cover != 1 else ''} "
                f'updated, {missing} missing '
                f"link{'s' if missing != 1 else ''} ({requests} requests)")
            if not dry_run:
                self.commit()
            else:
                self.rollback()
                tout.info('Dry run completed')

    def upstream_add(self, name, url):
        """Add a new upstream tree

        Args:
            name (str): Name of the tree
            url (str): URL for the tree
        """
        self.db.upstream_add(name, url)
        self.commit()

    def upstream_list(self):
        """List the upstream repos

        Shows a list of the repos, obtained from the database
        """
        udict = self.get_upstream_dict()

        for name, items in udict.items():
            url, is_default = items
            default = 'default' if is_default else ''
            print(f'{name:15.15} {default:8} {url}')

    def upstream_set_default(self, name):
        """Set the default upstream target

        Args:
            name (str): Name of the upstream remote to set as default, or None
                for none
        """
        self.db.upstream_set_default(name)
        self.commit()

    def upstream_get_default(self):
        """Get the default upstream target

        Return:
            str: Name of the upstream remote to set as default, or None if none
        """
        return self.db.upstream_get_default()

    def upstream_delete(self, name):
        """Delete an upstream target

        Args:
            name (str): Name of the upstream remote to delete
        """
        self.db.upstream_delete(name)
        self.commit()

    def version_remove(self, name, version, dry_run=False):
        """Remove a version of a series from the database

        Args:
            name (str): Name of series to remove, or None to use current one
            version (int): Version number to remove
            dry_run (bool): True to do a dry run
        """
        ser, version = self._parse_series_and_version(name, version)
        name = ser.name

        versions = self._ensure_version(ser, version)

        if versions == [version]:
            raise ValueError(
                f"Series '{ser.name}' only has one version: remove the series")

        self.db.ser_ver_remove(ser.idnum, version)
        if not dry_run:
            self.commit()
        else:
            self.rollback()

        tout.info(f"Removed version {version} from series '{name}'")
        if dry_run:
            tout.info('Dry run completed')

    def version_change(self, name, version, new_version, dry_run=False):
        """Change a version of a series to be a different version

        Args:
            name (str): Name of series to remove, or None to use current one
            version (int): Version number to change
            new_version (int): New version
            dry_run (bool): True to do a dry run
        """
        ser, version = self._parse_series_and_version(name, version)
        name = ser.name

        versions = self._ensure_version(ser, version)
        vstr = list(map(str, versions))
        if version not in versions:
            raise ValueError(
                f"Series '{ser.name}' does not have v{version}: "
                f"{' '.join(vstr)}")

        if not new_version:
            raise ValueError('Please provide a new version number')

        if new_version in versions:
            raise ValueError(
                f"Series '{ser.name}' already has a v{new_version}: "
                f"{' '.join(vstr)}")

        new_name = self._join_name_version(ser.name, new_version)

        svid = self.get_series_svid(ser.idnum, version)
        pwc = self.get_pcommit_dict(svid)
        count = len(pwc.values())
        series = patchstream.get_metadata(name, 0, count, git_dir=self.gitdir)

        self.update_series(name, series, version, new_name, dry_run,
                            add_vers=new_version, switch=True)
        self.db.ser_ver_set_version(svid, new_version)

        if not dry_run:
            self.commit()
        else:
            self.rollback()

        tout.info(f"Changed version {version} in series '{ser.name}' "
                  f"to {new_version} named '{new_name}'")
        if dry_run:
            tout.info('Dry run completed')

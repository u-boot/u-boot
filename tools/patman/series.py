# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2011 The Chromium OS Authors.
#

from __future__ import print_function

import collections
import concurrent.futures
import itertools
import os
import sys
import time

from patman import get_maintainer
from patman import settings
from u_boot_pylib import gitutil
from u_boot_pylib import terminal
from u_boot_pylib import tools

# Series-xxx tags that we understand
valid_series = ['to', 'cc', 'version', 'changes', 'prefix', 'notes', 'name',
                'cover_cc', 'process_log', 'links', 'patchwork_url', 'postfix']

class Series(dict):
    """Holds information about a patch series, including all tags.

    Vars:
        cc (list of str): Aliases/emails to Cc all patches to
        to (list of str): Aliases/emails to send patches to
        commits (list of Commit): Commit objects, one for each patch
        cover (list of str): Lines in the cover letter
        notes (list of str): Lines in the notes
        changes: (dict) List of changes for each version:
            key (int): version number
            value: tuple:
                commit (Commit): Commit this relates to, or None if related to a
                    cover letter
                info (str): change lines for this version (separated by \n)
        allow_overwrite (bool): Allow tags to overwrite an existing tag
        base_commit (Commit): Commit object at the base of this series
        branch (str): Branch name of this series
        desc (str): Description of the series (cover-letter title)
        idnum (int or None): Database rowid
        name (str): Series name, typically the branch name without any numeric
            suffix
        _generated_cc (dict) written in MakeCcFile()
            key: name of patch file
            value: list of email addresses
    """
    def __init__(self):
        self.cc = []
        self.to = []
        self.cover_cc = []
        self.commits = []
        self.cover = None
        self.notes = []
        self.changes = {}
        self.allow_overwrite = False
        self.base_commit = None
        self.branch = None
        self.desc = ''
        self.idnum = None
        self.name = None
        self._generated_cc = {}

    # These make us more like a dictionary
    def __setattr__(self, name, value):
        self[name] = value

    def __getattr__(self, name):
        return self[name]

    @staticmethod
    def from_fields(idnum, name, desc):
        ser = Series()
        ser.idnum = idnum
        ser.name = name
        ser.desc = desc
        return ser

    def AddTag(self, commit, line, name, value):
        """Add a new Series-xxx tag along with its value.

        Args:
            line: Source line containing tag (useful for debug/error messages)
            name: Tag name (part after 'Series-')
            value: Tag value (part after 'Series-xxx: ')

        Returns:
            String warning if something went wrong, else None
        """
        # If we already have it, then add to our list
        name = name.replace('-', '_')
        if name in self and not self.allow_overwrite:
            values = value.split(',')
            values = [str.strip() for str in values]
            if type(self[name]) != type([]):
                raise ValueError("In %s: line '%s': Cannot add another value "
                        "'%s' to series '%s'" %
                            (commit.hash, line, values, self[name]))
            self[name] += values

        # Otherwise just set the value
        elif name in valid_series:
            if name=="notes":
                self[name] = [value]
            else:
                self[name] = value
        else:
            return ("In %s: line '%s': Unknown 'Series-%s': valid "
                        "options are %s" % (commit.hash, line, name,
                            ', '.join(valid_series)))
        return None

    def AddCommit(self, commit):
        """Add a commit into our list of commits

        We create a list of tags in the commit subject also.

        Args:
            commit: Commit object to add
        """
        commit.check_tags()
        self.commits.append(commit)

    def ShowActions(self, args, cmd, process_tags, alias):
        """Show what actions we will/would perform

        Args:
            args: List of patch files we created
            cmd: The git command we would have run
            process_tags: Process tags as if they were aliases
            alias (dict): Alias dictionary
                key: alias
                value: list of aliases or email addresses
        """
        to_set = set(gitutil.build_email_list(self.to, alias));
        cc_set = set(gitutil.build_email_list(self.cc, alias));

        col = terminal.Color()
        print('Dry run, so not doing much. But I would do this:')
        print()
        print('Send a total of %d patch%s with %scover letter.' % (
                len(args), '' if len(args) == 1 else 'es',
                self.get('cover') and 'a ' or 'no '))

        # TODO: Colour the patches according to whether they passed checks
        for upto in range(len(args)):
            commit = self.commits[upto]
            print(col.build(col.GREEN, '   %s' % args[upto]))
            cc_list = list(self._generated_cc[commit.patch])
            for email in sorted(set(cc_list) - to_set - cc_set):
                if email == None:
                    email = col.build(col.YELLOW, '<alias not found>')
                if email:
                    print('      Cc: ', email)
        print
        for item in sorted(to_set):
            print('To:\t ', item)
        for item in sorted(cc_set - to_set):
            print('Cc:\t ', item)
        print('Version: ', self.get('version'))
        print('Prefix:\t ', self.get('prefix'))
        print('Postfix:\t ', self.get('postfix'))
        if self.cover:
            print('Cover: %d lines' % len(self.cover))
            cover_cc = gitutil.build_email_list(self.get('cover_cc', ''),
                                                alias)
            all_ccs = itertools.chain(cover_cc, *self._generated_cc.values())
            for email in sorted(set(all_ccs) - to_set - cc_set):
                    print('      Cc: ', email)
        if cmd:
            print('Git command: %s' % cmd)

    def MakeChangeLog(self, commit):
        """Create a list of changes for each version.

        Return:
            The change log as a list of strings, one per line

            Changes in v4:
            - Jog the dial back closer to the widget

            Changes in v2:
            - Fix the widget
            - Jog the dial

            If there are no new changes in a patch, a note will be added

            (no changes since v2)

            Changes in v2:
            - Fix the widget
            - Jog the dial
        """
        # Collect changes from the series and this commit
        changes = collections.defaultdict(list)
        for version, changelist in self.changes.items():
            changes[version] += changelist
        if commit:
            for version, changelist in commit.changes.items():
                changes[version] += [[commit, text] for text in changelist]

        versions = sorted(changes, reverse=True)
        newest_version = 1
        if 'version' in self:
            newest_version = max(newest_version, int(self.version))
        if versions:
            newest_version = max(newest_version, versions[0])

        final = []
        process_it = self.get('process_log', '').split(',')
        process_it = [item.strip() for item in process_it]
        need_blank = False
        for version in versions:
            out = []
            for this_commit, text in changes[version]:
                if commit and this_commit != commit:
                    continue
                if 'uniq' not in process_it or text not in out:
                    out.append(text)
            if 'sort' in process_it:
                out = sorted(out)
            have_changes = len(out) > 0
            line = 'Changes in v%d:' % version
            if have_changes:
                out.insert(0, line)
                if version < newest_version and len(final) == 0:
                    out.insert(0, '')
                    out.insert(0, '(no changes since v%d)' % version)
                    newest_version = 0
                # Only add a new line if we output something
                if need_blank:
                    out.insert(0, '')
                    need_blank = False
            final += out
            need_blank = need_blank or have_changes

        if len(final) > 0:
            final.append('')
        elif newest_version != 1:
            final = ['(no changes since v1)', '']
        return final

    def DoChecks(self):
        """Check that each version has a change log

        Print an error if something is wrong.
        """
        col = terminal.Color()
        if self.get('version'):
            changes_copy = dict(self.changes)
            for version in range(1, int(self.version) + 1):
                if self.changes.get(version):
                    del changes_copy[version]
                else:
                    if version > 1:
                        str = 'Change log missing for v%d' % version
                        print(col.build(col.RED, str))
            for version in changes_copy:
                str = 'Change log for unknown version v%d' % version
                print(col.build(col.RED, str))
        elif self.changes:
            str = 'Change log exists, but no version is set'
            print(col.build(col.RED, str))

    def GetCcForCommit(self, commit, process_tags, warn_on_error,
                       add_maintainers, limit, get_maintainer_script,
                       all_skips, alias, cwd):
        """Get the email CCs to use with a particular commit

        Uses subject tags and get_maintainers.pl script to find people to cc
        on a patch

        Args:
            commit (Commit): Commit to process
            process_tags (bool): Process tags as if they were aliases
            warn_on_error (bool): True to print a warning when an alias fails to
                match, False to ignore it.
            add_maintainers (bool or list of str): Either:
                True/False to call the get_maintainers to CC maintainers
                List of maintainers to include (for testing)
            limit (int): Limit the length of the Cc list (None if no limit)
            get_maintainer_script (str): The file name of the get_maintainer.pl
                script (or compatible).
            all_skips (set of str): Updated to include the set of bouncing email
                addresses that were dropped from the output. This is essentially
                a return value from this function.
            alias (dict): Alias dictionary
                key: alias
                value: list of aliases or email addresses
            cwd (str): Path to use for patch filenames (None to use current dir)

        Returns:
            list of str: List of email addresses to cc
        """
        cc = []
        if process_tags:
            cc += gitutil.build_email_list(commit.tags, alias,
                                           warn_on_error=warn_on_error)
        cc += gitutil.build_email_list(commit.cc_list, alias,
                                       warn_on_error=warn_on_error)
        if type(add_maintainers) == type(cc):
            cc += add_maintainers
        elif add_maintainers:
            fname = os.path.join(cwd or '', commit.patch)
            cc += get_maintainer.get_maintainer(get_maintainer_script, fname)
        all_skips |= set(cc) & set(settings.bounces)
        cc = list(set(cc) - set(settings.bounces))
        if limit is not None:
            cc = cc[:limit]
        return cc

    def MakeCcFile(self, process_tags, cover_fname, warn_on_error,
                   add_maintainers, limit, get_maintainer_script, alias,
                   cwd=None):
        """Make a cc file for us to use for per-commit Cc automation

        Also stores in self._generated_cc to make ShowActions() faster.

        Args:
            process_tags (bool): Process tags as if they were aliases
            cover_fname (str): If non-None the name of the cover letter.
            warn_on_error (bool): True to print a warning when an alias fails to
                match, False to ignore it.
            add_maintainers (bool or list of str): Either:
                True/False to call the get_maintainers to CC maintainers
                List of maintainers to include (for testing)
            limit (int): Limit the length of the Cc list (None if no limit)
            get_maintainer_script (str): The file name of the get_maintainer.pl
                script (or compatible).
            alias (dict): Alias dictionary
                key: alias
                value: list of aliases or email addresses
            cwd (str): Path to use for patch filenames (None to use current dir)
        Return:
            Filename of temp file created
        """
        col = terminal.Color()
        # Look for commit tags (of the form 'xxx:' at the start of the subject)
        fname = '/tmp/patman.%d' % os.getpid()
        fd = open(fname, 'w', encoding='utf-8')
        all_ccs = []
        all_skips = set()
        with concurrent.futures.ThreadPoolExecutor(max_workers=16) as executor:
            for i, commit in enumerate(self.commits):
                commit.seq = i
                commit.future = executor.submit(
                    self.GetCcForCommit, commit, process_tags, warn_on_error,
                    add_maintainers, limit, get_maintainer_script, all_skips,
                    alias, cwd)

            # Show progress any commits that are taking forever
            lastlen = 0
            while True:
                left = [commit for commit in self.commits
                        if not commit.future.done()]
                if not left:
                    break
                names = ', '.join(f'{c.seq + 1}:{c.subject}'
                                  for c in left[:2])
                out = f'\r{len(left)} remaining: {names}'[:79]
                spaces = ' ' * (lastlen - len(out))
                if lastlen:  # Don't print anything the first time
                    print(out, spaces, end='')
                    sys.stdout.flush()
                lastlen = len(out)
                time.sleep(.25)
            print(f'\rdone{" " * lastlen}\r', end='')
            print('Cc processing complete')

        for commit in self.commits:
            cc = commit.future.result()
            all_ccs += cc
            print(commit.patch, '\0'.join(sorted(set(cc))), file=fd)
            self._generated_cc[commit.patch] = cc

        for x in sorted(all_skips):
            print(col.build(col.YELLOW, f'Skipping "{x}"'))

        if cover_fname:
            cover_cc = gitutil.build_email_list(
                self.get('cover_cc', ''), alias)
            cover_cc = list(set(cover_cc + all_ccs))
            if limit is not None:
                cover_cc = cover_cc[:limit]
            cc_list = '\0'.join([x for x in sorted(cover_cc)])
            print(cover_fname, cc_list, file=fd)

        fd.close()
        return fname

    def AddChange(self, version, commit, info):
        """Add a new change line to a version.

        This will later appear in the change log.

        Args:
            version (int): version number to add change list to
            commit (Commit): Commit this relates to, or None if related to a
                cover letter
            info (str): change lines for this version (separated by \n)
        """
        if not self.changes.get(version):
            self.changes[version] = []
        self.changes[version].append([commit, info])

    def GetPatchPrefix(self):
        """Get the patch version string

        Return:
            Patch string, like 'RFC PATCH v5' or just 'PATCH'
        """
        git_prefix = gitutil.get_default_subject_prefix()
        if git_prefix:
            git_prefix = '%s][' % git_prefix
        else:
            git_prefix = ''

        version = ''
        if self.get('version'):
            version = ' v%s' % self['version']

        # Get patch name prefix
        prefix = ''
        if self.get('prefix'):
            prefix = '%s ' % self['prefix']

        postfix = ''
        if self.get('postfix'):
           postfix = ' %s' % self['postfix']
        return '%s%sPATCH%s%s' % (git_prefix, prefix, postfix, version)

    def get_links(self, links_str=None, cur_version=None):
        """Look up the patchwork links for each version

        Args:
            links_str (str): Links string to parse, or None to use self.links
            cur_version (int): Default version to assume for un-versioned links,
                or None to use self.version

        Return:
            dict:
                key (int): Version number
                value (str): Link string
        """
        if links_str is None:
            links_str = self.links if 'links' in self else ''
        if cur_version is None:
            cur_version = int(self.version) if 'version' in self else 1
        assert isinstance(cur_version, int)
        links = {}
        for item in links_str.split():
            if ':' in item:
                version, link = item.split(':')
                links[int(version)] = link
            else:
                links[cur_version] = item
        return links

    def build_links(self, links):
        """Build a string containing the links

        Args:
            links (dict):
                key (int): Version number
                value (str): Link string

        Return:
            str: Link string, e.g. '2:4433 1:2872'
        """
        out = ''
        for vers in sorted(links.keys(), reverse=True):
            out += f' {vers}:{links[vers]}'
        return out[1:]

    def get_link_for_version(self, find_vers, links_str=None):
        """Look up the patchwork link for a particular version

        Args:
            find_vers (int): Version to find
            links_str (str): Links string to parse, or None to use self.links

        Return:
            str: Series-links entry for that version, or None if not found
        """
        return self.get_links(links_str).get(find_vers)

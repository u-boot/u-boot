# Copyright (c) 2011 The Chromium OS Authors.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

import os

import gitutil
import terminal

# Series-xxx tags that we understand
valid_series = ['to', 'cc', 'version', 'changes', 'prefix', 'notes', 'name'];

class Series(dict):
    """Holds information about a patch series, including all tags.

    Vars:
        cc: List of aliases/emails to Cc all patches to
        commits: List of Commit objects, one for each patch
        cover: List of lines in the cover letter
        notes: List of lines in the notes
        changes: (dict) List of changes for each version, The key is
            the integer version number
    """
    def __init__(self):
        self.cc = []
        self.to = []
        self.commits = []
        self.cover = None
        self.notes = []
        self.changes = {}

    # These make us more like a dictionary
    def __setattr__(self, name, value):
        self[name] = value

    def __getattr__(self, name):
        return self[name]

    def AddTag(self, commit, line, name, value):
        """Add a new Series-xxx tag along with its value.

        Args:
            line: Source line containing tag (useful for debug/error messages)
            name: Tag name (part after 'Series-')
            value: Tag value (part after 'Series-xxx: ')
        """
        # If we already have it, then add to our list
        if name in self:
            values = value.split(',')
            values = [str.strip() for str in values]
            if type(self[name]) != type([]):
                raise ValueError("In %s: line '%s': Cannot add another value "
                        "'%s' to series '%s'" %
                            (commit.hash, line, values, self[name]))
            self[name] += values

        # Otherwise just set the value
        elif name in valid_series:
            self[name] = value
        else:
            raise ValueError("In %s: line '%s': Unknown 'Series-%s': valid "
                        "options are %s" % (commit.hash, line, name,
                            ', '.join(valid_series)))

    def AddCommit(self, commit):
        """Add a commit into our list of commits

        We create a list of tags in the commit subject also.

        Args:
            commit: Commit object to add
        """
        commit.CheckTags()
        self.commits.append(commit)

    def ShowActions(self, args, cmd, process_tags):
        """Show what actions we will/would perform

        Args:
            args: List of patch files we created
            cmd: The git command we would have run
            process_tags: Process tags as if they were aliases
        """
        col = terminal.Color()
        print 'Dry run, so not doing much. But I would do this:'
        print
        print 'Send a total of %d patch%s with %scover letter.' % (
                len(args), '' if len(args) == 1 else 'es',
                self.get('cover') and 'a ' or 'no ')

        # TODO: Colour the patches according to whether they passed checks
        for upto in range(len(args)):
            commit = self.commits[upto]
            print col.Color(col.GREEN, '   %s' % args[upto])
            cc_list = []
            if process_tags:
                cc_list += gitutil.BuildEmailList(commit.tags)
            cc_list += gitutil.BuildEmailList(commit.cc_list)

            # Skip items in To list
            if 'to' in self:
                try:
                    map(cc_list.remove, gitutil.BuildEmailList(self.to))
                except ValueError:
                    pass

            for email in cc_list:
                if email == None:
                    email = col.Color(col.YELLOW, "<alias '%s' not found>"
                            % tag)
                if email:
                    print '      Cc: ',email
        print
        for item in gitutil.BuildEmailList(self.get('to', '<none>')):
            print 'To:\t ', item
        for item in gitutil.BuildEmailList(self.cc):
            print 'Cc:\t ', item
        print 'Version: ', self.get('version')
        print 'Prefix:\t ', self.get('prefix')
        if self.cover:
            print 'Cover: %d lines' % len(self.cover)
        if cmd:
            print 'Git command: %s' % cmd

    def MakeChangeLog(self, commit):
        """Create a list of changes for each version.

        Return:
            The change log as a list of strings, one per line

            Changes in v4:
            - Jog the dial back closer to the widget

            Changes in v3: None
            Changes in v2:
            - Fix the widget
            - Jog the dial

            etc.
        """
        final = []
        need_blank = False
        for change in sorted(self.changes, reverse=True):
            out = []
            for this_commit, text in self.changes[change]:
                if commit and this_commit != commit:
                    continue
                out.append(text)
            line = 'Changes in v%d:' % change
            have_changes = len(out) > 0
            if have_changes:
                out.insert(0, line)
            else:
                out = [line + ' None']
            if need_blank:
                out.insert(0, '')
            final += out
            need_blank = have_changes
        if self.changes:
            final.append('')
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
                        print col.Color(col.RED, str)
            for version in changes_copy:
                str = 'Change log for unknown version v%d' % version
                print col.Color(col.RED, str)
        elif self.changes:
            str = 'Change log exists, but no version is set'
            print col.Color(col.RED, str)

    def MakeCcFile(self, process_tags):
        """Make a cc file for us to use for per-commit Cc automation

        Args:
            process_tags: Process tags as if they were aliases
        Return:
            Filename of temp file created
        """
        # Look for commit tags (of the form 'xxx:' at the start of the subject)
        fname = '/tmp/patman.%d' % os.getpid()
        fd = open(fname, 'w')
        for commit in self.commits:
            list = []
            if process_tags:
                list += gitutil.BuildEmailList(commit.tags)
            list += gitutil.BuildEmailList(commit.cc_list)
            print >>fd, commit.patch, ', '.join(list)

        fd.close()
        return fname

    def AddChange(self, version, commit, info):
        """Add a new change line to a version.

        This will later appear in the change log.

        Args:
            version: version number to add change list to
            info: change line for this version
        """
        if not self.changes.get(version):
            self.changes[version] = []
        self.changes[version].append([commit, info])

    def GetPatchPrefix(self):
        """Get the patch version string

        Return:
            Patch string, like 'RFC PATCH v5' or just 'PATCH'
        """
        version = ''
        if self.get('version'):
            version = ' v%s' % self['version']

        # Get patch name prefix
        prefix = ''
        if self.get('prefix'):
            prefix = '%s ' % self['prefix']
        return '%sPATCH%s' % (prefix, version)

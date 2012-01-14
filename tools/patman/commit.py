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

import re

# Separates a tag: at the beginning of the subject from the rest of it
re_subject_tag = re.compile('([^:]*):\s*(.*)')

class Commit:
    """Holds information about a single commit/patch in the series.

    Args:
        hash: Commit hash (as a string)

    Variables:
        hash: Commit hash
        subject: Subject line
        tags: List of maintainer tag strings
        changes: Dict containing a list of changes (single line strings).
            The dict is indexed by change version (an integer)
        cc_list: List of people to aliases/emails to cc on this commit
    """
    def __init__(self, hash):
        self.hash = hash
        self.subject = None
        self.tags = []
        self.changes = {}
        self.cc_list = []

    def AddChange(self, version, info):
        """Add a new change line to the change list for a version.

        Args:
            version: Patch set version (integer: 1, 2, 3)
            info: Description of change in this version
        """
        if not self.changes.get(version):
            self.changes[version] = []
        self.changes[version].append(info)

    def CheckTags(self):
        """Create a list of subject tags in the commit

        Subject tags look like this:

            propounder: Change the widget to propound correctly

        Multiple tags are supported. The list is updated in self.tag

        Returns:
            None if ok, else the name of a tag with no email alias
        """
        str = self.subject
        m = True
        while m:
            m = re_subject_tag.match(str)
            if m:
                tag = m.group(1)
                self.tags.append(tag)
                str = m.group(2)
        return None

    def AddCc(self, cc_list):
        """Add a list of people to Cc when we send this patch.

        Args:
            cc_list:    List of aliases or email addresses
        """
        self.cc_list += cc_list

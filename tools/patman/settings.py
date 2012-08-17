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

import ConfigParser
import os
import re

import command


def ReadGitAliases(fname):
    """Read a git alias file. This is in the form used by git:

    alias uboot  u-boot@lists.denx.de
    alias wd     Wolfgang Denk <wd@denx.de>

    Args:
        fname: Filename to read
    """
    try:
        fd = open(fname, 'r')
    except IOError:
        print "Warning: Cannot find alias file '%s'" % fname
        return

    re_line = re.compile('alias\s+(\S+)\s+(.*)')
    for line in fd.readlines():
        line = line.strip()
        if not line or line[0] == '#':
            continue

        m = re_line.match(line)
        if not m:
            print "Warning: Alias file line '%s' not understood" % line
            continue

        list = alias.get(m.group(1), [])
        for item in m.group(2).split(','):
            item = item.strip()
            if item:
                list.append(item)
        alias[m.group(1)] = list

    fd.close()

def Setup(config_fname=''):
    """Set up the settings module by reading config files.

    Args:
        config_fname:   Config filename to read ('' for default)
    """
    settings = ConfigParser.SafeConfigParser()
    if config_fname == '':
        config_fname = '%s/.config/patman' % os.getenv('HOME')
    if config_fname:
        settings.read(config_fname)

    for name, value in settings.items('alias'):
        alias[name] = value.split(',')


# These are the aliases we understand, indexed by alias. Each member is a list.
alias = {}

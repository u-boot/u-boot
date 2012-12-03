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
import gitutil

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

def CreatePatmanConfigFile(config_fname):
    """Creates a config file under $(HOME)/.patman if it can't find one.

    Args:
        config_fname: Default config filename i.e., $(HOME)/.patman

    Returns:
        None
    """
    name = gitutil.GetDefaultUserName()
    if name == None:
        name = raw_input("Enter name: ")

    email = gitutil.GetDefaultUserEmail()

    if email == None:
        email = raw_input("Enter email: ")

    try:
        f = open(config_fname, 'w')
    except IOError:
        print "Couldn't create patman config file\n"
        raise

    print >>f, "[alias]\nme: %s <%s>" % (name, email)
    f.close();

def _UpdateDefaults(parser, config):
    """Update the given OptionParser defaults based on config.

    We'll walk through all of the settings from the parser
    For each setting we'll look for a default in the option parser.
    If it's found we'll update the option parser default.

    The idea here is that the .patman file should be able to update
    defaults but that command line flags should still have the final
    say.

    Args:
        parser: An instance of an OptionParser whose defaults will be
            updated.
        config: An instance of SafeConfigParser that we will query
            for settings.
    """
    defaults = parser.get_default_values()
    for name, val in config.items('settings'):
        if hasattr(defaults, name):
            default_val = getattr(defaults, name)
            if isinstance(default_val, bool):
                val = config.getboolean('settings', name)
            elif isinstance(default_val, int):
                val = config.getint('settings', name)
            parser.set_default(name, val)
        else:
            print "WARNING: Unknown setting %s" % name

def Setup(parser, config_fname=''):
    """Set up the settings module by reading config files.

    Args:
        parser:         The parser to update
        config_fname:   Config filename to read ('' for default)
    """
    config = ConfigParser.SafeConfigParser()
    if config_fname == '':
        config_fname = '%s/.patman' % os.getenv('HOME')

    if not os.path.exists(config_fname):
        print "No config file found ~/.patman\nCreating one...\n"
        CreatePatmanConfigFile(config_fname)

    config.read(config_fname)

    for name, value in config.items('alias'):
        alias[name] = value.split(',')

    _UpdateDefaults(parser, config)

# These are the aliases we understand, indexed by alias. Each member is a list.
alias = {}

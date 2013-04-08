# Copyright (c) 2012 The Chromium OS Authors.
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


def Setup(fname=''):
    """Set up the buildman settings module by reading config files

    Args:
        config_fname:   Config filename to read ('' for default)
    """
    global settings
    global config_fname

    settings = ConfigParser.SafeConfigParser()
    config_fname = fname
    if config_fname == '':
        config_fname = '%s/.buildman' % os.getenv('HOME')
    if config_fname:
        settings.read(config_fname)

def GetItems(section):
    """Get the items from a section of the config.

    Args:
        section: name of section to retrieve

    Returns:
        List of (name, value) tuples for the section
    """
    try:
        return settings.items(section)
    except ConfigParser.NoSectionError as e:
        print e
        print ("Warning: No tool chains - please add a [toolchain] section "
                "to your buildman config file %s. See README for details" %
                config_fname)
        return []
    except:
        raise

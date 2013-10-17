# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
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
        return []
    except:
        raise

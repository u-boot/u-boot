#!/usr/bin/python
# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2017 Google, Inc
# Written by Simon Glass <sjg@chromium.org>
#

"""Scanning of U-Boot source for drivers and structs

This scans the source tree to find out things about all instances of
U_BOOT_DRIVER(), UCLASS_DRIVER and all struct declarations in header files.

See doc/driver-model/of-plat.rst for more informaiton
"""

import os
import re
import sys


def conv_name_to_c(name):
    """Convert a device-tree name to a C identifier

    This uses multiple replace() calls instead of re.sub() since it is faster
    (400ms for 1m calls versus 1000ms for the 're' version).

    Args:
        name (str): Name to convert
    Return:
        str: String containing the C version of this name
    """
    new = name.replace('@', '_at_')
    new = new.replace('-', '_')
    new = new.replace(',', '_')
    new = new.replace('.', '_')
    return new

def get_compat_name(node):
    """Get the node's list of compatible string as a C identifiers

    Args:
        node (fdt.Node): Node object to check
    Return:
        list of str: List of C identifiers for all the compatible strings
    """
    compat = node.props['compatible'].value
    if not isinstance(compat, list):
        compat = [compat]
    return [conv_name_to_c(c) for c in compat]


class Driver:
    """Information about a driver in U-Boot

    Attributes:
        name: Name of driver. For U_BOOT_DRIVER(x) this is 'x'
    """
    def __init__(self, name):
        self.name = name

    def __eq__(self, other):
        return self.name == other.name

    def __repr__(self):
        return "Driver(name='%s')" % self.name


class Scanner:
    """Scanning of the U-Boot source tree

    Properties:
        _basedir (str): Base directory of U-Boot source code. Defaults to the
            grandparent of this file's directory
        _drivers: Dict of valid driver names found in drivers/
            key: Driver name
            value: Driver for that driver
        _driver_aliases: Dict that holds aliases for driver names
            key: Driver alias declared with
                DM_DRIVER_ALIAS(driver_alias, driver_name)
            value: Driver name declared with U_BOOT_DRIVER(driver_name)
        _warning_disabled: true to disable warnings about driver names not found
        _drivers_additional (list or str): List of additional drivers to use
            during scanning
    """
    def __init__(self, basedir, warning_disabled, drivers_additional):
        """Set up a new Scanner
        """
        if not basedir:
            basedir = sys.argv[0].replace('tools/dtoc/dtoc', '')
            if basedir == '':
                basedir = './'
        self._basedir = basedir
        self._drivers = {}
        self._driver_aliases = {}
        self._drivers_additional = drivers_additional or []
        self._warning_disabled = warning_disabled

    def get_normalized_compat_name(self, node):
        """Get a node's normalized compat name

        Returns a valid driver name by retrieving node's list of compatible
        string as a C identifier and performing a check against _drivers
        and a lookup in driver_aliases printing a warning in case of failure.

        Args:
            node (Node): Node object to check
        Return:
            Tuple:
                Driver name associated with the first compatible string
                List of C identifiers for all the other compatible strings
                    (possibly empty)
                In case of no match found, the return will be the same as
                get_compat_name()
        """
        compat_list_c = get_compat_name(node)

        for compat_c in compat_list_c:
            if not compat_c in self._drivers.keys():
                compat_c = self._driver_aliases.get(compat_c)
                if not compat_c:
                    continue

            aliases_c = compat_list_c
            if compat_c in aliases_c:
                aliases_c.remove(compat_c)
            return compat_c, aliases_c

        if not self._warning_disabled:
            print('WARNING: the driver %s was not found in the driver list'
                  % (compat_list_c[0]))

        return compat_list_c[0], compat_list_c[1:]

    def scan_driver(self, fname):
        """Scan a driver file to build a list of driver names and aliases

        This procedure will populate self._drivers and self._driver_aliases

        Args
            fname: Driver filename to scan
        """
        with open(fname, encoding='utf-8') as inf:
            try:
                buff = inf.read()
            except UnicodeDecodeError:
                # This seems to happen on older Python versions
                print("Skipping file '%s' due to unicode error" % fname)
                return

            # The following re will search for driver names declared as
            # U_BOOT_DRIVER(driver_name)
            drivers = re.findall(r'U_BOOT_DRIVER\((.*)\)', buff)

            for driver in drivers:
                self._drivers[driver] = Driver(driver)

            # The following re will search for driver aliases declared as
            # DM_DRIVER_ALIAS(alias, driver_name)
            driver_aliases = re.findall(
                r'DM_DRIVER_ALIAS\(\s*(\w+)\s*,\s*(\w+)\s*\)',
                buff)

            for alias in driver_aliases: # pragma: no cover
                if len(alias) != 2:
                    continue
                self._driver_aliases[alias[1]] = alias[0]

    def scan_drivers(self):
        """Scan the driver folders to build a list of driver names and aliases

        This procedure will populate self._drivers and self._driver_aliases
        """
        for (dirpath, _, filenames) in os.walk(self._basedir):
            for fname in filenames:
                if not fname.endswith('.c'):
                    continue
                self.scan_driver(dirpath + '/' + fname)

        for fname in self._drivers_additional:
            if not isinstance(fname, str) or len(fname) == 0:
                continue
            if fname[0] == '/':
                self.scan_driver(fname)
            else:
                self.scan_driver(self._basedir + '/' + fname)

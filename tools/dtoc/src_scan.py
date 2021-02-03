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
        fname: Filename where the driver was found
        uclass_id: Name of uclass, e.g. 'UCLASS_I2C'
        compat: Driver data for each compatible string:
            key: Compatible string, e.g. 'rockchip,rk3288-grf'
            value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        fname: Filename where the driver was found
        priv (str): struct name of the priv_auto member, e.g. 'serial_priv'
        plat (str): struct name of the plat_auto member, e.g. 'serial_plat'
        child_priv (str): struct name of the per_child_auto member,
            e.g. 'pci_child_priv'
        child_plat (str): struct name of the per_child_plat_auto member,
            e.g. 'pci_child_plat'
    """
    def __init__(self, name, fname):
        self.name = name
        self.fname = fname
        self.uclass_id = None
        self.compat = None
        self.priv = ''
        self.plat = ''
        self.child_priv = ''
        self.child_plat = ''

    def __eq__(self, other):
        return (self.name == other.name and
                self.uclass_id == other.uclass_id and
                self.compat == other.compat and
                self.priv == other.priv and
                self.plat == other.plat)

    def __repr__(self):
        return ("Driver(name='%s', uclass_id='%s', compat=%s, priv=%s)" %
                (self.name, self.uclass_id, self.compat, self.priv))


class UclassDriver:
    """Holds information about a uclass driver

    Attributes:
        name: Uclass name, e.g. 'i2c' if the driver is for UCLASS_I2C
        uclass_id: Uclass ID, e.g. 'UCLASS_I2C'
        priv: struct name of the private data, e.g. 'i2c_priv'
        per_dev_priv (str): struct name of the priv_auto member, e.g. 'spi_info'
        per_dev_plat (str): struct name of the plat_auto member, e.g. 'i2c_chip'
        per_child_priv (str): struct name of the per_child_auto member,
            e.g. 'pci_child_priv'
        per_child_plat (str): struct name of the per_child_plat_auto member,
            e.g. 'pci_child_plat'
    """
    def __init__(self, name):
        self.name = name
        self.uclass_id = None
        self.priv = ''
        self.per_dev_priv = ''
        self.per_dev_plat = ''
        self.per_child_priv = ''
        self.per_child_plat = ''

    def __eq__(self, other):
        return (self.name == other.name and
                self.uclass_id == other.uclass_id and
                self.priv == other.priv)

    def __repr__(self):
        return ("UclassDriver(name='%s', uclass_id='%s')" %
                (self.name, self.uclass_id))

    def __hash__(self):
        # We can use the uclass ID since it is unique among uclasses
        return hash(self.uclass_id)


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
        _of_match: Dict holding information about compatible strings
            key: Name of struct udevice_id variable
            value: Dict of compatible info in that variable:
               key: Compatible string, e.g. 'rockchip,rk3288-grf'
               value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        _compat_to_driver: Maps compatible strings to Driver
        _uclass: Dict of uclass information
            key: uclass name, e.g. 'UCLASS_I2C'
            value: UClassDriver
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
        self._of_match = {}
        self._compat_to_driver = {}
        self._uclass = {}

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

    @classmethod
    def _get_re_for_member(cls, member):
        """_get_re_for_member: Get a compiled regular expression

        Args:
            member (str): Struct member name, e.g. 'priv_auto'

        Returns:
            re.Pattern: Compiled regular expression that parses:

               .member = sizeof(struct fred),

            and returns "fred" as group 1
        """
        return re.compile(r'^\s*.%s\s*=\s*sizeof\(struct\s+(.*)\),$' % member)

    def _parse_uclass_driver(self, fname, buff):
        """Parse a C file to extract uclass driver information contained within

        This parses UCLASS_DRIVER() structs to obtain various pieces of useful
        information.

        It updates the following member:
            _uclass: Dict of uclass information
                key: uclass name, e.g. 'UCLASS_I2C'
                value: UClassDriver

        Args:
            fname (str): Filename being parsed (used for warnings)
            buff (str): Contents of file
        """
        uc_drivers = {}

        # Collect the driver name and associated Driver
        driver = None
        re_driver = re.compile(r'UCLASS_DRIVER\((.*)\)')

        # Collect the uclass ID, e.g. 'UCLASS_SPI'
        re_id = re.compile(r'\s*\.id\s*=\s*(UCLASS_[A-Z0-9_]+)')

        # Matches the header/size information for uclass-private data
        re_priv = self._get_re_for_member('priv_auto')

        # Set up parsing for the auto members
        re_per_device_priv = self._get_re_for_member('per_device_auto')
        re_per_device_plat = self._get_re_for_member('per_device_plat_auto')
        re_per_child_priv = self._get_re_for_member('per_child_auto')
        re_per_child_plat = self._get_re_for_member('per_child_plat_auto')

        prefix = ''
        for line in buff.splitlines():
            # Handle line continuation
            if prefix:
                line = prefix + line
                prefix = ''
            if line.endswith('\\'):
                prefix = line[:-1]
                continue

            driver_match = re_driver.search(line)

            # If we have seen UCLASS_DRIVER()...
            if driver:
                m_id = re_id.search(line)
                m_priv = re_priv.match(line)
                m_per_dev_priv = re_per_device_priv.match(line)
                m_per_dev_plat = re_per_device_plat.match(line)
                m_per_child_priv = re_per_child_priv.match(line)
                m_per_child_plat = re_per_child_plat.match(line)
                if m_id:
                    driver.uclass_id = m_id.group(1)
                elif m_priv:
                    driver.priv = m_priv.group(1)
                elif m_per_dev_priv:
                    driver.per_dev_priv = m_per_dev_priv.group(1)
                elif m_per_dev_plat:
                    driver.per_dev_plat = m_per_dev_plat.group(1)
                elif m_per_child_priv:
                    driver.per_child_priv = m_per_child_priv.group(1)
                elif m_per_child_plat:
                    driver.per_child_plat = m_per_child_plat.group(1)
                elif '};' in line:
                    if not driver.uclass_id:
                        raise ValueError(
                            "%s: Cannot parse uclass ID in driver '%s'" %
                            (fname, driver.name))
                    uc_drivers[driver.uclass_id] = driver
                    driver = None

            elif driver_match:
                driver_name = driver_match.group(1)
                driver = UclassDriver(driver_name)

        self._uclass.update(uc_drivers)

    def _parse_driver(self, fname, buff):
        """Parse a C file to extract driver information contained within

        This parses U_BOOT_DRIVER() structs to obtain various pieces of useful
        information.

        It updates the following members:
            _drivers - updated with new Driver records for each driver found
                in the file
            _of_match - updated with each compatible string found in the file
            _compat_to_driver - Maps compatible string to Driver

        Args:
            fname (str): Filename being parsed (used for warnings)
            buff (str): Contents of file

        Raises:
            ValueError: Compatible variable is mentioned in .of_match in
                U_BOOT_DRIVER() but not found in the file
        """
        # Dict holding information about compatible strings collected in this
        # function so far
        #    key: Name of struct udevice_id variable
        #    value: Dict of compatible info in that variable:
        #       key: Compatible string, e.g. 'rockchip,rk3288-grf'
        #       value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        of_match = {}

        # Dict holding driver information collected in this function so far
        #    key: Driver name (C name as in U_BOOT_DRIVER(xxx))
        #    value: Driver
        drivers = {}

        # Collect the driver info
        driver = None
        re_driver = re.compile(r'U_BOOT_DRIVER\((.*)\)')

        # Collect the uclass ID, e.g. 'UCLASS_SPI'
        re_id = re.compile(r'\s*\.id\s*=\s*(UCLASS_[A-Z0-9_]+)')

        # Collect the compatible string, e.g. 'rockchip,rk3288-grf'
        compat = None
        re_compat = re.compile(r'{\s*.compatible\s*=\s*"(.*)"\s*'
                               r'(,\s*.data\s*=\s*(\S*))?\s*},')

        # This is a dict of compatible strings that were found:
        #    key: Compatible string, e.g. 'rockchip,rk3288-grf'
        #    value: Driver data, e,g, 'ROCKCHIP_SYSCON_GRF', or None
        compat_dict = {}

        # Holds the var nane of the udevice_id list, e.g.
        # 'rk3288_syscon_ids_noc' in
        # static const struct udevice_id rk3288_syscon_ids_noc[] = {
        ids_name = None
        re_ids = re.compile(r'struct udevice_id (.*)\[\]\s*=')

        # Matches the references to the udevice_id list
        re_of_match = re.compile(
            r'\.of_match\s*=\s*(of_match_ptr\()?([a-z0-9_]+)(\))?,')

        # Matches the struct name for priv, plat
        re_priv = self._get_re_for_member('priv_auto')
        re_plat = self._get_re_for_member('plat_auto')
        re_child_priv = self._get_re_for_member('per_child_auto')
        re_child_plat = self._get_re_for_member('per_child_plat_auto')

        prefix = ''
        for line in buff.splitlines():
            # Handle line continuation
            if prefix:
                line = prefix + line
                prefix = ''
            if line.endswith('\\'):
                prefix = line[:-1]
                continue

            driver_match = re_driver.search(line)

            # If this line contains U_BOOT_DRIVER()...
            if driver:
                m_id = re_id.search(line)
                m_of_match = re_of_match.search(line)
                m_priv = re_priv.match(line)
                m_plat = re_plat.match(line)
                m_cplat = re_child_plat.match(line)
                m_cpriv = re_child_priv.match(line)
                if m_priv:
                    driver.priv = m_priv.group(1)
                elif m_plat:
                    driver.plat = m_plat.group(1)
                elif m_cplat:
                    driver.child_plat = m_cplat.group(1)
                elif m_cpriv:
                    driver.child_priv = m_cpriv.group(1)
                elif m_id:
                    driver.uclass_id = m_id.group(1)
                elif m_of_match:
                    compat = m_of_match.group(2)
                elif '};' in line:
                    if driver.uclass_id and compat:
                        if compat not in of_match:
                            raise ValueError(
                                "%s: Unknown compatible var '%s' (found: %s)" %
                                (fname, compat, ','.join(of_match.keys())))
                        driver.compat = of_match[compat]

                        # This needs to be deterministic, since a driver may
                        # have multiple compatible strings pointing to it.
                        # We record the one earliest in the alphabet so it
                        # will produce the same result on all machines.
                        for compat_id in of_match[compat]:
                            old = self._compat_to_driver.get(compat_id)
                            if not old or driver.name < old.name:
                                self._compat_to_driver[compat_id] = driver
                        drivers[driver.name] = driver
                    else:
                        # The driver does not have a uclass or compat string.
                        # The first is required but the second is not, so just
                        # ignore this.
                        pass
                    driver = None
                    ids_name = None
                    compat = None
                    compat_dict = {}

            elif ids_name:
                compat_m = re_compat.search(line)
                if compat_m:
                    compat_dict[compat_m.group(1)] = compat_m.group(3)
                elif '};' in line:
                    of_match[ids_name] = compat_dict
                    ids_name = None
            elif driver_match:
                driver_name = driver_match.group(1)
                driver = Driver(driver_name, fname)
            else:
                ids_m = re_ids.search(line)
                if ids_m:
                    ids_name = ids_m.group(1)

        # Make the updates based on what we found
        self._drivers.update(drivers)
        self._of_match.update(of_match)

    def scan_driver(self, fname):
        """Scan a driver file to build a list of driver names and aliases

        It updates the following members:
            _drivers - updated with new Driver records for each driver found
                in the file
            _of_match - updated with each compatible string found in the file
            _compat_to_driver - Maps compatible string to Driver
            _driver_aliases - Maps alias names to driver name

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

            # If this file has any U_BOOT_DRIVER() declarations, process it to
            # obtain driver information
            if 'U_BOOT_DRIVER' in buff:
                self._parse_driver(fname, buff)
            if 'UCLASS_DRIVER' in buff:
                self._parse_uclass_driver(fname, buff)

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
            rel_path = dirpath[len(self._basedir):]
            if rel_path.startswith('/'):
                rel_path = rel_path[1:]
            if rel_path.startswith('build') or rel_path.startswith('.git'):
                continue
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

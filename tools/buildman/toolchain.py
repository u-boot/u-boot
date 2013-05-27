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

import glob
import os

import bsettings
import command

class Toolchain:
    """A single toolchain

    Public members:
        gcc: Full path to C compiler
        path: Directory path containing C compiler
        cross: Cross compile string, e.g. 'arm-linux-'
        arch: Architecture of toolchain as determined from the first
                component of the filename. E.g. arm-linux-gcc becomes arm
    """

    def __init__(self, fname, test, verbose=False):
        """Create a new toolchain object.

        Args:
            fname: Filename of the gcc component
            test: True to run the toolchain to test it
        """
        self.gcc = fname
        self.path = os.path.dirname(fname)
        self.cross = os.path.basename(fname)[:-3]
        pos = self.cross.find('-')
        self.arch = self.cross[:pos] if pos != -1 else 'sandbox'

        env = self.MakeEnvironment()

        # As a basic sanity check, run the C compiler with --version
        cmd = [fname, '--version']
        if test:
            result = command.RunPipe([cmd], capture=True, env=env)
            self.ok = result.return_code == 0
            if verbose:
                print 'Tool chain test: ',
                if self.ok:
                    print 'OK'
                else:
                    print 'BAD'
                    print 'Command: ', cmd
                    print result.stdout
                    print result.stderr
        else:
            self.ok = True
        self.priority = self.GetPriority(fname)

    def GetPriority(self, fname):
        """Return the priority of the toolchain.

        Toolchains are ranked according to their suitability by their
        filename prefix.

        Args:
            fname: Filename of toolchain
        Returns:
            Priority of toolchain, 0=highest, 20=lowest.
        """
        priority_list = ['-elf', '-unknown-linux-gnu', '-linux', '-elf',
            '-none-linux-gnueabi', '-uclinux', '-none-eabi',
            '-gentoo-linux-gnu', '-linux-gnueabi', '-le-linux', '-uclinux']
        for prio in range(len(priority_list)):
            if priority_list[prio] in fname:
                return prio
        return prio

    def MakeEnvironment(self):
        """Returns an environment for using the toolchain.

        Thie takes the current environment, adds CROSS_COMPILE and
        augments PATH so that the toolchain will operate correctly.
        """
        env = dict(os.environ)
        env['CROSS_COMPILE'] = self.cross
        env['PATH'] += (':' + self.path)
        return env


class Toolchains:
    """Manage a list of toolchains for building U-Boot

    We select one toolchain for each architecture type

    Public members:
        toolchains: Dict of Toolchain objects, keyed by architecture name
        paths: List of paths to check for toolchains (may contain wildcards)
    """

    def __init__(self):
        self.toolchains = {}
        self.paths = []
        for name, value in bsettings.GetItems('toolchain'):
            if '*' in value:
                self.paths += glob.glob(value)
            else:
                self.paths.append(value)


    def Add(self, fname, test=True, verbose=False):
        """Add a toolchain to our list

        We select the given toolchain as our preferred one for its
        architecture if it is a higher priority than the others.

        Args:
            fname: Filename of toolchain's gcc driver
            test: True to run the toolchain to test it
        """
        toolchain = Toolchain(fname, test, verbose)
        add_it = toolchain.ok
        if toolchain.arch in self.toolchains:
            add_it = (toolchain.priority <
                        self.toolchains[toolchain.arch].priority)
        if add_it:
            self.toolchains[toolchain.arch] = toolchain

    def Scan(self, verbose):
        """Scan for available toolchains and select the best for each arch.

        We look for all the toolchains we can file, figure out the
        architecture for each, and whether it works. Then we select the
        highest priority toolchain for each arch.

        Args:
            verbose: True to print out progress information
        """
        if verbose: print 'Scanning for tool chains'
        for path in self.paths:
            if verbose: print "   - scanning path '%s'" % path
            for subdir in ['.', 'bin', 'usr/bin']:
                dirname = os.path.join(path, subdir)
                if verbose: print "      - looking in '%s'" % dirname
                for fname in glob.glob(dirname + '/*gcc'):
                    if verbose: print "         - found '%s'" % fname
                    self.Add(fname, True, verbose)

    def List(self):
        """List out the selected toolchains for each architecture"""
        print 'List of available toolchains (%d):' % len(self.toolchains)
        if len(self.toolchains):
            for key, value in sorted(self.toolchains.iteritems()):
                print '%-10s: %s' % (key, value.gcc)
        else:
            print 'None'

    def Select(self, arch):
        """Returns the toolchain for a given architecture

        Args:
            args: Name of architecture (e.g. 'arm', 'ppc_8xx')

        returns:
            toolchain object, or None if none found
        """
        for name, value in bsettings.GetItems('toolchain-alias'):
            if arch == name:
                arch = value

        if not arch in self.toolchains:
            raise ValueError, ("No tool chain found for arch '%s'" % arch)
        return self.toolchains[arch]

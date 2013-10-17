# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import re
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
            result = command.RunPipe([cmd], capture=True, env=env,
                                     raise_on_error=False)
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
        toolchains = bsettings.GetItems('toolchain')
        if not toolchains:
            print ("Warning: No tool chains - please add a [toolchain] section"
                 " to your buildman config file %s. See README for details" %
                 config_fname)

        for name, value in toolchains:
            if '*' in value:
                self.paths += glob.glob(value)
            else:
                self.paths.append(value)
        self._make_flags = dict(bsettings.GetItems('make-flags'))

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

    def ResolveReferences(self, var_dict, args):
        """Resolve variable references in a string

        This converts ${blah} within the string to the value of blah.
        This function works recursively.

        Args:
            var_dict: Dictionary containing variables and their values
            args: String containing make arguments
        Returns:
            Resolved string

        >>> bsettings.Setup()
        >>> tcs = Toolchains()
        >>> tcs.Add('fred', False)
        >>> var_dict = {'oblique' : 'OBLIQUE', 'first' : 'fi${second}rst', \
                        'second' : '2nd'}
        >>> tcs.ResolveReferences(var_dict, 'this=${oblique}_set')
        'this=OBLIQUE_set'
        >>> tcs.ResolveReferences(var_dict, 'this=${oblique}_set${first}nd')
        'this=OBLIQUE_setfi2ndrstnd'
        """
        re_var = re.compile('(\$\{[a-z0-9A-Z]{1,}\})')

        while True:
            m = re_var.search(args)
            if not m:
                break
            lookup = m.group(0)[2:-1]
            value = var_dict.get(lookup, '')
            args = args[:m.start(0)] + value + args[m.end(0):]
        return args

    def GetMakeArguments(self, board):
        """Returns 'make' arguments for a given board

        The flags are in a section called 'make-flags'. Flags are named
        after the target they represent, for example snapper9260=TESTING=1
        will pass TESTING=1 to make when building the snapper9260 board.

        References to other boards can be added in the string also. For
        example:

        [make-flags]
        at91-boards=ENABLE_AT91_TEST=1
        snapper9260=${at91-boards} BUILD_TAG=442
        snapper9g45=${at91-boards} BUILD_TAG=443

        This will return 'ENABLE_AT91_TEST=1 BUILD_TAG=442' for snapper9260
        and 'ENABLE_AT91_TEST=1 BUILD_TAG=443' for snapper9g45.

        A special 'target' variable is set to the board target.

        Args:
            board: Board object for the board to check.
        Returns:
            'make' flags for that board, or '' if none
        """
        self._make_flags['target'] = board.target
        arg_str = self.ResolveReferences(self._make_flags,
                           self._make_flags.get(board.target, ''))
        args = arg_str.split(' ')
        i = 0
        while i < len(args):
            if not args[i]:
                del args[i]
            else:
                i += 1
        return args

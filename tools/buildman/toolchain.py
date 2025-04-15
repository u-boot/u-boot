# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2012 The Chromium OS Authors.
#

import re
import glob
from html.parser import HTMLParser
import os
import sys
import tempfile
import urllib.request, urllib.error, urllib.parse

from buildman import bsettings
from u_boot_pylib import command
from u_boot_pylib import terminal
from u_boot_pylib import tools

(PRIORITY_FULL_PREFIX, PRIORITY_PREFIX_GCC, PRIORITY_PREFIX_GCC_PATH,
    PRIORITY_CALC) = list(range(4))

(VAR_CROSS_COMPILE, VAR_PATH, VAR_ARCH, VAR_MAKE_ARGS) = range(4)

# Simple class to collect links from a page
class MyHTMLParser(HTMLParser):
    def __init__(self, arch):
        """Create a new parser

        After the parser runs, self.links will be set to a list of the links
        to .xz archives found in the page, and self.arch_link will be set to
        the one for the given architecture (or None if not found).

        Args:
            arch: Architecture to search for
        """
        HTMLParser.__init__(self)
        self.arch_link = None
        self.links = []
        self.re_arch = re.compile('[-_]%s-' % arch)

    def handle_starttag(self, tag, attrs):
        if tag == 'a':
            for tag, value in attrs:
                if tag == 'href':
                    if value and value.endswith('.xz'):
                        self.links.append(value)
                        if self.re_arch.search(value):
                            self.arch_link = value


class Toolchain:
    """A single toolchain

    Public members:
        gcc: Full path to C compiler
        path: Directory path containing C compiler
        cross: Cross compile string, e.g. 'arm-linux-'
        arch: Architecture of toolchain as determined from the first
                component of the filename. E.g. arm-linux-gcc becomes arm
        priority: Toolchain priority (0=highest, 20=lowest)
        override_toolchain: Toolchain to use for sandbox, overriding the normal
                one
    """
    def __init__(self, fname, test, verbose=False, priority=PRIORITY_CALC,
                 arch=None, override_toolchain=None):
        """Create a new toolchain object.

        Args:
            fname: Filename of the gcc component, possibly with ~ or $HOME in it
            test: True to run the toolchain to test it
            verbose: True to print out the information
            priority: Priority to use for this toolchain, or PRIORITY_CALC to
                calculate it
        """
        fname = os.path.expanduser(fname)
        self.gcc = fname
        self.path = os.path.dirname(fname)
        self.override_toolchain = override_toolchain

        # Find the CROSS_COMPILE prefix to use for U-Boot. For example,
        # 'arm-linux-gnueabihf-gcc' turns into 'arm-linux-gnueabihf-'.
        basename = os.path.basename(fname)
        pos = basename.rfind('-')
        self.cross = basename[:pos + 1] if pos != -1 else ''

        # The architecture is the first part of the name
        pos = self.cross.find('-')
        if arch:
            self.arch = arch
        else:
            self.arch = self.cross[:pos] if pos != -1 else 'sandbox'
        if self.arch == 'sandbox' and override_toolchain:
            self.gcc = override_toolchain

        env = self.MakeEnvironment(False)

        # As a basic sanity check, run the C compiler with --version
        cmd = [fname, '--version']
        if priority == PRIORITY_CALC:
            self.priority = self.GetPriority(fname)
        else:
            self.priority = priority
        if test:
            result = command.run_one(*cmd, capture=True, env=env,
                                     raise_on_error=False)
            self.ok = result.return_code == 0
            if verbose:
                print('Tool chain test: ', end=' ')
                if self.ok:
                    print("OK, arch='%s', priority %d" % (self.arch,
                                                          self.priority))
                else:
                    print('BAD')
                    print(f"Command: {' '.join(cmd)}")
                    print(result.stdout)
                    print(result.stderr)
        else:
            self.ok = True

    def GetPriority(self, fname):
        """Return the priority of the toolchain.

        Toolchains are ranked according to their suitability by their
        filename prefix.

        Args:
            fname: Filename of toolchain
        Returns:
            Priority of toolchain, PRIORITY_CALC=highest, 20=lowest.
        """
        priority_list = ['-elf', '-unknown-linux-gnu', '-linux',
            '-none-linux-gnueabi', '-none-linux-gnueabihf', '-uclinux',
            '-none-eabi', '-gentoo-linux-gnu', '-linux-gnueabi',
            '-linux-gnueabihf', '-le-linux', '-uclinux']
        for prio in range(len(priority_list)):
            if priority_list[prio] in fname:
                return PRIORITY_CALC + prio
        return PRIORITY_CALC + prio

    def GetWrapper(self, show_warning=True):
        """Get toolchain wrapper from the setting file.
        """
        value = ''
        for name, value in bsettings.get_items('toolchain-wrapper'):
            if not value:
                print("Warning: Wrapper not found")
        if value:
            value = value + ' '

        return value

    def GetEnvArgs(self, which):
        """Get an environment variable/args value based on the the toolchain

        Args:
            which: VAR_... value to get

        Returns:
            Value of that environment variable or arguments
        """
        if which == VAR_CROSS_COMPILE:
            wrapper = self.GetWrapper()
            base = '' if self.arch == 'sandbox' else self.path
            if (base == '' and self.cross == ''):
                return ''
            return wrapper + os.path.join(base, self.cross)
        elif which == VAR_PATH:
            return self.path
        elif which == VAR_ARCH:
            return self.arch
        elif which == VAR_MAKE_ARGS:
            args = self.MakeArgs()
            if args:
                return ' '.join(args)
            return ''
        else:
            raise ValueError('Unknown arg to GetEnvArgs (%d)' % which)

    def MakeEnvironment(self, full_path, env=None):
        """Returns an environment for using the toolchain.

        This takes the current environment and adds CROSS_COMPILE so that
        the tool chain will operate correctly. This also disables localized
        output and possibly Unicode encoded output of all build tools by
        adding LC_ALL=C. For the case where full_path is False, it prepends
        the toolchain to PATH

        Note that os.environb is used to obtain the environment, since in some
        cases the environment many contain non-ASCII characters and we see
        errors like:

          UnicodeEncodeError: 'utf-8' codec can't encode characters in position
             569-570: surrogates not allowed

        When running inside a Python venv, care is taken not to put the
        toolchain path before the venv path, so that builds initiated by
        buildman will still respect the venv.

        Args:
            full_path: Return the full path in CROSS_COMPILE and don't set
                PATH
            env (dict of bytes): Original environment, used for testing
        Returns:
            Dict containing the (bytes) environment to use. This is based on the
            current environment, with changes as needed to CROSS_COMPILE, PATH
            and LC_ALL.
        """
        env = dict(env or os.environb)

        wrapper = self.GetWrapper()

        if self.override_toolchain:
            # We'll use MakeArgs() to provide this
            pass
        elif full_path and self.cross:
            env[b'CROSS_COMPILE'] = tools.to_bytes(
                wrapper + os.path.join(self.path, self.cross))
        elif self.cross:
            env[b'CROSS_COMPILE'] = tools.to_bytes(wrapper + self.cross)

            # Detect a Python virtualenv and avoid defeating it
            if sys.prefix != sys.base_prefix:
                paths = env[b'PATH'].split(b':')
                new_paths = []
                to_insert = tools.to_bytes(self.path)
                insert_after = tools.to_bytes(sys.prefix)
                for path in paths:
                    new_paths.append(path)
                    if to_insert and path.startswith(insert_after):
                        new_paths.append(to_insert)
                        to_insert = None
                if to_insert:
                    new_paths.append(to_insert)
                env[b'PATH'] = b':'.join(new_paths)
            else:
                env[b'PATH'] = tools.to_bytes(self.path) + b':' + env[b'PATH']

        env[b'LC_ALL'] = b'C'

        return env

    def MakeArgs(self):
        """Create the 'make' arguments for a toolchain

        This is only used when the toolchain is being overridden. Since the
        U-Boot Makefile sets CC and HOSTCC explicitly we cannot rely on the
        environment (and MakeEnvironment()) to override these values. This
        function returns the arguments to accomplish this.

        Returns:
            List of arguments to pass to 'make'
        """
        if self.override_toolchain:
            return ['HOSTCC=%s' % self.override_toolchain,
                    'CC=%s' % self.override_toolchain]
        return []


class Toolchains:
    """Manage a list of toolchains for building U-Boot

    We select one toolchain for each architecture type

    Public members:
        toolchains: Dict of Toolchain objects, keyed by architecture name
        prefixes: Dict of prefixes to check, keyed by architecture. This can
            be a full path and toolchain prefix, for example
            {'x86', 'opt/i386-linux/bin/i386-linux-'}, or the name of
            something on the search path, for example
            {'arm', 'arm-linux-gnueabihf-'}. Wildcards are not supported.
        paths: List of paths to check for toolchains (may contain wildcards)
    """

    def __init__(self, override_toolchain=None):
        self.toolchains = {}
        self.prefixes = {}
        self.paths = []
        self.override_toolchain = override_toolchain
        self._make_flags = dict(bsettings.get_items('make-flags'))

    def GetPathList(self, show_warning=True):
        """Get a list of available toolchain paths

        Args:
            show_warning: True to show a warning if there are no tool chains.

        Returns:
            List of strings, each a path to a toolchain mentioned in the
            [toolchain] section of the settings file.
        """
        toolchains = bsettings.get_items('toolchain')
        if show_warning and not toolchains:
            print(("Warning: No tool chains. Please run 'buildman "
                   "--fetch-arch all' to download all available toolchains, or "
                   "add a [toolchain] section to your buildman config file "
                   "%s. See buildman.rst for details" %
                   bsettings.config_fname))

        paths = []
        for name, value in toolchains:
            fname = os.path.expanduser(value)
            if '*' in value:
                paths += glob.glob(fname)
            else:
                paths.append(fname)
        return paths

    def GetSettings(self, show_warning=True):
        """Get toolchain settings from the settings file.

        Args:
            show_warning: True to show a warning if there are no tool chains.
        """
        self.prefixes = bsettings.get_items('toolchain-prefix')
        self.paths += self.GetPathList(show_warning)

    def Add(self, fname, test=True, verbose=False, priority=PRIORITY_CALC,
            arch=None):
        """Add a toolchain to our list

        We select the given toolchain as our preferred one for its
        architecture if it is a higher priority than the others.

        Args:
            fname: Filename of toolchain's gcc driver
            test: True to run the toolchain to test it
            priority: Priority to use for this toolchain
            arch: Toolchain architecture, or None if not known
        """
        toolchain = Toolchain(fname, test, verbose, priority, arch,
                              self.override_toolchain)
        add_it = toolchain.ok
        if add_it:
            if toolchain.arch in self.toolchains:
                add_it = (toolchain.priority <
                          self.toolchains[toolchain.arch].priority)
            if add_it:
                self.toolchains[toolchain.arch] = toolchain
            elif verbose:
                print(("Toolchain '%s' at priority %d will be ignored because "
                       "another toolchain for arch '%s' has priority %d" %
                       (toolchain.gcc, toolchain.priority, toolchain.arch,
                        self.toolchains[toolchain.arch].priority)))

    def ScanPath(self, path, verbose):
        """Scan a path for a valid toolchain

        Args:
            path: Path to scan
            verbose: True to print out progress information
        Returns:
            Filename of C compiler if found, else None
        """
        fnames = []
        for subdir in ['.', 'bin', 'usr/bin']:
            dirname = os.path.join(path, subdir)
            if verbose: print("      - looking in '%s'" % dirname)
            for fname in glob.glob(dirname + '/*gcc'):
                if verbose: print("         - found '%s'" % fname)
                fnames.append(fname)
        return fnames

    def ScanPathEnv(self, fname):
        """Scan the PATH environment variable for a given filename.

        Args:
            fname: Filename to scan for
        Returns:
            List of matching pathanames, or [] if none
        """
        pathname_list = []
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            pathname = os.path.join(path, fname)
            if os.path.exists(pathname):
                pathname_list.append(pathname)
        return pathname_list

    def Scan(self, verbose, raise_on_error=True):
        """Scan for available toolchains and select the best for each arch.

        We look for all the toolchains we can file, figure out the
        architecture for each, and whether it works. Then we select the
        highest priority toolchain for each arch.

        Args:
            verbose: True to print out progress information
        """
        if verbose: print('Scanning for tool chains')
        for name, value in self.prefixes:
            fname = os.path.expanduser(value)
            if verbose: print("   - scanning prefix '%s'" % fname)
            if os.path.exists(fname):
                self.Add(fname, True, verbose, PRIORITY_FULL_PREFIX, name)
                continue
            fname += 'gcc'
            if os.path.exists(fname):
                self.Add(fname, True, verbose, PRIORITY_PREFIX_GCC, name)
                continue
            fname_list = self.ScanPathEnv(fname)
            for f in fname_list:
                self.Add(f, True, verbose, PRIORITY_PREFIX_GCC_PATH, name)
            if not fname_list:
                msg = f"No tool chain found for prefix '{fname}'"
                if raise_on_error:
                    raise ValueError(msg)
                else:
                    print(f'Error: {msg}')
        for path in self.paths:
            if verbose: print("   - scanning path '%s'" % path)
            fnames = self.ScanPath(path, verbose)
            for fname in fnames:
                self.Add(fname, True, verbose)

    def List(self):
        """List out the selected toolchains for each architecture"""
        col = terminal.Color()
        print(col.build(col.BLUE, 'List of available toolchains (%d):' %
                        len(self.toolchains)))
        if len(self.toolchains):
            for key, value in sorted(self.toolchains.items()):
                print('%-10s: %s' % (key, value.gcc))
        else:
            print('None')

    def Select(self, arch):
        """Returns the toolchain for a given architecture

        Args:
            args: Name of architecture (e.g. 'arm', 'ppc_8xx')

        returns:
            toolchain object, or None if none found
        """
        for tag, value in bsettings.get_items('toolchain-alias'):
            if arch == tag:
                for alias in value.split():
                    if alias in self.toolchains:
                        return self.toolchains[alias]

        if not arch in self.toolchains:
            raise ValueError("No tool chain found for arch '%s'" % arch)
        return self.toolchains[arch]

    def ResolveReferences(self, var_dict, args):
        """Resolve variable references in a string

        This converts ${blah} within the string to the value of blah.
        This function works recursively.

            Resolved string

        Args:
            var_dict: Dictionary containing variables and their values
            args: String containing make arguments
        Returns:
        >>> bsettings.setup(None)
        >>> tcs = Toolchains()
        >>> tcs.Add('fred', False)
        >>> var_dict = {'oblique' : 'OBLIQUE', 'first' : 'fi${second}rst', \
                        'second' : '2nd'}
        >>> tcs.ResolveReferences(var_dict, 'this=${oblique}_set')
        'this=OBLIQUE_set'
        >>> tcs.ResolveReferences(var_dict, 'this=${oblique}_set${first}nd')
        'this=OBLIQUE_setfi2ndrstnd'
        """
        re_var = re.compile(r'(\$\{[-_a-z0-9A-Z]{1,}\})')

        while True:
            m = re_var.search(args)
            if not m:
                break
            lookup = m.group(0)[2:-1]
            value = var_dict.get(lookup, '')
            args = args[:m.start(0)] + value + args[m.end(0):]
        return args

    def GetMakeArguments(self, brd):
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
            brd: Board object for the board to check.
        Returns:
            'make' flags for that board, or '' if none
        """
        self._make_flags['target'] = brd.target
        arg_str = self.ResolveReferences(self._make_flags,
                           self._make_flags.get(brd.target, ''))
        args = re.findall(r"(?:\".*?\"|\S)+", arg_str)
        i = 0
        while i < len(args):
            args[i] = args[i].replace('"', '')
            if not args[i]:
                del args[i]
            else:
                i += 1
        return args

    def LocateArchUrl(self, fetch_arch):
        """Find a toolchain available online

        Look in standard places for available toolchains. At present the
        only standard place is at kernel.org.

        Args:
            arch: Architecture to look for, or 'list' for all
        Returns:
            If fetch_arch is 'list', a tuple:
                Machine architecture (e.g. x86_64)
                List of toolchains
            else
                URL containing this toolchain, if avaialble, else None
        """
        arch = command.output_one_line('uname', '-m')
        if arch == 'aarch64':
            arch = 'arm64'
        base = 'https://www.kernel.org/pub/tools/crosstool/files/bin'
        versions = ['13.2.0', '12.2.0']
        links = []
        for version in versions:
            url = '%s/%s/%s/' % (base, arch, version)
            print('Checking: %s' % url)
            response = urllib.request.urlopen(url)
            html = tools.to_string(response.read())
            parser = MyHTMLParser(fetch_arch)
            parser.feed(html)
            if fetch_arch == 'list':
                links += parser.links
            elif parser.arch_link:
                return url + parser.arch_link
        if fetch_arch == 'list':
            return arch, links
        return None

    def Unpack(self, fname, dest):
        """Unpack a tar file

        Args:
            fname: Filename to unpack
            dest: Destination directory
        Returns:
            Directory name of the first entry in the archive, without the
            trailing /
        """
        stdout = command.output('tar', 'xvfJ', fname, '-C', dest)
        dirs = stdout.splitlines()[1].split('/')[:2]
        return '/'.join(dirs)

    def TestSettingsHasPath(self, path):
        """Check if buildman will find this toolchain

        Returns:
            True if the path is in settings, False if not
        """
        paths = self.GetPathList(False)
        return path in paths

    def ListArchs(self):
        """List architectures with available toolchains to download"""
        host_arch, archives = self.LocateArchUrl('list')
        re_arch = re.compile('[-a-z0-9.]*[-_]([^-]*)-.*')
        arch_set = set()
        for archive in archives:
            # Remove the host architecture from the start
            arch = re_arch.match(archive[len(host_arch):])
            if arch:
                if arch.group(1) != '2.0' and arch.group(1) != '64':
                    arch_set.add(arch.group(1))
        return sorted(arch_set)

    def FetchAndInstall(self, arch):
        """Fetch and install a new toolchain

        arch:
            Architecture to fetch, or 'list' to list
        """
        # Fist get the URL for this architecture
        col = terminal.Color()
        print(col.build(col.BLUE, "Downloading toolchain for arch '%s'" % arch))
        url = self.LocateArchUrl(arch)
        if not url:
            print(("Cannot find toolchain for arch '%s' - use 'list' to list" %
                   arch))
            return 2
        home = os.environ['HOME']
        dest = os.path.join(home, '.buildman-toolchains')
        if not os.path.exists(dest):
            os.mkdir(dest)

        # Download the tar file for this toolchain and unpack it
        tarfile, tmpdir = tools.download(url, '.buildman')
        if not tarfile:
            return 1
        print(col.build(col.GREEN, 'Unpacking to: %s' % dest), end=' ')
        sys.stdout.flush()
        path = self.Unpack(tarfile, dest)
        os.remove(tarfile)
        os.rmdir(tmpdir)
        print()

        # Check that the toolchain works
        print(col.build(col.GREEN, 'Testing'))
        dirpath = os.path.join(dest, path)
        compiler_fname_list = self.ScanPath(dirpath, True)
        if not compiler_fname_list:
            print('Could not locate C compiler - fetch failed.')
            return 1
        if len(compiler_fname_list) != 1:
            print(col.build(col.RED, 'Warning, ambiguous toolchains: %s' %
                            ', '.join(compiler_fname_list)))
        toolchain = Toolchain(compiler_fname_list[0], True, True)

        # Make sure that it will be found by buildman
        if not self.TestSettingsHasPath(dirpath):
            print(("Adding 'download' to config file '%s'" %
                   bsettings.config_fname))
            bsettings.set_item('toolchain', 'download', '%s/*/*' % dest)
        return 0

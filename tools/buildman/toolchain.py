# Copyright (c) 2012 The Chromium OS Authors.
#
# SPDX-License-Identifier:	GPL-2.0+
#

import re
import glob
from HTMLParser import HTMLParser
import os
import sys
import tempfile
import urllib2

import bsettings
import command

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
        self._match = '_%s-' % arch

    def handle_starttag(self, tag, attrs):
        if tag == 'a':
            for tag, value in attrs:
                if tag == 'href':
                    if value and value.endswith('.xz'):
                        self.links.append(value)
                        if self._match in value:
                            self.arch_link = value


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

        # Find the CROSS_COMPILE prefix to use for U-Boot. For example,
        # 'arm-linux-gnueabihf-gcc' turns into 'arm-linux-gnueabihf-'.
        basename = os.path.basename(fname)
        pos = basename.rfind('-')
        self.cross = basename[:pos + 1] if pos != -1 else ''

        # The architecture is the first part of the name
        pos = self.cross.find('-')
        self.arch = self.cross[:pos] if pos != -1 else 'sandbox'

        env = self.MakeEnvironment(False)

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
        priority_list = ['-elf', '-unknown-linux-gnu', '-linux',
            '-none-linux-gnueabi', '-uclinux', '-none-eabi',
            '-gentoo-linux-gnu', '-linux-gnueabi', '-le-linux', '-uclinux']
        for prio in range(len(priority_list)):
            if priority_list[prio] in fname:
                return prio
        return prio

    def MakeEnvironment(self, full_path):
        """Returns an environment for using the toolchain.

        Thie takes the current environment and adds CROSS_COMPILE so that
        the tool chain will operate correctly.

        Args:
            full_path: Return the full path in CROSS_COMPILE and don't set
                PATH
        """
        env = dict(os.environ)
        if full_path:
            env['CROSS_COMPILE'] = os.path.join(self.path, self.cross)
        else:
            env['CROSS_COMPILE'] = self.cross
            env['PATH'] = self.path + ':' + env['PATH']

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
        self._make_flags = dict(bsettings.GetItems('make-flags'))

    def GetPathList(self):
        """Get a list of available toolchain paths

        Returns:
            List of strings, each a path to a toolchain mentioned in the
            [toolchain] section of the settings file.
        """
        toolchains = bsettings.GetItems('toolchain')
        if not toolchains:
            print ("Warning: No tool chains - please add a [toolchain] section"
                 " to your buildman config file %s. See README for details" %
                 bsettings.config_fname)

        paths = []
        for name, value in toolchains:
            if '*' in value:
                paths += glob.glob(value)
            else:
                paths.append(value)
        return paths

    def GetSettings(self):
      self.paths += self.GetPathList()

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
            if verbose: print "      - looking in '%s'" % dirname
            for fname in glob.glob(dirname + '/*gcc'):
                if verbose: print "         - found '%s'" % fname
                fnames.append(fname)
        return fnames


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
            fnames = self.ScanPath(path, verbose)
            for fname in fnames:
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
        for tag, value in bsettings.GetItems('toolchain-alias'):
            if arch == tag:
                for alias in value.split():
                    if alias in self.toolchains:
                        return self.toolchains[alias]

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
        re_var = re.compile('(\$\{[-_a-z0-9A-Z]{1,}\})')

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
        arch = command.OutputOneLine('uname', '-m')
        base = 'https://www.kernel.org/pub/tools/crosstool/files/bin'
        versions = ['4.9.0', '4.6.3', '4.6.2', '4.5.1', '4.2.4']
        links = []
        for version in versions:
            url = '%s/%s/%s/' % (base, arch, version)
            print 'Checking: %s' % url
            response = urllib2.urlopen(url)
            html = response.read()
            parser = MyHTMLParser(fetch_arch)
            parser.feed(html)
            if fetch_arch == 'list':
                links += parser.links
            elif parser.arch_link:
                return url + parser.arch_link
        if fetch_arch == 'list':
            return arch, links
        return None

    def Download(self, url):
        """Download a file to a temporary directory

        Args:
            url: URL to download
        Returns:
            Tuple:
                Temporary directory name
                Full path to the downloaded archive file in that directory,
                    or None if there was an error while downloading
        """
        print "Downloading: %s" % url
        leaf = url.split('/')[-1]
        tmpdir = tempfile.mkdtemp('.buildman')
        response = urllib2.urlopen(url)
        fname = os.path.join(tmpdir, leaf)
        fd = open(fname, 'wb')
        meta = response.info()
        size = int(meta.getheaders("Content-Length")[0])
        done = 0
        block_size = 1 << 16
        status = ''

        # Read the file in chunks and show progress as we go
        while True:
            buffer = response.read(block_size)
            if not buffer:
                print chr(8) * (len(status) + 1), '\r',
                break

            done += len(buffer)
            fd.write(buffer)
            status = r"%10d MiB  [%3d%%]" % (done / 1024 / 1024,
                                             done * 100 / size)
            status = status + chr(8) * (len(status) + 1)
            print status,
            sys.stdout.flush()
        fd.close()
        if done != size:
            print 'Error, failed to download'
            os.remove(fname)
            fname = None
        return tmpdir, fname

    def Unpack(self, fname, dest):
        """Unpack a tar file

        Args:
            fname: Filename to unpack
            dest: Destination directory
        Returns:
            Directory name of the first entry in the archive, without the
            trailing /
        """
        stdout = command.Output('tar', 'xvfJ', fname, '-C', dest)
        return stdout.splitlines()[0][:-1]

    def TestSettingsHasPath(self, path):
        """Check if builmand will find this toolchain

        Returns:
            True if the path is in settings, False if not
        """
        paths = self.GetPathList()
        return path in paths

    def ListArchs(self):
        """List architectures with available toolchains to download"""
        host_arch, archives = self.LocateArchUrl('list')
        re_arch = re.compile('[-a-z0-9.]*_([^-]*)-.*')
        arch_set = set()
        for archive in archives:
            # Remove the host architecture from the start
            arch = re_arch.match(archive[len(host_arch):])
            if arch:
                arch_set.add(arch.group(1))
        return sorted(arch_set)

    def FetchAndInstall(self, arch):
        """Fetch and install a new toolchain

        arch:
            Architecture to fetch, or 'list' to list
        """
        # Fist get the URL for this architecture
        url = self.LocateArchUrl(arch)
        if not url:
            print ("Cannot find toolchain for arch '%s' - use 'list' to list" %
                   arch)
            return 2
        home = os.environ['HOME']
        dest = os.path.join(home, '.buildman-toolchains')
        if not os.path.exists(dest):
            os.mkdir(dest)

        # Download the tar file for this toolchain and unpack it
        tmpdir, tarfile = self.Download(url)
        if not tarfile:
            return 1
        print 'Unpacking to: %s' % dest,
        sys.stdout.flush()
        path = self.Unpack(tarfile, dest)
        os.remove(tarfile)
        os.rmdir(tmpdir)
        print

        # Check that the toolchain works
        print 'Testing'
        dirpath = os.path.join(dest, path)
        compiler_fname_list = self.ScanPath(dirpath, True)
        if not compiler_fname_list:
            print 'Could not locate C compiler - fetch failed.'
            return 1
        if len(compiler_fname_list) != 1:
            print ('Internal error, ambiguous toolchains: %s' %
                   (', '.join(compiler_fname)))
            return 1
        toolchain = Toolchain(compiler_fname_list[0], True, True)

        # Make sure that it will be found by buildman
        if not self.TestSettingsHasPath(dirpath):
            print ("Adding 'download' to config file '%s'" %
                   bsettings.config_fname)
            tools_dir = os.path.dirname(dirpath)
            bsettings.SetItem('toolchain', 'download', '%s/*' % tools_dir)
        return 0

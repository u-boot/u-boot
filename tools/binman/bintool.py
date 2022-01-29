# SPDX-License-Identifier: GPL-2.0+
# Copyright 2022 Google LLC
#
"""Base class for all bintools

This defines the common functionality for all bintools, including running
the tool, checking its version and fetching it if needed.
"""

import collections
import glob
import importlib
import multiprocessing
import os
import shutil
import tempfile
import urllib.error

from patman import command
from patman import terminal
from patman import tools
from patman import tout

BINMAN_DIR = os.path.dirname(os.path.realpath(__file__))

# Format string for listing bintools, see also the header in list_all()
FORMAT = '%-16.16s %-12.12s %-26.26s %s'

# List of known modules, to avoid importing the module multiple times
modules = {}

# Possible ways of fetching a tool (FETCH_COUNT is number of ways)
FETCH_ANY, FETCH_BIN, FETCH_BUILD, FETCH_COUNT = range(4)

FETCH_NAMES = {
    FETCH_ANY: 'any method',
    FETCH_BIN: 'binary download',
    FETCH_BUILD: 'build from source'
    }

# Status of tool fetching
FETCHED, FAIL, PRESENT, STATUS_COUNT = range(4)

DOWNLOAD_DESTDIR = os.path.join(os.getenv('HOME'), 'bin')

class Bintool:
    """Tool which operates on binaries to help produce entry contents

    This is the base class for all bintools
    """
    # List of bintools to regard as missing
    missing_list = []

    def __init__(self, name, desc):
        self.name = name
        self.desc = desc

    @staticmethod
    def find_bintool_class(btype):
        """Look up the bintool class for bintool

        Args:
            byte: Bintool to use, e.g. 'mkimage'

        Returns:
            The bintool class object if found, else a tuple:
                module name that could not be found
                exception received
        """
        # Convert something like 'u-boot' to 'u_boot' since we are only
        # interested in the type.
        module_name = btype.replace('-', '_')
        module = modules.get(module_name)

        # Import the module if we have not already done so
        if not module:
            try:
                module = importlib.import_module('binman.btool.' + module_name)
            except ImportError as exc:
                return module_name, exc
            modules[module_name] = module

        # Look up the expected class name
        return getattr(module, 'Bintool%s' % module_name)

    @staticmethod
    def create(name):
        """Create a new bintool object

        Args:
            name (str): Bintool to create, e.g. 'mkimage'

        Returns:
            A new object of the correct type (a subclass of Binutil)
        """
        cls = Bintool.find_bintool_class(name)
        if isinstance(cls, tuple):
            raise ValueError("Cannot import bintool module '%s': %s" % cls)

        # Call its constructor to get the object we want.
        obj = cls(name)
        return obj

    def show(self):
        """Show a line of information about a bintool"""
        if self.is_present():
            version = self.version()
        else:
            version = '-'
        print(FORMAT % (self.name, version, self.desc,
                        self.get_path() or '(not found)'))

    @classmethod
    def set_missing_list(cls, missing_list):
        cls.missing_list = missing_list or []

    @staticmethod
    def get_tool_list(include_testing=False):
        """Get a list of the known tools

        Returns:
            list of str: names of all tools known to binman
        """
        files = glob.glob(os.path.join(BINMAN_DIR, 'btool/*'))
        names = [os.path.splitext(os.path.basename(fname))[0]
                 for fname in files]
        names = [name for name in names if name[0] != '_']
        if include_testing:
            names.append('_testing')
        return sorted(names)

    @staticmethod
    def list_all():
        """List all the bintools known to binman"""
        names = Bintool.get_tool_list()
        print(FORMAT % ('Name', 'Version', 'Description', 'Path'))
        print(FORMAT % ('-' * 15,'-' * 11, '-' * 25, '-' * 30))
        for name in names:
            btool = Bintool.create(name)
            btool.show()

    def is_present(self):
        """Check if a bintool is available on the system

        Returns:
            bool: True if available, False if not
        """
        if self.name in self.missing_list:
            return False
        return bool(self.get_path())

    def get_path(self):
        """Get the path of a bintool

        Returns:
            str: Path to the tool, if available, else None
        """
        return tools.tool_find(self.name)

    def fetch_tool(self, method, col, skip_present):
        """Fetch a single tool

        Args:
            method (FETCH_...): Method to use
            col (terminal.Color): Color terminal object
            skip_present (boo;): Skip fetching if it is already present

        Returns:
            int: Result of fetch either FETCHED, FAIL, PRESENT
        """
        def try_fetch(meth):
            res = None
            try:
                res = self.fetch(meth)
            except urllib.error.URLError as uerr:
                message = uerr.reason
                print(col.build(col.RED, f'- {message}'))

            except ValueError as exc:
                print(f'Exception: {exc}')
            return res

        if skip_present and self.is_present():
            return PRESENT
        print(col.build(col.YELLOW, 'Fetch: %s' % self.name))
        if method == FETCH_ANY:
            for try_method in range(1, FETCH_COUNT):
                print(f'- trying method: {FETCH_NAMES[try_method]}')
                result = try_fetch(try_method)
                if result:
                    break
        else:
            result = try_fetch(method)
        if not result:
            return FAIL
        if result is not True:
            fname, tmpdir = result
            dest = os.path.join(DOWNLOAD_DESTDIR, self.name)
            print(f"- writing to '{dest}'")
            shutil.move(fname, dest)
            if tmpdir:
                shutil.rmtree(tmpdir)
        return FETCHED

    @staticmethod
    def fetch_tools(method, names_to_fetch):
        """Fetch bintools from a suitable place

        This fetches or builds the requested bintools so that they can be used
        by binman

        Args:
            names_to_fetch (list of str): names of bintools to fetch

        Returns:
            True on success, False on failure
        """
        def show_status(color, prompt, names):
            print(col.build(
                color, f'{prompt}:%s{len(names):2}: %s' %
                (' ' * (16 - len(prompt)), ' '.join(names))))

        col = terminal.Color()
        skip_present = False
        name_list = names_to_fetch
        if len(names_to_fetch) == 1 and names_to_fetch[0] in ['all', 'missing']:
            name_list = Bintool.get_tool_list()
            if names_to_fetch[0] == 'missing':
                skip_present = True
            print(col.build(col.YELLOW,
                            'Fetching tools:      %s' % ' '.join(name_list)))
        status = collections.defaultdict(list)
        for name in name_list:
            btool = Bintool.create(name)
            result = btool.fetch_tool(method, col, skip_present)
            status[result].append(name)
            if result == FAIL:
                if method == FETCH_ANY:
                    print('- failed to fetch with all methods')
                else:
                    print(f"- method '{FETCH_NAMES[method]}' is not supported")

        if len(name_list) > 1:
            if skip_present:
                show_status(col.GREEN, 'Already present', status[PRESENT])
            show_status(col.GREEN, 'Tools fetched', status[FETCHED])
            if status[FAIL]:
                show_status(col.RED, 'Failures', status[FAIL])
        return not status[FAIL]

    def run_cmd_result(self, *args, binary=False, raise_on_error=True):
        """Run the bintool using command-line arguments

        Args:
            args (list of str): Arguments to provide, in addition to the bintool
                name
            binary (bool): True to return output as bytes instead of str
            raise_on_error (bool): True to raise a ValueError exception if the
                tool returns a non-zero return code

        Returns:
            CommandResult: Resulting output from the bintool, or None if the
                tool is not present
        """
        if self.name in self.missing_list:
            return None
        name = os.path.expanduser(self.name)  # Expand paths containing ~
        all_args = (name,) + args
        env = tools.get_env_with_path()
        tout.detail(f"bintool: {' '.join(all_args)}")
        result = command.run_pipe(
            [all_args], capture=True, capture_stderr=True, env=env,
            raise_on_error=False, binary=binary)

        if result.return_code:
            # Return None if the tool was not found. In this case there is no
            # output from the tool and it does not appear on the path. We still
            # try to run it (as above) since RunPipe() allows faking the tool's
            # output
            if not any([result.stdout, result.stderr, tools.tool_find(name)]):
                tout.info(f"bintool '{name}' not found")
                return None
            if raise_on_error:
                tout.info(f"bintool '{name}' failed")
                raise ValueError("Error %d running '%s': %s" %
                                (result.return_code, ' '.join(all_args),
                                result.stderr or result.stdout))
        if result.stdout:
            tout.debug(result.stdout)
        if result.stderr:
            tout.debug(result.stderr)
        return result

    def run_cmd(self, *args, binary=False):
        """Run the bintool using command-line arguments

        Args:
            args (list of str): Arguments to provide, in addition to the bintool
                name
            binary (bool): True to return output as bytes instead of str

        Returns:
            str or bytes: Resulting stdout from the bintool
        """
        result = self.run_cmd_result(*args, binary=binary)
        if result:
            return result.stdout

    @classmethod
    def build_from_git(cls, git_repo, make_target, bintool_path):
        """Build a bintool from a git repo

        This clones the repo in a temporary directory, builds it with 'make',
        then returns the filename of the resulting executable bintool

        Args:
            git_repo (str): URL of git repo
            make_target (str): Target to pass to 'make' to build the tool
            bintool_path (str): Relative path of the tool in the repo, after
                build is complete

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
            or None on error
        """
        tmpdir = tempfile.mkdtemp(prefix='binmanf.')
        print(f"- clone git repo '{git_repo}' to '{tmpdir}'")
        tools.run('git', 'clone', '--depth', '1', git_repo, tmpdir)
        print(f"- build target '{make_target}'")
        tools.run('make', '-C', tmpdir, '-j', f'{multiprocessing.cpu_count()}',
                  make_target)
        fname = os.path.join(tmpdir, bintool_path)
        if not os.path.exists(fname):
            print(f"- File '{fname}' was not produced")
            return None
        return fname, tmpdir

    @classmethod
    def fetch_from_url(cls, url):
        """Fetch a bintool from a URL

        Args:
            url (str): URL to fetch from

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        fname, tmpdir = tools.download(url)
        tools.run('chmod', 'a+x', fname)
        return fname, tmpdir

    @classmethod
    def fetch_from_drive(cls, drive_id):
        """Fetch a bintool from Google drive

        Args:
            drive_id (str): ID of file to fetch. For a URL of the form
            'https://drive.google.com/file/d/xxx/view?usp=sharing' the value
            passed here should be 'xxx'

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
        """
        url = f'https://drive.google.com/uc?export=download&id={drive_id}'
        return cls.fetch_from_url(url)

    @classmethod
    def apt_install(cls, package):
        """Install a bintool using the 'aot' tool

        This requires use of servo so may request a password

        Args:
            package (str): Name of package to install

        Returns:
            True, assuming it completes without error
        """
        args = ['sudo', 'apt', 'install', '-y', package]
        print('- %s' % ' '.join(args))
        tools.run(*args)
        return True

    @staticmethod
    def WriteDocs(modules, test_missing=None):
        """Write out documentation about the various bintools to stdout

        Args:
            modules: List of modules to include
            test_missing: Used for testing. This is a module to report
                as missing
        """
        print('''.. SPDX-License-Identifier: GPL-2.0+

Binman bintool Documentation
============================

This file describes the bintools (binary tools) supported by binman. Bintools
are binman's name for external executables that it runs to generate or process
binaries. It is fairly easy to create new bintools. Just add a new file to the
'btool' directory. You can use existing bintools as examples.


''')
        modules = sorted(modules)
        missing = []
        for name in modules:
            module = Bintool.find_bintool_class(name)
            docs = getattr(module, '__doc__')
            if test_missing == name:
                docs = None
            if docs:
                lines = docs.splitlines()
                first_line = lines[0]
                rest = [line[4:] for line in lines[1:]]
                hdr = 'Bintool: %s: %s' % (name, first_line)
                print(hdr)
                print('-' * len(hdr))
                print('\n'.join(rest))
                print()
                print()
            else:
                missing.append(name)

        if missing:
            raise ValueError('Documentation is missing for modules: %s' %
                             ', '.join(missing))

    # pylint: disable=W0613
    def fetch(self, method):
        """Fetch handler for a bintool

        This should be implemented by the base class

        Args:
            method (FETCH_...): Method to use

        Returns:
            tuple:
                str: Filename of fetched file to copy to a suitable directory
                str: Name of temp directory to remove, or None
            or True if the file was fetched and already installed
            or None if no fetch() implementation is available

        Raises:
            Valuerror: Fetching could not be completed
        """
        print(f"No method to fetch bintool '{self.name}'")
        return False

    # pylint: disable=R0201
    def version(self):
        """Version handler for a bintool

        This should be implemented by the base class

        Returns:
            str: Version string for this bintool
        """
        return 'unknown'

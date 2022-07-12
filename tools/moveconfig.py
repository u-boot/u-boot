#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+
#
# Author: Masahiro Yamada <yamada.masahiro@socionext.com>
#

"""
Move config options from headers to defconfig files.

See doc/develop/moveconfig.rst for documentation.
"""

from argparse import ArgumentParser
import asteval
import collections
from contextlib import ExitStack
import copy
import difflib
import doctest
import filecmp
import fnmatch
import glob
import multiprocessing
import os
import queue
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest

from buildman import bsettings
from buildman import kconfiglib
from buildman import toolchain

SHOW_GNU_MAKE = 'scripts/show-gnu-make'
SLEEP_TIME=0.03

STATE_IDLE = 0
STATE_DEFCONFIG = 1
STATE_AUTOCONF = 2
STATE_SAVEDEFCONFIG = 3

ACTION_MOVE = 0
ACTION_NO_ENTRY = 1
ACTION_NO_ENTRY_WARN = 2
ACTION_NO_CHANGE = 3

COLOR_BLACK        = '0;30'
COLOR_RED          = '0;31'
COLOR_GREEN        = '0;32'
COLOR_BROWN        = '0;33'
COLOR_BLUE         = '0;34'
COLOR_PURPLE       = '0;35'
COLOR_CYAN         = '0;36'
COLOR_LIGHT_GRAY   = '0;37'
COLOR_DARK_GRAY    = '1;30'
COLOR_LIGHT_RED    = '1;31'
COLOR_LIGHT_GREEN  = '1;32'
COLOR_YELLOW       = '1;33'
COLOR_LIGHT_BLUE   = '1;34'
COLOR_LIGHT_PURPLE = '1;35'
COLOR_LIGHT_CYAN   = '1;36'
COLOR_WHITE        = '1;37'

AUTO_CONF_PATH = 'include/config/auto.conf'
CONFIG_DATABASE = 'moveconfig.db'

CONFIG_LEN = len('CONFIG_')

SIZES = {
    'SZ_1':    0x00000001, 'SZ_2':    0x00000002,
    'SZ_4':    0x00000004, 'SZ_8':    0x00000008,
    'SZ_16':   0x00000010, 'SZ_32':   0x00000020,
    'SZ_64':   0x00000040, 'SZ_128':  0x00000080,
    'SZ_256':  0x00000100, 'SZ_512':  0x00000200,
    'SZ_1K':   0x00000400, 'SZ_2K':   0x00000800,
    'SZ_4K':   0x00001000, 'SZ_8K':   0x00002000,
    'SZ_16K':  0x00004000, 'SZ_32K':  0x00008000,
    'SZ_64K':  0x00010000, 'SZ_128K': 0x00020000,
    'SZ_256K': 0x00040000, 'SZ_512K': 0x00080000,
    'SZ_1M':   0x00100000, 'SZ_2M':   0x00200000,
    'SZ_4M':   0x00400000, 'SZ_8M':   0x00800000,
    'SZ_16M':  0x01000000, 'SZ_32M':  0x02000000,
    'SZ_64M':  0x04000000, 'SZ_128M': 0x08000000,
    'SZ_256M': 0x10000000, 'SZ_512M': 0x20000000,
    'SZ_1G':   0x40000000, 'SZ_2G':   0x80000000,
    'SZ_4G':  0x100000000
}

RE_REMOVE_DEFCONFIG = re.compile(r'(.*)_defconfig')

### helper functions ###
def check_top_directory():
    """Exit if we are not at the top of source directory."""
    for fname in 'README', 'Licenses':
        if not os.path.exists(fname):
            sys.exit('Please run at the top of source directory.')

def check_clean_directory():
    """Exit if the source tree is not clean."""
    for fname in '.config', 'include/config':
        if os.path.exists(fname):
            sys.exit("source tree is not clean, please run 'make mrproper'")

def get_make_cmd():
    """Get the command name of GNU Make.

    U-Boot needs GNU Make for building, but the command name is not
    necessarily "make". (for example, "gmake" on FreeBSD).
    Returns the most appropriate command name on your system.
    """
    with subprocess.Popen([SHOW_GNU_MAKE], stdout=subprocess.PIPE) as proc:
        ret = proc.communicate()
        if proc.returncode:
            sys.exit('GNU Make not found')
    return ret[0].rstrip()

def get_matched_defconfig(line):
    """Get the defconfig files that match a pattern

    Args:
        line (str): Path or filename to match, e.g. 'configs/snow_defconfig' or
            'k2*_defconfig'. If no directory is provided, 'configs/' is
            prepended

    Returns:
        list of str: a list of matching defconfig files
    """
    dirname = os.path.dirname(line)
    if dirname:
        pattern = line
    else:
        pattern = os.path.join('configs', line)
    return glob.glob(pattern) + glob.glob(pattern + '_defconfig')

def get_matched_defconfigs(defconfigs_file):
    """Get all the defconfig files that match the patterns in a file.

    Args:
        defconfigs_file (str): File containing a list of defconfigs to process,
            or '-' to read the list from stdin

    Returns:
        list of str: A list of paths to defconfig files, with no duplicates
    """
    defconfigs = []
    with ExitStack() as stack:
        if defconfigs_file == '-':
            inf = sys.stdin
            defconfigs_file = 'stdin'
        else:
            inf = stack.enter_context(open(defconfigs_file, encoding='utf-8'))
        for i, line in enumerate(inf):
            line = line.strip()
            if not line:
                continue # skip blank lines silently
            if ' ' in line:
                line = line.split(' ')[0]  # handle 'git log' input
            matched = get_matched_defconfig(line)
            if not matched:
                print(f"warning: {defconfigs_file}:{i + 1}: no defconfig matched '{line}'",
                      file=sys.stderr)

            defconfigs += matched

    # use set() to drop multiple matching
    return [defconfig[len('configs') + 1:]  for defconfig in set(defconfigs)]

def get_all_defconfigs():
    """Get all the defconfig files under the configs/ directory.

    Returns:
        list of str: List of paths to defconfig files
    """
    defconfigs = []
    for (dirpath, _, filenames) in os.walk('configs'):
        dirpath = dirpath[len('configs') + 1:]
        for filename in fnmatch.filter(filenames, '*_defconfig'):
            defconfigs.append(os.path.join(dirpath, filename))

    return defconfigs

def color_text(color_enabled, color, string):
    """Return colored string."""
    if color_enabled:
        # LF should not be surrounded by the escape sequence.
        # Otherwise, additional whitespace or line-feed might be printed.
        return '\n'.join([ '\033[' + color + 'm' + s + '\033[0m' if s else ''
                           for s in string.split('\n') ])
    return string

def show_diff(alines, blines, file_path, color_enabled):
    """Show unidified diff.

    Args:
       alines (list of str): A list of lines (before)
       blines (list of str): A list of lines (after)
       file_path (str): Path to the file
       color_enabled (bool): Display the diff in color
    """
    diff = difflib.unified_diff(alines, blines,
                                fromfile=os.path.join('a', file_path),
                                tofile=os.path.join('b', file_path))

    for line in diff:
        if line.startswith('-') and not line.startswith('--'):
            print(color_text(color_enabled, COLOR_RED, line))
        elif line.startswith('+') and not line.startswith('++'):
            print(color_text(color_enabled, COLOR_GREEN, line))
        else:
            print(line)

def extend_matched_lines(lines, matched, pre_patterns, post_patterns,
                         extend_pre, extend_post):
    """Extend matched lines if desired patterns are found before/after already
    matched lines.

    Args:
      lines (list of str): list of lines handled.
      matched (list of int): list of line numbers that have been already
          matched (will be updated by this function)
      pre_patterns (list of re.Pattern): list of regular expression that should
          be matched as preamble
      post_patterns (list of re.Pattern): list of regular expression that should
          be matched as postamble
      extend_pre (bool): Add the line number of matched preamble to the matched
          list
      extend_post (bool): Add the line number of matched postamble to the
          matched list
    """
    extended_matched = []

    j = matched[0]

    for i in matched:
        if i == 0 or i < j:
            continue
        j = i
        while j in matched:
            j += 1
        if j >= len(lines):
            break

        for pat in pre_patterns:
            if pat.search(lines[i - 1]):
                break
        else:
            # not matched
            continue

        for pat in post_patterns:
            if pat.search(lines[j]):
                break
        else:
            # not matched
            continue

        if extend_pre:
            extended_matched.append(i - 1)
        if extend_post:
            extended_matched.append(j)

    matched += extended_matched
    matched.sort()

def confirm(args, prompt):
    """Ask the user to confirm something

    Args:
        args (Namespace ): program arguments

    Returns:
        bool: True to confirm, False to cancel/stop
    """
    if not args.yes:
        while True:
            choice = input(f'{prompt} [y/n]: ')
            choice = choice.lower()
            print(choice)
            if choice in ('y', 'n'):
                break

        if choice == 'n':
            return False

    return True

def write_file(fname, data):
    """Write data to a file

    Args:
        fname (str): Filename to write to
        data (list of str): Lines to write (with or without trailing newline);
            or str to write
    """
    with open(fname, 'w', encoding='utf-8') as out:
        if isinstance(data, list):
            for line in data:
                print(line.rstrip('\n'), file=out)
        else:
            out.write(data)

def read_file(fname, as_lines=True, skip_unicode=False):
    """Read a file and return the contents

    Args:
        fname (str): Filename to read from
        as_lines: Return file contents as a list of lines
        skip_unicode (bool): True to report unicode errors and continue

    Returns:
        iter of str: List of ;ines from the file with newline removed; str if
            as_lines is False with newlines intact; or None if a unicode error
            occurred

    Raises:
        UnicodeDecodeError: Unicode error occurred when reading
    """
    with open(fname, encoding='utf-8') as inf:
        try:
            if as_lines:
                return [line.rstrip('\n') for line in inf.readlines()]
            else:
                return inf.read()
        except UnicodeDecodeError as e:
            if not skip_unicode:
                raise
            print("Failed on file %s': %s" % (fname, e))
            return None

def cleanup_empty_blocks(header_path, args):
    """Clean up empty conditional blocks

    Args:
      header_path (str): path to the cleaned file.
      args (Namespace): program arguments
    """
    pattern = re.compile(r'^\s*#\s*if.*$\n^\s*#\s*endif.*$\n*', flags=re.M)
    data = read_file(header_path, as_lines=False, skip_unicode=True)
    if data is None:
        return

    new_data = pattern.sub('\n', data)

    show_diff(data.splitlines(True), new_data.splitlines(True), header_path,
              args.color)

    if args.dry_run:
        return

    if new_data != data:
        write_file(header_path, new_data)

def cleanup_one_header(header_path, patterns, args):
    """Clean regex-matched lines away from a file.

    Args:
      header_path: path to the cleaned file.
      patterns: list of regex patterns.  Any lines matching to these
                patterns are deleted.
      args (Namespace): program arguments
    """
    lines = read_file(header_path, skip_unicode=True)
    if lines is None:
        return

    matched = []
    for i, line in enumerate(lines):
        if i - 1 in matched and lines[i - 1].endswith('\\'):
            matched.append(i)
            continue
        for pattern in patterns:
            if pattern.search(line):
                matched.append(i)
                break

    if not matched:
        return

    # remove empty #ifdef ... #endif, successive blank lines
    pattern_if = re.compile(r'#\s*if(def|ndef)?\b') #  #if, #ifdef, #ifndef
    pattern_elif = re.compile(r'#\s*el(if|se)\b')   #  #elif, #else
    pattern_endif = re.compile(r'#\s*endif\b')      #  #endif
    pattern_blank = re.compile(r'^\s*$')            #  empty line

    while True:
        old_matched = copy.copy(matched)
        extend_matched_lines(lines, matched, [pattern_if],
                             [pattern_endif], True, True)
        extend_matched_lines(lines, matched, [pattern_elif],
                             [pattern_elif, pattern_endif], True, False)
        extend_matched_lines(lines, matched, [pattern_if, pattern_elif],
                             [pattern_blank], False, True)
        extend_matched_lines(lines, matched, [pattern_blank],
                             [pattern_elif, pattern_endif], True, False)
        extend_matched_lines(lines, matched, [pattern_blank],
                             [pattern_blank], True, False)
        if matched == old_matched:
            break

    tolines = copy.copy(lines)

    for i in reversed(matched):
        tolines.pop(i)

    show_diff(lines, tolines, header_path, args.color)

    if args.dry_run:
        return

    write_file(header_path, tolines)

def cleanup_headers(configs, args):
    """Delete config defines from board headers.

    Args:
      configs: A list of CONFIGs to remove.
      args (Namespace): program arguments
    """
    if not confirm(args, 'Clean up headers?'):
        return

    patterns = []
    for config in configs:
        patterns.append(re.compile(r'#\s*define\s+%s\b' % config))
        patterns.append(re.compile(r'#\s*undef\s+%s\b' % config))

    for dir in 'include', 'arch', 'board':
        for (dirpath, dirnames, filenames) in os.walk(dir):
            if dirpath == os.path.join('include', 'generated'):
                continue
            for filename in filenames:
                if not filename.endswith(('~', '.dts', '.dtsi', '.bin',
                                          '.elf','.aml','.dat')):
                    header_path = os.path.join(dirpath, filename)
                    # This file contains UTF-16 data and no CONFIG symbols
                    if header_path == 'include/video_font_data.h':
                        continue
                    cleanup_one_header(header_path, patterns, args)
                    cleanup_empty_blocks(header_path, args)

def cleanup_whitelist(configs, args):
    """Delete config whitelist entries

    Args:
      configs: A list of CONFIGs to remove.
      args (Namespace): program arguments
    """
    if not confirm(args, 'Clean up whitelist entries?'):
        return

    lines = read_file(os.path.join('scripts', 'config_whitelist.txt'))

    lines = [x for x in lines if x.strip() not in configs]

    write_file(os.path.join('scripts', 'config_whitelist.txt'), lines)

def find_matching(patterns, line):
    for pat in patterns:
        if pat.search(line):
            return True
    return False

def cleanup_readme(configs, args):
    """Delete config description in README

    Args:
      configs: A list of CONFIGs to remove.
      args (Namespace): program arguments
    """
    if not confirm(args, 'Clean up README?'):
        return

    patterns = []
    for config in configs:
        patterns.append(re.compile(r'^\s+%s' % config))

    lines = read_file('README')

    found = False
    newlines = []
    for line in lines:
        if not found:
            found = find_matching(patterns, line)
            if found:
                continue

        if found and re.search(r'^\s+CONFIG', line):
            found = False

        if not found:
            newlines.append(line)

    write_file('README', newlines)

def try_expand(line):
    """If value looks like an expression, try expanding it
    Otherwise just return the existing value
    """
    if line.find('=') == -1:
        return line

    try:
        aeval = asteval.Interpreter( usersyms=SIZES, minimal=True )
        cfg, val = re.split("=", line)
        val= val.strip('\"')
        if re.search(r'[*+-/]|<<|SZ_+|\(([^\)]+)\)', val):
            newval = hex(aeval(val))
            print('\tExpanded expression %s to %s' % (val, newval))
            return cfg+'='+newval
    except:
        print('\tFailed to expand expression in %s' % line)

    return line


### classes ###
class Progress:

    """Progress Indicator"""

    def __init__(self, total):
        """Create a new progress indicator.

        Args:
          total: A number of defconfig files to process.
        """
        self.current = 0
        self.total = total

    def inc(self):
        """Increment the number of processed defconfig files."""

        self.current += 1

    def show(self):
        """Display the progress."""
        print(' %d defconfigs out of %d\r' % (self.current, self.total), end=' ')
        sys.stdout.flush()


class KconfigScanner:
    """Kconfig scanner."""

    def __init__(self):
        """Scan all the Kconfig files and create a Config object."""
        # Define environment variables referenced from Kconfig
        os.environ['srctree'] = os.getcwd()
        os.environ['UBOOTVERSION'] = 'dummy'
        os.environ['KCONFIG_OBJDIR'] = ''
        self.conf = kconfiglib.Kconfig()


class KconfigParser:

    """A parser of .config and include/autoconf.mk."""

    re_arch = re.compile(r'CONFIG_SYS_ARCH="(.*)"')
    re_cpu = re.compile(r'CONFIG_SYS_CPU="(.*)"')

    def __init__(self, configs, args, build_dir):
        """Create a new parser.

        Args:
          configs: A list of CONFIGs to move.
          args (Namespace): program arguments
          build_dir: Build directory.
        """
        self.configs = configs
        self.args = args
        self.dotconfig = os.path.join(build_dir, '.config')
        self.autoconf = os.path.join(build_dir, 'include', 'autoconf.mk')
        self.spl_autoconf = os.path.join(build_dir, 'spl', 'include',
                                         'autoconf.mk')
        self.config_autoconf = os.path.join(build_dir, AUTO_CONF_PATH)
        self.defconfig = os.path.join(build_dir, 'defconfig')

    def get_arch(self):
        """Parse .config file and return the architecture.

        Returns:
          Architecture name (e.g. 'arm').
        """
        arch = ''
        cpu = ''
        for line in read_file(self.dotconfig):
            m = self.re_arch.match(line)
            if m:
                arch = m.group(1)
                continue
            m = self.re_cpu.match(line)
            if m:
                cpu = m.group(1)

        if not arch:
            return None

        # fix-up for aarch64
        if arch == 'arm' and cpu == 'armv8':
            arch = 'aarch64'

        return arch

    def parse_one_config(self, config, dotconfig_lines, autoconf_lines):
        """Parse .config, defconfig, include/autoconf.mk for one config.

        This function looks for the config options in the lines from
        defconfig, .config, and include/autoconf.mk in order to decide
        which action should be taken for this defconfig.

        Args:
          config: CONFIG name to parse.
          dotconfig_lines: lines from the .config file.
          autoconf_lines: lines from the include/autoconf.mk file.

        Returns:
          A tupple of the action for this defconfig and the line
          matched for the config.
        """
        not_set = '# %s is not set' % config

        for line in autoconf_lines:
            line = line.rstrip()
            if line.startswith(config + '='):
                new_val = line
                break
        else:
            new_val = not_set

        new_val = try_expand(new_val)

        for line in dotconfig_lines:
            line = line.rstrip()
            if line.startswith(config + '=') or line == not_set:
                old_val = line
                break
        else:
            if new_val == not_set:
                return (ACTION_NO_ENTRY, config)
            else:
                return (ACTION_NO_ENTRY_WARN, config)

        # If this CONFIG is neither bool nor trisate
        if old_val[-2:] != '=y' and old_val[-2:] != '=m' and old_val != not_set:
            # tools/scripts/define2mk.sed changes '1' to 'y'.
            # This is a problem if the CONFIG is int type.
            # Check the type in Kconfig and handle it correctly.
            if new_val[-2:] == '=y':
                new_val = new_val[:-1] + '1'

        return (ACTION_NO_CHANGE if old_val == new_val else ACTION_MOVE,
                new_val)

    def update_dotconfig(self):
        """Parse files for the config options and update the .config.

        This function parses the generated .config and include/autoconf.mk
        searching the target options.
        Move the config option(s) to the .config as needed.

        Args:
          defconfig: defconfig name.

        Returns:
          Return a tuple of (updated flag, log string).
          The "updated flag" is True if the .config was updated, False
          otherwise.  The "log string" shows what happend to the .config.
        """

        results = []
        updated = False
        suspicious = False
        rm_files = [self.config_autoconf, self.autoconf]

        if self.args.spl:
            if os.path.exists(self.spl_autoconf):
                autoconf_path = self.spl_autoconf
                rm_files.append(self.spl_autoconf)
            else:
                for f in rm_files:
                    os.remove(f)
                return (updated, suspicious,
                        color_text(self.args.color, COLOR_BROWN,
                                   "SPL is not enabled.  Skipped.") + '\n')
        else:
            autoconf_path = self.autoconf

        dotconfig_lines = read_file(self.dotconfig)

        autoconf_lines = read_file(autoconf_path)

        for config in self.configs:
            result = self.parse_one_config(config, dotconfig_lines,
                                           autoconf_lines)
            results.append(result)

        log = ''

        for (action, value) in results:
            if action == ACTION_MOVE:
                actlog = "Move '%s'" % value
                log_color = COLOR_LIGHT_GREEN
            elif action == ACTION_NO_ENTRY:
                actlog = '%s is not defined in Kconfig.  Do nothing.' % value
                log_color = COLOR_LIGHT_BLUE
            elif action == ACTION_NO_ENTRY_WARN:
                actlog = '%s is not defined in Kconfig (suspicious).  Do nothing.' % value
                log_color = COLOR_YELLOW
                suspicious = True
            elif action == ACTION_NO_CHANGE:
                actlog = "'%s' is the same as the define in Kconfig.  Do nothing." \
                         % value
                log_color = COLOR_LIGHT_PURPLE
            else:
                sys.exit('Internal Error. This should not happen.')

            log += color_text(self.args.color, log_color, actlog) + '\n'

        with open(self.dotconfig, 'a', encoding='utf-8') as out:
            for (action, value) in results:
                if action == ACTION_MOVE:
                    out.write(value + '\n')
                    updated = True

        self.results = results
        for f in rm_files:
            os.remove(f)

        return (updated, suspicious, log)

    def check_defconfig(self):
        """Check the defconfig after savedefconfig

        Returns:
          Return additional log if moved CONFIGs were removed again by
          'make savedefconfig'.
        """

        log = ''

        defconfig_lines = read_file(self.defconfig)

        for (action, value) in self.results:
            if action != ACTION_MOVE:
                continue
            if not value in defconfig_lines:
                log += color_text(self.args.color, COLOR_YELLOW,
                                  "'%s' was removed by savedefconfig.\n" %
                                  value)

        return log


class DatabaseThread(threading.Thread):
    """This thread processes results from Slot threads.

    It collects the data in the master config directary. There is only one
    result thread, and this helps to serialise the build output.
    """
    def __init__(self, config_db, db_queue):
        """Set up a new result thread

        Args:
            builder: Builder which will be sent each result
        """
        threading.Thread.__init__(self)
        self.config_db = config_db
        self.db_queue= db_queue

    def run(self):
        """Called to start up the result thread.

        We collect the next result job and pass it on to the build.
        """
        while True:
            defconfig, configs = self.db_queue.get()
            self.config_db[defconfig] = configs
            self.db_queue.task_done()


class Slot:

    """A slot to store a subprocess.

    Each instance of this class handles one subprocess.
    This class is useful to control multiple threads
    for faster processing.
    """

    def __init__(self, toolchains, configs, args, progress, devnull,
		 make_cmd, reference_src_dir, db_queue):
        """Create a new process slot.

        Args:
          toolchains: Toolchains object containing toolchains.
          configs: A list of CONFIGs to move.
          args: Program arguments
          progress: A progress indicator.
          devnull: A file object of '/dev/null'.
          make_cmd: command name of GNU Make.
          reference_src_dir: Determine the true starting config state from this
                             source tree.
          db_queue: output queue to write config info for the database
        """
        self.toolchains = toolchains
        self.args = args
        self.progress = progress
        self.build_dir = tempfile.mkdtemp()
        self.devnull = devnull
        self.make_cmd = (make_cmd, 'O=' + self.build_dir)
        self.reference_src_dir = reference_src_dir
        self.db_queue = db_queue
        self.parser = KconfigParser(configs, args, self.build_dir)
        self.state = STATE_IDLE
        self.failed_boards = set()
        self.suspicious_boards = set()

    def __del__(self):
        """Delete the working directory

        This function makes sure the temporary directory is cleaned away
        even if Python suddenly dies due to error.  It should be done in here
        because it is guaranteed the destructor is always invoked when the
        instance of the class gets unreferenced.

        If the subprocess is still running, wait until it finishes.
        """
        if self.state != STATE_IDLE:
            while self.ps.poll() == None:
                pass
        shutil.rmtree(self.build_dir)

    def add(self, defconfig):
        """Assign a new subprocess for defconfig and add it to the slot.

        If the slot is vacant, create a new subprocess for processing the
        given defconfig and add it to the slot.  Just returns False if
        the slot is occupied (i.e. the current subprocess is still running).

        Args:
          defconfig: defconfig name.

        Returns:
          Return True on success or False on failure
        """
        if self.state != STATE_IDLE:
            return False

        self.defconfig = defconfig
        self.log = ''
        self.current_src_dir = self.reference_src_dir
        self.do_defconfig()
        return True

    def poll(self):
        """Check the status of the subprocess and handle it as needed.

        Returns True if the slot is vacant (i.e. in idle state).
        If the configuration is successfully finished, assign a new
        subprocess to build include/autoconf.mk.
        If include/autoconf.mk is generated, invoke the parser to
        parse the .config and the include/autoconf.mk, moving
        config options to the .config as needed.
        If the .config was updated, run "make savedefconfig" to sync
        it, update the original defconfig, and then set the slot back
        to the idle state.

        Returns:
          Return True if the subprocess is terminated, False otherwise
        """
        if self.state == STATE_IDLE:
            return True

        if self.ps.poll() == None:
            return False

        if self.ps.poll() != 0:
            self.handle_error()
        elif self.state == STATE_DEFCONFIG:
            if self.reference_src_dir and not self.current_src_dir:
                self.do_savedefconfig()
            else:
                self.do_autoconf()
        elif self.state == STATE_AUTOCONF:
            if self.current_src_dir:
                self.current_src_dir = None
                self.do_defconfig()
            elif self.args.build_db:
                self.do_build_db()
            else:
                self.do_savedefconfig()
        elif self.state == STATE_SAVEDEFCONFIG:
            self.update_defconfig()
        else:
            sys.exit('Internal Error. This should not happen.')

        return True if self.state == STATE_IDLE else False

    def handle_error(self):
        """Handle error cases."""

        self.log += color_text(self.args.color, COLOR_LIGHT_RED,
                               'Failed to process.\n')
        if self.args.verbose:
            self.log += color_text(self.args.color, COLOR_LIGHT_CYAN,
                                   self.ps.stderr.read().decode())
        self.finish(False)

    def do_defconfig(self):
        """Run 'make <board>_defconfig' to create the .config file."""

        cmd = list(self.make_cmd)
        cmd.append(self.defconfig)
        self.ps = subprocess.Popen(cmd, stdout=self.devnull,
                                   stderr=subprocess.PIPE,
                                   cwd=self.current_src_dir)
        self.state = STATE_DEFCONFIG

    def do_autoconf(self):
        """Run 'make AUTO_CONF_PATH'."""

        arch = self.parser.get_arch()
        try:
            toolchain = self.toolchains.Select(arch)
        except ValueError:
            self.log += color_text(self.args.color, COLOR_YELLOW,
                    "Tool chain for '%s' is missing.  Do nothing.\n" % arch)
            self.finish(False)
            return
        env = toolchain.MakeEnvironment(False)

        cmd = list(self.make_cmd)
        cmd.append('KCONFIG_IGNORE_DUPLICATES=1')
        cmd.append(AUTO_CONF_PATH)
        self.ps = subprocess.Popen(cmd, stdout=self.devnull, env=env,
                                   stderr=subprocess.PIPE,
                                   cwd=self.current_src_dir)
        self.state = STATE_AUTOCONF

    def do_build_db(self):
        """Add the board to the database"""
        configs = {}
        for line in read_file(os.path.join(self.build_dir, AUTO_CONF_PATH)):
            if line.startswith('CONFIG'):
                config, value = line.split('=', 1)
                configs[config] = value.rstrip()
        self.db_queue.put([self.defconfig, configs])
        self.finish(True)

    def do_savedefconfig(self):
        """Update the .config and run 'make savedefconfig'."""

        (updated, suspicious, log) = self.parser.update_dotconfig()
        if suspicious:
            self.suspicious_boards.add(self.defconfig)
        self.log += log

        if not self.args.force_sync and not updated:
            self.finish(True)
            return
        if updated:
            self.log += color_text(self.args.color, COLOR_LIGHT_GREEN,
                                   'Syncing by savedefconfig...\n')
        else:
            self.log += 'Syncing by savedefconfig (forced by option)...\n'

        cmd = list(self.make_cmd)
        cmd.append('savedefconfig')
        self.ps = subprocess.Popen(cmd, stdout=self.devnull,
                                   stderr=subprocess.PIPE)
        self.state = STATE_SAVEDEFCONFIG

    def update_defconfig(self):
        """Update the input defconfig and go back to the idle state."""

        log = self.parser.check_defconfig()
        if log:
            self.suspicious_boards.add(self.defconfig)
            self.log += log
        orig_defconfig = os.path.join('configs', self.defconfig)
        new_defconfig = os.path.join(self.build_dir, 'defconfig')
        updated = not filecmp.cmp(orig_defconfig, new_defconfig)

        if updated:
            self.log += color_text(self.args.color, COLOR_LIGHT_BLUE,
                                   'defconfig was updated.\n')

        if not self.args.dry_run and updated:
            shutil.move(new_defconfig, orig_defconfig)
        self.finish(True)

    def finish(self, success):
        """Display log along with progress and go to the idle state.

        Args:
          success: Should be True when the defconfig was processed
                   successfully, or False when it fails.
        """
        # output at least 30 characters to hide the "* defconfigs out of *".
        log = self.defconfig.ljust(30) + '\n'

        log += '\n'.join([ '    ' + s for s in self.log.split('\n') ])
        # Some threads are running in parallel.
        # Print log atomically to not mix up logs from different threads.
        print(log, file=(sys.stdout if success else sys.stderr))

        if not success:
            if self.args.exit_on_error:
                sys.exit('Exit on error.')
            # If --exit-on-error flag is not set, skip this board and continue.
            # Record the failed board.
            self.failed_boards.add(self.defconfig)

        self.progress.inc()
        self.progress.show()
        self.state = STATE_IDLE

    def get_failed_boards(self):
        """Returns a set of failed boards (defconfigs) in this slot.
        """
        return self.failed_boards

    def get_suspicious_boards(self):
        """Returns a set of boards (defconfigs) with possible misconversion.
        """
        return self.suspicious_boards - self.failed_boards

class Slots:

    """Controller of the array of subprocess slots."""

    def __init__(self, toolchains, configs, args, progress,
		 reference_src_dir, db_queue):
        """Create a new slots controller.

        Args:
          toolchains: Toolchains object containing toolchains.
          configs: A list of CONFIGs to move.
          args: Program arguments
          progress: A progress indicator.
          reference_src_dir: Determine the true starting config state from this
                             source tree.
          db_queue: output queue to write config info for the database
        """
        self.args = args
        self.slots = []
        devnull = subprocess.DEVNULL
        make_cmd = get_make_cmd()
        for i in range(args.jobs):
            self.slots.append(Slot(toolchains, configs, args, progress,
				   devnull, make_cmd, reference_src_dir,
				   db_queue))

    def add(self, defconfig):
        """Add a new subprocess if a vacant slot is found.

        Args:
          defconfig: defconfig name to be put into.

        Returns:
          Return True on success or False on failure
        """
        for slot in self.slots:
            if slot.add(defconfig):
                return True
        return False

    def available(self):
        """Check if there is a vacant slot.

        Returns:
          Return True if at lease one vacant slot is found, False otherwise.
        """
        for slot in self.slots:
            if slot.poll():
                return True
        return False

    def empty(self):
        """Check if all slots are vacant.

        Returns:
          Return True if all the slots are vacant, False otherwise.
        """
        ret = True
        for slot in self.slots:
            if not slot.poll():
                ret = False
        return ret

    def show_failed_boards(self):
        """Display all of the failed boards (defconfigs)."""
        boards = set()
        output_file = 'moveconfig.failed'

        for slot in self.slots:
            boards |= slot.get_failed_boards()

        if boards:
            boards = '\n'.join(boards) + '\n'
            msg = 'The following boards were not processed due to error:\n'
            msg += boards
            msg += '(the list has been saved in %s)\n' % output_file
            print(color_text(self.args.color, COLOR_LIGHT_RED,
                                            msg), file=sys.stderr)

            write_file(output_file, boards)

    def show_suspicious_boards(self):
        """Display all boards (defconfigs) with possible misconversion."""
        boards = set()
        output_file = 'moveconfig.suspicious'

        for slot in self.slots:
            boards |= slot.get_suspicious_boards()

        if boards:
            boards = '\n'.join(boards) + '\n'
            msg = 'The following boards might have been converted incorrectly.\n'
            msg += 'It is highly recommended to check them manually:\n'
            msg += boards
            msg += '(the list has been saved in %s)\n' % output_file
            print(color_text(self.args.color, COLOR_YELLOW,
                                            msg), file=sys.stderr)

            write_file(output_file, boards)

class ReferenceSource:

    """Reference source against which original configs should be parsed."""

    def __init__(self, commit):
        """Create a reference source directory based on a specified commit.

        Args:
          commit: commit to git-clone
        """
        self.src_dir = tempfile.mkdtemp()
        print('Cloning git repo to a separate work directory...')
        subprocess.check_output(['git', 'clone', os.getcwd(), '.'],
                                cwd=self.src_dir)
        print("Checkout '%s' to build the original autoconf.mk." % \
            subprocess.check_output(['git', 'rev-parse', '--short', commit]).strip())
        subprocess.check_output(['git', 'checkout', commit],
                                stderr=subprocess.STDOUT, cwd=self.src_dir)

    def __del__(self):
        """Delete the reference source directory

        This function makes sure the temporary directory is cleaned away
        even if Python suddenly dies due to error.  It should be done in here
        because it is guaranteed the destructor is always invoked when the
        instance of the class gets unreferenced.
        """
        shutil.rmtree(self.src_dir)

    def get_dir(self):
        """Return the absolute path to the reference source directory."""

        return self.src_dir

def move_config(toolchains, configs, args, db_queue):
    """Move config options to defconfig files.

    Args:
      configs: A list of CONFIGs to move.
      args: Program arguments
    """
    if len(configs) == 0:
        if args.force_sync:
            print('No CONFIG is specified. You are probably syncing defconfigs.', end=' ')
        elif args.build_db:
            print('Building %s database' % CONFIG_DATABASE)
        else:
            print('Neither CONFIG nor --force-sync is specified. Nothing will happen.', end=' ')
    else:
        print('Move ' + ', '.join(configs), end=' ')
    print('(jobs: %d)\n' % args.jobs)

    if args.git_ref:
        reference_src = ReferenceSource(args.git_ref)
        reference_src_dir = reference_src.get_dir()
    else:
        reference_src_dir = None

    if args.defconfigs:
        defconfigs = get_matched_defconfigs(args.defconfigs)
    else:
        defconfigs = get_all_defconfigs()

    progress = Progress(len(defconfigs))
    slots = Slots(toolchains, configs, args, progress, reference_src_dir,
                  db_queue)

    # Main loop to process defconfig files:
    #  Add a new subprocess into a vacant slot.
    #  Sleep if there is no available slot.
    for defconfig in defconfigs:
        while not slots.add(defconfig):
            while not slots.available():
                # No available slot: sleep for a while
                time.sleep(SLEEP_TIME)

    # wait until all the subprocesses finish
    while not slots.empty():
        time.sleep(SLEEP_TIME)

    print('')
    slots.show_failed_boards()
    slots.show_suspicious_boards()

def find_kconfig_rules(kconf, config, imply_config):
    """Check whether a config has a 'select' or 'imply' keyword

    Args:
        kconf: Kconfiglib.Kconfig object
        config: Name of config to check (without CONFIG_ prefix)
        imply_config: Implying config (without CONFIG_ prefix) which may or
            may not have an 'imply' for 'config')

    Returns:
        Symbol object for 'config' if found, else None
    """
    sym = kconf.syms.get(imply_config)
    if sym:
        for sel, cond in (sym.selects + sym.implies):
            if sel.name == config:
                return sym
    return None

def check_imply_rule(kconf, config, imply_config):
    """Check if we can add an 'imply' option

    This finds imply_config in the Kconfig and looks to see if it is possible
    to add an 'imply' for 'config' to that part of the Kconfig.

    Args:
        kconf: Kconfiglib.Kconfig object
        config: Name of config to check (without CONFIG_ prefix)
        imply_config: Implying config (without CONFIG_ prefix) which may or
            may not have an 'imply' for 'config')

    Returns:
        tuple:
            filename of Kconfig file containing imply_config, or None if none
            line number within the Kconfig file, or 0 if none
            message indicating the result
    """
    sym = kconf.syms.get(imply_config)
    if not sym:
        return 'cannot find sym'
    nodes = sym.nodes
    if len(nodes) != 1:
        return '%d locations' % len(nodes)
    node = nodes[0]
    fname, linenum = node.filename, node.linenr
    cwd = os.getcwd()
    if cwd and fname.startswith(cwd):
        fname = fname[len(cwd) + 1:]
    file_line = ' at %s:%d' % (fname, linenum)
    data = read_file(fname)
    if data[linenum - 1] != 'config %s' % imply_config:
        return None, 0, 'bad sym format %s%s' % (data[linenum], file_line)
    return fname, linenum, 'adding%s' % file_line

def add_imply_rule(config, fname, linenum):
    """Add a new 'imply' option to a Kconfig

    Args:
        config: config option to add an imply for (without CONFIG_ prefix)
        fname: Kconfig filename to update
        linenum: Line number to place the 'imply' before

    Returns:
        Message indicating the result
    """
    file_line = ' at %s:%d' % (fname, linenum)
    data = read_file(fname)
    linenum -= 1

    for offset, line in enumerate(data[linenum:]):
        if line.strip().startswith('help') or not line:
            data.insert(linenum + offset, '\timply %s' % config)
            write_file(fname, data)
            return 'added%s' % file_line

    return 'could not insert%s'

(IMPLY_MIN_2, IMPLY_TARGET, IMPLY_CMD, IMPLY_NON_ARCH_BOARD) = (
    1, 2, 4, 8)

IMPLY_FLAGS = {
    'min2': [IMPLY_MIN_2, 'Show options which imply >2 boards (normally >5)'],
    'target': [IMPLY_TARGET, 'Allow CONFIG_TARGET_... options to imply'],
    'cmd': [IMPLY_CMD, 'Allow CONFIG_CMD_... to imply'],
    'non-arch-board': [
        IMPLY_NON_ARCH_BOARD,
        'Allow Kconfig options outside arch/ and /board/ to imply'],
}


def read_database():
    """Read in the config database

    Returns:
        tuple:
            set of all config options seen (each a str)
            set of all defconfigs seen (each a str)
            dict of configs for each defconfig:
                key: defconfig name, e.g. "MPC8548CDS_legacy_defconfig"
                value: dict:
                    key: CONFIG option
                    value: Value of option
            dict of defconfigs for each config:
                key: CONFIG option
                value: set of boards using that option

    """
    configs = {}

    # key is defconfig name, value is dict of (CONFIG_xxx, value)
    config_db = {}

    # Set of all config options we have seen
    all_configs = set()

    # Set of all defconfigs we have seen
    all_defconfigs = set()

    defconfig_db = collections.defaultdict(set)
    for line in read_file(CONFIG_DATABASE):
        line = line.rstrip()
        if not line:  # Separator between defconfigs
            config_db[defconfig] = configs
            all_defconfigs.add(defconfig)
            configs = {}
        elif line[0] == ' ':  # CONFIG line
            config, value = line.strip().split('=', 1)
            configs[config] = value
            defconfig_db[config].add(defconfig)
            all_configs.add(config)
        else:  # New defconfig
            defconfig = line

    return all_configs, all_defconfigs, config_db, defconfig_db


def do_imply_config(config_list, add_imply, imply_flags, skip_added,
                    check_kconfig=True, find_superset=False):
    """Find CONFIG options which imply those in the list

    Some CONFIG options can be implied by others and this can help to reduce
    the size of the defconfig files. For example, CONFIG_X86 implies
    CONFIG_CMD_IRQ, so we can put 'imply CMD_IRQ' under 'config X86' and
    all x86 boards will have that option, avoiding adding CONFIG_CMD_IRQ to
    each of the x86 defconfig files.

    This function uses the moveconfig database to find such options. It
    displays a list of things that could possibly imply those in the list.
    The algorithm ignores any that start with CONFIG_TARGET since these
    typically refer to only a few defconfigs (often one). It also does not
    display a config with less than 5 defconfigs.

    The algorithm works using sets. For each target config in config_list:
        - Get the set 'defconfigs' which use that target config
        - For each config (from a list of all configs):
            - Get the set 'imply_defconfig' of defconfigs which use that config
            -
            - If imply_defconfigs contains anything not in defconfigs then
              this config does not imply the target config

    Params:
        config_list: List of CONFIG options to check (each a string)
        add_imply: Automatically add an 'imply' for each config.
        imply_flags: Flags which control which implying configs are allowed
           (IMPLY_...)
        skip_added: Don't show options which already have an imply added.
        check_kconfig: Check if implied symbols already have an 'imply' or
            'select' for the target config, and show this information if so.
        find_superset: True to look for configs which are a superset of those
            already found. So for example if CONFIG_EXYNOS5 implies an option,
            but CONFIG_EXYNOS covers a larger set of defconfigs and also
            implies that option, this will drop the former in favour of the
            latter. In practice this option has not proved very used.

    Note the terminoloy:
        config - a CONFIG_XXX options (a string, e.g. 'CONFIG_CMD_EEPROM')
        defconfig - a defconfig file (a string, e.g. 'configs/snow_defconfig')
    """
    kconf = KconfigScanner().conf if check_kconfig else None
    if add_imply and add_imply != 'all':
        add_imply = add_imply.split(',')

    all_configs, all_defconfigs, config_db, defconfig_db = read_database()

    # Work through each target config option in turn, independently
    for config in config_list:
        defconfigs = defconfig_db.get(config)
        if not defconfigs:
            print('%s not found in any defconfig' % config)
            continue

        # Get the set of defconfigs without this one (since a config cannot
        # imply itself)
        non_defconfigs = all_defconfigs - defconfigs
        num_defconfigs = len(defconfigs)
        print('%s found in %d/%d defconfigs' % (config, num_defconfigs,
                                                len(all_configs)))

        # This will hold the results: key=config, value=defconfigs containing it
        imply_configs = {}
        rest_configs = all_configs - set([config])

        # Look at every possible config, except the target one
        for imply_config in rest_configs:
            if 'ERRATUM' in imply_config:
                continue
            if not imply_flags & IMPLY_CMD:
                if 'CONFIG_CMD' in imply_config:
                    continue
            if not imply_flags & IMPLY_TARGET:
                if 'CONFIG_TARGET' in imply_config:
                    continue

            # Find set of defconfigs that have this config
            imply_defconfig = defconfig_db[imply_config]

            # Get the intersection of this with defconfigs containing the
            # target config
            common_defconfigs = imply_defconfig & defconfigs

            # Get the set of defconfigs containing this config which DO NOT
            # also contain the taret config. If this set is non-empty it means
            # that this config affects other defconfigs as well as (possibly)
            # the ones affected by the target config. This means it implies
            # things we don't want to imply.
            not_common_defconfigs = imply_defconfig & non_defconfigs
            if not_common_defconfigs:
                continue

            # If there are common defconfigs, imply_config may be useful
            if common_defconfigs:
                skip = False
                if find_superset:
                    for prev in list(imply_configs.keys()):
                        prev_count = len(imply_configs[prev])
                        count = len(common_defconfigs)
                        if (prev_count > count and
                            (imply_configs[prev] & common_defconfigs ==
                            common_defconfigs)):
                            # skip imply_config because prev is a superset
                            skip = True
                            break
                        elif count > prev_count:
                            # delete prev because imply_config is a superset
                            del imply_configs[prev]
                if not skip:
                    imply_configs[imply_config] = common_defconfigs

        # Now we have a dict imply_configs of configs which imply each config
        # The value of each dict item is the set of defconfigs containing that
        # config. Rank them so that we print the configs that imply the largest
        # number of defconfigs first.
        ranked_iconfigs = sorted(imply_configs,
                            key=lambda k: len(imply_configs[k]), reverse=True)
        kconfig_info = ''
        cwd = os.getcwd()
        add_list = collections.defaultdict(list)
        for iconfig in ranked_iconfigs:
            num_common = len(imply_configs[iconfig])

            # Don't bother if there are less than 5 defconfigs affected.
            if num_common < (2 if imply_flags & IMPLY_MIN_2 else 5):
                continue
            missing = defconfigs - imply_configs[iconfig]
            missing_str = ', '.join(missing) if missing else 'all'
            missing_str = ''
            show = True
            if kconf:
                sym = find_kconfig_rules(kconf, config[CONFIG_LEN:],
                                         iconfig[CONFIG_LEN:])
                kconfig_info = ''
                if sym:
                    nodes = sym.nodes
                    if len(nodes) == 1:
                        fname, linenum = nodes[0].filename, nodes[0].linenr
                        if cwd and fname.startswith(cwd):
                            fname = fname[len(cwd) + 1:]
                        kconfig_info = '%s:%d' % (fname, linenum)
                        if skip_added:
                            show = False
                else:
                    sym = kconf.syms.get(iconfig[CONFIG_LEN:])
                    fname = ''
                    if sym:
                        nodes = sym.nodes
                        if len(nodes) == 1:
                            fname, linenum = nodes[0].filename, nodes[0].linenr
                            if cwd and fname.startswith(cwd):
                                fname = fname[len(cwd) + 1:]
                    in_arch_board = not sym or (fname.startswith('arch') or
                                                fname.startswith('board'))
                    if (not in_arch_board and
                        not imply_flags & IMPLY_NON_ARCH_BOARD):
                        continue

                    if add_imply and (add_imply == 'all' or
                                      iconfig in add_imply):
                        fname, linenum, kconfig_info = (check_imply_rule(kconf,
                                config[CONFIG_LEN:], iconfig[CONFIG_LEN:]))
                        if fname:
                            add_list[fname].append(linenum)

            if show and kconfig_info != 'skip':
                print('%5d : %-30s%-25s %s' % (num_common, iconfig.ljust(30),
                                              kconfig_info, missing_str))

        # Having collected a list of things to add, now we add them. We process
        # each file from the largest line number to the smallest so that
        # earlier additions do not affect our line numbers. E.g. if we added an
        # imply at line 20 it would change the position of each line after
        # that.
        for fname, linenums in add_list.items():
            for linenum in sorted(linenums, reverse=True):
                add_imply_rule(config[CONFIG_LEN:], fname, linenum)

def defconfig_matches(configs, re_match):
    """Check if any CONFIG option matches a regex

    The match must be complete, i.e. from the start to end of the CONFIG option.

    Args:
        configs (dict): Dict of CONFIG options:
            key: CONFIG option
            value: Value of option
        re_match (re.Pattern): Match to check

    Returns:
        bool: True if any CONFIG matches the regex
    """
    for cfg in configs:
        if re_match.fullmatch(cfg):
            return True
    return False

def do_find_config(config_list):
    """Find boards with a given combination of CONFIGs

    Params:
        config_list: List of CONFIG options to check (each a regex consisting
            of a config option, with or without a CONFIG_ prefix. If an option
            is preceded by a tilde (~) then it must be false, otherwise it must
            be true)
    """
    all_configs, all_defconfigs, config_db, defconfig_db = read_database()

    # Get the whitelist
    adhoc_configs = set(read_file('scripts/config_whitelist.txt'))

    # Start with all defconfigs
    out = all_defconfigs

    # Work through each config in turn
    adhoc = []
    for item in config_list:
        # Get the real config name and whether we want this config or not
        cfg = item
        want = True
        if cfg[0] == '~':
            want = False
            cfg = cfg[1:]

        if cfg in adhoc_configs:
            adhoc.append(cfg)
            continue

        # Search everything that is still in the running. If it has a config
        # that we want, or doesn't have one that we don't, add it into the
        # running for the next stage
        in_list = out
        out = set()
        re_match = re.compile(cfg)
        for defc in in_list:
            has_cfg = defconfig_matches(config_db[defc], re_match)
            if has_cfg == want:
                out.add(defc)
    if adhoc:
        print(f"Error: Not in Kconfig: %s" % ' '.join(adhoc))
    else:
        print(f'{len(out)} matches')
        print(' '.join(item.split('_defconfig')[0] for item in out))


def prefix_config(cfg):
    """Prefix a config with CONFIG_ if needed

    This handles ~ operator, which indicates that the CONFIG should be disabled

    >>> prefix_config('FRED')
    'CONFIG_FRED'
    >>> prefix_config('CONFIG_FRED')
    'CONFIG_FRED'
    >>> prefix_config('~FRED')
    '~CONFIG_FRED'
    >>> prefix_config('~CONFIG_FRED')
    '~CONFIG_FRED'
    >>> prefix_config('A123')
    'CONFIG_A123'
    """
    op = ''
    if cfg[0] == '~':
        op = cfg[0]
        cfg = cfg[1:]
    if not cfg.startswith('CONFIG_'):
        cfg = 'CONFIG_' + cfg
    return op + cfg


def main():
    try:
        cpu_count = multiprocessing.cpu_count()
    except NotImplementedError:
        cpu_count = 1

    epilog = '''Move config options from headers to defconfig files. See
doc/develop/moveconfig.rst for documentation.'''

    parser = ArgumentParser(epilog=epilog)
    # Add arguments here
    parser.add_argument('-a', '--add-imply', type=str, default='',
                      help='comma-separated list of CONFIG options to add '
                      "an 'imply' statement to for the CONFIG in -i")
    parser.add_argument('-A', '--skip-added', action='store_true', default=False,
                      help="don't show options which are already marked as "
                      'implying others')
    parser.add_argument('-b', '--build-db', action='store_true', default=False,
                      help='build a CONFIG database')
    parser.add_argument('-c', '--color', action='store_true', default=False,
                      help='display the log in color')
    parser.add_argument('-C', '--commit', action='store_true', default=False,
                      help='Create a git commit for the operation')
    parser.add_argument('-d', '--defconfigs', type=str,
                      help='a file containing a list of defconfigs to move, '
                      "one per line (for example 'snow_defconfig') "
                      "or '-' to read from stdin")
    parser.add_argument('-e', '--exit-on-error', action='store_true',
                      default=False,
                      help='exit immediately on any error')
    parser.add_argument('-f', '--find', action='store_true', default=False,
                      help='Find boards with a given config combination')
    parser.add_argument('-H', '--headers-only', dest='cleanup_headers_only',
                      action='store_true', default=False,
                      help='only cleanup the headers')
    parser.add_argument('-i', '--imply', action='store_true', default=False,
                      help='find options which imply others')
    parser.add_argument('-I', '--imply-flags', type=str, default='',
                      help="control the -i option ('help' for help")
    parser.add_argument('-j', '--jobs', type=int, default=cpu_count,
                      help='the number of jobs to run simultaneously')
    parser.add_argument('-n', '--dry-run', action='store_true', default=False,
                      help='perform a trial run (show log with no changes)')
    parser.add_argument('-r', '--git-ref', type=str,
                      help='the git ref to clone for building the autoconf.mk')
    parser.add_argument('-s', '--force-sync', action='store_true', default=False,
                      help='force sync by savedefconfig')
    parser.add_argument('-S', '--spl', action='store_true', default=False,
                      help='parse config options defined for SPL build')
    parser.add_argument('-t', '--test', action='store_true', default=False,
                      help='run unit tests')
    parser.add_argument('-y', '--yes', action='store_true', default=False,
                      help="respond 'yes' to any prompts")
    parser.add_argument('-v', '--verbose', action='store_true', default=False,
                      help='show any build errors as boards are built')
    parser.add_argument('configs', nargs='*')

    args = parser.parse_args()
    configs = args.configs

    if args.test:
        sys.argv = [sys.argv[0]]
        fail, count = doctest.testmod()
        if fail:
            return 1
        unittest.main()

    if not any((len(configs), args.force_sync, args.build_db, args.imply,
                args.find)):
        parser.print_usage()
        sys.exit(1)

    # prefix the option name with CONFIG_ if missing
    configs = [prefix_config(cfg) for cfg in configs]

    check_top_directory()

    if args.imply:
        imply_flags = 0
        if args.imply_flags == 'all':
            imply_flags = -1

        elif args.imply_flags:
            for flag in args.imply_flags.split(','):
                bad = flag not in IMPLY_FLAGS
                if bad:
                    print("Invalid flag '%s'" % flag)
                if flag == 'help' or bad:
                    print("Imply flags: (separate with ',')")
                    for name, info in IMPLY_FLAGS.items():
                        print(' %-15s: %s' % (name, info[1]))
                    parser.print_usage()
                    sys.exit(1)
                imply_flags |= IMPLY_FLAGS[flag][0]

        do_imply_config(configs, args.add_imply, imply_flags, args.skip_added)
        return

    if args.find:
        do_find_config(configs)
        return

    config_db = {}
    db_queue = queue.Queue()
    t = DatabaseThread(config_db, db_queue)
    t.setDaemon(True)
    t.start()

    if not args.cleanup_headers_only:
        check_clean_directory()
        bsettings.Setup('')
        toolchains = toolchain.Toolchains()
        toolchains.GetSettings()
        toolchains.Scan(verbose=False)
        move_config(toolchains, configs, args, db_queue)
        db_queue.join()

    if configs:
        cleanup_headers(configs, args)
        cleanup_whitelist(configs, args)
        cleanup_readme(configs, args)

    if args.commit:
        subprocess.call(['git', 'add', '-u'])
        if configs:
            msg = 'Convert %s %sto Kconfig' % (configs[0],
                    'et al ' if len(configs) > 1 else '')
            msg += ('\n\nThis converts the following to Kconfig:\n   %s\n' %
                    '\n   '.join(configs))
        else:
            msg = 'configs: Resync with savedefconfig'
            msg += '\n\nRsync all defconfig files using moveconfig.py'
        subprocess.call(['git', 'commit', '-s', '-m', msg])

    if args.build_db:
        with open(CONFIG_DATABASE, 'w', encoding='utf-8') as fd:
            for defconfig, configs in config_db.items():
                fd.write('%s\n' % defconfig)
                for config in sorted(configs.keys()):
                    fd.write('   %s=%s\n' % (config, configs[config]))
                fd.write('\n')

if __name__ == '__main__':
    sys.exit(main())

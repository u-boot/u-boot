# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Utility code shared across multiple tests.

import hashlib
import os
import os.path
import pytest
import sys
import time
import pytest

def md5sum_data(data):
    """Calculate the MD5 hash of some data.

    Args:
        data: The data to hash.

    Returns:
        The hash of the data, as a binary string.
    """

    h = hashlib.md5()
    h.update(data)
    return h.digest()

def md5sum_file(fn, max_length=None):
    """Calculate the MD5 hash of the contents of a file.

    Args:
        fn: The filename of the file to hash.
        max_length: The number of bytes to hash. If the file has more
            bytes than this, they will be ignored. If None or omitted, the
            entire file will be hashed.

    Returns:
        The hash of the file content, as a binary string.
    """

    with open(fn, 'rb') as fh:
        if max_length:
            params = [max_length]
        else:
            params = []
        data = fh.read(*params)
    return md5sum_data(data)

class PersistentRandomFile(object):
    """Generate and store information about a persistent file containing
    random data."""

    def __init__(self, u_boot_console, fn, size):
        """Create or process the persistent file.

        If the file does not exist, it is generated.

        If the file does exist, its content is hashed for later comparison.

        These files are always located in the "persistent data directory" of
        the current test run.

        Args:
            u_boot_console: A console connection to U-Boot.
            fn: The filename (without path) to create.
            size: The desired size of the file in bytes.

        Returns:
            Nothing.
        """

        self.fn = fn

        self.abs_fn = u_boot_console.config.persistent_data_dir + '/' + fn

        if os.path.exists(self.abs_fn):
            u_boot_console.log.action('Persistent data file ' + self.abs_fn +
                ' already exists')
            self.content_hash = md5sum_file(self.abs_fn)
        else:
            u_boot_console.log.action('Generating ' + self.abs_fn +
                ' (random, persistent, %d bytes)' % size)
            data = os.urandom(size)
            with open(self.abs_fn, 'wb') as fh:
                fh.write(data)
            self.content_hash = md5sum_data(data)

def attempt_to_open_file(fn):
    """Attempt to open a file, without throwing exceptions.

    Any errors (exceptions) that occur during the attempt to open the file
    are ignored. This is useful in order to test whether a file (in
    particular, a device node) exists and can be successfully opened, in order
    to poll for e.g. USB enumeration completion.

    Args:
        fn: The filename to attempt to open.

    Returns:
        An open file handle to the file, or None if the file could not be
            opened.
    """

    try:
        return open(fn, 'rb')
    except:
        return None

def wait_until_open_succeeds(fn):
    """Poll until a file can be opened, or a timeout occurs.

    Continually attempt to open a file, and return when this succeeds, or
    raise an exception after a timeout.

    Args:
        fn: The filename to attempt to open.

    Returns:
        An open file handle to the file.
    """

    for i in xrange(100):
        fh = attempt_to_open_file(fn)
        if fh:
            return fh
        time.sleep(0.1)
    raise Exception('File could not be opened')

def wait_until_file_open_fails(fn, ignore_errors):
    """Poll until a file cannot be opened, or a timeout occurs.

    Continually attempt to open a file, and return when this fails, or
    raise an exception after a timeout.

    Args:
        fn: The filename to attempt to open.
        ignore_errors: Indicate whether to ignore timeout errors. If True, the
            function will simply return if a timeout occurs, otherwise an
            exception will be raised.

    Returns:
        Nothing.
    """

    for i in xrange(100):
        fh = attempt_to_open_file(fn)
        if not fh:
            return
        fh.close()
        time.sleep(0.1)
    if ignore_errors:
        return
    raise Exception('File can still be opened')

def run_and_log(u_boot_console, cmd, ignore_errors=False):
    """Run a command and log its output.

    Args:
        u_boot_console: A console connection to U-Boot.
        cmd: The command to run, as an array of argv[].
        ignore_errors: Indicate whether to ignore errors. If True, the function
            will simply return if the command cannot be executed or exits with
            an error code, otherwise an exception will be raised if such
            problems occur.

    Returns:
        Nothing.
    """

    runner = u_boot_console.log.get_runner(cmd[0], sys.stdout)
    runner.run(cmd, ignore_errors=ignore_errors)
    runner.close()

ram_base = None
def find_ram_base(u_boot_console):
    """Find the running U-Boot's RAM location.

    Probe the running U-Boot to determine the address of the first bank
    of RAM. This is useful for tests that test reading/writing RAM, or
    load/save files that aren't associated with some standard address
    typically represented in an environment variable such as
    ${kernel_addr_r}. The value is cached so that it only needs to be
    actively read once.

    Args:
        u_boot_console: A console connection to U-Boot.

    Returns:
        The address of U-Boot's first RAM bank, as an integer.
    """

    global ram_base
    if u_boot_console.config.buildconfig.get('config_cmd_bdi', 'n') != 'y':
        pytest.skip('bdinfo command not supported')
    if ram_base == -1:
        pytest.skip('Previously failed to find RAM bank start')
    if ram_base is not None:
        return ram_base

    with u_boot_console.log.section('find_ram_base'):
        response = u_boot_console.run_command('bdinfo')
        for l in response.split('\n'):
            if '-> start' in l or 'memstart    =' in l:
                ram_base = int(l.split('=')[1].strip(), 16)
                break
        if ram_base is None:
            ram_base = -1
            raise Exception('Failed to find RAM bank start in `bdinfo`')

    return ram_base

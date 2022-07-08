#!/usr/bin/env python
# SPDX-License-Identifier: GPL-2.0+
#
# Modified by: Corey Goldberg, 2013
#
# Original code from:
#   Bazaar (bzrlib.tests.__init__.py, v2.6, copied Jun 01 2013)
#   Copyright (C) 2005-2011 Canonical Ltd

"""Python testtools extension for running unittest suites concurrently.

The `testtools` project provides a ConcurrentTestSuite class, but does
not provide a `make_tests` implementation needed to use it.

This allows you to parallelize a test run across a configurable number
of worker processes. While this can speed up CPU-bound test runs, it is
mainly useful for IO-bound tests that spend most of their time waiting for
data to arrive from someplace else and can benefit from cocncurrency.

Unix only.
"""

import os
import sys
import traceback
import unittest
from itertools import cycle
from multiprocessing import cpu_count

from subunit import ProtocolTestCase, TestProtocolClient
from subunit.test_results import AutoTimingTestResultDecorator

from testtools import ConcurrentTestSuite, iterate_tests
from testtools.content import TracebackContent, text_content


_all__ = [
    'ConcurrentTestSuite',
    'fork_for_tests',
    'partition_tests',
]


CPU_COUNT = cpu_count()


class BufferingTestProtocolClient(TestProtocolClient):
    """A TestProtocolClient which can buffer the test outputs

    This class captures the stdout and stderr output streams of the
    tests as it runs them, and includes the output texts in the subunit
    stream as additional details.

    Args:
        stream: A file-like object to write a subunit stream to
        buffer (bool): True to capture test stdout/stderr outputs and
            include them in the test details
    """
    def __init__(self, stream, buffer=True):
        super().__init__(stream)
        self.buffer = buffer

    def _addOutcome(self, outcome, test, error=None, details=None,
            error_permitted=True):
        """Report a test outcome to the subunit stream

        The parent class uses this function as a common implementation
        for various methods that report successes, errors, failures, etc.

        This version automatically upgrades the error tracebacks to the
        new 'details' format by wrapping them in a Content object, so
        that we can include the captured test output in the test result
        details.

        Args:
            outcome: A string describing the outcome - used as the
                event name in the subunit stream.
            test: The test case whose outcome is to be reported
            error: Standard unittest positional argument form - an
                exc_info tuple.
            details: New Testing-in-python drafted API; a dict from
                string to subunit.Content objects.
            error_permitted: If True then one and only one of error or
                details must be supplied. If False then error must not
                be supplied and details is still optional.
        """
        if details is None:
            details = {}

        # Parent will raise an exception if error_permitted is False but
        # error is not None. We want that exception in that case, so
        # don't touch error when error_permitted is explicitly False.
        if error_permitted and error is not None:
            # Parent class prefers error over details
            details['traceback'] = TracebackContent(error, test)
            error_permitted = False
            error = None

        if self.buffer:
            stdout = sys.stdout.getvalue()
            if stdout:
                details['stdout'] = text_content(stdout)

            stderr = sys.stderr.getvalue()
            if stderr:
                details['stderr'] = text_content(stderr)

        return super()._addOutcome(outcome, test, error=error,
                details=details, error_permitted=error_permitted)


def fork_for_tests(concurrency_num=CPU_COUNT, buffer=False):
    """Implementation of `make_tests` used to construct `ConcurrentTestSuite`.

    :param concurrency_num: number of processes to use.
    """
    if buffer:
        test_protocol_client_class = BufferingTestProtocolClient
    else:
        test_protocol_client_class = TestProtocolClient

    def do_fork(suite):
        """Take suite and start up multiple runners by forking (Unix only).

        :param suite: TestSuite object.

        :return: An iterable of TestCase-like objects which can each have
        run(result) called on them to feed tests to result.
        """
        result = []
        test_blocks = partition_tests(suite, concurrency_num)
        # Clear the tests from the original suite so it doesn't keep them alive
        suite._tests[:] = []
        for process_tests in test_blocks:
            process_suite = unittest.TestSuite(process_tests)
            # Also clear each split list so new suite has only reference
            process_tests[:] = []
            c2pread, c2pwrite = os.pipe()
            pid = os.fork()
            if pid == 0:
                try:
                    stream = os.fdopen(c2pwrite, 'wb')
                    os.close(c2pread)
                    # Leave stderr and stdout open so we can see test noise
                    # Close stdin so that the child goes away if it decides to
                    # read from stdin (otherwise its a roulette to see what
                    # child actually gets keystrokes for pdb etc).
                    sys.stdin.close()
                    subunit_result = AutoTimingTestResultDecorator(
                        test_protocol_client_class(stream)
                    )
                    process_suite.run(subunit_result)
                except:
                    # Try and report traceback on stream, but exit with error
                    # even if stream couldn't be created or something else
                    # goes wrong.  The traceback is formatted to a string and
                    # written in one go to avoid interleaving lines from
                    # multiple failing children.
                    try:
                        stream.write(traceback.format_exc())
                    finally:
                        os._exit(1)
                os._exit(0)
            else:
                os.close(c2pwrite)
                stream = os.fdopen(c2pread, 'rb')
                # If we don't pass the second argument here, it defaults
                # to sys.stdout.buffer down the line. But if we don't
                # pass it *now*, it may be resolved after sys.stdout is
                # replaced with a StringIO (to capture tests' outputs)
                # which doesn't have a buffer attribute and can end up
                # occasionally causing a 'broken-runner' error.
                test = ProtocolTestCase(stream, sys.stdout.buffer)
                result.append(test)
        return result
    return do_fork


def partition_tests(suite, count):
    """Partition suite into count lists of tests."""
    # This just assigns tests in a round-robin fashion.  On one hand this
    # splits up blocks of related tests that might run faster if they shared
    # resources, but on the other it avoids assigning blocks of slow tests to
    # just one partition.  So the slowest partition shouldn't be much slower
    # than the fastest.
    partitions = [list() for _ in range(count)]
    tests = iterate_tests(suite)
    for partition, test in zip(cycle(partitions), tests):
        partition.append(test)
    return partitions


if __name__ == '__main__':
    import time

    class SampleTestCase(unittest.TestCase):
        """Dummy tests that sleep for demo."""

        def test_me_1(self):
            time.sleep(0.5)

        def test_me_2(self):
            time.sleep(0.5)

        def test_me_3(self):
            time.sleep(0.5)

        def test_me_4(self):
            time.sleep(0.5)

    # Load tests from SampleTestCase defined above
    suite = unittest.TestLoader().loadTestsFromTestCase(SampleTestCase)
    runner = unittest.TextTestRunner()

    # Run tests sequentially
    runner.run(suite)

    # Run same tests across 4 processes
    suite = unittest.TestLoader().loadTestsFromTestCase(SampleTestCase)
    concurrent_suite = ConcurrentTestSuite(suite, fork_for_tests(4))
    runner.run(concurrent_suite)

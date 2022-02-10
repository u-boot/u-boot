# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc
#

from contextlib import contextmanager
import doctest
import glob
import multiprocessing
import os
import sys
import unittest

from patman import command

from io import StringIO

use_concurrent = True
try:
    from concurrencytest.concurrencytest import ConcurrentTestSuite
    from concurrencytest.concurrencytest import fork_for_tests
except:
    use_concurrent = False


def run_test_coverage(prog, filter_fname, exclude_list, build_dir, required=None,
                    extra_args=None):
    """Run tests and check that we get 100% coverage

    Args:
        prog: Program to run (with be passed a '-t' argument to run tests
        filter_fname: Normally all *.py files in the program's directory will
            be included. If this is not None, then it is used to filter the
            list so that only filenames that don't contain filter_fname are
            included.
        exclude_list: List of file patterns to exclude from the coverage
            calculation
        build_dir: Build directory, used to locate libfdt.py
        required: List of modules which must be in the coverage report
        extra_args (str): Extra arguments to pass to the tool before the -t/test
            arg

    Raises:
        ValueError if the code coverage is not 100%
    """
    # This uses the build output from sandbox_spl to get _libfdt.so
    path = os.path.dirname(prog)
    if filter_fname:
        glob_list = glob.glob(os.path.join(path, '*.py'))
        glob_list = [fname for fname in glob_list if filter_fname in fname]
    else:
        glob_list = []
    glob_list += exclude_list
    glob_list += ['*libfdt.py', '*site-packages*', '*dist-packages*']
    glob_list += ['*concurrencytest*']
    test_cmd = 'test' if 'binman' in prog or 'patman' in prog else '-t'
    prefix = ''
    if build_dir:
        prefix = 'PYTHONPATH=$PYTHONPATH:%s/sandbox_spl/tools ' % build_dir
    cmd = ('%spython3-coverage run '
           '--omit "%s" %s %s %s -P1' % (prefix, ','.join(glob_list),
                                         prog, extra_args or '', test_cmd))
    os.system(cmd)
    stdout = command.output('python3-coverage', 'report')
    lines = stdout.splitlines()
    if required:
        # Convert '/path/to/name.py' just the module name 'name'
        test_set = set([os.path.splitext(os.path.basename(line.split()[0]))[0]
                        for line in lines if '/etype/' in line])
        missing_list = required
        missing_list.discard('__init__')
        missing_list.difference_update(test_set)
        if missing_list:
            print('Missing tests for %s' % (', '.join(missing_list)))
            print(stdout)
            ok = False

    coverage = lines[-1].split(' ')[-1]
    ok = True
    print(coverage)
    if coverage != '100%':
        print(stdout)
        print("Type 'python3-coverage html' to get a report in "
              'htmlcov/index.html')
        print('Coverage error: %s, but should be 100%%' % coverage)
        ok = False
    if not ok:
        raise ValueError('Test coverage failure')


# Use this to suppress stdout/stderr output:
# with capture_sys_output() as (stdout, stderr)
#   ...do something...
@contextmanager
def capture_sys_output():
    capture_out, capture_err = StringIO(), StringIO()
    old_out, old_err = sys.stdout, sys.stderr
    try:
        sys.stdout, sys.stderr = capture_out, capture_err
        yield capture_out, capture_err
    finally:
        sys.stdout, sys.stderr = old_out, old_err


def report_result(toolname:str, test_name: str, result: unittest.TestResult):
    """Report the results from a suite of tests

    Args:
        toolname: Name of the tool that ran the tests
        test_name: Name of test that was run, or None for all
        result: A unittest.TestResult object containing the results
    """
    # Remove errors which just indicate a missing test. Since Python v3.5 If an
    # ImportError or AttributeError occurs while traversing name then a
    # synthetic test that raises that error when run will be returned. These
    # errors are included in the errors accumulated by result.errors.
    if test_name:
        errors = []

        for test, err in result.errors:
            if ("has no attribute '%s'" % test_name) not in err:
                errors.append((test, err))
            result.testsRun -= 1
        result.errors = errors

    print(result)
    for test, err in result.errors:
        print(test.id(), err)
    for test, err in result.failures:
        print(err, result.failures)
    if result.skipped:
        print('%d %s test%s SKIPPED:' % (len(result.skipped), toolname,
            's' if len(result.skipped) > 1 else ''))
        for skip_info in result.skipped:
            print('%s: %s' % (skip_info[0], skip_info[1]))
    if result.errors or result.failures:
        print('%s tests FAILED' % toolname)
        return 1
    return 0


def run_test_suites(result, debug, verbosity, test_preserve_dirs, processes,
                    test_name, toolpath, class_and_module_list):
    """Run a series of test suites and collect the results

    Args:
        result: A unittest.TestResult object to add the results to
        debug: True to enable debugging, which shows a full stack trace on error
        verbosity: Verbosity level to use (0-4)
        test_preserve_dirs: True to preserve the input directory used by tests
            so that it can be examined afterwards (only useful for debugging
            tests). If a single test is selected (in args[0]) it also preserves
            the output directory for this test. Both directories are displayed
            on the command line.
        processes: Number of processes to use to run tests (None=same as #CPUs)
        test_name: Name of test to run, or None for all
        toolpath: List of paths to use for tools
        class_and_module_list: List of test classes (type class) and module
           names (type str) to run
    """
    for module in class_and_module_list:
        if isinstance(module, str) and (not test_name or test_name == module):
            suite = doctest.DocTestSuite(module)
            suite.run(result)

    sys.argv = [sys.argv[0]]
    if debug:
        sys.argv.append('-D')
    if verbosity:
        sys.argv.append('-v%d' % verbosity)
    if toolpath:
        for path in toolpath:
            sys.argv += ['--toolpath', path]

    suite = unittest.TestSuite()
    loader = unittest.TestLoader()
    for module in class_and_module_list:
        if isinstance(module, str):
            continue
        # Test the test module about our arguments, if it is interested
        if hasattr(module, 'setup_test_args'):
            setup_test_args = getattr(module, 'setup_test_args')
            setup_test_args(preserve_indir=test_preserve_dirs,
                preserve_outdirs=test_preserve_dirs and test_name is not None,
                toolpath=toolpath, verbosity=verbosity)
        if test_name:
            try:
                suite.addTests(loader.loadTestsFromName(test_name, module))
            except AttributeError:
                continue
        else:
            suite.addTests(loader.loadTestsFromTestCase(module))
    if use_concurrent and processes != 1:
        concurrent_suite = ConcurrentTestSuite(suite,
                fork_for_tests(processes or multiprocessing.cpu_count()))
        concurrent_suite.run(result)
    else:
        suite.run(result)

# Copyright (c) 2015 Stephen Warren
# Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0

# Implementation of pytest run-time hook functions. These are invoked by
# pytest at certain points during operation, e.g. startup, for each executed
# test, at shutdown etc. These hooks perform functions such as:
# - Parsing custom command-line options.
# - Pullilng in user-specified board configuration.
# - Creating the U-Boot console test fixture.
# - Creating the HTML log file.
# - Monitoring each test's results.
# - Implementing custom pytest markers.

import atexit
import errno
import os
import os.path
import pexpect
import pytest
from _pytest.runner import runtestprotocol
import ConfigParser
import StringIO
import sys

# Globals: The HTML log file, and the connection to the U-Boot console.
log = None
console = None

def mkdir_p(path):
    """Create a directory path.

    This includes creating any intermediate/parent directories. Any errors
    caused due to already extant directories are ignored.

    Args:
        path: The directory path to create.

    Returns:
        Nothing.
    """

    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def pytest_addoption(parser):
    """pytest hook: Add custom command-line options to the cmdline parser.

    Args:
        parser: The pytest command-line parser.

    Returns:
        Nothing.
    """

    parser.addoption('--build-dir', default=None,
        help='U-Boot build directory (O=)')
    parser.addoption('--result-dir', default=None,
        help='U-Boot test result/tmp directory')
    parser.addoption('--persistent-data-dir', default=None,
        help='U-Boot test persistent generated data directory')
    parser.addoption('--board-type', '--bd', '-B', default='sandbox',
        help='U-Boot board type')
    parser.addoption('--board-identity', '--id', default='na',
        help='U-Boot board identity/instance')
    parser.addoption('--build', default=False, action='store_true',
        help='Compile U-Boot before running tests')

def pytest_configure(config):
    """pytest hook: Perform custom initialization at startup time.

    Args:
        config: The pytest configuration.

    Returns:
        Nothing.
    """

    global log
    global console
    global ubconfig

    test_py_dir = os.path.dirname(os.path.abspath(__file__))
    source_dir = os.path.dirname(os.path.dirname(test_py_dir))

    board_type = config.getoption('board_type')
    board_type_filename = board_type.replace('-', '_')

    board_identity = config.getoption('board_identity')
    board_identity_filename = board_identity.replace('-', '_')

    build_dir = config.getoption('build_dir')
    if not build_dir:
        build_dir = source_dir + '/build-' + board_type
    mkdir_p(build_dir)

    result_dir = config.getoption('result_dir')
    if not result_dir:
        result_dir = build_dir
    mkdir_p(result_dir)

    persistent_data_dir = config.getoption('persistent_data_dir')
    if not persistent_data_dir:
        persistent_data_dir = build_dir + '/persistent-data'
    mkdir_p(persistent_data_dir)

    import multiplexed_log
    log = multiplexed_log.Logfile(result_dir + '/test-log.html')

    if config.getoption('build'):
        if build_dir != source_dir:
            o_opt = 'O=%s' % build_dir
        else:
            o_opt = ''
        cmds = (
            ['make', o_opt, '-s', board_type + '_defconfig'],
            ['make', o_opt, '-s', '-j8'],
        )
        runner = log.get_runner('make', sys.stdout)
        for cmd in cmds:
            runner.run(cmd, cwd=source_dir)
        runner.close()

    class ArbitraryAttributeContainer(object):
        pass

    ubconfig = ArbitraryAttributeContainer()
    ubconfig.brd = dict()
    ubconfig.env = dict()

    modules = [
        (ubconfig.brd, 'u_boot_board_' + board_type_filename),
        (ubconfig.env, 'u_boot_boardenv_' + board_type_filename),
        (ubconfig.env, 'u_boot_boardenv_' + board_type_filename + '_' +
            board_identity_filename),
    ]
    for (dict_to_fill, module_name) in modules:
        try:
            module = __import__(module_name)
        except ImportError:
            continue
        dict_to_fill.update(module.__dict__)

    ubconfig.buildconfig = dict()

    for conf_file in ('.config', 'include/autoconf.mk'):
        dot_config = build_dir + '/' + conf_file
        if not os.path.exists(dot_config):
            raise Exception(conf_file + ' does not exist; ' +
                'try passing --build option?')

        with open(dot_config, 'rt') as f:
            ini_str = '[root]\n' + f.read()
            ini_sio = StringIO.StringIO(ini_str)
            parser = ConfigParser.RawConfigParser()
            parser.readfp(ini_sio)
            ubconfig.buildconfig.update(parser.items('root'))

    ubconfig.test_py_dir = test_py_dir
    ubconfig.source_dir = source_dir
    ubconfig.build_dir = build_dir
    ubconfig.result_dir = result_dir
    ubconfig.persistent_data_dir = persistent_data_dir
    ubconfig.board_type = board_type
    ubconfig.board_identity = board_identity

    env_vars = (
        'board_type',
        'board_identity',
        'source_dir',
        'test_py_dir',
        'build_dir',
        'result_dir',
        'persistent_data_dir',
    )
    for v in env_vars:
        os.environ['U_BOOT_' + v.upper()] = getattr(ubconfig, v)

    if board_type == 'sandbox':
        import u_boot_console_sandbox
        console = u_boot_console_sandbox.ConsoleSandbox(log, ubconfig)
    else:
        import u_boot_console_exec_attach
        console = u_boot_console_exec_attach.ConsoleExecAttach(log, ubconfig)

def pytest_generate_tests(metafunc):
    """pytest hook: parameterize test functions based on custom rules.

    If a test function takes parameter(s) (fixture names) of the form brd__xxx
    or env__xxx, the brd and env configuration dictionaries are consulted to
    find the list of values to use for those parameters, and the test is
    parametrized so that it runs once for each combination of values.

    Args:
        metafunc: The pytest test function.

    Returns:
        Nothing.
    """

    subconfigs = {
        'brd': console.config.brd,
        'env': console.config.env,
    }
    for fn in metafunc.fixturenames:
        parts = fn.split('__')
        if len(parts) < 2:
            continue
        if parts[0] not in subconfigs:
            continue
        subconfig = subconfigs[parts[0]]
        vals = []
        val = subconfig.get(fn, [])
        # If that exact name is a key in the data source:
        if val:
            # ... use the dict value as a single parameter value.
            vals = (val, )
        else:
            # ... otherwise, see if there's a key that contains a list of
            # values to use instead.
            vals = subconfig.get(fn + 's', [])
        metafunc.parametrize(fn, vals)

@pytest.fixture(scope='function')
def u_boot_console(request):
    """Generate the value of a test's u_boot_console fixture.

    Args:
        request: The pytest request.

    Returns:
        The fixture value.
    """

    console.ensure_spawned()
    return console

tests_not_run = set()
tests_failed = set()
tests_skipped = set()
tests_passed = set()

def pytest_itemcollected(item):
    """pytest hook: Called once for each test found during collection.

    This enables our custom result analysis code to see the list of all tests
    that should eventually be run.

    Args:
        item: The item that was collected.

    Returns:
        Nothing.
    """

    tests_not_run.add(item.name)

def cleanup():
    """Clean up all global state.

    Executed (via atexit) once the entire test process is complete. This
    includes logging the status of all tests, and the identity of any failed
    or skipped tests.

    Args:
        None.

    Returns:
        Nothing.
    """

    if console:
        console.close()
    if log:
        log.status_pass('%d passed' % len(tests_passed))
        if tests_skipped:
            log.status_skipped('%d skipped' % len(tests_skipped))
            for test in tests_skipped:
                log.status_skipped('... ' + test)
        if tests_failed:
            log.status_fail('%d failed' % len(tests_failed))
            for test in tests_failed:
                log.status_fail('... ' + test)
        if tests_not_run:
            log.status_fail('%d not run' % len(tests_not_run))
            for test in tests_not_run:
                log.status_fail('... ' + test)
        log.close()
atexit.register(cleanup)

def setup_boardspec(item):
    """Process any 'boardspec' marker for a test.

    Such a marker lists the set of board types that a test does/doesn't
    support. If tests are being executed on an unsupported board, the test is
    marked to be skipped.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    mark = item.get_marker('boardspec')
    if not mark:
        return
    required_boards = []
    for board in mark.args:
        if board.startswith('!'):
            if ubconfig.board_type == board[1:]:
                pytest.skip('board not supported')
                return
        else:
            required_boards.append(board)
    if required_boards and ubconfig.board_type not in required_boards:
        pytest.skip('board not supported')

def setup_buildconfigspec(item):
    """Process any 'buildconfigspec' marker for a test.

    Such a marker lists some U-Boot configuration feature that the test
    requires. If tests are being executed on an U-Boot build that doesn't
    have the required feature, the test is marked to be skipped.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    mark = item.get_marker('buildconfigspec')
    if not mark:
        return
    for option in mark.args:
        if not ubconfig.buildconfig.get('config_' + option.lower(), None):
            pytest.skip('.config feature not enabled')

def pytest_runtest_setup(item):
    """pytest hook: Configure (set up) a test item.

    Called once for each test to perform any custom configuration. This hook
    is used to skip the test if certain conditions apply.

    Args:
        item: The pytest test item.

    Returns:
        Nothing.
    """

    log.start_section(item.name)
    setup_boardspec(item)
    setup_buildconfigspec(item)

def pytest_runtest_protocol(item, nextitem):
    """pytest hook: Called to execute a test.

    This hook wraps the standard pytest runtestprotocol() function in order
    to acquire visibility into, and record, each test function's result.

    Args:
        item: The pytest test item to execute.
        nextitem: The pytest test item that will be executed after this one.

    Returns:
        A list of pytest reports (test result data).
    """

    reports = runtestprotocol(item, nextitem=nextitem)
    failed = None
    skipped = None
    for report in reports:
        if report.outcome == 'failed':
            failed = report
            break
        if report.outcome == 'skipped':
            if not skipped:
                skipped = report

    if failed:
        console.drain_console()
        tests_failed.add(item.name)
    elif skipped:
        tests_skipped.add(item.name)
    else:
        tests_passed.add(item.name)
    tests_not_run.remove(item.name)

    try:
        if failed:
            msg = 'FAILED:\n' + str(failed.longrepr)
            log.status_fail(msg)
        elif skipped:
            msg = 'SKIPPED:\n' + str(skipped.longrepr)
            log.status_skipped(msg)
        else:
            log.status_pass('OK')
    except:
        # If something went wrong with logging, it's better to let the test
        # process continue, which may report other exceptions that triggered
        # the logging issue (e.g. console.log wasn't created). Hence, just
        # squash the exception. If the test setup failed due to e.g. syntax
        # error somewhere else, this won't be seen. However, once that issue
        # is fixed, if this exception still exists, it will then be logged as
        # part of the test's stdout.
        import traceback
        print 'Exception occurred while logging runtest status:'
        traceback.print_exc()
        # FIXME: Can we force a test failure here?

    log.end_section(item.name)

    if failed:
        console.cleanup_spawn()

    return reports

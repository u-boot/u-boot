# SPDX-License-Identifier: GPL-2.0
# Copyright 2024 Google LLC

import pytest
import re

# List of test suites we expect to find with 'ut info' and 'ut all'
EXPECTED_SUITES = [
    'addrmap', 'bdinfo', 'bloblist', 'bootm', 'bootstd',
    'cmd', 'common', 'dm', 'env', 'exit', 'fdt_overlay',
    'fdt', 'font', 'hush', 'lib',
    'loadm', 'log', 'mbr', 'measurement', 'mem',
    'pci_mps', 'setexpr', 'upl',
    ]


# Set this to True to aid debugging of tests
DEBUG_ME = False


def collect_info(ubman, output):
    """Process the output from 'ut all'

    Args:
        ubman: U-Boot console object
        output: Output from running 'ut all'

    Returns:
        tuple:
            set: suite names that were found in output
            set: test names that were found in output
            dict: test count for each suite:
                key: suite name
                value: number of tests for the suite found in output
            set: missing suites (compared to EXPECTED_SUITES)
            set: extra suites (compared to EXPECTED_SUITES)
    """
    suites = set()
    tests = set()
    cur_suite = None
    test_count = None
    exp_test_count = {}

    # Collect suites{}
    for line in output.splitlines():
        line = line.rstrip()
        if DEBUG_ME:
            ubman.log.info(f'line: {line}')
        m = re.search('----Running ([^ ]*) tests----', line)
        if m:
            if DEBUG_ME and cur_suite and cur_suite != 'info':
                ubman.log.info(f'suite: {cur_suite} expected {exp_test_count[cur_suite]} found {test_count}')

            cur_suite = m.group(1)
            if DEBUG_ME:
                ubman.log.info(f'cur_suite: {cur_suite}')
            suites.add(cur_suite)

            test_count = 0
        m = re.match(rf'Running (\d+) {cur_suite} tests', line)
        if m:
            exp_test_count[cur_suite] = int(m.group(1))
        m = re.search(r'Test: (\w*): ([-a-z0-9_]*\.c)?( .*)?', line)
        if m:
            test_name = m.group(1)
            msg = m.group(3)
            if DEBUG_ME:
                ubman.log.info(f"test_name {test_name} msg '{msg}'")
            full_name = f'{cur_suite}.{test_name}'
            if msg == ' (flat tree)' and full_name not in tests:
                tests.add(full_name)
                test_count += 1
            if not msg or 'skipped as it is manual' in msg:
                tests.add(full_name)
                test_count += 1
        if DEBUG_ME:
            ubman.log.info(f'test_count {test_count}')
    if DEBUG_ME:
        ubman.log.info(f'suite: {cur_suite} expected {exp_test_count[cur_suite]} found {test_count}')
        ubman.log.info(f"Tests: {' '.join(sorted(list(tests)))}")

    # Figure out what is missing, or extra
    missing = set()
    extra = set(suites)
    for suite in EXPECTED_SUITES:
        if suite in extra:
            extra.remove(suite)
        else:
            missing.add(suite)

    return suites, tests, exp_test_count, missing, extra


def process_ut_info(ubman, output):
    """Process the output of the 'ut info' command

    Args:
        ubman: U-Boot console object
        output: Output from running 'ut all'

    Returns:
        tuple:
            int: Number of suites reported
            int: Number of tests reported
            dict: test count for each suite:
                key: suite name
                value: number of tests reported for the suite

    """
    suite_count = None
    total_test_count = None
    test_count = {}
    for line in output.splitlines():
        line = line.rstrip()
        if DEBUG_ME:
            ubman.log.info(f'line: {line}')
        m = re.match(r'Test suites: (.*)', line)
        if m:
            suite_count = int(m.group(1))
        m = re.match(r'Total tests: (.*)', line)
        if m:
            total_test_count = int(m.group(1))
        m = re.match(r'  *([0-9?]*)  (\w*)', line)
        if m:
            test_count[m.group(2)] = m.group(1)
    return suite_count, total_test_count, test_count


@pytest.mark.buildconfigspec('sandbox')
@pytest.mark.notbuildconfigspec('sandbox_spl')
@pytest.mark.notbuildconfigspec('sandbox64')
# This test is disabled since it fails; remove the leading 'x' to try it
def xtest_suite(ubman, u_boot_config):
    """Perform various checks on the unit tests, including:

       - The number of suites matches that reported by the 'ut info'
       - Where available, the number of tests is each suite matches that
         reported by 'ut -s info'
       - The total number of tests adds up to the total that are actually run
         with 'ut all'
       - All suites are run with 'ut all'
       - The expected set of suites is run (the list is hard-coded in this test)

    """
    buildconfig = u_boot_config.buildconfig
    with ubman.log.section('Run all unit tests'):
        # ut hush hush_test_simple_dollar prints "Unknown command" on purpose.
        with ubman.disable_check('unknown_command'):
            output = ubman.run_command('ut all')

    # Process the output from the run
    with ubman.log.section('Check output'):
        suites, all_tests, exp_test_count, missing, extra = collect_info(ubman,
                                                                         output)
    ubman.log.info(f'missing {missing}')
    ubman.log.info(f'extra {extra}')

    # Make sure we got a test count for each suite
    assert not (suites - exp_test_count.keys())

    # Deal with missing suites
    with ubman.log.section('Check missing suites'):
        if 'config_cmd_seama' not in buildconfig:
            ubman.log.info("CMD_SEAMA not enabled: Ignoring suite 'seama'")
            missing.discard('seama')

    # Run 'ut info' and compare with the log results
    with ubman.log.section('Check suite test-counts'):
        output = ubman.run_command('ut -s info')

        suite_count, total_test_count, test_count = process_ut_info(ubman,
                                                                    output)

        if missing or extra:
            ubman.log.info(f"suites: {' '.join(sorted(list(suites)))}")
            ubman.log.error(f'missing: {sorted(list(missing))}')
            ubman.log.error(f'extra: {sorted(list(extra))}')

        assert not missing, f'Missing suites {missing}'
        assert not extra, f'Extra suites {extra}'

        ubman.log.info(str(exp_test_count))
        for suite in EXPECTED_SUITES:
            assert test_count[suite] in ['?', str(exp_test_count[suite])], \
                f'suite {suite} expected {exp_test_count[suite]}'

        assert suite_count == len(EXPECTED_SUITES)
        assert total_test_count == len(all_tests)

    # Run three suites
    with ubman.log.section('Check multiple suites'):
        output = ubman.run_command('ut bloblist,setexpr,mem')
        assert 'Suites run: 3' in output

    # Run a particular test
    with ubman.log.section('Check single test'):
        output = ubman.run_command('ut bloblist reloc')
        assert 'Test: reloc: bloblist.c' in output

    # Run tests multiple times
    with ubman.log.section('Check multiple runs'):
        output = ubman.run_command('ut -r2 bloblist')
        lines = output.splitlines()
        run = len([line for line in lines if 'Test:' in line])
        count = re.search(r'Tests run: (\d*)', lines[-1]).group(1)

        assert run == 2 * int(count)

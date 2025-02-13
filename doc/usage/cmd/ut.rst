.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: ut (command)

ut command
==========

Synopsis
--------

::

    ut [-r<runs>] [-f] [-I<n>:<one_test>] [-r<n>] [<suite> | 'all' [<test>]]
    ut [-s] info

Description
-----------

The ut command runs unit tests written in C.

suite
    Specifies the suite to run, This can be a single suite, or a comma-separated
    list

test
    Speciifes a particular test to run, within a suite, or all suites

-f
    Forces running of a manual test.

-r <n>
    Specifies the number of types to run each test

-I <n>:<one_test>
    Test to run after <n> other tests have run.  This is used to find which test
    causes another test to fail. If the one test fails, testing stops
    immediately.

Typically the command is run on :ref:`arch/sandbox/sandbox:sandbox` since it
includes a near-complete set of emulators, no code-size limits, many CONFIG
options enabled and runs easily in CI without needing QEMU. It is also possible
to run some tests on real boards.

Each test is normally run once, although those marked with `UTF_DM` are
run with livetree and flattree where possible. To run a test more than once,
use the `-r` flag.

Manual tests are normally skipped by this command. Use `-f` to run them. See
:ref:`develop/tests_writing:mixing python and c` for more information on manual
tests.

When running unit tests, some may have side effects which cause a subsequent
test to break. This can sometimes be seen when using 'ut dm' or similar. To
fix this, select the 'one' test which breaks. Then tell the 'ut' command to
run this one test after a certain number of other tests have run. Using a
binary search method with `-I` you can quickly figure one which test is causing
the problem.

Generally all tests in the suite are run. To run just a single test from the
suite, provide the <test> argument.

To specify a list of suites to run, <suites> can also be a comma-separated list.

See :ref:`develop/tests_writing:writing c tests` for more information on how to
write unit tests.

ut all
~~~~~~

Instead of a suite name 'all' may be used to run all tests.

ut info
~~~~~~~

This provides information about the total number of suites and tests. Use the
`-s` flag to show a detailed list of suites.

Example
-------

Show information about tests::

    => ut info
    Test suites: 23
    Total tests: 833

List available unit-test suites::

    => ut info -s
    Test suites: 23
    Total tests: 833

    Tests  Suite         Purpose
    -----  ------------  -------------------------
        1  addrmap       very basic test of addrmap command
        4  bdinfo        bdinfo (board info) command
       14  bloblist      bloblist implementation
        7  bootm         bootm command
       66  bootstd       standard boot implementation
        2  cmd           various commands
       14  common        tests for common/ directory
       502 dm            driver model
        6  env           environment
        1  exit          shell exit and variables
       19  fdt           fdt command
       10  fdt_overlay   device tree overlays
        1  font          font command
       20  hush          hush behaviour
      115  lib           library functions
        2  loadm         loadm command parameters and loading memory blob
       18  log           logging functions
        1  mbr           mbr command
        1  measurement   TPM-based measured boot
       13  mem           memory-related commands
        1  pci_mps       PCI Express Maximum Payload Size
       11  setexpr       setexpr command
        4  upl           Universal payload support


Run one of the suites::

    => ut common
    Running 14 common tests
    Test: cli_ch_test: cread.c
    Test: cread_test: cread.c
    Test: dm_test_cyclic_running: cyclic.c
    Test: print_display_buffer: print.c
    Test: print_do_hex_dump: print.c
    Test: print_efi_ut: print.c
    Test: print_guid: print.c
    Test: print_hexdump_line: print.c
    Test: print_printf: print.c
    Test: snprint: print.c
    Test: test_autoboot: test_autoboot.c
    Enter password "a" in 1 seconds to stop autoboot
    Enter password "a" in 1 seconds to stop autoboot
    Enter password "a" in 1 seconds to stop autoboot
    Enter password "a" in 1 seconds to stop autoboot
    Enter password "a" in 1 seconds to stop autoboot
    Enter password "a" in 1 seconds to stop autoboot
    Autoboot password unlock not successful
    Test: test_event_base: event.c
    Test: test_event_probe: event.c
    Test: test_event_probe: event.c (flat tree)
    Test: test_event_simple: event.c
    Tests run: 14, 2611 ms, average 186 ms, skipped: 2, failures: 0

Run just a single test in a suite::

    => ut fdt_overlay change_int_property
    Test: fdt_overlay_init: cmd_ut_fdt_overlay.c
    Test: change_int_property: cmd_ut_fdt_overlay.c
    Tests run: 2, 0 ms, average 0 ms, failures: 0

Run a selection of three suites::

    => ut bloblist,mem,fdt_overlay
    Running 14 bloblist tests
    Test: align: bloblist.c
    Test: bad_blob: bloblist.c
    Test: blob: bloblist.c
    Test: blob_ensure: bloblist.c
    Test: blob_maxsize: bloblist.c
    Test: checksum: bloblist.c
    Test: cmd_info: bloblist.c
    Test: cmd_list: bloblist.c
    Test: grow: bloblist.c
    Test: init: bloblist.c
    Test: reloc: bloblist.c
    Test: resize_fail: bloblist.c
    Test: resize_last: bloblist.c
    Test: shrink: bloblist.c
    Tests run: 14, 1 ms, average: 0 ms, failures: 0
    Running 13 mem tests
    Test: cp_b: mem_copy.c
    Test: cp_l: mem_copy.c
    Test: cp_q: mem_copy.c
    Test: cp_w: mem_copy.c
    Test: ms_b: mem_search.c
    Test: ms_cont: mem_search.c
    Test: ms_cont_end: mem_search.c
    Test: ms_l: mem_search.c
    Test: ms_limit: mem_search.c
    Test: ms_mult: mem_search.c
    Test: ms_quiet: mem_search.c
    Test: ms_s: mem_search.c
    Test: ms_w: mem_search.c
    Tests run: 13, 13 ms, average: 1 ms, failures: 0
    Running 10 fdt_overlay tests
    Test: fdt_overlay_init: cmd_ut_fdt_overlay.c
    Test: add_node_by_path: cmd_ut_fdt_overlay.c
    Test: add_node_by_phandle: cmd_ut_fdt_overlay.c
    Test: add_str_property: cmd_ut_fdt_overlay.c
    Test: add_subnode_property: cmd_ut_fdt_overlay.c
    Test: change_int_property: cmd_ut_fdt_overlay.c
    Test: change_str_property: cmd_ut_fdt_overlay.c
    Test: local_phandle: cmd_ut_fdt_overlay.c
    Test: local_phandles: cmd_ut_fdt_overlay.c
    Test: stacked: cmd_ut_fdt_overlay.c
    Tests run: 10, 12 ms, average: 1 ms, failures: 0
    Suites run: 3, total tests run: 37, 26 ms, average: 0 ms, failures: 0
    Average test time: 0 ms, worst case 'mem' took 1 ms

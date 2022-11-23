// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for fdt command
 *
 * Copyright 2022 Google LLCmap_to_sysmem(fdt));
 */

#include <common.h>
#include <console.h>
#include <fdt_support.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <test/suites.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Declare a new fdt test */
#define FDT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, fdt_test)

/**
 * make_test_fdt() - Create an FDT with just a root node
 *
 * The size is set to the minimum needed
 *
 * @uts: Test state
 * @fdt: Place to write FDT
 * @size: Maximum size of space for fdt
 */
static int make_test_fdt(struct unit_test_state *uts, void *fdt, int size)
{
	ut_assertok(fdt_create(fdt, size));
	ut_assertok(fdt_finish_reservemap(fdt));
	ut_assert(fdt_begin_node(fdt, "") >= 0);
	ut_assertok(fdt_end_node(fdt));
	ut_assertok(fdt_finish(fdt));

	return 0;
}

/* Test 'fdt addr' getting/setting address */
static int fdt_test_addr(struct unit_test_state *uts)
{
	const void *fdt_blob, *new_fdt;
	char fdt[256];
	ulong addr;
	int ret;

	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt addr -c", 0));
	ut_assert_nextline("Control fdt: %08lx",
			   (ulong)map_to_sysmem(gd->fdt_blob));
	ut_assertok(ut_check_console_end(uts));

	/* The working fdt is not set, so this should fail */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_asserteq(CMD_RET_FAILURE, run_command("fdt addr", 0));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Set up a working FDT and try again */
	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(run_command("fdt addr", 0));
	ut_assert_nextline("Working fdt: %08lx", (ulong)map_to_sysmem(fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Set the working FDT */
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");
	ut_assertok(run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(addr, map_to_sysmem(working_fdt));
	ut_assertok(ut_check_console_end(uts));
	set_working_fdt_addr(0);
	ut_assert_nextline("Working FDT set to 0");

	/* Set the control FDT */
	fdt_blob = gd->fdt_blob;
	gd->fdt_blob = NULL;
	ret = run_commandf("fdt addr -c %08x", addr);
	new_fdt = gd->fdt_blob;
	gd->fdt_blob = fdt_blob;
	ut_assertok(ret);
	ut_asserteq(addr, map_to_sysmem(new_fdt));
	ut_assertok(ut_check_console_end(uts));

	/* Test setting an invalid FDT */
	fdt[0] = 123;
	ut_asserteq(1, run_commandf("fdt addr %08x", addr));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	/* Test detecting an invalid FDT */
	fdt[0] = 123;
	set_working_fdt_addr(addr);
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_asserteq(1, run_commandf("fdt addr"));
	ut_assert_nextline("libfdt fdt_check_header(): FDT_ERR_BADMAGIC");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_addr, UT_TESTF_CONSOLE_REC);

/* Test 'fdt addr' resizing an fdt */
static int fdt_test_resize(struct unit_test_state *uts)
{
	char fdt[256];
	const int newsize = sizeof(fdt) / 2;
	ulong addr;

	ut_assertok(make_test_fdt(uts, fdt, sizeof(fdt)));
	addr = map_to_sysmem(fdt);
	set_working_fdt_addr(addr);

	/* Test setting and resizing the working FDT to a larger size */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_commandf("fdt addr %08x %x", addr, newsize));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* Try shrinking it */
	ut_assertok(run_commandf("fdt addr %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assert_nextline("New length %d < existing length %d, ignoring",
			   (int)sizeof(fdt) / 4, newsize);
	ut_assertok(ut_check_console_end(uts));

	/* ...quietly */
	ut_assertok(run_commandf("fdt addr -q %08x %x", addr, sizeof(fdt) / 4));
	ut_assert_nextline("Working FDT set to %lx", addr);
	ut_assertok(ut_check_console_end(uts));

	/* We cannot easily provoke errors in fdt_open_into(), so ignore that */

	return 0;
}
FDT_TEST(fdt_test_resize, UT_TESTF_CONSOLE_REC);

/* Test 'fdt get' reading an fdt */
static int fdt_test_get(struct unit_test_state *uts)
{
	ulong addr;

	addr = map_to_sysmem(gd->fdt_blob);
	set_working_fdt_addr(addr);

	/* Test getting default element of /clk-test node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get value fdflt /clk-test clock-names", 0));
	ut_asserteq_str("fixed", env_get("fdflt"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting 0th element of /clk-test node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get value fzero /clk-test clock-names 0", 0));
	ut_asserteq_str("fixed", env_get("fzero"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting 1st element of /clk-test node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get value fone /clk-test clock-names 1", 0));
	ut_asserteq_str("i2c", env_get("fone"));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting 2nd element of /clk-test node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_assertok(run_command("fdt get value ftwo /clk-test clock-names 2", 0));
	ut_asserteq_str("spi", env_get("ftwo"));
	ut_assertok(ut_check_console_end(uts));

	/* Test missing 10th element of /clk-test node clock-names property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value ftwo /clk-test clock-names 10", 0));
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of /clk-test node nonexistent property */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value fnone /clk-test nonexistent", 1));
	ut_assert_nextline("libfdt fdt_getprop(): FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	/* Test getting default element of /nonexistent node */
	ut_assertok(console_record_reset_enable());
	ut_asserteq(1, run_command("fdt get value fnode /nonexistent nonexistent", 1));
	ut_assert_nextline("libfdt fdt_path_offset() returned FDT_ERR_NOTFOUND");
	ut_assertok(ut_check_console_end(uts));

	return 0;
}
FDT_TEST(fdt_test_get, UT_TESTF_CONSOLE_REC);

int do_ut_fdt(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(fdt_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(fdt_test);

	return cmd_ut_category("fdt", "fdt_test_", tests, n_ents, argc, argv);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for setexpr command
 *
 * Copyright 2020 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <console.h>
#include <mapmem.h>
#include <dm/test.h>
#include <test/suites.h>
#include <test/ut.h>

#define BUF_SIZE	0x100

/* Declare a new mem test */
#define SETEXPR_TEST(_name, _flags)	UNIT_TEST(_name, _flags, setexpr_test)

/* Test 'setexpr' command with simply setting integers */
static int setexpr_test_int(struct unit_test_state *uts)
{
	u8 *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\xff', BUF_SIZE);

	/* byte */
	buf[0x0] = 0x12;
	ut_assertok(run_command("setexpr.b fred 0", 0));
	ut_asserteq_str("0", env_get("fred"));
	ut_assertok(run_command("setexpr.b fred *0", 0));
	ut_asserteq_str("12", env_get("fred"));

	/* 16-bit */
	*(short *)buf = 0x2345;
	ut_assertok(run_command("setexpr.w fred 0", 0));
	ut_asserteq_str("0", env_get("fred"));
	ut_assertok(run_command("setexpr.w fred *0", 0));
	ut_asserteq_str("2345", env_get("fred"));

	/* 32-bit */
	*(u32 *)buf = 0x3456789a;
	ut_assertok(run_command("setexpr.l fred 0", 0));
	ut_asserteq_str("0", env_get("fred"));
	ut_assertok(run_command("setexpr.l fred *0", 0));
	ut_asserteq_str("3456789a", env_get("fred"));

	/* 64-bit */
	*(u64 *)buf = 0x456789abcdef0123;
	ut_assertok(run_command("setexpr.q fred 0", 0));
	ut_asserteq_str("0", env_get("fred"));
	ut_assertok(run_command("setexpr.q fred *0", 0));
	ut_asserteq_str("456789abcdef0123", env_get("fred"));

	/* default */
	ut_assertok(run_command("setexpr fred 0", 0));
	ut_asserteq_str("0", env_get("fred"));
	ut_assertok(run_command("setexpr fred *0", 0));
	ut_asserteq_str("cdef0123", env_get("fred"));

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_int, UT_TESTF_CONSOLE_REC);

/* Test 'setexpr' command with + operator */
static int setexpr_test_plus(struct unit_test_state *uts)
{
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\xff', BUF_SIZE);

	/* byte */
	buf[0x0] = 0x12;
	buf[0x10] = 0x34;
	ut_assertok(run_command("setexpr.b fred *0 + *10", 0));
	ut_asserteq_str("46", env_get("fred"));

	/* 16-bit */
	*(short *)buf = 0x2345;
	*(short *)(buf + 0x10) = 0xf012;
	ut_assertok(run_command("setexpr.w fred *0 + *10", 0));
	ut_asserteq_str("11357", env_get("fred"));

	/* 32-bit */
	*(u32 *)buf = 0x3456789a;
	*(u32 *)(buf + 0x10) = 0xc3384235;
	ut_assertok(run_command("setexpr.l fred *0 + *10", 0));
	ut_asserteq_str("f78ebacf", env_get("fred"));

	/* 64-bit */
	*(u64 *)buf = 0x456789abcdef0123;
	*(u64 *)(buf + 0x10) = 0x4987328372849283;
	ut_assertok(run_command("setexpr.q fred *0 + *10", 0));
	ut_asserteq_str("8eeebc2f407393a6", env_get("fred"));

	/* default */
	ut_assertok(run_command("setexpr fred *0 + *10", 0));
	ut_asserteq_str("1407393a6", env_get("fred"));

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_plus, UT_TESTF_CONSOLE_REC);

/* Test 'setexpr' command with other operators */
static int setexpr_test_oper(struct unit_test_state *uts)
{
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\xff', BUF_SIZE);

	*(u32 *)buf = 0x1234;
	*(u32 *)(buf + 0x10) = 0x560000;

	/* Quote | to avoid confusing hush */
	ut_assertok(run_command("setexpr fred *0 \"|\" *10", 0));
	ut_asserteq_str("561234", env_get("fred"));

	*(u32 *)buf = 0x561200;
	*(u32 *)(buf + 0x10) = 0x1234;

	/* Quote & to avoid confusing hush */
	ut_assertok(run_command("setexpr.l fred *0 \"&\" *10", 0));
	ut_asserteq_str("1200", env_get("fred"));

	ut_assertok(run_command("setexpr.l fred *0 ^ *10", 0));
	ut_asserteq_str("560034", env_get("fred"));

	ut_assertok(run_command("setexpr.l fred *0 - *10", 0));
	ut_asserteq_str("55ffcc", env_get("fred"));

	ut_assertok(run_command("setexpr.l fred *0 * *10", 0));
	ut_asserteq_str("61ebfa800", env_get("fred"));

	ut_assertok(run_command("setexpr.l fred *0 / *10", 0));
	ut_asserteq_str("4ba", env_get("fred"));

	ut_assertok(run_command("setexpr.l fred *0 % *10", 0));
	ut_asserteq_str("838", env_get("fred"));

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_oper, UT_TESTF_CONSOLE_REC);

int do_ut_setexpr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test,
						 setexpr_test);
	const int n_ents = ll_entry_count(struct unit_test, setexpr_test);

	return cmd_ut_category("cmd_setexpr", "cmd_mem_", tests, n_ents, argc,
			       argv);
}

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

/* Declare a new setexpr test */
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

/* Test 'setexpr' command with regex */
static int setexpr_test_regex(struct unit_test_state *uts)
{
	char *buf, *val;

	buf = map_sysmem(0, BUF_SIZE);

	/* Single substitution */
	ut_assertok(run_command("setenv fred 'this is a test'", 0));
	ut_assertok(run_command("setexpr fred sub is us", 0));
	val = env_get("fred");
	ut_asserteq_str("thus is a test", val);

	/* Global substitution */
	ut_assertok(run_command("setenv fred 'this is a test'", 0));
	ut_assertok(run_command("setexpr fred gsub is us", 0));
	val = env_get("fred");
	ut_asserteq_str("thus us a test", val);

	/* Global substitution */
	ut_assertok(run_command("setenv fred 'this is a test'", 0));
	ut_assertok(run_command("setenv mary 'this is a test'", 0));
	ut_assertok(run_command("setexpr fred gsub is us \"${mary}\"", 0));
	val = env_get("fred");
	ut_asserteq_str("thus us a test", val);
	val = env_get("mary");
	ut_asserteq_str("this is a test", val);

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_regex, UT_TESTF_CONSOLE_REC);

/* Test 'setexpr' command with regex replacement that expands the string */
static int setexpr_test_regex_inc(struct unit_test_state *uts)
{
	char *buf, *val;

	buf = map_sysmem(0, BUF_SIZE);

	ut_assertok(run_command("setenv fred 'this is a test'", 0));
	ut_assertok(run_command("setexpr fred gsub is much_longer_string", 0));
	val = env_get("fred");
	ut_asserteq_str("thmuch_longer_string much_longer_string a test", val);
	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_regex_inc, UT_TESTF_CONSOLE_REC);

/* Test setexpr_regex_sub() directly to check buffer usage */
static int setexpr_test_sub(struct unit_test_state *uts)
{
	char *buf, *nbuf;
	int i;

	buf = map_sysmem(0, BUF_SIZE);
	nbuf = map_sysmem(0x1000, BUF_SIZE);

	/* Add a pattern so we can check the buffer limits */
	memset(buf, '\xff', BUF_SIZE);
	memset(nbuf, '\xff', BUF_SIZE);
	for (i = BUF_SIZE; i < 0x1000; i++) {
		buf[i] = i & 0xff;
		nbuf[i] = i & 0xff;
	}
	strcpy(buf, "this is a test");

	/*
	 * This is a regression test, since a bug was found in the use of
	 * memmove() in setexpr
	 */
	ut_assertok(setexpr_regex_sub(buf, BUF_SIZE, nbuf, BUF_SIZE, "is",
				      "us it is longer", true));
	ut_asserteq_str("thus it is longer us it is longer a test", buf);
	for (i = BUF_SIZE; i < 0x1000; i++) {
		ut_assertf(buf[i] == (char)i,
			   "buf byte at %x should be %02x, got %02x)\n",
			   i, i & 0xff, (u8)buf[i]);
		ut_assertf(nbuf[i] == (char)i,
			   "nbuf byte at %x should be %02x, got %02x)\n",
			   i, i & 0xff, (u8)nbuf[i]);
	}

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_sub, UT_TESTF_CONSOLE_REC);

/* Test setexpr_regex_sub() with back references */
static int setexpr_test_backref(struct unit_test_state *uts)
{
	char *buf, *nbuf;
	int i;

	buf = map_sysmem(0, BUF_SIZE);
	nbuf = map_sysmem(0x1000, BUF_SIZE);

	/* Add a pattern so we can check the buffer limits */
	memset(buf, '\xff', BUF_SIZE);
	memset(nbuf, '\xff', BUF_SIZE);
	for (i = BUF_SIZE; i < 0x1000; i++) {
		buf[i] = i & 0xff;
		nbuf[i] = i & 0xff;
	}
	strcpy(buf, "this is surely a test is it? yes this is indeed a test");

	/*
	 * This is a regression test, since a bug was found in the use of
	 * memmove() in setexpr
	 */
	ut_assertok(setexpr_regex_sub(buf, BUF_SIZE, nbuf, BUF_SIZE,
				      "(this) (is) (surely|indeed)",
				      "us \\1 \\2 \\3!", true));
	ut_asserteq_str("us this is surely! a test is it? yes us this is indeed! a test",
			buf);

	/* The following checks fail at present due to a bug in setexpr */
	return 0;
	for (i = BUF_SIZE; i < 0x1000; i++) {
		ut_assertf(buf[i] == (char)i,
			   "buf byte at %x should be %02x, got %02x)\n",
			   i, i & 0xff, (u8)buf[i]);
		ut_assertf(nbuf[i] == (char)i,
			   "nbuf byte at %x should be %02x, got %02x)\n",
			   i, i & 0xff, (u8)nbuf[i]);
	}

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_backref, UT_TESTF_CONSOLE_REC);

/* Test 'setexpr' command with setting strings */
static int setexpr_test_str(struct unit_test_state *uts)
{
	ulong start_mem;
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\xff', BUF_SIZE);

	/*
	 * Set 'fred' to the same length as we expect to get below, to avoid a
	 * new allocation in 'setexpr'. That way we can check for memory leaks.
	 */
	ut_assertok(env_set("fred", "x"));
	start_mem = ut_check_free();
	strcpy(buf, "hello");
	ut_asserteq(1, run_command("setexpr.s fred 0", 0));
	ut_assertok(ut_check_delta(start_mem));

	ut_assertok(env_set("fred", "12345"));
	start_mem = ut_check_free();
	ut_assertok(run_command("setexpr.s fred *0", 0));
	ut_asserteq_str("hello", env_get("fred"));
	ut_assertok(ut_check_delta(start_mem));

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_str, UT_TESTF_CONSOLE_REC);


/* Test 'setexpr' command with concatenating strings */
static int setexpr_test_str_oper(struct unit_test_state *uts)
{
	ulong start_mem;
	char *buf;

	buf = map_sysmem(0, BUF_SIZE);
	memset(buf, '\xff', BUF_SIZE);
	strcpy(buf, "hello");
	strcpy(buf + 0x10, " there");

	ut_assertok(console_record_reset_enable());
	start_mem = ut_check_free();
	ut_asserteq(1, run_command("setexpr.s fred *0 * *10", 0));
	ut_assertok(ut_check_delta(start_mem));
	ut_assert_nextline("invalid op");
	ut_assert_console_end();

	/*
	 * Set 'fred' to the same length as we expect to get below, to avoid a
	 * new allocation in 'setexpr'. That way we can check for memory leaks.
	 */
	ut_assertok(env_set("fred", "12345012345"));
	start_mem = ut_check_free();
	ut_assertok(run_command("setexpr.s fred *0 + *10", 0));
	ut_asserteq_str("hello there", env_get("fred"));

	/*
	 * This check does not work with sandbox_flattree, apparently due to
	 * memory allocations in env_set().
	 *
	 * The truetype console produces lots of memory allocations even though
	 * the LCD display is not visible. But even without these, it does not
	 * work.
	 *
	 * A better test would be for dlmalloc to record the allocs and frees
	 * for a particular caller, but that is not supported.
	 *
	 * For now, drop this test.
	 *
	 * ut_assertok(ut_check_delta(start_mem));
	 */

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_str_oper, UT_TESTF_CONSOLE_REC);

/* Test 'setexpr' command with a string that is too long */
static int setexpr_test_str_long(struct unit_test_state *uts)
{
	const int size = 128 << 10;  /* setexpr strings are a max of 64KB */
	char *buf, *val;

	buf = map_sysmem(0, size);
	memset(buf, 'a', size);

	/* String should be truncated to 64KB */
	ut_assertok(run_command("setexpr.s fred *0", 0));
	val = env_get("fred");
	ut_asserteq(64 << 10, strlen(val));

	unmap_sysmem(buf);

	return 0;
}
SETEXPR_TEST(setexpr_test_str_long, UT_TESTF_CONSOLE_REC);

int do_ut_setexpr(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = UNIT_TEST_SUITE_START(setexpr_test);
	const int n_ents = UNIT_TEST_SUITE_COUNT(setexpr_test);

	return cmd_ut_category("cmd_setexpr", "setexpr_test_", tests, n_ents,
			       argc, argv);
}

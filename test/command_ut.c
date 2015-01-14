/*
 * Copyright (c) 2012, The Chromium Authors
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define DEBUG

#include <common.h>
#ifdef CONFIG_SANDBOX
#include <os.h>
#endif

static const char test_cmd[] = "setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3\0"
		"setenv list ${list}4";

static int do_ut_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	printf("%s: Testing commands\n", __func__);
	run_command("env default -f -a", 0);

	/* run a single command */
	run_command("setenv single 1", 0);
	assert(!strcmp("1", getenv("single")));

	/* make sure that compound statements work */
#ifdef CONFIG_SYS_HUSH_PARSER
	run_command("if test -n ${single} ; then setenv check 1; fi", 0);
	assert(!strcmp("1", getenv("check")));
	run_command("setenv check", 0);
#endif

	/* commands separated by ; */
	run_command_list("setenv list 1; setenv list ${list}1", -1, 0);
	assert(!strcmp("11", getenv("list")));

	/* commands separated by \n */
	run_command_list("setenv list 1\n setenv list ${list}1", -1, 0);
	assert(!strcmp("11", getenv("list")));

	/* command followed by \n and nothing else */
	run_command_list("setenv list 1${list}\n", -1, 0);
	assert(!strcmp("111", getenv("list")));

	/* three commands in a row */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", -1, 0);
	assert(!strcmp("123", getenv("list")));

	/* a command string with \0 in it. Stuff after \0 should be ignored */
	run_command("setenv list", 0);
	run_command_list(test_cmd, sizeof(test_cmd), 0);
	assert(!strcmp("123", getenv("list")));

	/*
	 * a command list where we limit execution to only the first command
	 * using the length parameter.
	 */
	run_command_list("setenv list 1\n setenv list ${list}2; "
		"setenv list ${list}3", strlen("setenv list 1"), 0);
	assert(!strcmp("1", getenv("list")));

	assert(run_command("false", 0) == 1);
	assert(run_command("echo", 0) == 0);
	assert(run_command_list("false", -1, 0) == 1);
	assert(run_command_list("echo", -1, 0) == 0);

	run_command("setenv foo 'setenv monty 1; setenv python 2'", 0);
	run_command("run foo", 0);
	assert(getenv("monty") != NULL);
	assert(!strcmp("1", getenv("monty")));
	assert(getenv("python") != NULL);
	assert(!strcmp("2", getenv("python")));

#ifdef CONFIG_SYS_HUSH_PARSER
	run_command("setenv foo 'setenv black 1\nsetenv adder 2'", 0);
	run_command("run foo", 0);
	assert(getenv("black") != NULL);
	assert(!strcmp("1", getenv("black")));
	assert(getenv("adder") != NULL);
	assert(!strcmp("2", getenv("adder")));

	/* Test the 'test' command */

#define HUSH_TEST(name, expr, expected_result) \
	run_command("if test " expr " ; then " \
			"setenv " #name "_" #expected_result " y; else " \
			"setenv " #name "_" #expected_result " n; fi", 0); \
	assert(!strcmp(#expected_result, getenv(#name "_" #expected_result))); \
	setenv(#name "_" #expected_result, NULL);

	/* Basic operators */
	HUSH_TEST(streq, "aaa = aaa", y);
	HUSH_TEST(streq, "aaa = bbb", n);

	HUSH_TEST(strneq, "aaa != bbb", y);
	HUSH_TEST(strneq, "aaa != aaa", n);

	HUSH_TEST(strlt, "aaa < bbb", y);
	HUSH_TEST(strlt, "bbb < aaa", n);

	HUSH_TEST(strgt, "bbb > aaa", y);
	HUSH_TEST(strgt, "aaa > bbb", n);

	HUSH_TEST(eq, "123 -eq 123", y);
	HUSH_TEST(eq, "123 -eq 456", n);

	HUSH_TEST(ne, "123 -ne 456", y);
	HUSH_TEST(ne, "123 -ne 123", n);

	HUSH_TEST(lt, "123 -lt 456", y);
	HUSH_TEST(lt_eq, "123 -lt 123", n);
	HUSH_TEST(lt, "456 -lt 123", n);

	HUSH_TEST(le, "123 -le 456", y);
	HUSH_TEST(le_eq, "123 -le 123", y);
	HUSH_TEST(le, "456 -le 123", n);

	HUSH_TEST(gt, "456 -gt 123", y);
	HUSH_TEST(gt_eq, "123 -gt 123", n);
	HUSH_TEST(gt, "123 -gt 456", n);

	HUSH_TEST(ge, "456 -ge 123", y);
	HUSH_TEST(ge_eq, "123 -ge 123", y);
	HUSH_TEST(ge, "123 -ge 456", n);

	HUSH_TEST(z, "-z \"\"", y);
	HUSH_TEST(z, "-z \"aaa\"", n);

	HUSH_TEST(n, "-n \"aaa\"", y);
	HUSH_TEST(n, "-n \"\"", n);

	/* Inversion of simple tests */
	HUSH_TEST(streq_inv, "! aaa = aaa", n);
	HUSH_TEST(streq_inv, "! aaa = bbb", y);

	HUSH_TEST(streq_inv_inv, "! ! aaa = aaa", y);
	HUSH_TEST(streq_inv_inv, "! ! aaa = bbb", n);

	/* Binary operators */
	HUSH_TEST(or_0_0, "aaa != aaa -o bbb != bbb", n);
	HUSH_TEST(or_0_1, "aaa != aaa -o bbb = bbb", y);
	HUSH_TEST(or_1_0, "aaa = aaa -o bbb != bbb", y);
	HUSH_TEST(or_1_1, "aaa = aaa -o bbb = bbb", y);

	HUSH_TEST(and_0_0, "aaa != aaa -a bbb != bbb", n);
	HUSH_TEST(and_0_1, "aaa != aaa -a bbb = bbb", n);
	HUSH_TEST(and_1_0, "aaa = aaa -a bbb != bbb", n);
	HUSH_TEST(and_1_1, "aaa = aaa -a bbb = bbb", y);

	/* Inversion within binary operators */
	HUSH_TEST(or_0_0_inv, "! aaa != aaa -o ! bbb != bbb", y);
	HUSH_TEST(or_0_1_inv, "! aaa != aaa -o ! bbb = bbb", y);
	HUSH_TEST(or_1_0_inv, "! aaa = aaa -o ! bbb != bbb", y);
	HUSH_TEST(or_1_1_inv, "! aaa = aaa -o ! bbb = bbb", n);

	HUSH_TEST(or_0_0_inv_inv, "! ! aaa != aaa -o ! ! bbb != bbb", n);
	HUSH_TEST(or_0_1_inv_inv, "! ! aaa != aaa -o ! ! bbb = bbb", y);
	HUSH_TEST(or_1_0_inv_inv, "! ! aaa = aaa -o ! ! bbb != bbb", y);
	HUSH_TEST(or_1_1_inv_inv, "! ! aaa = aaa -o ! ! bbb = bbb", y);

	setenv("ut_var_nonexistent", NULL);
	setenv("ut_var_exists", "1");
	HUSH_TEST(z_varexp_quoted, "-z \"$ut_var_nonexistent\"", y);
	HUSH_TEST(z_varexp_quoted, "-z \"$ut_var_exists\"", n);
	setenv("ut_var_exists", NULL);

	run_command("setenv ut_var_space \" \"", 0);
	assert(!strcmp(getenv("ut_var_space"), " "));
	run_command("setenv ut_var_test $ut_var_space", 0);
	assert(!getenv("ut_var_test"));
	run_command("setenv ut_var_test \"$ut_var_space\"", 0);
	assert(!strcmp(getenv("ut_var_test"), " "));
	run_command("setenv ut_var_test \" 1${ut_var_space}${ut_var_space} 2 \"", 0);
	assert(!strcmp(getenv("ut_var_test"), " 1   2 "));
	setenv("ut_var_space", NULL);
	setenv("ut_var_test", NULL);

#ifdef CONFIG_SANDBOX
	/* File existence */
	HUSH_TEST(e, "-e hostfs - creating_this_file_breaks_uboot_unit_test", n);
	run_command("sb save hostfs - creating_this_file_breaks_uboot_unit_test 0 1", 0);
	HUSH_TEST(e, "-e hostfs - creating_this_file_breaks_uboot_unit_test", y);
	/* Perhaps this could be replaced by an "rm" shell command one day */
	assert(!os_unlink("creating_this_file_breaks_uboot_unit_test"));
	HUSH_TEST(e, "-e hostfs - creating_this_file_breaks_uboot_unit_test", n);
#endif
#endif

	assert(run_command("", 0) == 0);
	assert(run_command(" ", 0) == 0);

	assert(run_command("'", 0) == 1);

	printf("%s: Everything went swimmingly\n", __func__);
	return 0;
}

U_BOOT_CMD(
	ut_cmd,	5,	1,	do_ut_cmd,
	"Very basic test of command parsers",
	""
);

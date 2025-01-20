// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2015
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#include <command.h>
#include <console.h>
#include <vsprintf.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

/**
 * struct suite - A set of tests for a certain topic
 *
 * All tests end up in a single 'struct unit_test' linker-list array, in order
 * of the suite they are in
 *
 * @name: Name of suite
 * @start: First test in suite
 * @end: End test in suite (points to the first test in the next suite)
 * @cmd: Command to use to run the suite
 */
struct suite {
	const char *name;
	struct unit_test *start;
	struct unit_test *end;
	ut_cmd_func cmd;
};

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[]);

static int do_ut_info(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[]);

int cmd_ut_category(const char *name, const char *prefix,
		    struct unit_test *tests, int n_ents,
		    int argc, char *const argv[])
{
	struct unit_test_state uts;
	const char *test_insert = NULL;
	int runs_per_text = 1;
	bool force_run = false;
	int ret;

	while (argc > 1 && *argv[1] == '-') {
		const char *str = argv[1];

		switch (str[1]) {
		case 'r':
			runs_per_text = dectoul(str + 2, NULL);
			break;
		case 'f':
			force_run = true;
			break;
		case 'I':
			test_insert = str + 2;
			break;
		}
		argv++;
		argc--;
	}

	ut_init_state(&uts);
	ret = ut_run_list(&uts, name, prefix, tests, n_ents,
			  cmd_arg1(argc, argv), runs_per_text, force_run,
			  test_insert);
	ut_uninit_state(&uts);

	return ret ? CMD_RET_FAILURE : 0;
}

/* declare linker-list symbols for the start and end of a suite */
#define SUITE_DECL(_name) \
	ll_start_decl(suite_start_ ## _name, struct unit_test, ut_ ## _name); \
	ll_end_decl(suite_end_ ## _name, struct unit_test, ut_ ## _name)

/* declare a test suite which uses a subcommand to run */
#define SUITE_CMD(_name, _cmd_func) { \
	#_name, \
	suite_start_ ## _name, \
	suite_end_ ## _name, \
	_cmd_func, \
	}

/* declare a test suite which can be run directly without a subcommand */
#define SUITE(_name) { \
	#_name, \
	suite_start_ ## _name, \
	suite_end_ ## _name, \
	NULL, \
	}

SUITE_DECL(bdinfo);
SUITE_DECL(bootstd);
SUITE_DECL(cmd);
SUITE_DECL(common);
SUITE_DECL(dm);
SUITE_DECL(env);
SUITE_DECL(exit);
SUITE_DECL(fdt);
SUITE_DECL(font);
SUITE_DECL(optee);
SUITE_DECL(overlay);
SUITE_DECL(lib);
SUITE_DECL(log);
SUITE_DECL(mbr);
SUITE_DECL(mem);
SUITE_DECL(setexpr);
SUITE_DECL(measurement);
SUITE_DECL(bloblist);
SUITE_DECL(bootm);
SUITE_DECL(addrmap);
SUITE_DECL(hush);
SUITE_DECL(loadm);
SUITE_DECL(pci_mps);
SUITE_DECL(seama);
SUITE_DECL(upl);

static struct suite suites[] = {
	SUITE(bdinfo),
#ifdef CONFIG_UT_BOOTSTD
	SUITE_CMD(bootstd, do_ut_bootstd),
#endif
	SUITE(cmd),
	SUITE(common),
	SUITE(dm),
	SUITE(env),
	SUITE(exit),
	SUITE(fdt),
	SUITE(font),
#ifdef CONFIG_UT_OPTEE
	SUITE_CMD(optee, do_ut_optee),
#endif
#ifdef CONFIG_UT_OVERLAY
	SUITE_CMD(overlay, do_ut_overlay),
#endif
	SUITE(lib),
	SUITE(log),
	SUITE(mbr),
	SUITE(mem),
	SUITE(setexpr),
	SUITE(measurement),
	SUITE(bloblist),
	SUITE(bootm),
	SUITE(addrmap),
	SUITE(hush),
	SUITE(loadm),
	SUITE(pci_mps),
	SUITE(seama),
	SUITE(upl),
};

/**
 * has_tests() - Check if a suite has tests, i.e. is supported in this build
 *
 * If the suite is run using a command, we have to assume that tests may be
 * present, since we have no visibility
 *
 * @ste: Suite to check
 * Return: true if supported, false if not
 */
static bool has_tests(struct suite *ste)
{
	int n_ents = ste->end - ste->start;

	return n_ents || ste->cmd;
}

/** run_suite() - Run a suite of tests */
static int run_suite(struct suite *ste, struct cmd_tbl *cmdtp, int flag,
		     int argc, char *const argv[])
{
	int ret;

	if (ste->cmd) {
		ret = ste->cmd(cmdtp, flag, argc, argv);
	} else {
		int n_ents = ste->end - ste->start;
		char prefix[30];

		/* use a standard prefix */
		snprintf(prefix, sizeof(prefix), "%s_test", ste->name);
		ret = cmd_ut_category(ste->name, prefix, ste->start, n_ents,
				      argc, argv);
	}

	return ret;
}

static int do_ut_all(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int i;
	int retval;
	int any_fail = 0;

	for (i = 0; i < ARRAY_SIZE(suites); i++) {
		struct suite *ste = &suites[i];
		char *const argv[] = {(char *)ste->name, NULL};

		if (has_tests(ste)) {
			printf("----Running %s tests----\n", ste->name);
			retval = run_suite(ste, cmdtp, flag, 1, argv);
			if (!any_fail)
				any_fail = retval;
		}
	}

	return any_fail;
}

static int do_ut_info(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	int suite_count, i;
	const char *flags;

	for (suite_count = 0, i = 0; i < ARRAY_SIZE(suites); i++) {
		struct suite *ste = &suites[i];

		if (has_tests(ste))
			suite_count++;
	}

	printf("Test suites: %d\n", suite_count);
	printf("Total tests: %d\n", (int)UNIT_TEST_ALL_COUNT());

	flags = cmd_arg1(argc, argv);
	if (flags && !strcmp("-s", flags)) {
		int i;

		puts("\nTests  Suite\n");
		puts("-----  -----\n");
		for (i = 0; i < ARRAY_SIZE(suites); i++) {
			struct suite *ste = &suites[i];
			long n_ent = ste->end - ste->start;

			if (n_ent)
				printf("%5ld  %s\n", n_ent, ste->name);
			else if (ste->cmd)
				printf("%5s  %s\n", "?", ste->name);
		}
	}

	return 0;
}

static struct suite *find_suite(const char *name)
{
	struct suite *ste;
	int i;

	for (i = 0, ste = suites; i < ARRAY_SIZE(suites); i++, ste++) {
		if (!strcmp(ste->name, name))
			return ste;
	}

	return NULL;
}

static int do_ut(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct suite *ste;
	const char *name;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "ut" arg */
	argc--;
	argv++;

	name = argv[0];
	if (!strcmp(name, "all")) {
		ret = do_ut_all(cmdtp, flag, argc, argv);
	} else if (!strcmp(name, "info")) {
		ret = do_ut_info(cmdtp, flag, argc, argv);
	} else {
		ste = find_suite(argv[0]);
		if (!ste) {
			printf("Suite '%s' not found\n", argv[0]);
			return CMD_RET_FAILURE;
		} else if (!has_tests(ste)) {
			/* perhaps a Kconfig option needs to be set? */
			printf("Suite '%s' is not enabled\n", argv[0]);
			return CMD_RET_FAILURE;
		}

		ret = run_suite(ste, cmdtp, flag, argc, argv);
	}
	if (ret)
		return ret;

	return 0;
}

U_BOOT_LONGHELP(ut,
	"[-r] [-f] [<suite>] - run unit tests\n"
	"   -r<runs>   Number of times to run each test\n"
	"   -f         Force 'manual' tests to run as well\n"
	"   <suite>    Test suite to run, or all\n"
	"\n"
	"\nOptions for <suite>:"
	"\nall - execute all enabled tests"
	"\ninfo [-s] - show info about tests [and suites]"
#ifdef CONFIG_CMD_ADDRMAP
	"\naddrmap - very basic test of addrmap command"
#endif
#ifdef CONFIG_CMD_BDI
	"\nbdinfo - bdinfo command"
#endif
#ifdef CONFIG_SANDBOX
	"\nbloblist - bloblist implementation"
#endif
#ifdef CONFIG_BOOTSTD
	"\nbootstd - standard boot implementation"
#endif
#ifdef CONFIG_CMDLINE
	"\ncmd - test various commands"
#endif
	"\ncommon   - tests for common/ directory"
#ifdef CONFIG_UT_DM
	"\ndm - driver model"
#endif
#ifdef CONFIG_UT_ENV
	"\nenv - environment"
#endif
#ifdef CONFIG_CMD_FDT
	"\nfdt - fdt command"
#endif
#ifdef CONFIG_CONSOLE_TRUETYPE
	"\nfont - font command"
#endif
#if CONFIG_IS_ENABLED(HUSH_PARSER)
	"\nhush - Test hush behavior"
#endif
#ifdef CONFIG_CMD_LOADM
	"\nloadm - loadm command parameters and loading memory blob"
#endif
#ifdef CONFIG_UT_LIB
	"\nlib - library functions"
#endif
#ifdef CONFIG_UT_LOG
	"\nlog - logging functions"
#endif
	"\nmem - memory-related commands"
#ifdef CONFIG_UT_OPTEE
	"\noptee - test OP-TEE"
#endif
#ifdef CONFIG_UT_OVERLAY
	"\noverlay - device tree overlays"
#endif
#ifdef CONFIG_CMD_PCI_MPS
	"\npci_mps - PCI Express Maximum Payload Size"
#endif
	"\nsetexpr - setexpr command"
#ifdef CONFIG_CMD_SEAMA
	"\nseama - seama command parameters loading and decoding"
#endif
	);

U_BOOT_CMD(
	ut, CONFIG_SYS_MAXARGS, 1, do_ut,
	"unit tests", ut_help_text
);

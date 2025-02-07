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
 * @help: Help-string to show for this suite
 */
struct suite {
	const char *name;
	struct unit_test *start;
	struct unit_test *end;
	ut_cmd_func cmd;
	const char *help;
};

static int do_ut_all(struct unit_test_state *uts, struct cmd_tbl *cmdtp,
		     int flag, int argc, char *const argv[]);

static int do_ut_info(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[]);

int cmd_ut_category(struct unit_test_state *uts, const char *name,
		    const char *prefix, struct unit_test *tests, int n_ents,
		    int argc, char *const argv[])
{
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

	ret = ut_run_list(uts, name, prefix, tests, n_ents,
			  cmd_arg1(argc, argv), runs_per_text, force_run,
			  test_insert);

	return ret ? CMD_RET_FAILURE : 0;
}

/* declare linker-list symbols for the start and end of a suite */
#define SUITE_DECL(_name) \
	ll_start_decl(suite_start_ ## _name, struct unit_test, ut_ ## _name); \
	ll_end_decl(suite_end_ ## _name, struct unit_test, ut_ ## _name)

/* declare a test suite which uses a subcommand to run */
#define SUITE_CMD(_name, _cmd_func, _help) { \
	#_name, \
	suite_start_ ## _name, \
	suite_end_ ## _name, \
	_cmd_func, \
	_help, \
	}

/* declare a test suite which can be run directly without a subcommand */
#define SUITE(_name, _help) { \
	#_name, \
	suite_start_ ## _name, \
	suite_end_ ## _name, \
	NULL, \
	_help, \
	}

SUITE_DECL(addrmap);
SUITE_DECL(bdinfo);
SUITE_DECL(bloblist);
SUITE_DECL(bootm);
SUITE_DECL(bootstd);
SUITE_DECL(cmd);
SUITE_DECL(common);
SUITE_DECL(dm);
SUITE_DECL(env);
SUITE_DECL(exit);
SUITE_DECL(fdt);
SUITE_DECL(fdt_overlay);
SUITE_DECL(font);
SUITE_DECL(hush);
SUITE_DECL(lib);
SUITE_DECL(loadm);
SUITE_DECL(log);
SUITE_DECL(mbr);
SUITE_DECL(measurement);
SUITE_DECL(mem);
SUITE_DECL(optee);
SUITE_DECL(pci_mps);
SUITE_DECL(seama);
SUITE_DECL(setexpr);
SUITE_DECL(upl);

static struct suite suites[] = {
	SUITE(addrmap, "very basic test of addrmap command"),
	SUITE(bdinfo, "bdinfo (board info) command"),
	SUITE(bloblist, "bloblist implementation"),
	SUITE(bootm, "bootm command"),
#ifdef CONFIG_UT_BOOTSTD
	SUITE_CMD(bootstd, do_ut_bootstd, "standard boot implementation"),
#endif
	SUITE(cmd, "various commands"),
	SUITE(common, "tests for common/ directory"),
	SUITE(dm, "driver model"),
	SUITE(env, "environment"),
	SUITE(exit, "shell exit and variables"),
	SUITE(fdt, "fdt command"),
	SUITE(fdt_overlay, "device tree overlays"),
	SUITE(font, "font command"),
	SUITE(hush, "hush behaviour"),
	SUITE(lib, "library functions"),
	SUITE(loadm, "loadm command parameters and loading memory blob"),
	SUITE(log, "logging functions"),
	SUITE(mbr, "mbr command"),
	SUITE(measurement, "TPM-based measured boot"),
	SUITE(mem, "memory-related commands"),
#ifdef CONFIG_UT_OPTEE
	SUITE_CMD(optee, do_ut_optee, "OP-TEE"),
#endif
	SUITE(pci_mps, "PCI Express Maximum Payload Size"),
	SUITE(seama, "seama command parameters loading and decoding"),
	SUITE(setexpr, "setexpr command"),
	SUITE(upl, "Universal payload support"),
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
static int run_suite(struct unit_test_state *uts, struct suite *ste,
		     struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int ret;

	if (ste->cmd) {
		ret = ste->cmd(uts, cmdtp, flag, argc, argv);
	} else {
		int n_ents = ste->end - ste->start;
		char prefix[30];

		/* use a standard prefix */
		snprintf(prefix, sizeof(prefix), "%s_test_", ste->name);
		ret = cmd_ut_category(uts, ste->name, prefix, ste->start,
				      n_ents, argc, argv);
	}

	return ret;
}

static void show_stats(struct unit_test_state *uts)
{
	if (uts->run_count < 2)
		return;

	ut_report(&uts->total, uts->run_count);
	if (CONFIG_IS_ENABLED(UNIT_TEST_DURATION) &&
	    uts->total.test_count && uts->worst) {
		ulong avg = uts->total.duration_ms / uts->total.test_count;

		printf("Average test time: %ld ms, worst case '%s' took %d ms\n",
		       avg, uts->worst->name, uts->worst_ms);
	}
}

static void update_stats(struct unit_test_state *uts, const struct suite *ste)
{
	if (CONFIG_IS_ENABLED(UNIT_TEST_DURATION) && uts->cur.test_count) {
		ulong avg;

		avg = uts->cur.duration_ms ?
			uts->cur.duration_ms /
			uts->cur.test_count : 0;
		if (avg > uts->worst_ms) {
			uts->worst_ms = avg;
			uts->worst = ste;
		}
	}
}

static int do_ut_all(struct unit_test_state *uts, struct cmd_tbl *cmdtp,
		     int flag, int argc, char *const argv[])
{
	int i;
	int retval;
	int any_fail = 0;

	for (i = 0; i < ARRAY_SIZE(suites); i++) {
		struct suite *ste = &suites[i];
		char *const argv[] = {(char *)ste->name, NULL};

		if (has_tests(ste)) {
			printf("----Running %s tests----\n", ste->name);
			retval = run_suite(uts, ste, cmdtp, flag, 1, argv);
			if (!any_fail)
				any_fail = retval;
			update_stats(uts, ste);
		}
	}
	ut_report(&uts->total, uts->run_count);

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
		int i, total;

		puts("\nTests  Suite         Purpose");
		puts("\n-----  ------------  -------------------------\n");
		for (i = 0, total = 0; i < ARRAY_SIZE(suites); i++) {
			struct suite *ste = &suites[i];
			long n_ent = ste->end - ste->start;

			if (n_ent)
				printf("%5ld", n_ent);
			else if (ste->cmd)
				printf("%5s", "?");
			else  /* suite is not present */
				continue;
			printf("  %-13.13s %s\n", ste->name, ste->help);
			total += n_ent;
		}
		puts("-----  ------------  -------------------------\n");
		printf("%5d  %-13.13s\n", total, "Total");

		if (UNIT_TEST_ALL_COUNT() != total)
			puts("Error: Suite test-count does not match total\n");
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
	struct unit_test_state uts;
	struct suite *ste;
	const char *name;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* drop initial "ut" arg */
	argc--;
	argv++;

	ut_init_state(&uts);
	name = argv[0];
	if (!strcmp(name, "all")) {
		ret = do_ut_all(&uts, cmdtp, flag, argc, argv);
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

		ret = run_suite(&uts, ste, cmdtp, flag, argc, argv);
	}
	show_stats(&uts);
	if (ret)
		return ret;
	ut_uninit_state(&uts);

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
	);

U_BOOT_CMD(
	ut, CONFIG_SYS_MAXARGS, 1, do_ut,
	"unit tests", ut_help_text
);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for bootm routines
 *
 * Copyright 2020 Google LLC
 */

#include <common.h>
#include <bootm.h>
#include <test/suites.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

#define BOOTM_TEST(_name, _flags)	UNIT_TEST(_name, _flags, bootm_test)

#define CONSOLE_STR	"console=/dev/ttyS0"

/* Test silent processing in the bootargs variable */
static int bootm_test_silent_var(struct unit_test_state *uts)
{
	/* 'silent_linux' not set should do nothing */
	env_set("silent_linux", NULL);
	env_set("bootargs", CONSOLE_STR);
	ut_assertok(fixup_silent_linux());
	ut_asserteq_str(CONSOLE_STR, env_get("bootargs"));

	env_set("bootargs", NULL);
	ut_assertok(fixup_silent_linux());
	ut_assertnull(env_get("bootargs"));

	ut_assertok(env_set("silent_linux", "no"));
	env_set("bootargs", CONSOLE_STR);
	ut_assertok(fixup_silent_linux());
	ut_asserteq_str(CONSOLE_STR, env_get("bootargs"));

	ut_assertok(env_set("silent_linux", "yes"));
	env_set("bootargs", CONSOLE_STR);
	ut_assertok(fixup_silent_linux());
	ut_asserteq_str("console=", env_get("bootargs"));

	/* Empty buffer should still add the string */
	env_set("bootargs", NULL);
	ut_assertok(fixup_silent_linux());
	ut_asserteq_str("console=", env_get("bootargs"));

	return 0;
}
BOOTM_TEST(bootm_test_silent_var, 0);

int do_ut_bootm(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, bootm_test);
	const int n_ents = ll_entry_count(struct unit_test, bootm_test);

	return cmd_ut_category("bootm", "bootm_test_", tests, n_ents,
			       argc, argv);
}

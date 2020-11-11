// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * Logging function tests.
 */

#include <common.h>
#include <console.h>
#include <log.h>
#include <test/log.h>
#include <test/suites.h>

int do_ut_log(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, log_test);
	const int n_ents = ll_entry_count(struct unit_test, log_test);

	return cmd_ut_category("log", "log_test_",
			       tests, n_ents, argc, argv);
}

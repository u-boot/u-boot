// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Texas Instruments Incorporated - https://www.ti.com/
 *	Anshul Dalal <anshuld@ti.com>
 *
 */

 #include <command.h>
 #include <test/ut.h>
 #include <test/hush.h>

#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int hush_test_negation(struct unit_test_state *uts)
{
	if (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		ut_fail(uts, __FILE__, __LINE__, __func__,
			"Negation '!' is only available in modern parser");
		return CMD_RET_FAILURE;
	}

	ut_asserteq(0, run_command("! false", 0));
	ut_asserteq(1, run_command("! true", 0));

	return 0;
}

HUSH_TEST(hush_test_negation, 0);

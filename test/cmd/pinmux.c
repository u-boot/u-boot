// SPDX-License-Identifier: GPL-2.0+
/*
 * Executes tests for pinmux command
 *
 * Copyright (C) 2021, STMicroelectronics - All Rights Reserved
 */

#include <command.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_cmd_pinmux_status_pinname(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_LED, 2, &dev));

	/* Test that 'pinmux status <pinname>' displays the selected pin. */
	run_command("pinmux status a5", 0);
	ut_assert_nextlinen("a5        : gpio output .");
	ut_assert_console_end();

	run_command("pinmux status P7", 0);
	ut_assert_nextlinen("P7        : GPIO2 bias-pull-down input-enable.");
	ut_assert_console_end();

	run_command("pinmux status P9", 0);
	if (IS_ENABLED(CONFIG_LOGF_FUNC)) {
		ut_assert_nextlinen(
			"   single_of_to_plat() single-pinctrl pinctrl-single-no-width: missing register width");
	} else {
		ut_assert_nextlinen(
			"single-pinctrl pinctrl-single-no-width: missing register width");
	}
	ut_assert_nextlinen("P9 not found");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cmd_pinmux_status_pinname, UTF_SCAN_PDATA | UTF_SCAN_FDT |
	UTF_CONSOLE);

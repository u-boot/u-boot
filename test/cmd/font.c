// SPDX-License-Identifier: GPL-2.0+
/*
 * Tests for font command
 *
 * Copyright 2022 Google LLC
 */

#include <console.h>
#include <dm.h>
#include <video_console.h>
#include <test/ut.h>

/* Declare a new fdt test */
#define FONT_TEST(_name, _flags)	UNIT_TEST(_name, _flags, font)

/* Test 'fdt addr' resizing an fdt */
static int font_test_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	const char *name;
	int max_metrics;
	uint size;
	int ret;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	ut_assertok(uclass_first_device_err(UCLASS_VIDEO_CONSOLE, &dev));

	ut_assertok(run_command("font list", 0));
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE_NIMBUS))
		ut_assert_nextline("nimbus_sans_l_regular");
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE_ANKACODER))
		ut_assert_nextline("ankacoder_c75_r");
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE_CANTORAONE))
		ut_assert_nextline("cantoraone_regular");
	ut_assert_console_end();

	ut_assertok(vidconsole_get_font_size(dev, &name, &size));
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE_ANKACODER))
		ut_asserteq_str("ankacoder_c75_r", name);
	else
		ut_asserteq_str("nimbus_sans_l_regular", name);
	ut_asserteq(CONFIG_CONSOLE_TRUETYPE_SIZE, size);

	if (!IS_ENABLED(CONFIG_CONSOLE_TRUETYPE_CANTORAONE))
		return 0;

	max_metrics = 1;
	if (IS_ENABLED(CONFIG_CONSOLE_TRUETYPE))
		max_metrics = IF_ENABLED_INT(CONFIG_CONSOLE_TRUETYPE,
				     CONFIG_CONSOLE_TRUETYPE_MAX_METRICS);

	ret = run_command("font select cantoraone_regular 40", 0);
	if (max_metrics < 2) {
		ut_asserteq(1, ret);
		ut_assert_nextline("Failed (error -7)");
		ut_assert_console_end();
		return 0;
	}

	ut_assertok(ret);
	ut_assert_console_end();

	ut_assertok(vidconsole_get_font_size(dev, &name, &size));
	ut_asserteq_str("cantoraone_regular", name);
	ut_asserteq(40, size);
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(run_command("font size", 0));
	ut_assert_nextline("40");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(run_command("font size 30", 0));
	ut_assert_console_end();

	ut_assertok(run_command("font size", 0));
	ut_assert_nextline("30");
	ut_assertok(ut_check_console_end(uts));

	ut_assertok(vidconsole_get_font_size(dev, &name, &size));
	ut_asserteq_str("cantoraone_regular", name);
	ut_asserteq(30, size);

	return 0;
}
FONT_TEST(font_test_base, UTF_SCAN_PDATA | UTF_SCAN_FDT | UTF_CONSOLE |
	  UTF_DM);

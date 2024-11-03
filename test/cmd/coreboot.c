// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for coreboot commands
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <cedit.h>
#include <command.h>
#include <dm.h>
#include <expo.h>
#include <rtc.h>
#include <test/cedit-test.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>
#include "../../boot/scene_internal.h"

enum {
	CSUM_LOC	= 0x3f0 / 8,
};

/**
 * test_cmd_cbsysinfo() - test the cbsysinfo command produces expected output
 *
 * This includes ensuring that the coreboot build has the expected options
 * enabled
 */
static int test_cmd_cbsysinfo(struct unit_test_state *uts)
{
	ut_assertok(run_command("cbsysinfo", 0));
	ut_assert_nextlinen("Coreboot table at");

	/* Make sure CMOS options are enabled */
	ut_assert_skip_to_line(
		" 1c0    1    e   1  power_on_after_fail    0:Disable 1:Enable");
	ut_assert_skip_to_line("CMOS start  : 1c0");
	ut_assert_nextline("   CMOS end    : 1cf");
	ut_assert_nextline("   CMOS csum loc: 3f0");

	/* Make sure the linear frame buffer is enabled */
	ut_assert_skip_to_linen("Framebuffer");
	ut_assert_nextlinen("   Phys addr");

	ut_assert_skip_to_line("Chrome OS VPD: 00000000");
	ut_assert_nextlinen("RSDP");
	ut_assert_nextlinen("Unimpl.");
	ut_assert_console_end();

	return 0;
}
CMD_TEST(test_cmd_cbsysinfo, UTF_CONSOLE);

/* test cbcmos command */
static int test_cmd_cbcmos(struct unit_test_state *uts)
{
	u16 old_csum, new_csum;
	struct udevice *dev;

	/* initially the checksum should be correct */
	ut_assertok(run_command("cbcmos check", 0));
	ut_assert_console_end();

	/* make a change to the checksum */
	ut_assertok(uclass_first_device_err(UCLASS_RTC, &dev));
	ut_assertok(rtc_read16(dev, CSUM_LOC, &old_csum));
	ut_assertok(rtc_write16(dev, CSUM_LOC, old_csum + 1));

	/* now the command should fail */
	ut_asserteq(1, run_command("cbcmos check", 0));
	ut_assert_nextline("Checksum %04x error: calculated %04x",
			   old_csum + 1, old_csum);
	ut_assert_console_end();

	/* now get it to fix the checksum */
	ut_assertok(run_command("cbcmos update", 0));
	ut_assert_nextline("Checksum %04x written", old_csum);
	ut_assert_console_end();

	/* check the RTC looks right */
	ut_assertok(rtc_read16(dev, CSUM_LOC, &new_csum));
	ut_asserteq(old_csum, new_csum);
	ut_assert_console_end();

	return 0;
}
CMD_TEST(test_cmd_cbcmos, UTF_CONSOLE);

/* test 'cedit cb_load' command */
static int test_cmd_cedit_cb_load(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct video_priv *vid_priv;
	struct scene_obj_txt *txt;
	struct scene *scn;
	struct expo *exp;
	int scn_id;

	ut_assertok(run_command("cedit cb_load", 0));
	ut_assertok(run_command("cedit read_cmos", 0));
	ut_assert_console_end();

	exp = cur_exp;
	scn_id = cedit_prepare(exp, &vid_priv, &scn);
	ut_assert(scn_id > 0);
	ut_assertnonnull(scn);

	/* just do a very basic test that the first menu is present */
	menu = scene_obj_find(scn, scn->highlight_id, SCENEOBJT_NONE);
	ut_assertnonnull(menu);

	txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_NONE);
	ut_assertnonnull(txt);
	ut_asserteq_str("Boot option", expo_get_str(exp, txt->str_id));

	return 0;
}
CMD_TEST(test_cmd_cedit_cb_load, UTF_CONSOLE);

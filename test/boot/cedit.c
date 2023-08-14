// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cedit.h>
#include <expo.h>
#include <test/ut.h>
#include "bootstd_common.h"
#include <test/cedit-test.h>
#include "../../boot/scene_internal.h"

/* Check the cedit command */
static int cedit_base(struct unit_test_state *uts)
{
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	struct scene_obj_txt *txt;
	struct expo *exp;
	struct scene *scn;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	console_record_reset_enable();

	/*
	 * ^N  Move down to second menu
	 * ^M  Open menu
	 * ^N  Move down to second item
	 * ^M  Select item
	 * \e  Quit
	 */
	console_in_puts("\x0e\x0d\x0e\x0d\e");
	ut_assertok(run_command("cedit run", 0));

	exp = cur_exp;
	scn = expo_lookup_scene_id(exp, exp->scene_id);
	ut_assertnonnull(scn);

	menu = scene_obj_find(scn, scn->highlight_id, SCENEOBJT_NONE);
	ut_assertnonnull(menu);

	txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_NONE);
	ut_assertnonnull(txt);
	ut_asserteq_str("AC Power", expo_get_str(exp, txt->str_id));

	ut_asserteq(ID_AC_ON, menu->cur_item_id);

	return 0;
}
BOOTSTD_TEST(cedit_base, 0);

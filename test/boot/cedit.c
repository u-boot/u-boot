// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cedit.h>
#include <expo.h>
#include <mapmem.h>
#include <dm/ofnode.h>
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

/* Check the cedit write_fdt commands */
static int cedit_fdt(struct unit_test_state *uts)
{
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	ulong addr = 0x1000;
	struct ofprop prop;
	struct scene *scn;
	oftree tree;
	ofnode node;
	void *fdt;
	int i;

	console_record_reset_enable();
	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, &vid_priv, &scn));

	ut_assertok(run_command("cedit write_fdt hostfs - settings.dtb", 0));
	ut_assertok(run_commandf("load hostfs - %lx settings.dtb", addr));
	ut_assert_nextlinen("1024 bytes read");

	fdt = map_sysmem(addr, 1024);
	tree = oftree_from_fdt(fdt);
	node = ofnode_find_subnode(oftree_root(tree), CEDIT_NODE_NAME);

	ut_asserteq(ID_CPU_SPEED_1,
		    ofnode_read_u32_default(node, "cpu-speed", 0));
	ut_asserteq_str("2 GHz", ofnode_read_string(node, "cpu-speed-str"));
	ut_assert(ofnode_valid(node));

	/* There should only be 4 properties */
	for (i = 0, ofnode_first_property(node, &prop); ofprop_valid(&prop);
	     i++, ofnode_next_property(&prop))
		;
	ut_asserteq(4, i);

	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(cedit_fdt, 0);

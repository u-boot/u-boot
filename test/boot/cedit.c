// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <cedit.h>
#include <dm.h>
#include <env.h>
#include <expo.h>
#include <mapmem.h>
#include <dm/ofnode.h>
#include <test/ut.h>
#include <test/video.h>
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

	/*
	 * ^N  Move down to second menu
	 * ^M  Open menu
	 * ^N  Move down to second item
	 * ^M  Select item
	 * \e  Quit
	 *
	 * cedit_run() returns -EACCESS so this command returns CMD_RET_FAILURE
	 */
	console_in_puts("\x0e\x0d\x0e\x0d\e");
	ut_asserteq(1, run_command("cedit run", 0));

	exp = cur_exp;
	scn = expo_lookup_scene_id(exp, exp->scene_id);
	ut_assertnonnull(scn);

	menu = scene_obj_find(scn, scn->highlight_id, SCENEOBJT_NONE);
	ut_assertnonnull(menu);

	txt = scene_obj_find(scn, menu->title_id, SCENEOBJT_NONE);
	ut_assertnonnull(txt);
	ut_asserteq_str("AC Power", expo_get_str(exp, txt->gen.str_id));

	ut_asserteq(ID_AC_ON, menu->cur_item_id);

	return 0;
}
BOOTSTD_TEST(cedit_base, UTF_CONSOLE);

/* Check the cedit write_fdt and read_fdt commands */
static int cedit_fdt(struct unit_test_state *uts)
{
	struct scene_obj_textline *tline;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	struct udevice *dev;
	ulong addr = 0x1000;
	struct ofprop prop;
	struct scene *scn;
	oftree tree;
	ofnode node;
	char *str;
	void *fdt;
	int i;

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	vid_priv = dev_get_uclass_priv(dev);

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, dev, &scn));

	/* get a menu to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	/* get a textline to fiddle with too */
	tline = scene_obj_find(scn, ID_MACHINE_NAME, SCENEOBJT_TEXTLINE);
	ut_assertnonnull(tline);
	str = abuf_data(&tline->buf);
	strcpy(str, "my-machine");

	ut_assertok(run_command("cedit write_fdt hostfs - settings.dtb", 0));
	ut_assertok(run_commandf("load hostfs - %lx settings.dtb", addr));
	ut_assert_nextlinen("1024 bytes read");

	fdt = map_sysmem(addr, 1024);
	tree = oftree_from_fdt(fdt);
	node = ofnode_find_subnode(oftree_root(tree), CEDIT_NODE_NAME);
	ut_assert(ofnode_valid(node));

	ut_asserteq(ID_CPU_SPEED_2,
		    ofnode_read_u32_default(node, "cpu-speed", 0));
	ut_asserteq(3,
		    ofnode_read_u32_default(node, "cpu-speed-value", 0));
	ut_asserteq_str("2.5 GHz", ofnode_read_string(node, "cpu-speed-str"));
	ut_asserteq_str("my-machine", ofnode_read_string(node, "machine-name"));

	/* There should only be 7 properties */
	for (i = 0, ofnode_first_property(node, &prop); ofprop_valid(&prop);
	     i++, ofnode_next_property(&prop))
		;
	ut_asserteq(7, i);

	ut_assert_console_end();

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	*str = '\0';

	/* load in the settings and make sure they update */
	ut_assertok(run_command("cedit read_fdt hostfs - settings.dtb", 0));
	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq_str("my-machine", ofnode_read_string(node, "machine-name"));

	ut_assertnonnull(menu);
	ut_assert_console_end();

	return 0;
}
BOOTSTD_TEST(cedit_fdt, UTF_CONSOLE);

/* Check the cedit write_env and read_env commands */
static int cedit_env(struct unit_test_state *uts)
{
	struct scene_obj_textline *tline;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct scene_obj_menu *menu;
	struct udevice *dev;
	struct scene *scn;
	char *str;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	vid_priv = dev_get_uclass_priv(dev);

	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, dev, &scn));

	/* get a menu to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	/* get a textline to fiddle with too */
	tline = scene_obj_find(scn, ID_MACHINE_NAME, SCENEOBJT_TEXTLINE);
	ut_assertnonnull(tline);
	str = abuf_data(&tline->buf);
	strcpy(str, "my-machine");

	ut_assertok(run_command("cedit write_env -v", 0));
	ut_assert_nextlinen("c.cpu-speed=11");
	ut_assert_nextlinen("c.cpu-speed-str=2.5 GHz");
	ut_assert_nextlinen("c.cpu-speed-value=3");
	ut_assert_nextlinen("c.power-loss=14");
	ut_assert_nextlinen("c.power-loss-str=Always Off");
	ut_assert_nextlinen("c.power-loss-value=0");
	ut_assert_nextlinen("c.machine-name=my-machine");
	ut_assert_console_end();

	ut_asserteq(11, env_get_ulong("c.cpu-speed", 10, 0));
	ut_asserteq_str("2.5 GHz", env_get("c.cpu-speed-str"));
	ut_asserteq_str("my-machine", env_get("c.machine-name"));

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	*str = '\0';

	ut_assertok(run_command("cedit read_env -v", 0));
	ut_assert_nextlinen("c.cpu-speed=11");
	ut_assert_nextlinen("c.power-loss=14");
	ut_assert_nextlinen("c.machine-name=my-machine");
	ut_assert_console_end();

	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq_str("my-machine", env_get("c.machine-name"));

	return 0;
}
BOOTSTD_TEST(cedit_env, UTF_CONSOLE);

/* Check the cedit write_cmos and read_cmos commands */
static int cedit_cmos(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu, *menu2;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct udevice *dev;
	struct scene *scn;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	ut_assertok(uclass_first_device_err(UCLASS_VIDEO, &dev));
	vid_priv = dev_get_uclass_priv(dev);
	ut_asserteq(ID_SCENE1, cedit_prepare(cur_exp, dev, &scn));

	/* get the menus to fiddle with */
	menu = scene_obj_find(scn, ID_CPU_SPEED, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	menu->cur_item_id = ID_CPU_SPEED_2;

	menu2 = scene_obj_find(scn, ID_POWER_LOSS, SCENEOBJT_MENU);
	ut_assertnonnull(menu2);
	menu2->cur_item_id = ID_AC_MEMORY;

	ut_assertok(run_command("cedit write_cmos -v", 0));
	ut_assert_nextlinen("Write 2 bytes from offset 80 to 84");
	ut_assert_console_end();

	/* reset the expo */
	menu->cur_item_id = ID_CPU_SPEED_1;
	menu2->cur_item_id = ID_AC_OFF;

	ut_assertok(run_command("cedit read_cmos -v", 0));
	ut_assert_nextlinen("Read 2 bytes from offset 80 to 84");
	ut_assert_console_end();

	ut_asserteq(ID_CPU_SPEED_2, menu->cur_item_id);
	ut_asserteq(ID_AC_MEMORY, menu2->cur_item_id);

	return 0;
}
BOOTSTD_TEST(cedit_cmos, UTF_CONSOLE);

/* Check the cedit displays correctly */
static int cedit_render(struct unit_test_state *uts)
{
	struct scene_obj_menu *menu;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct expo_action evt;
	struct expo_action act;
	struct udevice *dev, *con;
	struct stdio_dev *sdev;
	struct scene *scn;
	struct expo *exp;
	int i;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	exp = cur_exp;
	sdev = stdio_get_by_name("vidconsole");
	ut_assertnonnull(sdev);
	con = sdev->priv;

	dev = dev_get_parent(con);
	vid_priv = dev_get_uclass_priv(dev);
	ut_asserteq(ID_SCENE1, cedit_prepare(exp, dev, &scn));

	menu = scene_obj_find(scn, ID_POWER_LOSS, SCENEOBJT_MENU);
	ut_assertnonnull(menu);
	ut_asserteq(ID_AC_OFF, menu->cur_item_id);

	ut_assertok(expo_render(exp));
	ut_asserteq(4929, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* move to the second menu */
	act.type = EXPOACT_POINT_OBJ;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4986, video_compress_fb(uts, dev, false));

	/* open the menu */
	act.type = EXPOACT_OPEN;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(5393, video_compress_fb(uts, dev, false));

	/* close the menu */
	act.type = EXPOACT_CLOSE;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4986, video_compress_fb(uts, dev, false));

	/* open the menu again to check it looks the same */
	act.type = EXPOACT_OPEN;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(5393, video_compress_fb(uts, dev, false));

	/* close the menu */
	act.type = EXPOACT_CLOSE;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4986, video_compress_fb(uts, dev, false));

	act.type = EXPOACT_OPEN;
	act.select.id = ID_POWER_LOSS;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(5393, video_compress_fb(uts, dev, false));

	act.type = EXPOACT_POINT_ITEM;
	act.select.id = ID_AC_ON;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(5365, video_compress_fb(uts, dev, false));

	/* select it */
	act.type = EXPOACT_SELECT;
	act.select.id = ID_AC_ON;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4980, video_compress_fb(uts, dev, false));

	ut_asserteq(ID_AC_ON, menu->cur_item_id);

	/* move to the line-edit field */
	act.type = EXPOACT_POINT_OBJ;
	act.select.id = ID_MACHINE_NAME;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4862, video_compress_fb(uts, dev, false));

	/* open it */
	act.type = EXPOACT_OPEN;
	act.select.id = ID_MACHINE_NAME;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(4851, video_compress_fb(uts, dev, false));

	/*
	 * Send some keypresses. Note that the console must be enabled so that
	 * the characters actually reach the putc_xy() in console_truetype,
	 * since in scene_textline_send_key(), the lineedit restores the
	 * vidconsole state, outputs the character and then saves the state
	 * again. If the character is never output, then the state won't be
	 * updated and the lineedit will be inconsistent.
	 */
	ut_unsilence_console(uts);
	for (i = 'a'; i < 'd'; i++)
		ut_assertok(scene_send_key(scn, i, &evt));
	ut_silence_console(uts);
	ut_assertok(cedit_arange(exp, vid_priv, scn->id));
	ut_assertok(expo_render(exp));
	ut_asserteq(4996, video_compress_fb(uts, dev, false));

	expo_destroy(exp);
	cur_exp = NULL;

	return 0;
}
BOOTSTD_TEST(cedit_render, UTF_DM | UTF_SCAN_FDT);

/* Check the cedit displays lineedits correctly */
static int cedit_render_lineedit(struct unit_test_state *uts)
{
	struct scene_obj_textline *tline;
	struct video_priv *vid_priv;
	extern struct expo *cur_exp;
	struct expo_action evt;
	struct expo_action act;
	struct udevice *dev, *con;
	struct stdio_dev *sdev;
	struct scene *scn;
	struct expo *exp;
	char *str;
	int i;

	ut_assertok(run_command("cedit load hostfs - cedit.dtb", 0));

	exp = cur_exp;
	sdev = stdio_get_by_name("vidconsole");
	ut_assertnonnull(sdev);
	con = sdev->priv;

	dev = dev_get_parent(con);
	vid_priv = dev_get_uclass_priv(dev);
	ut_asserteq(ID_SCENE1, cedit_prepare(exp, dev, &scn));

	/* set up an initial value for the textline */
	tline = scene_obj_find(scn, ID_MACHINE_NAME, SCENEOBJT_TEXTLINE);
	ut_assertnonnull(tline);
	str = abuf_data(&tline->buf);
	strcpy(str, "my-machine");
	ut_asserteq(20, tline->pos);

	ut_assertok(expo_render(exp));
	ut_asserteq(5336, video_compress_fb(uts, dev, false));
	ut_assertok(video_check_copy_fb(uts, dev));

	/* move to the line-edit field */
	act.type = EXPOACT_POINT_OBJ;
	act.select.id = ID_MACHINE_NAME;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	ut_assertok(expo_render(exp));
	ut_asserteq(5363, video_compress_fb(uts, dev, false));

	/* open it */
	act.type = EXPOACT_OPEN;
	act.select.id = ID_MACHINE_NAME;
	ut_assertok(cedit_do_action(exp, scn, vid_priv, &act));
	// ut_asserteq(0, tline->pos);
	ut_assertok(expo_render(exp));
	ut_asserteq(5283, video_compress_fb(uts, dev, false));

	/* delete some characters */
	ut_unsilence_console(uts);
	for (i = 0; i < 3; i++)
		ut_assertok(scene_send_key(scn, '\b', &evt));
	ut_silence_console(uts);
	ut_asserteq_str("my-mach", str);

	ut_assertok(cedit_arange(exp, vid_priv, scn->id));
	ut_assertok(expo_render(exp));
	ut_asserteq(5170, video_compress_fb(uts, dev, false));

	expo_destroy(exp);
	cur_exp = NULL;

	return 0;
}
BOOTSTD_TEST(cedit_render_lineedit, UTF_DM | UTF_SCAN_FDT);

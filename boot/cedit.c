// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of configuration editor
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cli.h>
#include <dm.h>
#include <expo.h>
#include <menu.h>
#include <video.h>
#include <linux/delay.h>
#include "scene_internal.h"

int cedit_arange(struct expo *exp, struct video_priv *vpriv, uint scene_id)
{
	struct scene_obj_txt *txt;
	struct scene_obj *obj;
	struct scene *scn;
	int y;

	scn = expo_lookup_scene_id(exp, scene_id);
	if (!scn)
		return log_msg_ret("scn", -ENOENT);

	txt = scene_obj_find_by_name(scn, "prompt");
	if (txt)
		scene_obj_set_pos(scn, txt->obj.id, 0, vpriv->ysize - 50);

	txt = scene_obj_find_by_name(scn, "title");
	if (txt)
		scene_obj_set_pos(scn, txt->obj.id, 200, 10);

	y = 100;
	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->type == SCENEOBJT_MENU) {
			scene_obj_set_pos(scn, obj->id, 50, y);
			scene_menu_arrange(scn, (struct scene_obj_menu *)obj);
			y += 50;
		}
	}

	return 0;
}

int cedit_run(struct expo *exp)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct video_priv *vid_priv;
	uint scene_id;
	struct udevice *dev;
	struct scene *scn;
	bool done;
	int ret;

	cli_ch_init(cch);

	/* For now we only support a video console */
	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (ret)
		return log_msg_ret("vid", ret);
	ret = expo_set_display(exp, dev);
	if (ret)
		return log_msg_ret("dis", ret);

	ret = expo_first_scene_id(exp);
	if (ret < 0)
		return log_msg_ret("scn", ret);
	scene_id = ret;

	ret = expo_set_scene_id(exp, scene_id);
	if (ret)
		return log_msg_ret("sid", ret);

	exp->popup = true;

	/* This is not supported for now */
	if (0)
		expo_set_text_mode(exp, true);

	vid_priv = dev_get_uclass_priv(dev);

	scn = expo_lookup_scene_id(exp, scene_id);
	scene_highlight_first(scn);

	cedit_arange(exp, vid_priv, scene_id);

	ret = expo_calc_dims(exp);
	if (ret)
		return log_msg_ret("dim", ret);

	done = false;
	do {
		struct expo_action act;
		int ichar, key;

		ret = expo_render(exp);
		if (ret)
			break;

		ichar = cli_ch_process(cch, 0);
		if (!ichar) {
			while (!ichar && !tstc()) {
				schedule();
				mdelay(2);
				ichar = cli_ch_process(cch, -ETIMEDOUT);
			}
			if (!ichar) {
				ichar = getchar();
				ichar = cli_ch_process(cch, ichar);
			}
		}

		key = 0;
		if (ichar) {
			key = bootmenu_conv_key(ichar);
			if (key == BKEY_NONE)
				key = ichar;
		}
		if (!key)
			continue;

		ret = expo_send_key(exp, key);
		if (ret)
			break;

		ret = expo_action_get(exp, &act);
		if (!ret) {
			switch (act.type) {
			case EXPOACT_POINT_OBJ:
				scene_set_highlight_id(scn, act.select.id);
				cedit_arange(exp, vid_priv, scene_id);
				break;
			case EXPOACT_OPEN:
				scene_set_open(scn, act.select.id, true);
				cedit_arange(exp, vid_priv, scene_id);
				break;
			case EXPOACT_CLOSE:
				scene_set_open(scn, act.select.id, false);
				cedit_arange(exp, vid_priv, scene_id);
				break;
			case EXPOACT_SELECT:
				scene_set_open(scn, scn->highlight_id, false);
				cedit_arange(exp, vid_priv, scene_id);
				break;
			case EXPOACT_QUIT:
				log_debug("quitting\n");
				done = true;
				break;
			default:
				break;
			}
		}
	} while (!done);

	if (ret)
		return log_msg_ret("end", ret);

	return 0;
}

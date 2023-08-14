// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of configuration editor
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY LOGC_EXPO

#include <common.h>
#include <abuf.h>
#include <cedit.h>
#include <cli.h>
#include <dm.h>
#include <env.h>
#include <expo.h>
#include <menu.h>
#include <video.h>
#include <linux/delay.h>
#include "scene_internal.h"

/**
 * struct cedit_iter_priv - private data for cedit operations
 *
 * @buf: Buffer to use when writing settings to the devicetree
 * @node: Node to read from when reading settings from devicetree
 * @verbose: true to show writing to environment variables
 */
struct cedit_iter_priv {
	struct abuf *buf;
	ofnode node;
	bool verbose;
};

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

int cedit_prepare(struct expo *exp, struct video_priv **vid_privp,
		  struct scene **scnp)
{
	struct video_priv *vid_priv;
	struct udevice *dev;
	struct scene *scn;
	uint scene_id;
	int ret;

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

	*vid_privp = vid_priv;
	*scnp = scn;

	return scene_id;
}

int cedit_run(struct expo *exp)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct video_priv *vid_priv;
	uint scene_id;
	struct scene *scn;
	bool done;
	int ret;

	cli_ch_init(cch);
	ret = cedit_prepare(exp, &vid_priv, &scn);
	if (ret < 0)
		return log_msg_ret("prep", ret);
	scene_id = ret;

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

static int check_space(int ret, struct abuf *buf)
{
	if (ret == -FDT_ERR_NOSPACE) {
		if (!abuf_realloc_inc(buf, CEDIT_SIZE_INC))
			return log_msg_ret("spc", -ENOMEM);
		ret = fdt_resize(abuf_data(buf), abuf_data(buf),
				 abuf_size(buf));
		if (ret)
			return log_msg_ret("res", -EFAULT);
	}

	return 0;
}

static int get_cur_menuitem_text(const struct scene_obj_menu *menu,
				 const char **strp)
{
	struct scene *scn = menu->obj.scene;
	const struct scene_menitem *mi;
	const struct scene_obj_txt *txt;
	const char *str;

	mi = scene_menuitem_find(menu, menu->cur_item_id);
	if (!mi)
		return log_msg_ret("mi", -ENOENT);

	txt = scene_obj_find(scn, mi->label_id, SCENEOBJT_TEXT);
	if (!txt)
		return log_msg_ret("txt", -ENOENT);

	str = expo_get_str(scn->expo, txt->str_id);
	if (!str)
		return log_msg_ret("str", -ENOENT);
	*strp = str;

	return 0;
}

static int h_write_settings(struct scene_obj *obj, void *vpriv)
{
	struct cedit_iter_priv *priv = vpriv;
	struct abuf *buf = priv->buf;

	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_IMAGE:
	case SCENEOBJT_TEXT:
		break;
	case SCENEOBJT_MENU: {
		const struct scene_obj_menu *menu;
		const char *str;
		char name[80];
		int ret, i;

		menu = (struct scene_obj_menu *)obj;
		ret = -EAGAIN;
		for (i = 0; ret && i < 2; i++) {
			ret = fdt_property_u32(abuf_data(buf), obj->name,
					       menu->cur_item_id);
			if (!i) {
				ret = check_space(ret, buf);
				if (ret)
					return log_msg_ret("res", -ENOMEM);
			}
		}
		/* this should not happen */
		if (ret)
			return log_msg_ret("wrt", -EFAULT);

		ret = get_cur_menuitem_text(menu, &str);
		if (ret)
			return log_msg_ret("mis", ret);

		snprintf(name, sizeof(name), "%s-str", obj->name);
		ret = -EAGAIN;
		for (i = 0; ret && i < 2; i++) {
			ret = fdt_property_string(abuf_data(buf), name, str);
			if (!i) {
				ret = check_space(ret, buf);
				if (ret)
					return log_msg_ret("rs2", -ENOMEM);
			}
		}

		/* this should not happen */
		if (ret)
			return log_msg_ret("wr2", -EFAULT);

		break;
	}
	}

	return 0;
}

int cedit_write_settings(struct expo *exp, struct abuf *buf)
{
	struct cedit_iter_priv priv;
	void *fdt;
	int ret;

	abuf_init(buf);
	if (!abuf_realloc(buf, CEDIT_SIZE_INC))
		return log_msg_ret("buf", -ENOMEM);

	fdt = abuf_data(buf);
	ret = fdt_create(fdt, abuf_size(buf));
	if (!ret)
		ret = fdt_finish_reservemap(fdt);
	if (!ret)
		ret = fdt_begin_node(fdt, "");
	if (!ret)
		ret = fdt_begin_node(fdt, CEDIT_NODE_NAME);
	if (ret) {
		log_debug("Failed to start FDT (err=%d)\n", ret);
		return log_msg_ret("sta", -EINVAL);
	}

	/* write out the items */
	priv.buf = buf;
	ret = expo_iter_scene_objs(exp, h_write_settings, &priv);
	if (ret) {
		log_debug("Failed to write settings (err=%d)\n", ret);
		return log_msg_ret("set", ret);
	}

	ret = fdt_end_node(fdt);
	if (!ret)
		ret = fdt_end_node(fdt);
	if (!ret)
		ret = fdt_finish(fdt);
	if (ret) {
		log_debug("Failed to finish FDT (err=%d)\n", ret);
		return log_msg_ret("fin", -EINVAL);
	}

	return 0;
}

static int h_read_settings(struct scene_obj *obj, void *vpriv)
{
	struct cedit_iter_priv *priv = vpriv;
	ofnode node = priv->node;

	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_IMAGE:
	case SCENEOBJT_TEXT:
		break;
	case SCENEOBJT_MENU: {
		struct scene_obj_menu *menu;
		uint val;

		if (ofnode_read_u32(node, obj->name, &val))
			return log_msg_ret("rd", -ENOENT);
		menu = (struct scene_obj_menu *)obj;
		menu->cur_item_id = val;

		break;
	}
	}

	return 0;
}

int cedit_read_settings(struct expo *exp, oftree tree)
{
	struct cedit_iter_priv priv;
	ofnode root, node;
	int ret;

	root = oftree_root(tree);
	if (!ofnode_valid(root))
		return log_msg_ret("roo", -ENOENT);
	node = ofnode_find_subnode(root, CEDIT_NODE_NAME);
	if (!ofnode_valid(node))
		return log_msg_ret("pat", -ENOENT);

	/* read in the items */
	priv.node = node;
	ret = expo_iter_scene_objs(exp, h_read_settings, &priv);
	if (ret) {
		log_debug("Failed to read settings (err=%d)\n", ret);
		return log_msg_ret("set", ret);
	}

	return 0;
}

static int h_write_settings_env(struct scene_obj *obj, void *vpriv)
{
	const struct scene_obj_menu *menu;
	struct cedit_iter_priv *priv = vpriv;
	char name[80], var[60];
	const char *str;
	int val, ret;

	if (obj->type != SCENEOBJT_MENU)
		return 0;

	menu = (struct scene_obj_menu *)obj;
	val = menu->cur_item_id;
	snprintf(var, sizeof(var), "c.%s", obj->name);

	if (priv->verbose)
		printf("%s=%d\n", var, val);

	ret = env_set_ulong(var, val);
	if (ret)
		return log_msg_ret("set", ret);

	ret = get_cur_menuitem_text(menu, &str);
	if (ret)
		return log_msg_ret("mis", ret);

	snprintf(name, sizeof(name), "c.%s-str", obj->name);
	if (priv->verbose)
		printf("%s=%s\n", name, str);

	ret = env_set(name, str);
	if (ret)
		return log_msg_ret("st2", ret);

	return 0;
}

int cedit_write_settings_env(struct expo *exp, bool verbose)
{
	struct cedit_iter_priv priv;
	int ret;

	/* write out the items */
	priv.verbose = verbose;
	ret = expo_iter_scene_objs(exp, h_write_settings_env, &priv);
	if (ret) {
		log_debug("Failed to write settings to env (err=%d)\n", ret);
		return log_msg_ret("set", ret);
	}

	return 0;
}

static int h_read_settings_env(struct scene_obj *obj, void *vpriv)
{
	struct cedit_iter_priv *priv = vpriv;
	struct scene_obj_menu *menu;
	char var[60];
	int val, ret;

	if (obj->type != SCENEOBJT_MENU)
		return 0;

	menu = (struct scene_obj_menu *)obj;
	val = menu->cur_item_id;
	snprintf(var, sizeof(var), "c.%s", obj->name);

	val = env_get_ulong(var, 10, 0);
	if (priv->verbose)
		printf("%s=%d\n", var, val);
	if (!val)
		return log_msg_ret("get", -ENOENT);

	/*
	 * note that no validation is done here, to make sure the ID is valid
	 * and actually points to a menu item
	 */
	menu->cur_item_id = val;

	return 0;
}

int cedit_read_settings_env(struct expo *exp, bool verbose)
{
	struct cedit_iter_priv priv;
	int ret;

	/* write out the items */
	priv.verbose = verbose;
	ret = expo_iter_scene_objs(exp, h_read_settings_env, &priv);
	if (ret) {
		log_debug("Failed to read settings from env (err=%d)\n", ret);
		return log_msg_ret("set", ret);
	}

	return 0;
}

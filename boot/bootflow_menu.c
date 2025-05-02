// SPDX-License-Identifier: GPL-2.0+
/*
 * Provide a menu of available bootflows and related options
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <bootflow.h>
#include <bootmeth.h>
#include <bootstd.h>
#include <cli.h>
#include <dm.h>
#include <expo.h>
#include <malloc.h>
#include <menu.h>
#include <video_console.h>
#include <watchdog.h>
#include <linux/delay.h>
#include "bootflow_internal.h"

/**
 * struct menu_priv - information about the menu
 *
 * @num_bootflows: Number of bootflows in the menu
 */
struct menu_priv {
	int num_bootflows;
};

int bootflow_menu_new(struct expo **expp)
{
	struct scene_obj_menu *menu;
	struct menu_priv *priv;
	struct scene *scn;
	struct expo *exp;
	void *logo;
	int ret;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
		return log_msg_ret("prv", -ENOMEM);

	ret = expo_new("bootflows", priv, &exp);
	if (ret)
		return log_msg_ret("exp", ret);

	ret = scene_new(exp, "main", MAIN, &scn);
	if (ret < 0)
		return log_msg_ret("scn", ret);

	ret |= scene_txt_str(scn, "prompt", OBJ_PROMPT, STR_PROMPT,
			     "UP and DOWN to choose, ENTER to select", NULL);

	ret = scene_menu(scn, "main", OBJ_MENU, &menu);
	ret |= scene_obj_set_pos(scn, OBJ_MENU, MARGIN_LEFT, 100);
	ret |= scene_txt_str(scn, "title", OBJ_MENU_TITLE, STR_MENU_TITLE,
			     "U-Boot - Boot Menu", NULL);
	ret |= scene_menu_set_title(scn, OBJ_MENU, OBJ_PROMPT);

	logo = video_get_u_boot_logo();
	if (logo) {
		ret |= scene_img(scn, "ulogo", OBJ_U_BOOT_LOGO, logo, NULL);
		ret |= scene_obj_set_pos(scn, OBJ_U_BOOT_LOGO, -4, 4);
	}

	ret |= scene_txt_str(scn, "cur_item", OBJ_POINTER, STR_POINTER, ">",
			     NULL);
	ret |= scene_menu_set_pointer(scn, OBJ_MENU, OBJ_POINTER);
	if (ret < 0)
		return log_msg_ret("new", -EINVAL);

	*expp = exp;

	return 0;
}

int bootflow_menu_add_all(struct expo *exp)
{
	struct menu_priv *priv = exp->priv;
	struct udevice *last_bootdev;
	struct bootflow *bflow;
	struct scene *scn;
	uint scene_id;
	int ret, i;

	ret = expo_first_scene_id(exp);
	if (ret < 0)
		return log_msg_ret("scn", ret);
	scene_id = ret;
	scn = expo_lookup_scene_id(exp, scene_id);

	last_bootdev = NULL;
	for (ret = bootflow_first_glob(&bflow), i = 0; !ret && i < 36;
	     ret = bootflow_next_glob(&bflow), i++) {
		struct bootmeth_uc_plat *ucp;
		char str[2], *label, *key;
		uint preview_id;
		bool add_gap;

		if (bflow->state != BOOTFLOWST_READY)
			continue;

		/* No media to show for BOOTMETHF_GLOBAL bootmeths */
		ucp = dev_get_uclass_plat(bflow->method);
		if (ucp->flags & BOOTMETHF_GLOBAL)
			continue;

		*str = i < 10 ? '0' + i : 'A' + i - 10;
		str[1] = '\0';
		key = strdup(str);
		if (!key)
			return log_msg_ret("key", -ENOMEM);
		label = strdup(dev_get_parent(bflow->dev)->name);
		if (!label) {
			free(key);
			return log_msg_ret("nam", -ENOMEM);
		}

		add_gap = last_bootdev != bflow->dev;
		last_bootdev = bflow->dev;

		ret = expo_str(exp, "prompt", STR_POINTER, ">");
		ret |= scene_txt_str(scn, "label", ITEM_LABEL + i,
				      STR_LABEL + i, label, NULL);
		ret |= scene_txt_str(scn, "desc", ITEM_DESC + i, STR_DESC + i,
				    bflow->os_name ? bflow->os_name :
				    bflow->name, NULL);
		ret |= scene_txt_str(scn, "key", ITEM_KEY + i, STR_KEY + i, key,
				      NULL);
		preview_id = 0;
		if (bflow->logo) {
			preview_id = ITEM_PREVIEW + i;
			ret |= scene_img(scn, "preview", preview_id,
					     bflow->logo, NULL);
		}
		ret |= scene_menuitem(scn, OBJ_MENU, "item", ITEM + i,
					  ITEM_KEY + i, ITEM_LABEL + i,
					  ITEM_DESC + i, preview_id,
					  add_gap ? SCENEMIF_GAP_BEFORE : 0,
					  NULL);

		if (ret < 0)
			return log_msg_ret("itm", -EINVAL);
		priv->num_bootflows++;
	}

	ret = scene_arrange(scn);
	if (ret)
		return log_msg_ret("arr", ret);

	return 0;
}

int bootflow_menu_apply_theme(struct expo *exp, ofnode node)
{
	struct menu_priv *priv = exp->priv;
	struct scene *scn;
	u32 font_size;
	int ret;

	log_debug("Applying theme %s\n", ofnode_get_name(node));
	scn = expo_lookup_scene_id(exp, MAIN);
	if (!scn)
		return log_msg_ret("scn", -ENOENT);

	/* Avoid error-checking optional items */
	if (!ofnode_read_u32(node, "font-size", &font_size)) {
		int i;

		log_debug("font size %d\n", font_size);
		scene_txt_set_font(scn, OBJ_PROMPT, NULL, font_size);
		scene_txt_set_font(scn, OBJ_POINTER, NULL, font_size);
		for (i = 0; i < priv->num_bootflows; i++) {
			ret = scene_txt_set_font(scn, ITEM_DESC + i, NULL,
						 font_size);
			if (ret)
				return log_msg_ret("des", ret);
			scene_txt_set_font(scn, ITEM_KEY + i, NULL, font_size);
			scene_txt_set_font(scn, ITEM_LABEL + i, NULL,
					   font_size);
		}
	}

	ret = scene_arrange(scn);
	if (ret)
		return log_msg_ret("arr", ret);

	return 0;
}

int bootflow_menu_start(struct bootstd_priv *std, bool text_mode,
			struct expo **expp)
{
	struct udevice *dev;
	struct expo *exp;
	int ret;

	ret = bootflow_menu_new(&exp);
	if (ret)
		return log_msg_ret("bmn", ret);
	ret = bootflow_menu_add_all(exp);
	if (ret)
		return log_msg_ret("bma", ret);

	if (ofnode_valid(std->theme)) {
		ret = bootflow_menu_apply_theme(exp, std->theme);
		if (ret)
			return log_msg_ret("thm", ret);
	}

	/* For now we only support a video console */
	ret = uclass_first_device_err(UCLASS_VIDEO, &dev);
	if (ret)
		return log_msg_ret("vid", ret);
	ret = expo_set_display(exp, dev);
	if (ret)
		return log_msg_ret("dis", ret);

	ret = expo_set_scene_id(exp, MAIN);
	if (ret)
		return log_msg_ret("scn", ret);

	if (text_mode)
		expo_set_text_mode(exp, text_mode);

	*expp = exp;

	return 0;
}

int bootflow_menu_poll(struct expo *exp, struct bootflow **bflowp)
{
	struct bootflow *sel_bflow;
	struct expo_action act;
	int ret;

	sel_bflow = NULL;
	*bflowp = NULL;

	ret = expo_poll(exp, &act);
	if (ret)
		return log_msg_ret("bmp", ret);

	switch (act.type) {
	case EXPOACT_SELECT: {
		struct bootflow *bflow;
		int i;

		for (ret = bootflow_first_glob(&bflow), i = 0; !ret && i < 36;
		     ret = bootflow_next_glob(&bflow), i++) {
			if (i == act.select.id - ITEM) {
				*bflowp = bflow;
				// printf("found %p\n", bflow);
				return 0;
			}
		}
		break;
	}
	case EXPOACT_POINT_ITEM: {
		struct scene *scn = expo_lookup_scene_id(exp, MAIN);

		if (!scn)
			return log_msg_ret("bms", -ENOENT);
		ret = scene_menu_select_item(scn, OBJ_MENU, act.select.id);
		if (ret)
			return log_msg_ret("bmp", ret);
		break;
	}
	case EXPOACT_QUIT:
		return -EPIPE;
	default:
		break;
	}

	return -EAGAIN;
}

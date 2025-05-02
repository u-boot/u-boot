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
 * @last_bootdev: bootdev of the last bootflow added to the menu, NULL if none
 */
struct menu_priv {
	int num_bootflows;
	struct udevice *last_bootdev;
};

int bootflow_menu_new(struct expo **expp)
{
	struct scene_obj_menu *menu;
	struct menu_priv *priv;
	struct scene *scn;
	struct expo *exp;
	bool use_font;
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

	ret = scene_box(scn, "box", OBJ_BOX, 2, NULL);
	if (ret < 0)
		return log_msg_ret("bmb", ret);
	ret |= scene_obj_set_bbox(scn, OBJ_BOX, 30, 90, 1366 - 30, 720);

	ret = scene_menu(scn, "main", OBJ_MENU, &menu);
	ret |= scene_obj_set_pos(scn, OBJ_MENU, MARGIN_LEFT, 100);
	ret |= scene_txt_str(scn, "title", OBJ_MENU_TITLE, STR_MENU_TITLE,
			     "U-Boot - Boot Menu", NULL);
	ret |= scene_obj_set_bbox(scn, OBJ_MENU_TITLE, 0, 32,
				  SCENEOB_DISPLAY_MAX, 30);
	ret |= scene_obj_set_halign(scn, OBJ_MENU_TITLE, SCENEOA_CENTRE);

	logo = video_get_u_boot_logo();
	if (logo) {
		ret |= scene_img(scn, "ulogo", OBJ_U_BOOT_LOGO, logo, NULL);
		ret |= scene_obj_set_pos(scn, OBJ_U_BOOT_LOGO, 1165, 100);
	}

	ret |= scene_txt_str(scn, "prompt1a", OBJ_PROMPT1A, STR_PROMPT1A,
	     "Use the \x18 and \x19 keys to select which entry is highlighted.",
	     NULL);
	ret |= scene_txt_str(scn, "prompt1b", OBJ_PROMPT1B, STR_PROMPT1B,
	     "Use the UP and DOWN keys to select which entry is highlighted.",
	     NULL);
	ret |= scene_txt_str(scn, "prompt2", OBJ_PROMPT2, STR_PROMPT2,
	     "Press enter to boot the selected OS, 'e' to edit the commands "
	     "before booting or 'c' for a command-line. ESC to return to "
	     "previous menu", NULL);
	ret |= scene_txt_str(scn, "autoboot", OBJ_AUTOBOOT, STR_AUTOBOOT,
	     "The highlighted entry will be executed automatically in %ds.",
	     NULL);
	ret |= scene_obj_set_bbox(scn, OBJ_PROMPT1A, 0, 590,
				  SCENEOB_DISPLAY_MAX, 30);
	ret |= scene_obj_set_bbox(scn, OBJ_PROMPT1B, 0, 620,
				  SCENEOB_DISPLAY_MAX, 30);
	ret |= scene_obj_set_bbox(scn, OBJ_PROMPT2, 100, 650,
				  1366 - 100, 700);
	ret |= scene_obj_set_bbox(scn, OBJ_AUTOBOOT, 0, 720,
				  SCENEOB_DISPLAY_MAX, 750);
	ret |= scene_obj_set_halign(scn, OBJ_PROMPT1A, SCENEOA_CENTRE);
	ret |= scene_obj_set_halign(scn, OBJ_PROMPT1B, SCENEOA_CENTRE);
	ret |= scene_obj_set_halign(scn, OBJ_PROMPT2, SCENEOA_CENTRE);
	ret |= scene_obj_set_valign(scn, OBJ_PROMPT2, SCENEOA_CENTRE);
	ret |= scene_obj_set_halign(scn, OBJ_AUTOBOOT, SCENEOA_CENTRE);

	use_font = IS_ENABLED(CONFIG_CONSOLE_TRUETYPE);
	scene_obj_set_hide(scn, OBJ_PROMPT1A, use_font);
	scene_obj_set_hide(scn, OBJ_PROMPT1B, !use_font);
	scene_obj_set_hide(scn, OBJ_AUTOBOOT, use_font);

	ret |= scene_txt_str(scn, "cur_item", OBJ_POINTER, STR_POINTER, ">",
			     NULL);
	ret |= scene_menu_set_pointer(scn, OBJ_MENU, OBJ_POINTER);
	if (ret < 0)
		return log_msg_ret("new", -EINVAL);

	exp->show_highlight = true;

	*expp = exp;

	return 0;
}

int bootflow_menu_add(struct expo *exp, struct bootflow *bflow, int seq,
		      struct scene **scnp)
{
	struct menu_priv *priv = exp->priv;
	char str[2], *label, *key;
	struct udevice *media;
	struct scene *scn;
	const char *name;
	uint preview_id;
	uint scene_id;
	bool add_gap;
	int ret;

	ret = expo_first_scene_id(exp);
	if (ret < 0)
		return log_msg_ret("scn", ret);
	scene_id = ret;
	scn = expo_lookup_scene_id(exp, scene_id);

	*str = seq < 10 ? '0' + seq : 'A' + seq - 10;
	str[1] = '\0';
	key = strdup(str);
	if (!key)
		return log_msg_ret("key", -ENOMEM);

	media = dev_get_parent(bflow->dev);
	if (device_get_uclass_id(media) == UCLASS_MASS_STORAGE)
		name = "usb";
	else
		name = media->name;
	label = strdup(name);

	if (!label) {
		free(key);
		return log_msg_ret("nam", -ENOMEM);
	}

	add_gap = priv->last_bootdev != bflow->dev;

	/* disable this gap for now, since it looks a little ugly */
	add_gap = false;
	priv->last_bootdev = bflow->dev;

	ret = expo_str(exp, "prompt", STR_POINTER, ">");
	ret |= scene_txt_str(scn, "label", ITEM_LABEL + seq,
			      STR_LABEL + seq, label, NULL);
	ret |= scene_txt_str(scn, "desc", ITEM_DESC + seq, STR_DESC + seq,
			    bflow->os_name ? bflow->os_name :
			    bflow->name, NULL);
	ret |= scene_txt_str(scn, "key", ITEM_KEY + seq, STR_KEY + seq, key,
			      NULL);
	preview_id = 0;
	if (bflow->logo) {
		preview_id = ITEM_PREVIEW + seq;
		ret |= scene_img(scn, "preview", preview_id,
				     bflow->logo, NULL);
	}
	ret |= scene_menuitem(scn, OBJ_MENU, "item", ITEM + seq,
				  ITEM_KEY + seq, ITEM_LABEL + seq,
				  ITEM_DESC + seq, preview_id,
				  add_gap ? SCENEMIF_GAP_BEFORE : 0,
				  NULL);

	if (ret < 0)
		return log_msg_ret("itm", -EINVAL);
	priv->num_bootflows++;
	*scnp = scn;

	return 0;
}

int bootflow_menu_add_all(struct expo *exp)
{
	struct bootflow *bflow;
	struct scene *scn;
	int ret, i;

	for (ret = bootflow_first_glob(&bflow), i = 0; !ret && i < 36;
	     ret = bootflow_next_glob(&bflow), i++) {
		struct bootmeth_uc_plat *ucp;

		if (bflow->state != BOOTFLOWST_READY)
			continue;

		/* No media to show for BOOTMETHF_GLOBAL bootmeths */
		ucp = dev_get_uclass_plat(bflow->method);
		if (ucp->flags & BOOTMETHF_GLOBAL)
			continue;

		ret = bootflow_menu_add(exp, bflow, i, &scn);
		if (ret)
			return log_msg_ret("bao", ret);
	}

	return 0;
}

int bootflow_menu_setup(struct bootstd_priv *std, bool text_mode,
			struct expo **expp)
{
	struct udevice *dev;
	struct expo *exp;
	int ret;

	ret = bootflow_menu_new(&exp);
	if (ret)
		return log_msg_ret("bmn", ret);

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

int bootflow_menu_start(struct bootstd_priv *std, bool text_mode,
			struct expo **expp)
{
	struct scene *scn;
	struct expo *exp;
	uint scene_id;
	int ret;

	ret = bootflow_menu_setup(std, text_mode, &exp);
	if (ret)
		return log_msg_ret("bmd", ret);

	ret = bootflow_menu_add_all(exp);
	if (ret)
		return log_msg_ret("bma", ret);

	if (ofnode_valid(std->theme)) {
		ret = expo_apply_theme(exp, std->theme);
		if (ret)
			return log_msg_ret("thm", ret);
	}

	ret = expo_calc_dims(exp);
	if (ret)
		return log_msg_ret("bmd", ret);

	ret = expo_first_scene_id(exp);
	if (ret < 0)
		return log_msg_ret("scn", ret);
	scene_id = ret;
	scn = expo_lookup_scene_id(exp, scene_id);

	scene_set_highlight_id(scn, OBJ_MENU);

	ret = scene_arrange(scn);
	if (ret)
		return log_msg_ret("arr", ret);

	*expp = exp;

	return 0;
}

int bootflow_menu_poll(struct expo *exp, int *seqp)
{
	struct bootflow *sel_bflow;
	struct expo_action act;
	struct scene *scn;
	int item, ret;

	sel_bflow = NULL;

	scn = expo_lookup_scene_id(exp, exp->scene_id);

	item = scene_menu_get_cur_item(scn, OBJ_MENU);
	*seqp = item > 0 ? item - ITEM : -1;

	ret = expo_poll(exp, &act);
	if (ret)
		return log_msg_ret("bmp", ret);

	switch (act.type) {
	case EXPOACT_SELECT:
		*seqp = act.select.id - ITEM;
		break;
	case EXPOACT_POINT_ITEM: {
		struct scene *scn = expo_lookup_scene_id(exp, MAIN);

		if (!scn)
			return log_msg_ret("bms", -ENOENT);
		ret = scene_menu_select_item(scn, OBJ_MENU, act.select.id);
		if (ret)
			return log_msg_ret("bmp", ret);
		return -ERESTART;
	}
	case EXPOACT_QUIT:
		return -EPIPE;
	default:
		return -EAGAIN;
	}

	return 0;
}

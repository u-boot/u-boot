// SPDX-License-Identifier: GPL-2.0+
/*
 * Building an expo from an FDT description
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_EXPO

#include <cedit.h>
#include <ctype.h>
#include <errno.h>
#include <expo.h>
#include <log.h>
#include <malloc.h>
#include <vsprintf.h>
#include <asm/cb_sysinfo.h>

/**
 * struct build_info - Information to use when building
 */
struct build_info {
	const struct cb_cmos_option_table *tab;
	struct cedit_priv *priv;
};

/**
 * convert_to_title() - Convert text to 'title' format and allocate a string
 *
 * Converts "this_is_a_test" to "This is a test" so it looks better
 *
 * @text: Text to convert
 * Return: Allocated string, or NULL if out of memory
 */
static char *convert_to_title(const char *text)
{
	int len = strlen(text);
	char *buf, *s;

	buf = malloc(len + 1);
	if (!buf)
		return NULL;

	for (s = buf; *text; s++, text++) {
		if (s == buf)
			*s = toupper(*text);
		else if (*text == '_')
			*s = ' ';
		else
			*s = *text;
	}
	*s = '\0';

	return buf;
}

/**
 * menu_build() - Build a menu and add it to a scene
 *
 * See doc/developer/expo.rst for a description of the format
 *
 * @info: Build information
 * @entry: CMOS entry to build a menu for
 * @scn: Scene to add the menu to
 * @objp: Returns the object pointer
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
static int menu_build(struct build_info *info,
		      const struct cb_cmos_entries *entry, struct scene *scn,
		      struct scene_obj **objp)
{
	struct scene_obj_menu *menu;
	const void *ptr, *end;
	uint menu_id;
	char *title;
	int ret, i;

	ret = scene_menu(scn, entry->name, 0, &menu);
	if (ret < 0)
		return log_msg_ret("men", ret);
	menu_id = ret;

	title = convert_to_title(entry->name);
	if (!title)
		return log_msg_ret("con", -ENOMEM);

	/* Set the title */
	ret = scene_txt_str(scn, "title", 0, 0, title, NULL);
	if (ret < 0)
		return log_msg_ret("tit", ret);
	menu->title_id = ret;

	end = (void *)info->tab + info->tab->size;
	for (ptr = (void *)info->tab + info->tab->header_length, i = 0;
	     ptr < end; i++) {
		const struct cb_cmos_enums *enums = ptr;
		struct scene_menitem *item;
		uint label;

		ptr += enums->size;
		if (enums->tag != CB_TAG_OPTION_ENUM ||
		    enums->config_id != entry->config_id)
			continue;

		ret = scene_txt_str(scn, enums->text, 0, 0, enums->text, NULL);
		if (ret < 0)
			return log_msg_ret("tit", ret);
		label = ret;

		ret = scene_menuitem(scn, menu_id, simple_xtoa(i), 0, 0, label,
				     0, 0, 0, &item);
		if (ret < 0)
			return log_msg_ret("mi", ret);
		item->value = enums->value;
	}
	*objp = &menu->obj;

	return 0;
}

/**
 * scene_build() - Build a scene and all its objects
 *
 * See doc/developer/expo.rst for a description of the format
 *
 * @info: Build information
 * @scn: Scene to add the object to
 * Returns: 0 if OK, -ENOMEM if out of memory, -EINVAL if there is a format
 * error, -ENOENT if there is a references to a non-existent string
 */
static int scene_build(struct build_info *info, struct expo *exp)
{
	struct scene_obj_menu *menu;
	const void *ptr, *end;
	struct scene_obj *obj;
	struct scene *scn;
	uint label, menu_id;
	int ret;

	ret = scene_new(exp, "cmos", 0, &scn);
	if (ret < 0)
		return log_msg_ret("scn", ret);

	ret = scene_txt_str(scn, "title", 0, 0, "CMOS RAM settings", NULL);
	if (ret < 0)
		return log_msg_ret("add", ret);
	scn->title_id = ret;

	ret = scene_txt_str(scn, "prompt", 0, 0,
			    "UP and DOWN to choose, ENTER to select", NULL);
	if (ret < 0)
		return log_msg_ret("add", ret);

	end = (void *)info->tab + info->tab->size;
	for (ptr = (void *)info->tab + info->tab->header_length; ptr < end;) {
		const struct cb_cmos_entries *entry;
		const struct cb_record *rec = ptr;

		entry = ptr;
		ptr += rec->size;
		if (rec->tag != CB_TAG_OPTION)
			continue;
		switch (entry->config) {
		case 'e':
			ret = menu_build(info, entry, scn, &obj);
			break;
		default:
			continue;
		}
		if (ret < 0)
			return log_msg_ret("add", ret);

		obj->start_bit = entry->bit;
		obj->bit_length = entry->length;
	}

	ret = scene_menu(scn, "save", EXPOID_SAVE, &menu);
	if (ret < 0)
		return log_msg_ret("men", ret);
	menu_id = ret;

	ret = scene_txt_str(scn, "save", 0, 0, "Save and exit", NULL);
	if (ret < 0)
		return log_msg_ret("sav", ret);
	label = ret;
	ret = scene_menuitem(scn, menu_id, "save", 0, 0, label,
			     0, 0, 0, NULL);
	if (ret < 0)
		return log_msg_ret("mi", ret);

	ret = scene_menu(scn, "nosave", EXPOID_DISCARD, &menu);
	if (ret < 0)
		return log_msg_ret("men", ret);
	menu_id = ret;

	ret = scene_txt_str(scn, "nosave", 0, 0, "Exit without saving", NULL);
	if (ret < 0)
		return log_msg_ret("nos", ret);
	label = ret;
	ret = scene_menuitem(scn, menu_id, "exit", 0, 0, label,
			     0, 0, 0, NULL);
	if (ret < 0)
		return log_msg_ret("mi", ret);

	return 0;
}

static int build_it(struct build_info *info, struct expo **expp)
{
	struct expo *exp;
	int ret;

	ret = expo_new("coreboot", NULL, &exp);
	if (ret)
		return log_msg_ret("exp", ret);
	expo_set_dynamic_start(exp, EXPOID_BASE_ID);

	ret = scene_build(info, exp);
	if (ret < 0)
		return log_msg_ret("scn", ret);

	*expp = exp;

	return 0;
}

int cb_expo_build(struct expo **expp)
{
	struct build_info info;
	struct expo *exp;
	int ret;

	info.tab = lib_sysinfo.option_table;
	if (!info.tab)
		return log_msg_ret("tab", -ENOENT);

	ret = build_it(&info, &exp);
	if (ret)
		return log_msg_ret("bui", ret);
	*expp = exp;

	return 0;
}

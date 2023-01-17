// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a scene, a collection of text/image/menu items in an expo
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <expo.h>
#include <malloc.h>
#include <mapmem.h>
#include <video.h>
#include <video_console.h>
#include <linux/input.h>
#include "scene_internal.h"

uint resolve_id(struct expo *exp, uint id)
{
	if (!id)
		id = exp->next_id++;
	else if (id >= exp->next_id)
		exp->next_id = id + 1;

	return id;
}

int scene_new(struct expo *exp, const char *name, uint id, struct scene **scnp)
{
	struct scene *scn;

	scn = calloc(1, sizeof(struct scene));
	if (!scn)
		return log_msg_ret("expo", -ENOMEM);
	scn->name = strdup(name);
	if (!scn->name) {
		free(scn);
		return log_msg_ret("name", -ENOMEM);
	}

	INIT_LIST_HEAD(&scn->obj_head);
	scn->id = resolve_id(exp, id);
	scn->expo = exp;
	list_add_tail(&scn->sibling, &exp->scene_head);

	*scnp = scn;

	return scn->id;
}

void scene_obj_destroy(struct scene_obj *obj)
{
	if (obj->type == SCENEOBJT_MENU)
		scene_menu_destroy((struct scene_obj_menu *)obj);
	free(obj->name);
	free(obj);
}

void scene_destroy(struct scene *scn)
{
	struct scene_obj *obj, *next;

	list_for_each_entry_safe(obj, next, &scn->obj_head, sibling)
		scene_obj_destroy(obj);

	free(scn->name);
	free(scn->title);
	free(scn);
}

int scene_title_set(struct scene *scn, const char *title)
{
	free(scn->title);
	scn->title = strdup(title);
	if (!scn->title)
		return log_msg_ret("tit", -ENOMEM);

	return 0;
}

int scene_obj_count(struct scene *scn)
{
	struct scene_obj *obj;
	int count = 0;

	list_for_each_entry(obj, &scn->obj_head, sibling)
		count++;

	return count;
}

void *scene_obj_find(struct scene *scn, uint id, enum scene_obj_t type)
{
	struct scene_obj *obj;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->id == id &&
		    (type == SCENEOBJT_NONE || obj->type == type))
			return obj;
	}

	return NULL;
}

int scene_obj_add(struct scene *scn, const char *name, uint id,
		  enum scene_obj_t type, uint size, struct scene_obj **objp)
{
	struct scene_obj *obj;

	obj = calloc(1, size);
	if (!obj)
		return log_msg_ret("obj", -ENOMEM);
	obj->name = strdup(name);
	if (!obj->name) {
		free(obj);
		return log_msg_ret("name", -ENOMEM);
	}

	obj->id = resolve_id(scn->expo, id);
	obj->scene = scn;
	obj->type = type;
	list_add_tail(&obj->sibling, &scn->obj_head);
	*objp = obj;

	return obj->id;
}

int scene_img(struct scene *scn, const char *name, uint id, char *data,
	      struct scene_obj_img **imgp)
{
	struct scene_obj_img *img;
	int ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_IMAGE,
			    sizeof(struct scene_obj_img),
			    (struct scene_obj **)&img);
	if (ret < 0)
		return log_msg_ret("obj", -ENOMEM);

	img->data = data;

	if (imgp)
		*imgp = img;

	return img->obj.id;
}

int scene_txt(struct scene *scn, const char *name, uint id, uint str_id,
	      struct scene_obj_txt **txtp)
{
	struct scene_obj_txt *txt;
	int ret;

	ret = scene_obj_add(scn, name, id, SCENEOBJT_TEXT,
			    sizeof(struct scene_obj_txt),
			    (struct scene_obj **)&txt);
	if (ret < 0)
		return log_msg_ret("obj", -ENOMEM);

	txt->str_id = str_id;

	if (txtp)
		*txtp = txt;

	return txt->obj.id;
}

int scene_txt_str(struct scene *scn, const char *name, uint id, uint str_id,
		  const char *str, struct scene_obj_txt **txtp)
{
	struct scene_obj_txt *txt;
	int ret;

	ret = expo_str(scn->expo, name, str_id, str);
	if (ret < 0)
		return log_msg_ret("str", ret);
	else if (ret != str_id)
		return log_msg_ret("id", -EEXIST);

	ret = scene_obj_add(scn, name, id, SCENEOBJT_TEXT,
			    sizeof(struct scene_obj_txt),
			    (struct scene_obj **)&txt);
	if (ret < 0)
		return log_msg_ret("obj", -ENOMEM);

	txt->str_id = str_id;

	if (txtp)
		*txtp = txt;

	return txt->obj.id;
}

int scene_txt_set_font(struct scene *scn, uint id, const char *font_name,
		       uint font_size)
{
	struct scene_obj_txt *txt;

	txt = scene_obj_find(scn, id, SCENEOBJT_TEXT);
	if (!txt)
		return log_msg_ret("find", -ENOENT);
	txt->font_name = font_name;
	txt->font_size = font_size;

	return 0;
}

int scene_obj_set_pos(struct scene *scn, uint id, int x, int y)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);
	obj->x = x;
	obj->y = y;
	if (obj->type == SCENEOBJT_MENU)
		scene_menu_arrange(scn, (struct scene_obj_menu *)obj);

	return 0;
}

int scene_obj_set_hide(struct scene *scn, uint id, bool hide)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);
	obj->hide = hide;

	return 0;
}

int scene_obj_get_hw(struct scene *scn, uint id, int *widthp)
{
	struct scene_obj *obj;

	obj = scene_obj_find(scn, id, SCENEOBJT_NONE);
	if (!obj)
		return log_msg_ret("find", -ENOENT);

	switch (obj->type) {
	case SCENEOBJT_NONE:
	case SCENEOBJT_MENU:
		break;
	case SCENEOBJT_IMAGE: {
		struct scene_obj_img *img = (struct scene_obj_img *)obj;
		ulong width, height;
		uint bpix;

		video_bmp_get_info(img->data, &width, &height, &bpix);
		if (widthp)
			*widthp = width;
		return height;
	}
	case SCENEOBJT_TEXT: {
		struct scene_obj_txt *txt = (struct scene_obj_txt *)obj;
		struct expo *exp = scn->expo;

		if (widthp)
			*widthp = 16; /* fake value for now */
		if (txt->font_size)
			return txt->font_size;
		if (exp->display)
			return video_default_font_height(exp->display);

		/* use a sensible default */
		return 16;
	}
	}

	return 0;
}

/**
 * scene_obj_render() - Render an object
 *
 */
static int scene_obj_render(struct scene_obj *obj, bool text_mode)
{
	struct scene *scn = obj->scene;
	struct expo *exp = scn->expo;
	struct udevice *cons, *dev = exp->display;
	int x, y, ret;

	cons = NULL;
	if (!text_mode) {
		ret = device_find_first_child_by_uclass(dev,
							UCLASS_VIDEO_CONSOLE,
							&cons);
	}

	x = obj->x;
	y = obj->y;

	switch (obj->type) {
	case SCENEOBJT_NONE:
		break;
	case SCENEOBJT_IMAGE: {
		struct scene_obj_img *img = (struct scene_obj_img *)obj;

		if (!cons)
			return -ENOTSUPP;
		ret = video_bmp_display(dev, map_to_sysmem(img->data), x, y,
					true);
		if (ret < 0)
			return log_msg_ret("img", ret);
		break;
	}
	case SCENEOBJT_TEXT: {
		struct scene_obj_txt *txt = (struct scene_obj_txt *)obj;
		const char *str;

		if (!cons)
			return -ENOTSUPP;

		if (txt->font_name || txt->font_size) {
			ret = vidconsole_select_font(cons,
						     txt->font_name,
						     txt->font_size);
		} else {
			ret = vidconsole_select_font(cons, NULL, 0);
		}
		if (ret && ret != -ENOSYS)
			return log_msg_ret("font", ret);
		vidconsole_set_cursor_pos(cons, x, y);
		str = expo_get_str(exp, txt->str_id);
		if (str)
			vidconsole_put_string(cons, str);
		break;
	}
	case SCENEOBJT_MENU: {
		struct scene_obj_menu *menu = (struct scene_obj_menu *)obj;
		/*
		 * With a vidconsole, the text and item pointer are rendered as
		 * normal objects so we don't need to do anything here. The menu
		 * simply controls where they are positioned.
		 */
		if (cons)
			return -ENOTSUPP;

		ret = scene_menu_display(menu);
		if (ret < 0)
			return log_msg_ret("img", ret);

		break;
	}
	}

	return 0;
}

int scene_arrange(struct scene *scn)
{
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->type == SCENEOBJT_MENU) {
			struct scene_obj_menu *menu;

			menu = (struct scene_obj_menu *)obj,
			ret = scene_menu_arrange(scn, menu);
			if (ret)
				return log_msg_ret("arr", ret);
		}
	}

	return 0;
}

int scene_render(struct scene *scn)
{
	struct expo *exp = scn->expo;
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (!obj->hide) {
			ret = scene_obj_render(obj, exp->text_mode);
			if (ret && ret != -ENOTSUPP)
				return log_msg_ret("ren", ret);
		}
	}

	return 0;
}

int scene_send_key(struct scene *scn, int key, struct expo_action *event)
{
	struct scene_obj *obj;
	int ret;

	list_for_each_entry(obj, &scn->obj_head, sibling) {
		if (obj->type == SCENEOBJT_MENU) {
			struct scene_obj_menu *menu;

			menu = (struct scene_obj_menu *)obj,
			ret = scene_menu_send_key(scn, menu, key, event);
			if (ret)
				return log_msg_ret("key", ret);

			/* only allow one menu */
			ret = scene_menu_arrange(scn, menu);
			if (ret)
				return log_msg_ret("arr", ret);
			break;
		}
	}

	return 0;
}

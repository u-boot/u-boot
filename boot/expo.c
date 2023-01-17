// SPDX-License-Identifier: GPL-2.0+
/*
 * Implementation of a expo, a collection of scenes providing menu options
 *
 * Copyright 2022 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <expo.h>
#include <malloc.h>
#include <video.h>
#include "scene_internal.h"

int expo_new(const char *name, void *priv, struct expo **expp)
{
	struct expo *exp;

	exp = calloc(1, sizeof(struct expo));
	if (!exp)
		return log_msg_ret("expo", -ENOMEM);
	exp->name = strdup(name);
	if (!exp->name) {
		free(exp);
		return log_msg_ret("name", -ENOMEM);
	}
	exp->priv = priv;
	INIT_LIST_HEAD(&exp->scene_head);
	INIT_LIST_HEAD(&exp->str_head);

	*expp = exp;

	return 0;
}

static void estr_destroy(struct expo_string *estr)
{
	free(estr);
}

void expo_destroy(struct expo *exp)
{
	struct scene *scn, *next;
	struct expo_string *estr, *enext;

	list_for_each_entry_safe(scn, next, &exp->scene_head, sibling)
		scene_destroy(scn);

	list_for_each_entry_safe(estr, enext, &exp->str_head, sibling)
		estr_destroy(estr);

	free(exp->name);
	free(exp);
}

int expo_str(struct expo *exp, const char *name, uint id, const char *str)
{
	struct expo_string *estr;

	estr = calloc(1, sizeof(struct expo_string));
	if (!estr)
		return log_msg_ret("obj", -ENOMEM);

	estr->id = resolve_id(exp, id);
	estr->str = str;
	list_add_tail(&estr->sibling, &exp->str_head);

	return estr->id;
}

const char *expo_get_str(struct expo *exp, uint id)
{
	struct expo_string *estr;

	list_for_each_entry(estr, &exp->str_head, sibling) {
		if (estr->id == id)
			return estr->str;
	}

	return NULL;
}

int expo_set_display(struct expo *exp, struct udevice *dev)
{
	exp->display = dev;

	return 0;
}

void exp_set_text_mode(struct expo *exp, bool text_mode)
{
	exp->text_mode = text_mode;
}

struct scene *expo_lookup_scene_id(struct expo *exp, uint scene_id)
{
	struct scene *scn;

	list_for_each_entry(scn, &exp->scene_head, sibling) {
		if (scn->id == scene_id)
			return scn;
	}

	return NULL;
}

int expo_set_scene_id(struct expo *exp, uint scene_id)
{
	if (!expo_lookup_scene_id(exp, scene_id))
		return log_msg_ret("id", -ENOENT);
	exp->scene_id = scene_id;

	return 0;
}

int expo_render(struct expo *exp)
{
	struct udevice *dev = exp->display;
	struct video_priv *vid_priv = dev_get_uclass_priv(dev);
	struct scene *scn = NULL;
	u32 colour;
	int ret;

	colour = video_index_to_colour(vid_priv, VID_WHITE);
	ret = video_fill(dev, colour);
	if (ret)
		return log_msg_ret("fill", ret);

	if (exp->scene_id) {
		scn = expo_lookup_scene_id(exp, exp->scene_id);
		if (!scn)
			return log_msg_ret("scn", -ENOENT);

		ret = scene_render(scn);
		if (ret)
			return log_msg_ret("ren", ret);
	}

	video_sync(dev, true);

	return scn ? 0 : -ECHILD;
}

int expo_send_key(struct expo *exp, int key)
{
	struct scene *scn = NULL;

	if (exp->scene_id) {
		int ret;

		scn = expo_lookup_scene_id(exp, exp->scene_id);
		if (!scn)
			return log_msg_ret("scn", -ENOENT);

		ret = scene_send_key(scn, key, &exp->action);
		if (ret)
			return log_msg_ret("key", ret);
	}

	return scn ? 0 : -ECHILD;
}

int expo_action_get(struct expo *exp, struct expo_action *act)
{
	*act = exp->action;
	exp->action.type = EXPOACT_NONE;

	return act->type == EXPOACT_NONE ? -EAGAIN : 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <malloc.h>
#include <asm/global_data.h>
#include <dm/tag.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/types.h>

struct udevice;

DECLARE_GLOBAL_DATA_PTR;

int dev_tag_set_ptr(struct udevice *dev, enum dm_tag_t tag, void *ptr)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag)
			return -EEXIST;
	}

	node = calloc(sizeof(*node), 1);
	if (!node)
		return -ENOMEM;

	node->dev = dev;
	node->tag = tag;
	node->ptr = ptr;
	list_add_tail(&node->sibling, (struct list_head *)&gd->dmtag_list);

	return 0;
}

int dev_tag_set_val(struct udevice *dev, enum dm_tag_t tag, ulong val)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag)
			return -EEXIST;
	}

	node = calloc(sizeof(*node), 1);
	if (!node)
		return -ENOMEM;

	node->dev = dev;
	node->tag = tag;
	node->val = val;
	list_add_tail(&node->sibling, (struct list_head *)&gd->dmtag_list);

	return 0;
}

int dev_tag_get_ptr(struct udevice *dev, enum dm_tag_t tag, void **ptrp)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			*ptrp = node->ptr;
			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_get_val(struct udevice *dev, enum dm_tag_t tag, ulong *valp)
{
	struct dmtag_node *node;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry(node, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			*valp = node->val;
			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_del(struct udevice *dev, enum dm_tag_t tag)
{
	struct dmtag_node *node, *tmp;

	if (!dev || tag >= DM_TAG_COUNT)
		return -EINVAL;

	list_for_each_entry_safe(node, tmp, &gd->dmtag_list, sibling) {
		if (node->dev == dev && node->tag == tag) {
			list_del(&node->sibling);
			free(node);

			return 0;
		}
	}

	return -ENOENT;
}

int dev_tag_del_all(struct udevice *dev)
{
	struct dmtag_node *node, *tmp;
	bool found = false;

	if (!dev)
		return -EINVAL;

	list_for_each_entry_safe(node, tmp, &gd->dmtag_list, sibling) {
		if (node->dev == dev) {
			list_del(&node->sibling);
			free(node);
			found = true;
		}
	}

	if (found)
		return 0;

	return -ENOENT;
}

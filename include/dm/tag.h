/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#ifndef _DM_TAG_H
#define _DM_TAG_H

#include <linux/list.h>
#include <linux/types.h>

struct udevice;

enum dm_tag_t {
	/* EFI_LOADER */
	DM_TAG_EFI = 0,

	DM_TAG_COUNT,
};

/**
 * dmtag_node
 *
 * @sibling: List of dm-tag nodes
 * @dev:     Associated udevice
 * @tag:     Tag type
 * @ptr:     Pointer as a value
 * @val:     Value
 */
struct dmtag_node {
	struct list_head sibling;
	struct  udevice *dev;
	enum dm_tag_t tag;
	union {
		void *ptr;
		ulong val;
	};
};

/**
 * dev_tag_set_ptr() - set a tag's value as a pointer
 * @dev: Device to operate
 * @tag: Tag type
 * @ptr: Pointer to set
 *
 * Set the value, @ptr, as of @tag associated with the device, @dev
 *
 * Return: 0 on success, -ve on error
 */
int dev_tag_set_ptr(struct udevice *dev, enum dm_tag_t tag, void *ptr);

/**
 * dev_tag_set_val() set a tag's value as an integer
 * @dev: Device to operate
 * @tag: Tag type
 * @val: Value to set
 *
 * Set the value, @val, as of @tag associated with the device, @dev
 *
 * Return: on success, -ve on error
 */
int dev_tag_set_val(struct udevice *dev, enum dm_tag_t tag, ulong val);

/**
 * dev_tag_get_ptr() - get a tag's value as a pointer
 * @dev: Device to operate
 * @tag: Tag type
 * @ptrp: Pointer to tag's value (pointer)
 *
 * Get a tag's value as a pointer
 *
 * Return: on success, -ve on error
 */
int dev_tag_get_ptr(struct udevice *dev, enum dm_tag_t tag, void **ptrp);

/**
 * dev_tag_get_val() - get a tag's value as an integer
 * @dev: Device to operate
 * @tag: Tag type
 * @valp: Pointer to tag's value (ulong)
 *
 * Get a tag's value as an integer
 *
 * Return: 0 on success, -ve on error
 */
int dev_tag_get_val(struct udevice *dev, enum dm_tag_t tag, ulong *valp);

/**
 * dev_tag_del() - delete a tag
 * @dev: Device to operate
 * @tag: Tag type
 *
 * Delete a tag of @tag associated with the device, @dev
 *
 * Return: 0 on success, -ve on error
 */
int dev_tag_del(struct udevice *dev, enum dm_tag_t tag);

/**
 * dev_tag_del_all() - delete all tags
 * @dev: Device to operate
 *
 * Delete all the tags associated with the device, @dev
 *
 * Return: 0 on success, -ve on error
 */
int dev_tag_del_all(struct udevice *dev);

#endif /* _DM_TAG_H */

// SPDX-License-Identifier: Intel
/*
 * Access to binman information at runtime
 *
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <binman.h>
#include <dm.h>
#include <malloc.h>

struct binman_info {
	ofnode image;
};

static struct binman_info *binman;

int binman_entry_find(const char *name, struct binman_entry *entry)
{
	ofnode node;
	int ret;

	node = ofnode_find_subnode(binman->image, name);
	if (!ofnode_valid(node))
		return log_msg_ret("no binman node", -ENOENT);

	ret = ofnode_read_u32(node, "image-pos", &entry->image_pos);
	if (ret)
		return log_msg_ret("bad binman node1", ret);
	ret = ofnode_read_u32(node, "size", &entry->size);
	if (ret)
		return log_msg_ret("bad binman node2", ret);

	return 0;
}

int binman_init(void)
{
	binman = malloc(sizeof(struct binman_info));
	if (!binman)
		return log_msg_ret("space for binman", -ENOMEM);
	binman->image = ofnode_path("/binman");
	if (!ofnode_valid(binman->image))
		return log_msg_ret("binman node", -EINVAL);

	return 0;
}

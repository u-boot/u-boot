// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/of_extra.h>
#include <dm/ofnode.h>

int of_read_fmap_entry(ofnode node, const char *name,
		       struct fmap_entry *entry)
{
	const char *prop;
	u32 reg[2];

	if (ofnode_read_u32_array(node, "reg", reg, 2)) {
		debug("Node '%s' has bad/missing 'reg' property\n", name);
		return -FDT_ERR_NOTFOUND;
	}
	entry->offset = reg[0];
	entry->length = reg[1];
	entry->used = ofnode_read_s32_default(node, "used", entry->length);
	prop = ofnode_read_string(node, "compress");
	entry->compress_algo = prop && !strcmp(prop, "lzo") ?
		FMAP_COMPRESS_LZO : FMAP_COMPRESS_NONE;
	prop = ofnode_read_string(node, "hash");
	if (prop)
		entry->hash_size = strlen(prop);
	entry->hash_algo = prop ? FMAP_HASH_SHA256 : FMAP_HASH_NONE;
	entry->hash = (uint8_t *)prop;

	return 0;
}

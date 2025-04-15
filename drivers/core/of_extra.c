// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <log.h>
#include <linux/libfdt.h>
#include <dm/of_access.h>
#include <dm/of_extra.h>
#include <dm/ofnode.h>
#include <dm/util.h>

int ofnode_read_fmap_entry(ofnode node, struct fmap_entry *entry)
{
	const char *prop;
	ofnode subnode;

	if (ofnode_read_u32(node, "image-pos", &entry->offset)) {
		dm_warn("Node '%s' has bad/missing 'image-pos' property\n",
			ofnode_get_name(node));
		return log_msg_ret("image-pos", -ENOENT);
	}
	if (ofnode_read_u32(node, "size", &entry->length)) {
		dm_warn("Node '%s' has bad/missing 'size' property\n",
			ofnode_get_name(node));
		return log_msg_ret("size", -ENOENT);
	}
	entry->used = ofnode_read_s32_default(node, "used", entry->length);
	prop = ofnode_read_string(node, "compress");
	if (prop) {
		if (!strcmp(prop, "lz4"))
			entry->compress_algo = FMAP_COMPRESS_LZ4;
		else if (!strcmp(prop, "lzma"))
			entry->compress_algo = FMAP_COMPRESS_LZMA;
		else
			return log_msg_ret("compression algo", -EINVAL);
	} else {
		entry->compress_algo = FMAP_COMPRESS_NONE;
	}
	entry->unc_length = ofnode_read_s32_default(node, "uncomp-size",
						    entry->length);
	subnode = ofnode_find_subnode(node, "hash");
	if (ofnode_valid(subnode)) {
		prop = ofnode_read_prop(subnode, "value", &entry->hash_size);

		/* Assume it is sha256 */
		entry->hash_algo = prop ? FMAP_HASH_SHA256 : FMAP_HASH_NONE;
		entry->hash = (uint8_t *)prop;
	}

	return 0;
}

int ofnode_decode_region(ofnode node, const char *prop_name, fdt_addr_t *basep,
			 fdt_size_t *sizep)
{
	const fdt_addr_t *cell;
	int len;

	log_debug("%s: %s: %s\n", __func__, ofnode_get_name(node), prop_name);
	cell = ofnode_get_property(node, prop_name, &len);
	if (!cell || (len < sizeof(fdt_addr_t) * 2)) {
		dm_warn("cell=%p, len=%d\n", cell, len);
		return -1;
	}

	*basep = fdt_addr_to_cpu(*cell);
	*sizep = fdt_size_to_cpu(cell[1]);
	log_debug("%s: base=%08lx, size=%lx\n", __func__, (ulong)*basep,
		  (ulong)*sizep);

	return 0;
}

int ofnode_decode_memory_region(ofnode config_node, const char *mem_type,
				const char *suffix, fdt_addr_t *basep,
				fdt_size_t *sizep)
{
	char prop_name[50];
	const char *mem;
	fdt_size_t size, offset_size;
	fdt_addr_t base, offset;
	ofnode node;

	if (!ofnode_valid(config_node)) {
		config_node = ofnode_path("/config");
		if (!ofnode_valid(config_node)) {
			dm_warn("%s: Cannot find /config node\n", __func__);
			return -ENOENT;
		}
	}
	if (!suffix)
		suffix = "";

	snprintf(prop_name, sizeof(prop_name), "%s-memory%s", mem_type,
		 suffix);
	mem = ofnode_read_string(config_node, prop_name);
	if (!mem) {
		dm_warn("%s: No memory type for '%s', using /memory\n", __func__,
			prop_name);
		mem = "/memory";
	}

	node = ofnode_path(mem);
	if (!ofnode_valid(node)) {
		dm_warn("%s: Failed to find node '%s'\n", __func__, mem);
		return -ENOENT;
	}

	/*
	 * Not strictly correct - the memory may have multiple banks. We just
	 * use the first
	 */
	if (ofnode_decode_region(node, "reg", &base, &size)) {
		dm_warn("%s: Failed to decode memory region %s\n", __func__,
			mem);
		return -EINVAL;
	}

	snprintf(prop_name, sizeof(prop_name), "%s-offset%s", mem_type,
		 suffix);
	if (ofnode_decode_region(config_node, prop_name, &offset,
				 &offset_size)) {
		dm_warn("%s: Failed to decode memory region '%s'\n", __func__,
			prop_name);
		return -EINVAL;
	}

	*basep = base + offset;
	*sizep = offset_size;

	return 0;
}

bool ofnode_phy_is_fixed_link(ofnode eth_node, ofnode *phy_node)
{
	ofnode node, subnode;
	int len;

	subnode = ofnode_find_subnode(eth_node, "fixed-link");
	if (ofnode_valid(subnode)) {
		/* new binding */
		node = subnode;
	} else if (ofnode_get_property(eth_node, "fixed-link", &len) &&
		   len == (5 * sizeof(__be32))) {
		/* old binding */
		node = eth_node;
	} else {
		return false;
	}

	if (phy_node)
		*phy_node = node;

	return true;
}

bool ofnode_eth_uses_inband_aneg(ofnode eth_node)
{
	bool inband_aneg = false;
	const char *managed;

	managed = ofnode_read_string(eth_node, "managed");
	if (managed && !strcmp(managed, "in-band-status"))
		inband_aneg = true;

	return inband_aneg;
}

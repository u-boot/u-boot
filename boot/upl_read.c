// SPDX-License-Identifier: GPL-2.0+
/*
 * UPL handoff parsing
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY UCLASS_BOOTSTD

#include <log.h>
#include <upl.h>
#include <dm/ofnode.h>
#include "upl_common.h"

/**
 * read_addr() - Read an address
 *
 * Reads an address in the correct format, either 32- or 64-bit
 *
 * @upl: UPL state
 * @node: Node to read from
 * @prop: Property name to read
 * @addr: Place to put the address
 * Return: 0 if OK, -ve on error
 */
static int read_addr(const struct upl *upl, ofnode node, const char *prop,
		     ulong *addrp)
{
	int ret;

	if (upl->addr_cells == 1) {
		u32 val;

		ret = ofnode_read_u32(node, prop, &val);
		if (!ret)
			*addrp = val;
	} else {
		u64 val;

		ret = ofnode_read_u64(node, prop, &val);
		if (!ret)
			*addrp = val;
	}

	return ret;
}

/**
 * read_size() - Read a size
 *
 * Reads a size in the correct format, either 32- or 64-bit
 *
 * @upl: UPL state
 * @node: Node to read from
 * @prop: Property name to read
 * @addr: Place to put the size
 * Return: 0 if OK, -ve on error
 */
static int read_size(const struct upl *upl, ofnode node, const char *prop,
		     ulong *sizep)
{
	int ret;

	if (upl->size_cells == 1) {
		u32 val;

		ret = ofnode_read_u32(node, prop, &val);
		if (!ret)
			*sizep = val;
	} else {
		u64 val;

		ret = ofnode_read_u64(node, prop, &val);
		if (!ret)
			*sizep = val;
	}

	return ret;
}

/**
 * ofnode_read_bitmask() - Read a bit mask from a string list
 *
 * @node: Node to read from
 * @prop: Property name to read
 * @names: Array of names for each bit
 * @count: Number of array entries
 * @value: Returns resulting bit-mask value on success
 * Return: 0 if OK, -EINVAL if a bit number is not defined, -ENOSPC if the
 * string is too long for the (internal) buffer, -EINVAL if no such property
 */
static int ofnode_read_bitmask(ofnode node, const char *prop,
			       const char *const names[], uint count,
			       uint *valuep)
{
	const char **list;
	const char **strp;
	uint val;
	uint bit;
	int ret;

	ret = ofnode_read_string_list(node, prop, &list);
	if (ret < 0)
		return log_msg_ret("rea", ret);

	val = 0;
	for (strp = list; *strp; strp++) {
		const char *str = *strp;
		bool found = false;

		for (bit = 0; bit < count; bit++) {
			if (!strcmp(str, names[bit])) {
				found = true;
				break;
			}
		}
		if (found)
			val |= BIT(bit);
		else
			log_warning("%s/%s: Invalid value '%s'\n",
				    ofnode_get_name(node), prop, str);
	}
	*valuep = val;

	return 0;
}

/**
 * ofnode_read_value() - Read a string value as an int using a lookup
 *
 * @node: Node to read from
 * @prop: Property name to read
 * @names: Array of names for each int value
 * @count: Number of array entries
 * @valuep: Returns int value read
 * Return: 0 if OK, -EINVAL if a bit number is not defined, -ENOENT if the
 * property does not exist
 */
static int ofnode_read_value(ofnode node, const char *prop,
			     const char *const names[], uint count,
			     uint *valuep)
{
	const char *str;
	int i;

	str = ofnode_read_string(node, prop);
	if (!str)
		return log_msg_ret("rd", -ENOENT);

	for (i = 0; i < count; i++) {
		if (!strcmp(names[i], str)) {
			*valuep = i;
			return 0;
		}
	}

	log_debug("Unnamed value '%s'\n", str);
	return log_msg_ret("val", -EINVAL);
}

static int read_uint(ofnode node, const char *prop, uint *valp)
{
	u32 val;
	int ret;

	ret = ofnode_read_u32(node, prop, &val);
	if (ret)
		return ret;
	*valp = val;

	return 0;
}

/**
 * decode_root_props() - Decode root properties from the tree
 *
 * @upl: UPL state
 * @node: Node to decode
 * Return 0 if OK, -ve on error
 */
static int decode_root_props(struct upl *upl, ofnode node)
{
	int ret;

	ret = read_uint(node, UPLP_ADDRESS_CELLS, &upl->addr_cells);
	if (!ret)
		ret = read_uint(node, UPLP_SIZE_CELLS, &upl->size_cells);
	if (ret)
		return log_msg_ret("cel", ret);

	return 0;
}

/**
 * decode_root_props() - Decode UPL parameters from the tree
 *
 * @upl: UPL state
 * @node: Node to decode
 * Return 0 if OK, -ve on error
 */
static int decode_upl_params(struct upl *upl, ofnode options)
{
	ofnode node;
	int ret;

	node = ofnode_find_subnode(options, UPLN_UPL_PARAMS);
	if (!ofnode_valid(node))
		return log_msg_ret("par", -EINVAL);
	log_debug("decoding '%s'\n", ofnode_get_name(node));

	ret = read_addr(upl, node, UPLP_SMBIOS, &upl->smbios);
	if (ret)
		return log_msg_ret("smb", ret);
	ret = read_addr(upl, node, UPLP_ACPI, &upl->acpi);
	if (ret)
		return log_msg_ret("acp", ret);
	ret = ofnode_read_bitmask(node, UPLP_BOOTMODE, bootmode_names,
				  UPLBM_COUNT, &upl->bootmode);
	if (ret)
		return log_msg_ret("boo", ret);
	ret = read_uint(node, UPLP_ADDR_WIDTH, &upl->addr_width);
	if (ret)
		return log_msg_ret("add", ret);
	ret = read_uint(node, UPLP_ACPI_NVS_SIZE, &upl->acpi_nvs_size);
	if (ret)
		return log_msg_ret("nvs", ret);

	return 0;
}

/**
 * decode_upl_images() - Decode /options/upl-image nodes
 *
 * @node: /options node in which to look for the node
 * Return 0 if OK, -ve on error
 */
static int decode_upl_images(struct upl *upl, ofnode options)
{
	ofnode node, images;
	int ret;

	images = ofnode_find_subnode(options, UPLN_UPL_IMAGE);
	if (!ofnode_valid(images))
		return log_msg_ret("img", -EINVAL);
	log_debug("decoding '%s'\n", ofnode_get_name(images));

	ret = read_addr(upl, images, UPLP_FIT, &upl->fit);
	if (!ret)
		ret = read_uint(images, UPLP_CONF_OFFSET, &upl->conf_offset);
	if (ret)
		return log_msg_ret("cnf", ret);

	ofnode_for_each_subnode(node, images) {
		struct upl_image img;

		ret = read_addr(upl, node, UPLP_LOAD, &img.load);
		if (!ret)
			ret = read_size(upl, node, UPLP_SIZE, &img.size);
		if (!ret)
			ret = read_uint(node, UPLP_OFFSET, &img.offset);
		img.description = ofnode_read_string(node, UPLP_DESCRIPTION);
		if (!img.description)
			return log_msg_ret("sim", ret);
		if (!alist_add(&upl->image, img))
			return log_msg_ret("img", -ENOMEM);
	}

	return 0;
}

/**
 * decode_addr_size() - Decide a set of addr/size pairs
 *
 * Each base/size value from the devicetree is written to the region list
 *
 * @upl: UPL state
 * @buf: Bytes to decode
 * @size: Number of bytes to decode
 * @regions: List of regions to process (struct memregion)
 * Returns: number of regions found, if OK, else -ve on error
 */
static int decode_addr_size(const struct upl *upl, const char *buf, int size,
			    struct alist *regions)
{
	const char *ptr, *end = buf + size;
	int i;

	alist_init_struct(regions, struct memregion);
	ptr = buf;
	for (i = 0; ptr < end; i++) {
		struct memregion reg;

		if (upl->addr_cells == 1)
			reg.base = fdt32_to_cpu(*(u32 *)ptr);
		else
			reg.base = fdt64_to_cpu(*(u64 *)ptr);
		ptr += upl->addr_cells * sizeof(u32);

		if (upl->size_cells == 1)
			reg.size = fdt32_to_cpu(*(u32 *)ptr);
		else
			reg.size = fdt64_to_cpu(*(u64 *)ptr);
		ptr += upl->size_cells * sizeof(u32);
		if (ptr > end)
			return -ENOSPC;

		if (!alist_add(regions, reg))
			return log_msg_ret("reg", -ENOMEM);
	}

	return i;
}

/**
 * node_matches_at() - Check if a node name matches "base@..."
 *
 * Return: true if the node name matches the base string followed by an @ sign;
 * false otherwise
 */
static bool node_matches_at(ofnode node, const char *base)
{
	const char *name = ofnode_get_name(node);
	int len = strlen(base);

	return !strncmp(base, name, len) && name[len] == '@';
}

/**
 * decode_upl_memory_node() - Decode a /memory node from the tree
 *
 * @upl: UPL state
 * @node: Node to decode
 * Return 0 if OK, -ve on error
 */
static int decode_upl_memory_node(struct upl *upl, ofnode node)
{
	struct upl_mem mem;
	const char *buf;
	int size, len;

	buf = ofnode_read_prop(node, UPLP_REG, &size);
	if (!buf) {
		log_warning("Node '%s': Missing '%s' property\n",
			    ofnode_get_name(node), UPLP_REG);
		return log_msg_ret("reg", -EINVAL);
	}
	len = decode_addr_size(upl, buf, size, &mem.region);
	if (len < 0)
		return log_msg_ret("buf", len);
	mem.hotpluggable = ofnode_read_bool(node, UPLP_HOTPLUGGABLE);
	if (!alist_add(&upl->mem, mem))
		return log_msg_ret("mem", -ENOMEM);

	return 0;
}

/**
 * decode_upl_memmap() - Decode memory-map nodes from the tree
 *
 * @upl: UPL state
 * @root: Parent node containing the /memory-map nodes
 * Return 0 if OK, -ve on error
 */
static int decode_upl_memmap(struct upl *upl, ofnode root)
{
	ofnode node;

	ofnode_for_each_subnode(node, root) {
		struct upl_memmap memmap;
		int size, len, ret;
		const char *buf;

		memmap.name = ofnode_get_name(node);
		memmap.usage = 0;

		buf = ofnode_read_prop(node, UPLP_REG, &size);
		if (!buf) {
			log_warning("Node '%s': Missing '%s' property\n",
				    ofnode_get_name(node), UPLP_REG);
			continue;
		}

		len = decode_addr_size(upl, buf, size, &memmap.region);
		if (len < 0)
			return log_msg_ret("buf", len);
		ret = ofnode_read_bitmask(node, UPLP_USAGE, usage_names,
					  UPLUS_COUNT, &memmap.usage);
		if (ret && ret != -EINVAL)	/* optional property */
			return log_msg_ret("bit", ret);

		if (!alist_add(&upl->memmap, memmap))
			return log_msg_ret("mmp", -ENOMEM);
	}

	return 0;
}

/**
 * decode_upl_memres() - Decode reserved-memory nodes from the tree
 *
 * @upl: UPL state
 * @root: Parent node containing the reserved-memory nodes
 * Return 0 if OK, -ve on error
 */
static int decode_upl_memres(struct upl *upl, ofnode root)
{
	ofnode node;

	ofnode_for_each_subnode(node, root) {
		struct upl_memres memres;
		const char *buf;
		int size, len;

		log_debug("decoding '%s'\n", ofnode_get_name(node));
		memres.name = ofnode_get_name(node);

		buf = ofnode_read_prop(node, UPLP_REG, &size);
		if (!buf) {
			log_warning("Node '%s': Missing 'reg' property\n",
				    ofnode_get_name(node));
			continue;
		}

		len = decode_addr_size(upl, buf, size, &memres.region);
		if (len < 0)
			return log_msg_ret("buf", len);
		memres.no_map = ofnode_read_bool(node, UPLP_NO_MAP);

		if (!alist_add(&upl->memres, memres))
			return log_msg_ret("mre", -ENOMEM);
	}

	return 0;
}

/**
 * decode_upl_serial() - Decode the serial node
 *
 * @upl: UPL state
 * @root: Parent node contain node
 * Return 0 if OK, -ve on error
 */
static int decode_upl_serial(struct upl *upl, ofnode node)
{
	struct upl_serial *ser = &upl->serial;
	const char *buf;
	int len, size;
	int ret;

	ser->compatible = ofnode_read_string(node, UPLP_COMPATIBLE);
	if (!ser->compatible) {
		log_warning("Node '%s': Missing compatible string\n",
			    ofnode_get_name(node));
		return log_msg_ret("com", -EINVAL);
	}
	ret = read_uint(node, UPLP_CLOCK_FREQUENCY, &ser->clock_frequency);
	if (!ret)
		ret = read_uint(node, UPLP_CURRENT_SPEED, &ser->current_speed);
	if (ret)
		return log_msg_ret("spe", ret);

	buf = ofnode_read_prop(node, UPLP_REG, &size);
	if (!buf) {
		log_warning("Node '%s': Missing 'reg' property\n",
			    ofnode_get_name(node));
		return log_msg_ret("reg", -EINVAL);
	}

	len = decode_addr_size(upl, buf, sizeof(buf), &ser->reg);
	if (len < 0)
		return log_msg_ret("buf", len);

	/* set defaults */
	ser->reg_io_shift = UPLD_REG_IO_SHIFT;
	ser->reg_offset = UPLD_REG_OFFSET;
	ser->reg_io_width = UPLD_REG_IO_WIDTH;
	read_uint(node, UPLP_REG_IO_SHIFT, &ser->reg_io_shift);
	read_uint(node, UPLP_REG_OFFSET, &ser->reg_offset);
	read_uint(node, UPLP_REG_IO_WIDTH, &ser->reg_io_width);
	read_addr(upl, node, UPLP_VIRTUAL_REG, &ser->virtual_reg);
	ret = ofnode_read_value(node, UPLP_ACCESS_TYPE, access_types,
				ARRAY_SIZE(access_types), &ser->access_type);
	if (ret && ret != -ENOENT)
		return log_msg_ret("ser", ret);

	return 0;
}

/**
 * decode_upl_graphics() - Decode graphics node
 *
 * @upl: UPL state
 * @root: Node to decode
 * Return 0 if OK, -ve on error
 */
static int decode_upl_graphics(struct upl *upl, ofnode node)
{
	struct upl_graphics *gra = &upl->graphics;
	const char *buf, *compat;
	int len, size;
	int ret;

	compat = ofnode_read_string(node, UPLP_COMPATIBLE);
	if (!compat) {
		log_warning("Node '%s': Missing compatible string\n",
			    ofnode_get_name(node));
		return log_msg_ret("com", -EINVAL);
	}
	if (strcmp(UPLC_GRAPHICS, compat)) {
		log_warning("Node '%s': Ignoring compatible '%s'\n",
			    ofnode_get_name(node), compat);
		return 0;
	}

	buf = ofnode_read_prop(node, UPLP_REG, &size);
	if (!buf) {
		log_warning("Node '%s': Missing 'reg' property\n",
			    ofnode_get_name(node));
		return log_msg_ret("reg", -EINVAL);
	}

	len = decode_addr_size(upl, buf, size, &gra->reg);
	if (len < 0)
		return log_msg_ret("buf", len);

	ret = read_uint(node, UPLP_WIDTH, &gra->width);
	if (!ret)
		ret = read_uint(node, UPLP_HEIGHT, &gra->height);
	if (!ret)
		ret = read_uint(node, UPLP_STRIDE, &gra->stride);
	if (!ret) {
		ret = ofnode_read_value(node, UPLP_GRAPHICS_FORMAT,
					graphics_formats,
					ARRAY_SIZE(graphics_formats),
					&gra->format);
	}
	if (ret)
		return log_msg_ret("pro", ret);

	return 0;
}

int upl_read_handoff(struct upl *upl, oftree tree)
{
	ofnode root, node;
	int ret;

	if (!oftree_valid(tree))
		return log_msg_ret("tre", -EINVAL);

	root = oftree_root(tree);

	upl_init(upl);
	ret = decode_root_props(upl, root);
	if (ret)
		return log_msg_ret("roo", ret);

	ofnode_for_each_subnode(node, root) {
		const char *name = ofnode_get_name(node);

		log_debug("decoding '%s'\n", name);
		if (!strcmp(UPLN_OPTIONS, name)) {
			ret = decode_upl_params(upl, node);
			if (ret)
				return log_msg_ret("opt", ret);

			ret = decode_upl_images(upl, node);
		} else if (node_matches_at(node, UPLN_MEMORY)) {
			ret = decode_upl_memory_node(upl, node);
		} else if (!strcmp(UPLN_MEMORY_MAP, name)) {
			ret = decode_upl_memmap(upl, node);
		} else if (!strcmp(UPLN_MEMORY_RESERVED, name)) {
			ret = decode_upl_memres(upl, node);
		} else if (node_matches_at(node, UPLN_SERIAL)) {
			ret = decode_upl_serial(upl, node);
		} else if (node_matches_at(node, UPLN_GRAPHICS)) {
			ret = decode_upl_graphics(upl, node);
		} else {
			log_debug("Unknown node '%s'\n", name);
			ret = 0;
		}
		if (ret)
			return log_msg_ret("err", ret);
	}

	return 0;
}

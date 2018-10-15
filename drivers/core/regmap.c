// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <regmap.h>
#include <asm/io.h>
#include <dm/of_addr.h>
#include <linux/ioport.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * regmap_alloc() - Allocate a regmap with a given number of ranges.
 *
 * @count: Number of ranges to be allocated for the regmap.
 * Return: A pointer to the newly allocated regmap, or NULL on error.
 */
static struct regmap *regmap_alloc(int count)
{
	struct regmap *map;

	map = malloc(sizeof(*map) + sizeof(map->ranges[0]) * count);
	if (!map)
		return NULL;
	map->range_count = count;

	return map;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int regmap_init_mem_platdata(struct udevice *dev, fdt_val_t *reg, int count,
			     struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	for (range = map->ranges; count > 0; reg += 2, range++, count--) {
		range->start = *reg;
		range->size = reg[1];
	}

	*mapp = map;

	return 0;
}
#else
/**
 * init_range() - Initialize a single range of a regmap
 * @node:     Device node that will use the map in question
 * @range:    Pointer to a regmap_range structure that will be initialized
 * @addr_len: The length of the addr parts of the reg property
 * @size_len: The length of the size parts of the reg property
 * @index:    The index of the range to initialize
 *
 * This function will read the necessary 'reg' information from the device tree
 * (the 'addr' part, and the 'length' part), and initialize the range in
 * quesion.
 *
 * Return: 0 if OK, -ve on error
 */
static int init_range(ofnode node, struct regmap_range *range, int addr_len,
		      int size_len, int index)
{
	fdt_size_t sz;
	struct resource r;

	if (of_live_active()) {
		int ret;

		ret = of_address_to_resource(ofnode_to_np(node),
					     index, &r);
		if (ret) {
			debug("%s: Could not read resource of range %d (ret = %d)\n",
			      ofnode_get_name(node), index, ret);
			return ret;
		}

		range->start = r.start;
		range->size = r.end - r.start + 1;
	} else {
		int offset = ofnode_to_offset(node);

		range->start = fdtdec_get_addr_size_fixed(gd->fdt_blob, offset,
							  "reg", index,
							  addr_len, size_len,
							  &sz, true);
		if (range->start == FDT_ADDR_T_NONE) {
			debug("%s: Could not read start of range %d\n",
			      ofnode_get_name(node), index);
			return -EINVAL;
		}

		range->size = sz;
	}

	return 0;
}

int regmap_init_mem(ofnode node, struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;
	int count;
	int addr_len, size_len, both_len;
	int len;
	int index;

	addr_len = ofnode_read_simple_addr_cells(ofnode_get_parent(node));
	if (addr_len < 0) {
		debug("%s: Error while reading the addr length (ret = %d)\n",
		      ofnode_get_name(node), addr_len);
		return addr_len;
	}

	size_len = ofnode_read_simple_size_cells(ofnode_get_parent(node));
	if (size_len < 0) {
		debug("%s: Error while reading the size length: (ret = %d)\n",
		      ofnode_get_name(node), size_len);
		return size_len;
	}

	both_len = addr_len + size_len;
	if (!both_len) {
		debug("%s: Both addr and size length are zero\n",
		      ofnode_get_name(node));
		return -EINVAL;
	}

	len = ofnode_read_size(node, "reg");
	if (len < 0) {
		debug("%s: Error while reading reg size (ret = %d)\n",
		      ofnode_get_name(node), len);
		return len;
	}
	len /= sizeof(fdt32_t);
	count = len / both_len;
	if (!count) {
		debug("%s: Not enough data in reg property\n",
		      ofnode_get_name(node));
		return -EINVAL;
	}

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	for (range = map->ranges, index = 0; count > 0;
	     count--, range++, index++) {
		int ret = init_range(node, range, addr_len, size_len, index);

		if (ret)
			return ret;
	}

	*mapp = map;

	return 0;
}
#endif

void *regmap_get_range(struct regmap *map, unsigned int range_num)
{
	struct regmap_range *range;

	if (range_num >= map->range_count)
		return NULL;
	range = &map->ranges[range_num];

	return map_sysmem(range->start, range->size);
}

int regmap_uninit(struct regmap *map)
{
	free(map);

	return 0;
}

int regmap_read(struct regmap *map, uint offset, uint *valp)
{
	u32 *ptr = map_physmem(map->ranges[0].start + offset, 4, MAP_NOCACHE);

	*valp = le32_to_cpu(readl(ptr));

	return 0;
}

int regmap_write(struct regmap *map, uint offset, uint val)
{
	u32 *ptr = map_physmem(map->ranges[0].start + offset, 4, MAP_NOCACHE);

	writel(cpu_to_le32(val), ptr);

	return 0;
}

int regmap_update_bits(struct regmap *map, uint offset, uint mask, uint val)
{
	uint reg;
	int ret;

	ret = regmap_read(map, offset, &reg);
	if (ret)
		return ret;

	reg &= ~mask;

	return regmap_write(map, offset, reg | val);
}

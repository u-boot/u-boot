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
int regmap_init_mem(ofnode node, struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;
	int count;
	int addr_len, size_len, both_len;
	int len;
	int index;
	struct resource r;

	addr_len = ofnode_read_simple_addr_cells(ofnode_get_parent(node));
	size_len = ofnode_read_simple_size_cells(ofnode_get_parent(node));
	both_len = addr_len + size_len;

	len = ofnode_read_size(node, "reg");
	if (len < 0)
		return len;
	len /= sizeof(fdt32_t);
	count = len / both_len;
	if (!count)
		return -EINVAL;

	map = regmap_alloc(count);
	if (!map)
		return -ENOMEM;

	for (range = map->ranges, index = 0; count > 0;
	     count--, range++, index++) {
		fdt_size_t sz;
		if (of_live_active()) {
			of_address_to_resource(ofnode_to_np(node), index, &r);
			range->start = r.start;
			range->size = r.end - r.start + 1;
		} else {
			range->start = fdtdec_get_addr_size_fixed(gd->fdt_blob,
					ofnode_to_offset(node), "reg", index,
					addr_len, size_len, &sz, true);
			range->size = sz;
		}
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

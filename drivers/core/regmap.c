/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <libfdt.h>
#include <malloc.h>
#include <mapmem.h>
#include <regmap.h>

#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

static struct regmap *regmap_alloc_count(int count)
{
	struct regmap *map;

	map = malloc(sizeof(struct regmap));
	if (!map)
		return NULL;
	if (count <= 1) {
		map->range = &map->base_range;
	} else {
		map->range = malloc(count * sizeof(struct regmap_range));
		if (!map->range) {
			free(map);
			return NULL;
		}
	}
	map->range_count = count;

	return map;
}

#if CONFIG_IS_ENABLED(OF_PLATDATA)
int regmap_init_mem_platdata(struct udevice *dev, u32 *reg, int count,
			     struct regmap **mapp)
{
	struct regmap_range *range;
	struct regmap *map;

	map = regmap_alloc_count(count);
	if (!map)
		return -ENOMEM;

	map->base = *reg;
	for (range = map->range; count > 0; reg += 2, range++, count--) {
		range->start = *reg;
		range->size = reg[1];
	}

	*mapp = map;

	return 0;
}
#else
int regmap_init_mem(struct udevice *dev, struct regmap **mapp)
{
	const void *blob = gd->fdt_blob;
	struct regmap_range *range;
	const fdt32_t *cell;
	struct regmap *map;
	int count;
	int addr_len, size_len, both_len;
	int parent;
	int len;
	int index;

	parent = dev_of_offset(dev->parent);
	addr_len = fdt_address_cells(blob, parent);
	size_len = fdt_size_cells(blob, parent);
	both_len = addr_len + size_len;

	cell = fdt_getprop(blob, dev_of_offset(dev), "reg", &len);
	len /= sizeof(*cell);
	count = len / both_len;
	if (!cell || !count)
		return -EINVAL;

	map = regmap_alloc_count(count);
	if (!map)
		return -ENOMEM;

	for (range = map->range, index = 0; count > 0;
	     count--, cell += both_len, range++, index++) {
		fdt_size_t sz;
		range->start = fdtdec_get_addr_size_fixed(blob, dev->of_offset,
				"reg", index, addr_len, size_len, &sz, true);
		range->size = sz;
	}
	map->base = map->range[0].start;

	*mapp = map;

	return 0;
}
#endif

void *regmap_get_range(struct regmap *map, unsigned int range_num)
{
	struct regmap_range *range;

	if (range_num >= map->range_count)
		return NULL;
	range = &map->range[range_num];

	return map_sysmem(range->start, range->size);
}

int regmap_uninit(struct regmap *map)
{
	if (map->range_count > 1)
		free(map->range);
	free(map);

	return 0;
}

int regmap_read(struct regmap *map, uint offset, uint *valp)
{
	uint32_t *ptr = map_physmem(map->base + offset, 4, MAP_NOCACHE);

	*valp = le32_to_cpu(readl(ptr));

	return 0;
}

int regmap_write(struct regmap *map, uint offset, uint val)
{
	uint32_t *ptr = map_physmem(map->base + offset, 4, MAP_NOCACHE);

	writel(cpu_to_le32(val), ptr);

	return 0;
}

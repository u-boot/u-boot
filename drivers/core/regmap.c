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

DECLARE_GLOBAL_DATA_PTR;

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

	parent = dev->parent->of_offset;
	addr_len = fdt_address_cells(blob, parent);
	size_len = fdt_size_cells(blob, parent);
	both_len = addr_len + size_len;

	cell = fdt_getprop(blob, dev->of_offset, "reg", &len);
	len /= sizeof(*cell);
	count = len / both_len;
	if (!cell || !count)
		return -EINVAL;

	map = malloc(sizeof(struct regmap));
	if (!map)
		return -ENOMEM;

	if (count <= 1) {
		map->range = &map->base_range;
	} else {
		map->range = malloc(count * sizeof(struct regmap_range));
		if (!map->range) {
			free(map);
			return -ENOMEM;
		}
	}

	map->base = fdtdec_get_number(cell, addr_len);
	map->range_count = count;

	for (range = map->range; count > 0;
	     count--, cell += both_len, range++) {
		range->start = fdtdec_get_number(cell, addr_len);
		range->size = fdtdec_get_number(cell + addr_len, size_len);
	}

	*mapp = map;

	return 0;
}

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

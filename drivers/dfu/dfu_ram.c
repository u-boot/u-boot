/*
 * (C) Copyright 2013
 * Afzal Mohammed <afzal.mohd.ma@gmail.com>
 *
 * Reference: dfu_mmc.c
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <malloc.h>
#include <errno.h>
#include <dfu.h>

static int dfu_transfer_medium_ram(enum dfu_op op, struct dfu_entity *dfu,
				   u64 offset, void *buf, long *len)
{
	if (dfu->layout != DFU_RAM_ADDR) {
		error("unsupported layout: %s\n", dfu_get_layout(dfu->layout));
		return  -EINVAL;
	}

	if (offset > dfu->data.ram.size) {
		error("request exceeds allowed area\n");
		return -EINVAL;
	}

	if (op == DFU_OP_WRITE)
		memcpy(dfu->data.ram.start + offset, buf, *len);
	else
		memcpy(buf, dfu->data.ram.start + offset, *len);

	return 0;
}

static int dfu_write_medium_ram(struct dfu_entity *dfu, u64 offset,
				void *buf, long *len)
{
	return dfu_transfer_medium_ram(DFU_OP_WRITE, dfu, offset, buf, len);
}

static int dfu_read_medium_ram(struct dfu_entity *dfu, u64 offset,
			       void *buf, long *len)
{
	if (!*len) {
		*len = dfu->data.ram.size;
		return 0;
	}

	return dfu_transfer_medium_ram(DFU_OP_READ, dfu, offset, buf, len);
}

int dfu_fill_entity_ram(struct dfu_entity *dfu, char *s)
{
	char *st;

	dfu->dev_type = DFU_DEV_RAM;
	st = strsep(&s, " ");
	if (strcmp(st, "ram")) {
		error("unsupported device: %s\n", st);
		return -ENODEV;
	}

	dfu->layout = DFU_RAM_ADDR;
	dfu->data.ram.start = (void *)simple_strtoul(s, &s, 16);
	s++;
	dfu->data.ram.size = simple_strtoul(s, &s, 16);

	dfu->write_medium = dfu_write_medium_ram;
	dfu->read_medium = dfu_read_medium_ram;

	dfu->inited = 0;

	return 0;
}

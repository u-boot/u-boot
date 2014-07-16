/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <dm-demo.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* Shape size */
#define WIDTH	8
#define HEIGHT	6

struct shape_data {
	int num_chars;	/* Number of non-space characters output so far */
};

/* Crazy little function to draw shapes on the console */
static int shape_hello(struct udevice *dev, int ch)
{
	const struct dm_demo_pdata *pdata = dev_get_platdata(dev);
	struct shape_data *data = dev_get_priv(dev);
	static const struct shape {
		int start;
		int end;
		int dstart;
		int dend;
	} shapes[3] = {
		{ 0, 1, 0, 1 },
		{ 0, WIDTH, 0, 0 },
		{ HEIGHT / 2 - 1, WIDTH - HEIGHT / 2 + 1, -1, 1},
	};
	struct shape shape;
	unsigned int index;
	int line, pos, inside;
	const char *colour = pdata->colour;
	int first = 0;

	if (!ch)
		ch = pdata->default_char;
	if (!ch)
		ch = '@';

	index = (pdata->sides / 2) - 1;
	if (index >= ARRAY_SIZE(shapes))
		return -EIO;
	shape = shapes[index];

	for (line = 0; line < HEIGHT; line++) {
		first = 1;
		for (pos = 0; pos < WIDTH; pos++) {
			inside = pos >= shape.start && pos < shape.end;
			if (inside) {
				putc(first ? *colour++ : ch);
				data->num_chars++;
				first = 0;
				if (!*colour)
					colour = pdata->colour;
			} else {
				putc(' ');
			}
		}
		putc('\n');
		shape.start += shape.dstart;
		shape.end += shape.dend;
		if (shape.start < 0) {
			shape.dstart = -shape.dstart;
			shape.dend = -shape.dend;
			shape.start += shape.dstart;
			shape.end += shape.dend;
		}
	}

	return 0;
}

static int shape_status(struct udevice *dev, int *status)
{
	struct shape_data *data = dev_get_priv(dev);

	*status = data->num_chars;
	return 0;
}

static const struct demo_ops shape_ops = {
	.hello = shape_hello,
	.status = shape_status,
};

static int shape_ofdata_to_platdata(struct udevice *dev)
{
	struct dm_demo_pdata *pdata = dev_get_platdata(dev);
	int ret;

	/* Parse the data that is common with all demo devices */
	ret = demo_parse_dt(dev);
	if (ret)
		return ret;

	/* Parse the data that only we need */
	pdata->default_char = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					     "character", '@');

	return 0;
}

static const struct udevice_id demo_shape_id[] = {
	{ "demo-shape", 0 },
	{ },
};

U_BOOT_DRIVER(demo_shape_drv) = {
	.name	= "demo_shape_drv",
	.of_match = demo_shape_id,
	.id	= UCLASS_DEMO,
	.ofdata_to_platdata = shape_ofdata_to_platdata,
	.ops	= &shape_ops,
	.priv_auto_alloc_size = sizeof(struct shape_data),
	.platdata_auto_alloc_size = sizeof(struct dm_demo_pdata),
};

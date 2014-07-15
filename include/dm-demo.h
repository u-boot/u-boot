/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DM_DEMO_H
#define __DM_DEMO_H

#include <dm.h>

/**
 * struct dm_demo_pdata - configuration data for demo instance
 *
 * @colour: Color of the demo
 * @sides: Numbers of sides
 * @default_char: Default ASCII character to output (65 = 'A')
 */
struct dm_demo_pdata {
	const char *colour;
	int sides;
	int default_char;
};

struct demo_ops {
	int (*hello)(struct device *dev, int ch);
	int (*status)(struct device *dev, int *status);
};

int demo_hello(struct device *dev, int ch);
int demo_status(struct device *dev, int *status);
int demo_list(void);

int demo_parse_dt(struct device *dev);

#endif

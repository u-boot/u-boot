/*
 * Copyright (C) 2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>

#include <asm/arch/platform.h>

#include "sm.h"

struct device_state {
	int refcount;
};

static struct device_state device_state[NR_DEVICES];

static int claim_resource(const struct resource *res)
{
	int ret = 0;

	switch (res->type) {
	case RESOURCE_GPIO:
		ret = gpio_set_func(res->u.gpio.gpio_dev,
				    res->u.gpio.start,
				    res->u.gpio.nr_pins,
				    res->u.gpio.func);
		break;
	case RESOURCE_CLOCK:
		ret = pm_enable_clock(res->u.clock.id, res->u.clock.index);
		break;
	}

	return ret;
}

static void free_resource(const struct resource *res)
{
	switch (res->type) {
	case RESOURCE_GPIO:
		gpio_free(res->u.gpio.gpio_dev, res->u.gpio.start,
			  res->u.gpio.nr_pins);
		break;
	case RESOURCE_CLOCK:
		pm_disable_clock(res->u.clock.id, res->u.clock.index);
		break;
	}
}

static int init_dev(const struct device *dev)
{
	unsigned int i;
	int ret = 0;

	for (i = 0; i < dev->nr_resources; i++) {
		ret = claim_resource(&dev->resource[i]);
		if (ret)
			goto cleanup;
	}

	return 0;

cleanup:
	while (i--)
		free_resource(&dev->resource[i]);

	return ret;
}

const struct device *get_device(enum device_id devid)
{
	struct device_state *devstate;
	const struct device *dev;
	unsigned long flags;
	int initialized = 0;
	int ret = 0;

	devstate = &device_state[devid];
	dev = &chip_device[devid];

	flags = disable_interrupts();
	if (devstate->refcount++)
		initialized = 1;
	if (flags)
		enable_interrupts();

	if (!initialized)
		ret = init_dev(dev);

	return ret ? NULL : dev;
}

void put_device(const struct device *dev)
{
	struct device_state *devstate;
	unsigned long devid, flags;

	devid = (unsigned long)(dev - chip_device) / sizeof(struct device);
	devstate = &device_state[devid];

	flags = disable_interrupts();
	devstate--;
	if (!devstate) {
		unsigned int i;
		for (i = 0; i < dev->nr_resources; i++)
			free_resource(&dev->resource[i]);
	}
	if (flags)
		enable_interrupts();
}

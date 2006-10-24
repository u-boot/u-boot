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

#include <asm/errno.h>
#include <asm/io.h>
#include <asm/arch/platform.h>

#include "pio2.h"

struct pio_state {
	const struct device *dev;
	u32 alloc_mask;
};

static struct pio_state pio_state[CFG_NR_PIOS];

int gpio_set_func(enum device_id gpio_devid, unsigned int start,
		  unsigned int nr_pins, enum gpio_func func)
{
	const struct device *gpio;
	struct pio_state *state;
	u32 mask;

	state = &pio_state[gpio_devid - DEVICE_PIOA];

	gpio = get_device(gpio_devid);
	if (!gpio)
		return -EBUSY;

	state->dev = gpio;
	mask = ((1 << nr_pins) - 1) << start;

	if (mask & state->alloc_mask) {
		put_device(gpio);
		return -EBUSY;
	}
	state->alloc_mask |= mask;

	switch (func) {
	case GPIO_FUNC_GPIO:
		/* TODO */
		return -EINVAL;
	case GPIO_FUNC_A:
		pio2_writel(gpio, ASR, mask);
		pio2_writel(gpio, PDR, mask);
		pio2_writel(gpio, PUDR, mask);
		break;
	case GPIO_FUNC_B:
		pio2_writel(gpio, BSR, mask);
		pio2_writel(gpio, PDR, mask);
		pio2_writel(gpio, PUDR, mask);
		break;
	}

	return 0;
}

void gpio_free(enum device_id gpio_devid, unsigned int start,
	       unsigned int nr_pins)
{
	const struct device *gpio;
	struct pio_state *state;
	u32 mask;

	state = &pio_state[gpio_devid - DEVICE_PIOA];
	gpio = state->dev;
	mask = ((1 << nr_pins) - 1) << start;

	pio2_writel(gpio, ODR, mask);
	pio2_writel(gpio, PER, mask);

	state->alloc_mask &= ~mask;
	put_device(gpio);
}

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

#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/memory-map.h>

#include "pio2.h"

void gpio_select_periph_A(unsigned int pin, int use_pullup)
{
	void *base = gpio_pin_to_addr(pin);
	uint32_t mask = 1 << (pin & 0x1f);

	if (!base)
		panic("Invalid GPIO pin %u\n", pin);

	pio2_writel(base, ASR, mask);
	pio2_writel(base, PDR, mask);
	if (use_pullup)
		pio2_writel(base, PUER, mask);
	else
		pio2_writel(base, PUDR, mask);
}

void gpio_select_periph_B(unsigned int pin, int use_pullup)
{
	void *base = gpio_pin_to_addr(pin);
	uint32_t mask = 1 << (pin & 0x1f);

	if (!base)
		panic("Invalid GPIO pin %u\n", pin);

	pio2_writel(base, BSR, mask);
	pio2_writel(base, PDR, mask);
	if (use_pullup)
		pio2_writel(base, PUER, mask);
	else
		pio2_writel(base, PUDR, mask);
}

void gpio_select_pio(unsigned int pin, unsigned long gpiof_flags)
{
	void *base = gpio_pin_to_addr(pin);
	uint32_t mask = 1 << (pin & 0x1f);

	if (!base)
		panic("Invalid GPIO pin %u\n", pin);

	if (gpiof_flags & GPIOF_OUTPUT) {
		if (gpiof_flags & GPIOF_MULTIDRV)
			pio2_writel(base, MDER, mask);
		else
			pio2_writel(base, MDDR, mask);
		pio2_writel(base, PUDR, mask);
		pio2_writel(base, OER, mask);
	} else {
		if (gpiof_flags & GPIOF_PULLUP)
			pio2_writel(base, PUER, mask);
		else
			pio2_writel(base, PUDR, mask);
		if (gpiof_flags & GPIOF_DEGLITCH)
			pio2_writel(base, IFER, mask);
		else
			pio2_writel(base, IFDR, mask);
		pio2_writel(base, ODR, mask);
	}

	pio2_writel(base, PER, mask);
}

void gpio_set_value(unsigned int pin, int value)
{
	void *base = gpio_pin_to_addr(pin);
	uint32_t mask = 1 << (pin & 0x1f);

	if (!base)
		panic("Invalid GPIO pin %u\n", pin);

	if (value)
		pio2_writel(base, SODR, mask);
	else
		pio2_writel(base, CODR, mask);
}

int gpio_get_value(unsigned int pin)
{
	void *base = gpio_pin_to_addr(pin);
	int value;

	if (!base)
		panic("Invalid GPIO pin %u\n", pin);

	value = pio2_readl(base, PDSR);
	return (value >> (pin & 0x1f)) & 1;
}

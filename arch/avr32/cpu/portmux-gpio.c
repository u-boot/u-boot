/*
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>

void portmux_select_peripheral(void *port, unsigned long pin_mask,
		enum portmux_function func, unsigned long flags)
{
	/* Both pull-up and pull-down set means buskeeper */
	if (flags & PORTMUX_PULL_DOWN)
		gpio_writel(port, PDERS, pin_mask);
	else
		gpio_writel(port, PDERC, pin_mask);
	if (flags & PORTMUX_PULL_UP)
		gpio_writel(port, PUERS, pin_mask);
	else
		gpio_writel(port, PUERC, pin_mask);

	/* Select drive strength */
	if (flags & PORTMUX_DRIVE_LOW)
		gpio_writel(port, ODCR0S, pin_mask);
	else
		gpio_writel(port, ODCR0C, pin_mask);
	if (flags & PORTMUX_DRIVE_HIGH)
		gpio_writel(port, ODCR1S, pin_mask);
	else
		gpio_writel(port, ODCR1C, pin_mask);

	/* Select function */
	if (func & PORTMUX_FUNC_B)
		gpio_writel(port, PMR0S, pin_mask);
	else
		gpio_writel(port, PMR0C, pin_mask);
	if (func & PORTMUX_FUNC_C)
		gpio_writel(port, PMR1S, pin_mask);
	else
		gpio_writel(port, PMR1C, pin_mask);

	/* Disable GPIO (i.e. enable peripheral) */
	gpio_writel(port, GPERC, pin_mask);
}

void portmux_select_gpio(void *port, unsigned long pin_mask,
		unsigned long flags)
{
	/* Both pull-up and pull-down set means buskeeper */
	if (flags & PORTMUX_PULL_DOWN)
		gpio_writel(port, PDERS, pin_mask);
	else
		gpio_writel(port, PDERC, pin_mask);
	if (flags & PORTMUX_PULL_UP)
		gpio_writel(port, PUERS, pin_mask);
	else
		gpio_writel(port, PUERC, pin_mask);

	/* Enable open-drain mode if requested */
	if (flags & PORTMUX_OPEN_DRAIN)
		gpio_writel(port, ODMERS, pin_mask);
	else
		gpio_writel(port, ODMERC, pin_mask);

	/* Select drive strength */
	if (flags & PORTMUX_DRIVE_LOW)
		gpio_writel(port, ODCR0S, pin_mask);
	else
		gpio_writel(port, ODCR0C, pin_mask);
	if (flags & PORTMUX_DRIVE_HIGH)
		gpio_writel(port, ODCR1S, pin_mask);
	else
		gpio_writel(port, ODCR1C, pin_mask);

	/* Select direction and initial pin state */
	if (flags & PORTMUX_DIR_OUTPUT) {
		if (flags & PORTMUX_INIT_HIGH)
			gpio_writel(port, OVRS, pin_mask);
		else
			gpio_writel(port, OVRC, pin_mask);
		gpio_writel(port, ODERS, pin_mask);
	} else {
		gpio_writel(port, ODERC, pin_mask);
	}

	/* Enable GPIO */
	gpio_writel(port, GPERS, pin_mask);
}

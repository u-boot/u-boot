/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 *
 * This work is derived from the linux 2.6.27 kernel source
 * To fetch, use the kernel repository
 * git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
 * Use the v2.6.27 tag.
 *
 * Below is the original's header including its copyright
 *
 *  linux/arch/arm/plat-omap/gpio.c
 *
 * Support functions for OMAP GPIO
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 * Written by Juha Yrjölä <juha.yrjola@nokia.com>
 */
#ifndef _GPIO_H
#define _GPIO_H

#include <asm/arch/cpu.h>

struct gpio_bank {
	void *base;
	int method;
};

extern const struct gpio_bank *const omap_gpio_bank;

#define METHOD_GPIO_24XX	4

/**
 * Check if gpio is valid.
 *
 * @param gpio	GPIO number
 * @return 1 if ok, 0 on error
 */
int gpio_is_valid(int gpio);
#endif /* _GPIO_H_ */

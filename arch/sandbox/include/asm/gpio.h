/*
 * This is the interface to the sandbox GPIO driver for test code which
 * wants to change the GPIO values reported to U-Boot.
 *
 * Copyright (c) 2011 The Chromium OS Authors.
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

#ifndef __ASM_SANDBOX_GPIO_H
#define __ASM_SANDBOX_GPIO_H

/*
 * We use the generic interface, and add a back-channel.
 *
 * The back-channel functions are declared in this file. They should not be used
 * except in test code.
 *
 * Test code can, for example, call sandbox_gpio_set_value() to set the value of
 * a simulated GPIO. From then on, normal code in U-Boot will see this new
 * value when it calls gpio_get_value().
 *
 * NOTE: DO NOT use the functions in this file except in test code!
 */
#include <asm-generic/gpio.h>

/**
 * Return the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param gp	GPIO number
 * @return -1 on error, 0 if GPIO is low, >0 if high
 */
int sandbox_gpio_get_value(unsigned gp);

/**
 * Set the simulated value of a GPIO (used only in sandbox test code)
 *
 * @param gp	GPIO number
 * @param value	value to set (0 for low, non-zero for high)
 * @return -1 on error, 0 if ok
 */
int sandbox_gpio_set_value(unsigned gp, int value);

/**
 * Return the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param gp	GPIO number
 * @return -1 on error, 0 if GPIO is input, >0 if output
 */
int sandbox_gpio_get_direction(unsigned gp);

/**
 * Set the simulated direction of a GPIO (used only in sandbox test code)
 *
 * @param gp	GPIO number
 * @param output 0 to set as input, 1 to set as output
 * @return -1 on error, 0 if ok
 */
int sandbox_gpio_set_direction(unsigned gp, int output);

/* Display information about each GPIO */
void gpio_info(void);

#define gpio_status()	gpio_info()

#endif

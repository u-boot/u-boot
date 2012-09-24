/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
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


#ifndef __ASM_ARCH_SPEAR_GPIO_H
#define __ASM_ARCH_SPEAR_GPIO_H

enum gpio_direction {
	GPIO_DIRECTION_IN,
	GPIO_DIRECTION_OUT,
};

struct gpio_regs {
	u32 gpiodata[0x100];	/* 0x000 ... 0x3fc */
	u32 gpiodir;		/* 0x400 */
};

#define SPEAR_GPIO_COUNT		8
#define DATA_REG_ADDR(gpio)		(1 << (gpio + 2))

#endif	/* __ASM_ARCH_SPEAR_GPIO_H */

/*
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, <sbabic@denx.de>
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


#ifndef __ASM_ARCH_IMX_GPIO_H
#define __ASM_ARCH_IMX_GPIO_H

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
/* GPIO registers */
struct gpio_regs {
	u32 gpio_dr;	/* data */
	u32 gpio_dir;	/* direction */
	u32 gpio_psr;	/* pad satus */
};
#endif

#define IMX_GPIO_NR(port, index)		((((port)-1)*32)+((index)&31))

#endif

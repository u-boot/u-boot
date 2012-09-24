/*
 * Copyright (C) 2012
 * Philippe Reynes <tremyfr@yahoo.fr>
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


#ifndef __ASM_ARCH_MX27_GPIO_H
#define __ASM_ARCH_MX27_GPIO_H

/* GPIO registers */
struct gpio_regs {
	u32 gpio_dir; /* DDIR */
	u32 ocr1;
	u32 ocr2;
	u32 iconfa1;
	u32 iconfa2;
	u32 iconfb1;
	u32 iconfb2;
	u32 gpio_dr; /* DR */
	u32 gius;
	u32 gpio_psr; /* SSR */
	u32 icr1;
	u32 icr2;
	u32 imr;
	u32 isr;
	u32 gpr;
	u32 swr;
	u32 puen;
	u32 res[0x2f];
};

/* This structure is used by the function imx_gpio_mode */
struct gpio_port_regs {
	struct gpio_regs port[6];
};

#endif

/*
 * Copyright (C) 2012
 * Philippe Reynes <tremyfr@yahoo.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

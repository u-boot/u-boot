/*
 * (C) Copyright 2014
 * Vikas Manocha, ST Micoelectronics, vikas.manocha@st.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_STV0991_GPIO_H
#define __ASM_ARCH_STV0991_GPIO_H

enum gpio_direction {
	GPIO_DIRECTION_IN,
	GPIO_DIRECTION_OUT,
};

struct gpio_regs {
	u32 data;		/* offset 0x0 */
	u32 reserved[0xff];	/* 0x4--0x3fc */
	u32 dir;		/* offset 0x400 */
};

#endif	/* __ASM_ARCH_STV0991_GPIO_H */

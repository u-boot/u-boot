/*
 * Copyright (C) 2012 Vikram Narayananan
 * <vikram186@gmail.com>
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
 */

#ifndef _BCM2835_GPIO_H_
#define _BCM2835_GPIO_H_

#define BCM2835_GPIO_BASE		0x20200000
#define BCM2835_GPIO_COUNT		54

#define BCM2835_GPIO_FSEL_MASK		0x7
#define BCM2835_GPIO_INPUT		0x0
#define BCM2835_GPIO_OUTPUT		0x1
#define BCM2835_GPIO_ALT0		0x4
#define BCM2835_GPIO_ALT1		0x5
#define BCM2835_GPIO_ALT2		0x6
#define BCM2835_GPIO_ALT3		0x7
#define BCM2835_GPIO_ALT4		0x3
#define BCM2835_GPIO_ALT5		0x2

#define BCM2835_GPIO_COMMON_BANK(gpio)	((gpio < 32) ? 0 : 1)
#define BCM2835_GPIO_COMMON_SHIFT(gpio)	(gpio & 0x1f)

#define BCM2835_GPIO_FSEL_BANK(gpio)	(gpio / 10)
#define BCM2835_GPIO_FSEL_SHIFT(gpio)	((gpio % 10) * 3)

struct bcm2835_gpio_regs {
	u32 gpfsel[6];
	u32 reserved1;
	u32 gpset[2];
	u32 reserved2;
	u32 gpclr[2];
	u32 reserved3;
	u32 gplev[2];
	u32 reserved4;
	u32 gpeds[2];
	u32 reserved5;
	u32 gpren[2];
	u32 reserved6;
	u32 gpfen[2];
	u32 reserved7;
	u32 gphen[2];
	u32 reserved8;
	u32 gplen[2];
	u32 reserved9;
	u32 gparen[2];
	u32 reserved10;
	u32 gppud;
	u32 gppudclk[2];
};

#endif /* _BCM2835_GPIO_H_ */

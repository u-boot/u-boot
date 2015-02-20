/*
 * (C) Copyright 2009 Alessandro Rubini
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __NMK_GPIO_H__
#define __NMK_GPIO_H__

/*
 * These functions are called from the soft-i2c driver, but
 * are also used by board files to set output bits.
 */

enum nmk_af { /* alternate function settings */
	GPIO_GPIO = 0,
	GPIO_ALT_A,
	GPIO_ALT_B,
	GPIO_ALT_C
};

extern void nmk_gpio_af(int gpio, int alternate_function);
extern void nmk_gpio_dir(int gpio, int dir);
extern void nmk_gpio_set(int gpio, int val);
extern int nmk_gpio_get(int gpio);

#endif /* __NMK_GPIO_H__ */

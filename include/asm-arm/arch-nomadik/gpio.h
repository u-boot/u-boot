/*
 * (C) Copyright 2009 Alessandro Rubini
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

/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* 4xx PPC's have 2 GPIO controllers */
#if defined(CONFIG_405EZ) ||					\
	defined(CONFIG_440EP) || defined(CONFIG_440GR) ||	\
	defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
#define GPIO_GROUP_MAX	2
#else
#define GPIO_GROUP_MAX	1
#endif

#define GPIO_MAX	32
#define GPIO_ALT1_SEL	0x40000000
#define GPIO_ALT2_SEL	0x80000000
#define GPIO_ALT3_SEL	0xc0000000
#define GPIO_IN_SEL	0x40000000
#define GPIO_MASK	0xc0000000

#define GPIO_VAL(gpio)	(0x80000000 >> (gpio))

#ifndef __ASSEMBLY__
typedef enum gpio_select { GPIO_SEL, GPIO_ALT1, GPIO_ALT2, GPIO_ALT3 } gpio_select_t;
typedef enum gpio_driver { GPIO_DIS, GPIO_IN, GPIO_OUT, GPIO_BI } gpio_driver_t;
typedef enum gpio_out	 { GPIO_OUT_0, GPIO_OUT_1, GPIO_OUT_NO_CHG } gpio_out_t;

typedef struct {
	unsigned long add;	/* gpio core base address	*/
	gpio_driver_t in_out;	/* Driver Setting		*/
	gpio_select_t alt_nb;	/* Selected Alternate		*/
} gpio_param_s;
#endif

void gpio_config(int pin, int in_out, int gpio_alt, int out_val);
void gpio_write_bit(int pin, int val);
void gpio_set_chip_configuration(void);

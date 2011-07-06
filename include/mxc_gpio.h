/*
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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

#ifndef __MXC_GPIO_H
#define __MXC_GPIO_H

/* Converts a GPIO port number and the internal bit position
 * to the GPIO number
 */
#define MXC_GPIO_PORT_TO_NUM(port, bit) (((port - 1) << 5) + (bit & 0x1f))

enum mxc_gpio_direction {
	MXC_GPIO_DIRECTION_IN,
	MXC_GPIO_DIRECTION_OUT,
};

#ifdef CONFIG_MXC_GPIO
extern int mxc_gpio_direction(unsigned int gpio,
			       enum mxc_gpio_direction direction);
extern void mxc_gpio_set(unsigned int gpio, unsigned int value);
extern int mxc_gpio_get(unsigned int gpio);
#else
static inline int mxc_gpio_direction(unsigned int gpio,
				      enum mxc_gpio_direction direction)
{
	return 1;
}
static inline int mxc_gpio_get(unsigned int gpio)
{
	return 1;
}
static inline void mxc_gpio_set(unsigned int gpio, unsigned int value)
{
}
#endif

#endif

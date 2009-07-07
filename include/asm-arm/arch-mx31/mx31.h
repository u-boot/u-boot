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

#ifndef __ASM_ARCH_MX31_H
#define __ASM_ARCH_MX31_H

extern u32 mx31_get_ipg_clk(void);
extern void mx31_gpio_mux(unsigned long mode);

enum mx31_gpio_direction {
	MX31_GPIO_DIRECTION_IN,
	MX31_GPIO_DIRECTION_OUT,
};

#ifdef CONFIG_MX31_GPIO
extern int mx31_gpio_direction(unsigned int gpio,
			       enum mx31_gpio_direction direction);
extern void mx31_gpio_set(unsigned int gpio, unsigned int value);
#else
static inline int mx31_gpio_direction(unsigned int gpio,
				      enum mx31_gpio_direction direction)
{
	return 1;
}
static inline void mx31_gpio_set(unsigned int gpio, unsigned int value)
{
}
#endif

void mx31_uart1_hw_init(void);
void mx31_spi2_hw_init(void);

#endif /* __ASM_ARCH_MX31_H */

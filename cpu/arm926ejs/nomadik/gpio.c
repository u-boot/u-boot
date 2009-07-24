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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>

static unsigned long gpio_base[4] = {
	NOMADIK_GPIO0_BASE,
	NOMADIK_GPIO1_BASE,
	NOMADIK_GPIO2_BASE,
	NOMADIK_GPIO3_BASE
};

enum gpio_registers {
	GPIO_DAT =	0x00,		/* data register */
	GPIO_DATS =	0x04,		/* data set */
	GPIO_DATC =	0x08,		/* data clear */
	GPIO_PDIS =	0x0c,		/* pull disable */
	GPIO_DIR =	0x10,		/* direction */
	GPIO_DIRS =	0x14,		/* direction set */
	GPIO_DIRC =	0x18,		/* direction clear */
	GPIO_AFSLA =	0x20,		/* alternate function select A */
	GPIO_AFSLB =	0x24,		/* alternate function select B */
};

static inline unsigned long gpio_to_base(int gpio)
{
	return gpio_base[gpio / 32];
}

static inline u32 gpio_to_bit(int gpio)
{
	return 1 << (gpio & 0x1f);
}

void nmk_gpio_af(int gpio, int alternate_function)
{
	unsigned long base = gpio_to_base(gpio);
	u32 bit = gpio_to_bit(gpio);
	u32 afunc, bfunc;

	/* alternate function is 0..3, with one bit per register */
	afunc = readl(base + GPIO_AFSLA) & ~bit;
	bfunc = readl(base + GPIO_AFSLB) & ~bit;
	if (alternate_function & 1) afunc |= bit;
	if (alternate_function & 2) bfunc |= bit;
	writel(afunc, base + GPIO_AFSLA);
	writel(bfunc, base + GPIO_AFSLB);
}

void nmk_gpio_dir(int gpio, int dir)
{
	unsigned long base = gpio_to_base(gpio);
	u32 bit = gpio_to_bit(gpio);

	if (dir)
		writel(bit, base + GPIO_DIRS);
	else
		writel(bit, base + GPIO_DIRC);
}

void nmk_gpio_set(int gpio, int val)
{
	unsigned long base = gpio_to_base(gpio);
	u32 bit = gpio_to_bit(gpio);

	if (val)
		writel(bit, base + GPIO_DATS);
	else
		writel(bit, base + GPIO_DATC);
}

int nmk_gpio_get(int gpio)
{
	unsigned long base = gpio_to_base(gpio);
	u32 bit = gpio_to_bit(gpio);

	return readl(base + GPIO_DAT) & bit;
}

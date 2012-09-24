/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 *
 * Copyright (C) 2011
 * Stefano Babic, DENX Software Engineering, <sbabic@denx.de>
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
#include <asm/arch/imx-regs.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

enum mxc_gpio_direction {
	MXC_GPIO_DIRECTION_IN,
	MXC_GPIO_DIRECTION_OUT,
};

#define GPIO_TO_PORT(n)		(n / 32)

/* GPIO port description */
static unsigned long gpio_ports[] = {
	[0] = GPIO1_BASE_ADDR,
	[1] = GPIO2_BASE_ADDR,
	[2] = GPIO3_BASE_ADDR,
#if defined(CONFIG_MX25) || defined(CONFIG_MX51) || defined(CONFIG_MX53) || \
		defined(CONFIG_MX6Q)
	[3] = GPIO4_BASE_ADDR,
#endif
#if defined(CONFIG_MX53) || defined(CONFIG_MX6Q)
	[4] = GPIO5_BASE_ADDR,
	[5] = GPIO6_BASE_ADDR,
	[6] = GPIO7_BASE_ADDR,
#endif
};

static int mxc_gpio_direction(unsigned int gpio,
	enum mxc_gpio_direction direction)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 l;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	l = readl(&regs->gpio_dir);

	switch (direction) {
	case MXC_GPIO_DIRECTION_OUT:
		l |= 1 << gpio;
		break;
	case MXC_GPIO_DIRECTION_IN:
		l &= ~(1 << gpio);
	}
	writel(l, &regs->gpio_dir);

	return 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 l;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	l = readl(&regs->gpio_dr);
	if (value)
		l |= 1 << gpio;
	else
		l &= ~(1 << gpio);
	writel(l, &regs->gpio_dr);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	struct gpio_regs *regs;
	u32 val;

	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;

	gpio &= 0x1f;

	regs = (struct gpio_regs *)gpio_ports[port];

	val = (readl(&regs->gpio_dr) >> gpio) & 0x01;

	return val;
}

int gpio_request(unsigned gpio, const char *label)
{
	unsigned int port = GPIO_TO_PORT(gpio);
	if (port >= ARRAY_SIZE(gpio_ports))
		return -1;
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	return mxc_gpio_direction(gpio, MXC_GPIO_DIRECTION_IN);
}

int gpio_direction_output(unsigned gpio, int value)
{
	int ret = mxc_gpio_direction(gpio, MXC_GPIO_DIRECTION_OUT);

	if (ret < 0)
		return ret;

	return gpio_set_value(gpio, value);
}

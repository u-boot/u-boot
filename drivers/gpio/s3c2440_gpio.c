/*
 * Copyright (C) 2012
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/arch/s3c2440.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define GPIO_INPUT  0x0
#define GPIO_OUTPUT 0x1

/* 0x4 means that we want DAT and not CON register */
#define GPIO_PORT(x)	((((x) >> 5) & 0x3) + 0x4)
#define GPIO_BIT(x)		((x) & 0x3f)

/*
 * It's how we calculate the full port address
 * We have to get the number of the port + 1 (Port A is at 0x56000001 ...)
 * We move it at the second digit, and finally we add 0x4 because we want
 * to modify GPIO DAT and not CON
 */
#define GPIO_FULLPORT(x) (S3C24X0_GPIO_BASE | ((GPIO_PORT(gpio) + 1) << 1))

int gpio_set_value(unsigned gpio, int value)
{
	unsigned l = readl(GPIO_FULLPORT(gpio));
	unsigned bit;
	unsigned port = GPIO_FULLPORT(gpio);

	/*
	 * All GPIO Port have a configuration on
	 * 2 bits excepted the first GPIO (A) which
	 * have only 1 bit of configuration.
	 */
	if (!GPIO_PORT(gpio))
		bit = (0x1 << GPIO_BIT(gpio));
	else
		bit = (0x3 << GPIO_BIT(gpio));

	if (value)
		l |= bit;
	else
		l &= ~bit;

	return writel(l, port);
}

int gpio_get_value(unsigned gpio)
{
	unsigned l = readl(GPIO_FULLPORT(gpio));

	if (GPIO_PORT(gpio) == 0) /* PORT A */
		return (l >> GPIO_BIT(gpio)) & 0x1;
	return (l >> GPIO_BIT(gpio)) & 0x3;
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	return writel(GPIO_INPUT << GPIO_BIT(gpio), GPIO_FULLPORT(gpio));
}

int gpio_direction_output(unsigned gpio, int value)
{
	writel(GPIO_OUTPUT << GPIO_BIT(gpio), GPIO_FULLPORT(gpio));
	return gpio_set_value(gpio, value);
}

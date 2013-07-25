/*
 * OpenRISC gpio driver
 *
 * Copyright (C) 2011 Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 *
 * based on nios2 gpio driver
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * when CONFIG_SYS_GPIO_BASE is not defined, board may provide
 * its own driver.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef CONFIG_SYS_GPIO_BASE
#include <asm/io.h>

static inline int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

static inline int gpio_free(unsigned gpio)
{
	return 0;
}

static inline int gpio_get_value(unsigned gpio)
{
	return (readb(CONFIG_SYS_GPIO_BASE + gpio/8) >> gpio%8) & 0x1;
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	u8 tmp = readb(CONFIG_SYS_GPIO_BASE + gpio/8);

	if (value)
		tmp |= (1 << gpio%8);
	else
		tmp &= ~(1 << gpio%8);
	writeb(tmp, CONFIG_SYS_GPIO_BASE + gpio/8);
}

static inline int gpio_direction_input(unsigned gpio)
{
	gpio_set_value(gpio + CONFIG_SYS_GPIO_WIDTH, 0);

	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	gpio_set_value(gpio + CONFIG_SYS_GPIO_WIDTH, 1);
	gpio_set_value(gpio, value);

	return 0;
}

static inline int gpio_is_valid(int number)
{
	return ((unsigned)number) < CONFIG_SYS_GPIO_WIDTH;
}
#else
extern int gpio_request(unsigned gpio, const char *label);
extern int gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_is_valid(int number);
#endif /* CONFIG_SYS_GPIO_BASE */

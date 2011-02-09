/*
 * board gpio driver
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */
#include <common.h>
#include <asm/io.h>

#ifndef CONFIG_SYS_GPIO_BASE

#define ALTERA_PIO_BASE LED_PIO_BASE
#define ALTERA_PIO_WIDTH LED_PIO_WIDTH
#define ALTERA_PIO_DATA (ALTERA_PIO_BASE + 0)
#define ALTERA_PIO_DIR (ALTERA_PIO_BASE + 4)
static u32 pio_data_reg;
static u32 pio_dir_reg;

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
	u32 mask = 1 << gpio;
	writel(pio_dir_reg &= ~mask, ALTERA_PIO_DIR);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	u32 mask = 1 << gpio;
	if (value)
		pio_data_reg |= mask;
	else
		pio_data_reg &= ~mask;
	writel(pio_data_reg, ALTERA_PIO_DATA);
	writel(pio_dir_reg |= mask, ALTERA_PIO_DIR);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	u32 mask = 1 << gpio;
	if (pio_dir_reg & mask)
		return (pio_data_reg & mask) ? 1 : 0;
	else
		return (readl(ALTERA_PIO_DATA) & mask) ? 1 : 0;
}

void gpio_set_value(unsigned gpio, int value)
{
	u32 mask = 1 << gpio;
	if (value)
		pio_data_reg |= mask;
	else
		pio_data_reg &= ~mask;
	writel(pio_data_reg, ALTERA_PIO_DATA);
}

int gpio_is_valid(int number)
{
	return ((unsigned)number) < ALTERA_PIO_WIDTH;
}
#endif

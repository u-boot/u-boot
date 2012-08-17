/*
 * nios2 gpio driver
 *
 * This gpio core is described in http://nioswiki.com/GPIO
 * bit[0] data
 * bit[1] output enable
 *
 * When CONFIG_SYS_GPIO_BASE is not defined, the board may either
 * provide its own driver or the altera_pio driver may be used.
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ASM_NIOS2_GPIO_H_
#define _ASM_NIOS2_GPIO_H_

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

static inline int gpio_direction_input(unsigned gpio)
{
	writel(1, CONFIG_SYS_GPIO_BASE + (gpio << 2));
	return 0;
}

static inline int gpio_direction_output(unsigned gpio, int value)
{
	writel(value ? 3 : 2, CONFIG_SYS_GPIO_BASE + (gpio << 2));
	return 0;
}

static inline int gpio_get_value(unsigned gpio)
{
	return readl(CONFIG_SYS_GPIO_BASE + (gpio << 2));
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	writel(value ? 3 : 2, CONFIG_SYS_GPIO_BASE + (gpio << 2));
}

static inline int gpio_is_valid(int number)
{
	return ((unsigned)number) < CONFIG_SYS_GPIO_WIDTH;
}
#else
#ifdef CONFIG_ALTERA_PIO
extern int altera_pio_init(u32 base, u8 width, char iot,
			   u32 rstval, u32 negmask,
			   const char *label);

extern void altera_pio_info(void);
#define gpio_status() altera_pio_info()
#endif

extern int gpio_request(unsigned gpio, const char *label);
extern int gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_is_valid(int number);
#endif /* CONFIG_SYS_GPIO_BASE */

#endif /* _ASM_NIOS2_GPIO_H_ */

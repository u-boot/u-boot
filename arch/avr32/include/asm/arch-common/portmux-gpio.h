/*
 * Copyright (C) 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __AVR32_PORTMUX_GPIO_H__
#define __AVR32_PORTMUX_GPIO_H__

#include <asm/io.h>

/* Register layout for this specific device */
#include <asm/arch/gpio-impl.h>

/* Register access macros */
#define gpio_readl(port, reg)						\
	__raw_readl(&((struct gpio_regs *)port)->reg)
#define gpio_writel(gpio, reg, value)					\
	__raw_writel(value, &((struct gpio_regs *)port)->reg)

/* Portmux API starts here. See doc/README.AVR32-port-muxing */

enum portmux_function {
	PORTMUX_FUNC_A,
	PORTMUX_FUNC_B,
	PORTMUX_FUNC_C,
	PORTMUX_FUNC_D,
};

#define PORTMUX_DIR_INPUT	(0 << 0)
#define PORTMUX_DIR_OUTPUT	(1 << 0)
#define PORTMUX_INIT_LOW	(0 << 1)
#define PORTMUX_INIT_HIGH	(1 << 1)
#define PORTMUX_PULL_UP		(1 << 2)
#define PORTMUX_PULL_DOWN	(2 << 2)
#define PORTMUX_BUSKEEPER	(3 << 2)
#define PORTMUX_DRIVE_MIN	(0 << 4)
#define PORTMUX_DRIVE_LOW	(1 << 4)
#define PORTMUX_DRIVE_HIGH	(2 << 4)
#define PORTMUX_DRIVE_MAX	(3 << 4)
#define PORTMUX_OPEN_DRAIN	(1 << 6)

void portmux_select_peripheral(void *port, unsigned long pin_mask,
		enum portmux_function func, unsigned long flags);
void portmux_select_gpio(void *port, unsigned long pin_mask,
		unsigned long flags);

/* Internal helper functions */

static inline void *gpio_pin_to_port(unsigned int pin)
{
	return (void *)GPIO_BASE + (pin >> 5) * 0x200;
}

static inline void __gpio_set_output_value(void *port, unsigned int pin,
		int value)
{
	if (value)
		gpio_writel(port, OVRS, 1 << pin);
	else
		gpio_writel(port, OVRC, 1 << pin);
}

static inline int __gpio_get_input_value(void *port, unsigned int pin)
{
	return (gpio_readl(port, PVR) >> pin) & 1;
}

void gpio_set_output_value(unsigned int pin, int value);
int gpio_get_input_value(unsigned int pin);

/* GPIO API starts here */

/*
 * GCC doesn't realize that the constant case is extremely trivial,
 * so we need to help it make the right decision by using
 * always_inline.
 */
__attribute__((always_inline))
static inline void gpio_set_value(unsigned int pin, int value)
{
	if (__builtin_constant_p(pin))
		__gpio_set_output_value(gpio_pin_to_port(pin),
				pin & 0x1f, value);
	else
		gpio_set_output_value(pin, value);
}

__attribute__((always_inline))
static inline int gpio_get_value(unsigned int pin)
{
	if (__builtin_constant_p(pin))
		return __gpio_get_input_value(gpio_pin_to_port(pin),
				pin & 0x1f);
	else
		return gpio_get_input_value(pin);
}

#endif /* __AVR32_PORTMUX_GPIO_H__ */

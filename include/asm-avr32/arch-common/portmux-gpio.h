/*
 * Copyright (C) 2008 Atmel Corporation
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
#ifndef __AVR32_PORTMUX_GPIO_H__
#define __AVR32_PORTMUX_GPIO_H__

#include <asm/io.h>

/* Register offsets */
struct gpio_regs {
	u32	GPER;
	u32	GPERS;
	u32	GPERC;
	u32	GPERT;
	u32	PMR0;
	u32	PMR0S;
	u32	PMR0C;
	u32	PMR0T;
	u32	PMR1;
	u32	PMR1S;
	u32	PMR1C;
	u32	PMR1T;
	u32	__reserved0[4];
	u32	ODER;
	u32	ODERS;
	u32	ODERC;
	u32	ODERT;
	u32	OVR;
	u32	OVRS;
	u32	OVRC;
	u32	OVRT;
	u32	PVR;
	u32	__reserved_PVRS;
	u32	__reserved_PVRC;
	u32	__reserved_PVRT;
	u32	PUER;
	u32	PUERS;
	u32	PUERC;
	u32	PUERT;
	u32	PDER;
	u32	PDERS;
	u32	PDERC;
	u32	PDERT;
	u32	IER;
	u32	IERS;
	u32	IERC;
	u32	IERT;
	u32	IMR0;
	u32	IMR0S;
	u32	IMR0C;
	u32	IMR0T;
	u32	IMR1;
	u32	IMR1S;
	u32	IMR1C;
	u32	IMR1T;
	u32	GFER;
	u32	GFERS;
	u32	GFERC;
	u32	GFERT;
	u32	IFR;
	u32	__reserved_IFRS;
	u32	IFRC;
	u32	__reserved_IFRT;
	u32	ODMER;
	u32	ODMERS;
	u32	ODMERC;
	u32	ODMERT;
	u32	__reserved1[4];
	u32	ODCR0;
	u32	ODCR0S;
	u32	ODCR0C;
	u32	ODCR0T;
	u32	ODCR1;
	u32	ODCR1S;
	u32	ODCR1C;
	u32	ODCR1T;
	u32	__reserved2[4];
	u32	OSRR0;
	u32	OSRR0S;
	u32	OSRR0C;
	u32	OSRR0T;
	u32	__reserved3[8];
	u32	STER;
	u32	STERS;
	u32	STERC;
	u32	STERT;
	u32	__reserved4[35];
	u32	VERSION;
};

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

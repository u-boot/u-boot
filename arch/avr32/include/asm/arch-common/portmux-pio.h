/*
 * Copyright (C) 2006, 2008 Atmel Corporation
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
#ifndef __AVR32_PORTMUX_PIO_H__
#define __AVR32_PORTMUX_PIO_H__

#include <asm/io.h>

/* PIO register offsets */
#define PIO_PER			0x0000
#define PIO_PDR			0x0004
#define PIO_PSR			0x0008
#define PIO_OER			0x0010
#define PIO_ODR			0x0014
#define PIO_OSR			0x0018
#define PIO_IFER		0x0020
#define PIO_IFDR		0x0024
#define PIO_ISFR		0x0028
#define PIO_SODR		0x0030
#define PIO_CODR		0x0034
#define PIO_ODSR		0x0038
#define PIO_PDSR		0x003c
#define PIO_IER			0x0040
#define PIO_IDR			0x0044
#define PIO_IMR			0x0048
#define PIO_ISR			0x004c
#define PIO_MDER		0x0050
#define PIO_MDDR		0x0054
#define PIO_MDSR		0x0058
#define PIO_PUDR		0x0060
#define PIO_PUER		0x0064
#define PIO_PUSR		0x0068
#define PIO_ASR			0x0070
#define PIO_BSR			0x0074
#define PIO_ABSR		0x0078
#define PIO_OWER		0x00a0
#define PIO_OWDR		0x00a4
#define PIO_OWSR		0x00a8

/* Hardware register access */
#define pio_readl(base, reg)				\
	__raw_readl((void *)base + PIO_##reg)
#define pio_writel(base, reg, value)			\
	__raw_writel((value), (void *)base + PIO_##reg)

/* Portmux API starts here. See doc/README.AVR32-port-muxing */

enum portmux_function {
	PORTMUX_FUNC_A,
	PORTMUX_FUNC_B,
};

/* Pull-down, buskeeper and drive strength are not supported */
#define PORTMUX_DIR_INPUT	(0 << 0)
#define PORTMUX_DIR_OUTPUT	(1 << 0)
#define PORTMUX_INIT_LOW	(0 << 1)
#define PORTMUX_INIT_HIGH	(1 << 1)
#define PORTMUX_PULL_UP		(1 << 2)
#define PORTMUX_PULL_DOWN	(0)
#define PORTMUX_BUSKEEPER	PORTMUX_PULL_UP
#define PORTMUX_DRIVE_MIN	(0)
#define PORTMUX_DRIVE_LOW	(0)
#define PORTMUX_DRIVE_HIGH	(0)
#define PORTMUX_DRIVE_MAX	(0)
#define PORTMUX_OPEN_DRAIN	(1 << 3)

void portmux_select_peripheral(void *port, unsigned long pin_mask,
		enum portmux_function func, unsigned long flags);
void portmux_select_gpio(void *port, unsigned long pin_mask,
		unsigned long flags);

/* Internal helper functions */

static inline void __pio_set_output_value(void *port, unsigned int pin,
		int value)
{
	/*
	 * value will usually be constant, but it's pretty cheap
	 * either way.
	 */
	if (value)
		pio_writel(port, SODR, 1 << pin);
	else
		pio_writel(port, CODR, 1 << pin);
}

static inline int __pio_get_input_value(void *port, unsigned int pin)
{
	return (pio_readl(port, PDSR) >> pin) & 1;
}

void pio_set_output_value(unsigned int pin, int value);
int pio_get_input_value(unsigned int pin);

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
		__pio_set_output_value(pio_pin_to_port(pin), pin & 0x1f, value);
	else
		pio_set_output_value(pin, value);
}

__attribute__((always_inline))
static inline int gpio_get_value(unsigned int pin)
{
	if (__builtin_constant_p(pin))
		return __pio_get_input_value(pio_pin_to_port(pin), pin & 0x1f);
	else
		return pio_get_input_value(pin);
}

#endif /* __AVR32_PORTMUX_PIO_H__ */

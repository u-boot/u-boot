/*
 * Copyright (C) 2006, 2008 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>

#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>

void portmux_select_peripheral(void *port, unsigned long pin_mask,
		enum portmux_function func, unsigned long flags)
{
	if (flags & PORTMUX_PULL_UP)
		pio_writel(port, PUER, pin_mask);
	else
		pio_writel(port, PUDR, pin_mask);

	switch (func) {
	case PORTMUX_FUNC_A:
		pio_writel(port, ASR, pin_mask);
		break;
	case PORTMUX_FUNC_B:
		pio_writel(port, BSR, pin_mask);
		break;
	}

	pio_writel(port, PDR, pin_mask);
}

void portmux_select_gpio(void *port, unsigned long pin_mask,
		unsigned long flags)
{
	if (flags & PORTMUX_PULL_UP)
		pio_writel(port, PUER, pin_mask);
	else
		pio_writel(port, PUDR, pin_mask);

	if (flags & PORTMUX_OPEN_DRAIN)
		pio_writel(port, MDER, pin_mask);
	else
		pio_writel(port, MDDR, pin_mask);

	if (flags & PORTMUX_DIR_OUTPUT) {
		if (flags & PORTMUX_INIT_HIGH)
			pio_writel(port, SODR, pin_mask);
		else
			pio_writel(port, CODR, pin_mask);
		pio_writel(port, OER, pin_mask);
	} else {
		pio_writel(port, ODR, pin_mask);
	}

	pio_writel(port, PER, pin_mask);
}

void pio_set_output_value(unsigned int pin, int value)
{
	void *port = pio_pin_to_port(pin);

	if (!port)
		panic("Invalid GPIO pin %u\n", pin);

	__pio_set_output_value(port, pin & 0x1f, value);
}

int pio_get_input_value(unsigned int pin)
{
	void *port = pio_pin_to_port(pin);

	if (!port)
		panic("Invalid GPIO pin %u\n", pin);

	return __pio_get_input_value(port, pin & 0x1f);
}

/*
 * Copyright (C) 2013 Bo Shen <voice.shen@atmel.com>
 *
 * Copyright (C) 2009 Jens Scharsig (js_at_ng@scharsoft.de)
 *
 *  Copyright (C) 2005 HP Labs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/sizes.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/gpio.h>

static struct at91_port *at91_pio_get_port(unsigned port)
{
	switch (port) {
	case AT91_PIO_PORTA:
		return (struct at91_port *)ATMEL_BASE_PIOA;
	case AT91_PIO_PORTB:
		return (struct at91_port *)ATMEL_BASE_PIOB;
	case AT91_PIO_PORTC:
		return (struct at91_port *)ATMEL_BASE_PIOC;
#if (ATMEL_PIO_PORTS > 3)
	case AT91_PIO_PORTD:
		return (struct at91_port *)ATMEL_BASE_PIOD;
#if (ATMEL_PIO_PORTS > 4)
	case AT91_PIO_PORTE:
		return (struct at91_port *)ATMEL_BASE_PIOE;
#endif
#endif
	default:
		return NULL;
	}
}

int at91_set_pio_pullup(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		if (use_pullup)
			writel(1 << pin, &at91_port->puer);
		else
			writel(1 << pin, &at91_port->pudr);
		writel(mask, &at91_port->per);
	}

	return 0;
}

/*
 * mux the pin to the "GPIO" peripheral role.
 */
int at91_set_pio_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(mask, &at91_port->per);
	}

	return 0;
}

/*
 * mux the pin to the "A" internal peripheral role.
 */
int at91_set_a_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
#if defined(CPU_HAS_PIO3)
		writel(readl(&at91_port->abcdsr1) & ~mask,
		       &at91_port->abcdsr1);
		writel(readl(&at91_port->abcdsr2) & ~mask,
		       &at91_port->abcdsr2);
#else
		writel(mask, &at91_port->asr);
#endif
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "B" internal peripheral role.
 */
int at91_set_b_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
#if defined(CPU_HAS_PIO3)
		writel(readl(&at91_port->abcdsr1) | mask,
		       &at91_port->abcdsr1);
		writel(readl(&at91_port->abcdsr2) & ~mask,
		       &at91_port->abcdsr2);
#else
		writel(mask, &at91_port->bsr);
#endif
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

#if defined(CPU_HAS_PIO3)
/*
 * mux the pin to the "C" internal peripheral role.
 */
int at91_set_c_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->abcdsr1) & ~mask,
		       &at91_port->abcdsr1);
		writel(readl(&at91_port->abcdsr2) | mask,
		       &at91_port->abcdsr2);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}

/*
 * mux the pin to the "D" internal peripheral role.
 */
int at91_set_d_periph(unsigned port, unsigned pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(readl(&at91_port->abcdsr1) | mask,
		       &at91_port->abcdsr1);
		writel(readl(&at91_port->abcdsr2) | mask,
		       &at91_port->abcdsr2);
		writel(mask, &at91_port->pdr);
	}

	return 0;
}
#endif

/*
 * mux the pin to the gpio controller (instead of "A" or "B" peripheral), and
 * configure it for an input.
 */
int at91_set_pio_input(unsigned port, u32 pin, int use_pullup)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		at91_set_pio_pullup(port, pin, use_pullup);
		writel(mask, &at91_port->odr);
		writel(mask, &at91_port->per);
	}

	return 0;
}

/*
 * mux the pin to the gpio controller (instead of "A" or "B" peripheral),
 * and configure it for an output.
 */
int at91_set_pio_output(unsigned port, u32 pin, int value)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if ((port < ATMEL_PIO_PORTS) && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->idr);
		writel(mask, &at91_port->pudr);
		if (value)
			writel(mask, &at91_port->sodr);
		else
			writel(mask, &at91_port->codr);
		writel(mask, &at91_port->oer);
		writel(mask, &at91_port->per);
	}

	return 0;
}

/*
 * enable/disable the glitch filter. mostly used with IRQ handling.
 */
int at91_set_pio_deglitch(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		if (is_on) {
#if defined(CPU_HAS_PIO3)
			writel(mask, &at91_port->ifscdr);
#endif
			writel(mask, &at91_port->ifer);
		} else {
			writel(mask, &at91_port->ifdr);
		}
	}

	return 0;
}

#if defined(CPU_HAS_PIO3)
/*
 * enable/disable the debounce filter.
 */
int at91_set_pio_debounce(unsigned port, unsigned pin, int is_on, int div)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		if (is_on) {
			writel(mask, &at91_port->ifscer);
			writel(div & PIO_SCDR_DIV, &at91_port->scdr);
			writel(mask, &at91_port->ifer);
		} else {
			writel(mask, &at91_port->ifdr);
		}
	}

	return 0;
}

/*
 * enable/disable the pull-down.
 * If pull-up already enabled while calling the function, we disable it.
 */
int at91_set_pio_pulldown(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(mask, &at91_port->pudr);
		if (is_on)
			writel(mask, &at91_port->ppder);
		else
			writel(mask, &at91_port->ppddr);
	}

	return 0;
}

/*
 * disable Schmitt trigger
 */
int at91_set_pio_disable_schmitt_trig(unsigned port, unsigned pin)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		writel(readl(&at91_port->schmitt) | mask,
		       &at91_port->schmitt);
	}

	return 0;
}
#endif

/*
 * enable/disable the multi-driver. This is only valid for output and
 * allows the output pin to run as an open collector output.
 */
int at91_set_pio_multi_drive(unsigned port, unsigned pin, int is_on)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		if (is_on)
			writel(mask, &at91_port->mder);
		else
			writel(mask, &at91_port->mddr);
	}

	return 0;
}

/*
 * assuming the pin is muxed as a gpio output, set its value.
 */
int at91_set_pio_value(unsigned port, unsigned pin, int value)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		if (value)
			writel(mask, &at91_port->sodr);
		else
			writel(mask, &at91_port->codr);
	}

	return 0;
}

/*
 * read the pin's value (works even if it's not muxed as a gpio).
 */
int at91_get_pio_value(unsigned port, unsigned pin)
{
	struct at91_port *at91_port = at91_pio_get_port(port);
	u32 pdsr = 0, mask;

	if (at91_port && (pin < 32)) {
		mask = 1 << pin;
		pdsr = readl(&at91_port->pdsr) & mask;
	}

	return pdsr != 0;
}

/* Common GPIO API */

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
	at91_set_pio_input(at91_gpio_to_port(gpio),
			   at91_gpio_to_pin(gpio), 0);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	at91_set_pio_output(at91_gpio_to_port(gpio),
			    at91_gpio_to_pin(gpio), value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	return at91_get_pio_value(at91_gpio_to_port(gpio),
				  at91_gpio_to_pin(gpio));
}

int gpio_set_value(unsigned gpio, int value)
{
	at91_set_pio_value(at91_gpio_to_port(gpio),
			   at91_gpio_to_pin(gpio), value);

	return 0;
}

/*
 * Copyright (C) 2010, 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/ep93xx.h>
#include <config.h>
#include <status_led.h>

static uint8_t saved_state[2] = {STATUS_LED_OFF, STATUS_LED_OFF};
static uint32_t gpio_pin[2] = {1 << STATUS_LED_GREEN,
			       1 << STATUS_LED_RED};

static inline void switch_LED_on(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) | gpio_pin[led], &gpio->pedr);
	saved_state[led] = STATUS_LED_ON;
}

static inline void switch_LED_off(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) & ~gpio_pin[led], &gpio->pedr);
	saved_state[led] = STATUS_LED_OFF;
}

void red_led_on(void)
{
	switch_LED_on(STATUS_LED_RED);
}

void red_led_off(void)
{
	switch_LED_off(STATUS_LED_RED);
}

void green_led_on(void)
{
	switch_LED_on(STATUS_LED_GREEN);
}

void green_led_off(void)
{
	switch_LED_off(STATUS_LED_GREEN);
}

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_RED])
			red_led_off();
		else
			red_led_on();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_GREEN])
			green_led_off();
		else
			green_led_on();
	}
}

void __led_set(led_id_t mask, int state)
{
	if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == state)
			red_led_on();
		else
			red_led_off();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == state)
			green_led_on();
		else
			green_led_off();
	}
}

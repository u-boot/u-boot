/*
 * Copyright (C) 2010, 2009 Matthias Kaehlcke <matthias@kaehlcke.net>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/io.h>
#include <asm/arch/ep93xx.h>
#include <config.h>
#include <status_led.h>

static uint8_t saved_state[2] = {STATUS_LED_OFF, STATUS_LED_OFF};
static uint32_t gpio_pin[2] = {1 << STATUS_LED_GREEN,
			       1 << STATUS_LED_RED};

inline void switch_LED_on(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) | gpio_pin[led], &gpio->pedr);
	saved_state[led] = STATUS_LED_ON;
}

inline void switch_LED_off(uint8_t led)
{
	register struct gpio_regs *gpio = (struct gpio_regs *)GPIO_BASE;

	writel(readl(&gpio->pedr) & ~gpio_pin[led], &gpio->pedr);
	saved_state[led] = STATUS_LED_OFF;
}

void red_LED_on(void)
{
	switch_LED_on(STATUS_LED_RED);
}

void red_LED_off(void)
{
	switch_LED_off(STATUS_LED_RED);
}

void green_LED_on(void)
{
	switch_LED_on(STATUS_LED_GREEN);
}

void green_LED_off(void)
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
			red_LED_off();
		else
			red_LED_on();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_GREEN])
			green_LED_off();
		else
			green_LED_on();
	}
}

void __led_set(led_id_t mask, int state)
{
	if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == state)
			red_LED_on();
		else
			red_LED_off();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == state)
			green_LED_on();
		else
			green_LED_off();
	}
}

/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 * (C) Copyright 2009
 * Eric Benard <eric@eukrea.com>
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

#include <common.h>
#include <status_led.h>
#include <asm/arch/at91sam9260.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

static unsigned int saved_state[4] = {STATUS_LED_OFF, STATUS_LED_OFF,
		STATUS_LED_OFF, STATUS_LED_OFF};

void coloured_LED_init(void)
{
	at91_pmc_t *pmc = (at91_pmc_t *) ATMEL_BASE_PMC;

	/* Enable clock */
	writel(1 << ATMEL_ID_PIOC, &pmc->pcer);

	at91_set_pio_output(CONFIG_RED_LED, 1);
	at91_set_pio_output(CONFIG_GREEN_LED, 1);
	at91_set_pio_output(CONFIG_YELLOW_LED, 1);
	at91_set_pio_output(CONFIG_BLUE_LED, 1);

	at91_set_pio_value(CONFIG_RED_LED, 1);
	at91_set_pio_value(CONFIG_GREEN_LED, 1);
	at91_set_pio_value(CONFIG_YELLOW_LED, 1);
	at91_set_pio_value(CONFIG_BLUE_LED, 1);
}

void red_led_off(void)
{
	at91_set_pio_value(CONFIG_RED_LED, 1);
	saved_state[STATUS_LED_RED] = STATUS_LED_OFF;
}

void green_led_off(void)
{
	at91_set_pio_value(CONFIG_GREEN_LED, 1);
	saved_state[STATUS_LED_GREEN] = STATUS_LED_OFF;
}

void yellow_led_off(void)
{
	at91_set_pio_value(CONFIG_YELLOW_LED, 1);
	saved_state[STATUS_LED_YELLOW] = STATUS_LED_OFF;
}

void blue_led_off(void)
{
	at91_set_pio_value(CONFIG_BLUE_LED, 1);
	saved_state[STATUS_LED_BLUE] = STATUS_LED_OFF;
}

void red_led_on(void)
{
	at91_set_pio_value(CONFIG_RED_LED, 0);
	saved_state[STATUS_LED_RED] = STATUS_LED_ON;
}

void green_led_on(void)
{
	at91_set_pio_value(CONFIG_GREEN_LED, 0);
	saved_state[STATUS_LED_GREEN] = STATUS_LED_ON;
}

void yellow_led_on(void)
{
	at91_set_pio_value(CONFIG_YELLOW_LED, 0);
	saved_state[STATUS_LED_YELLOW] = STATUS_LED_ON;
}

void blue_led_on(void)
{
	at91_set_pio_value(CONFIG_BLUE_LED, 0);
	saved_state[STATUS_LED_BLUE] = STATUS_LED_ON;
}

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	if (STATUS_LED_BLUE == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_BLUE])
			blue_led_off();
		else
			blue_led_on();
	} else if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_RED])
			red_led_off();
		else
			red_led_on();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_GREEN])
			green_led_off();
		else
			green_led_on();
	} else if (STATUS_LED_YELLOW == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_YELLOW])
			yellow_led_off();
		else
			yellow_led_on();
	}
}

void __led_set(led_id_t mask, int state)
{
	if (STATUS_LED_BLUE == mask) {
		if (STATUS_LED_ON == state)
			blue_led_on();
		else
			blue_led_off();
	} else if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == state)
			red_led_on();
		else
			red_led_off();
	} else if (STATUS_LED_GREEN == mask) {
		if (STATUS_LED_ON == state)
			green_led_on();
		else
			green_led_off();
	} else if (STATUS_LED_YELLOW == mask) {
		if (STATUS_LED_ON == state)
			yellow_led_on();
		else
			yellow_led_off();
	}
}

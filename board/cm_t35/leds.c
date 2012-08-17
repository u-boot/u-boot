/*
 * (C) Copyright 2011
 * CompuLab, Ltd. <www.compulab.co.il>
 *
 * Author: Igor Grinberg <grinberg@compulab.co.il>
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
 * Foundation, Inc.
 */
#include <common.h>
#include <status_led.h>
#include <asm/gpio.h>

static unsigned int leds[] = { GREEN_LED_GPIO };

void __led_init(led_id_t mask, int state)
{
	if (gpio_request(leds[mask], "") != 0) {
		printf("%s: failed requesting GPIO%u\n", __func__, leds[mask]);
		return;
	}

	gpio_direction_output(leds[mask], 0);
}

void __led_set(led_id_t mask, int state)
{
	gpio_set_value(leds[mask], state == STATUS_LED_ON);
}

void __led_toggle(led_id_t mask)
{
	gpio_set_value(leds[mask], !gpio_get_value(leds[mask]));
}

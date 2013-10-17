/*
 * (C) Copyright 2011 - 2013 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Author: Igor Grinberg <grinberg@compulab.co.il>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

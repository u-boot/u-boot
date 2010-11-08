/*
 * Status LED driver based on GPIO access conventions of Linux
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <status_led.h>
#include <asm/gpio.h>

void __led_init(led_id_t mask, int state)
{
	gpio_request(mask, "gpio_led");
	gpio_direction_output(mask, state == STATUS_LED_ON);
}

void __led_set(led_id_t mask, int state)
{
	gpio_set_value(mask, state == STATUS_LED_ON);
}

void __led_toggle(led_id_t mask)
{
	gpio_set_value(mask, !gpio_get_value(mask));
}

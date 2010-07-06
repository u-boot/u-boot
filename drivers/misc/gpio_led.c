/*
 * Status LED driver based on GPIO access conventions of Linux
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <status_led.h>
#include <asm/gpio.h>

/* assume led is active low */

void __led_init(led_id_t mask, int state)
{
	gpio_direction_output(mask, (state == STATUS_LED_ON) ? 0 : 1);
}

void __led_set(led_id_t mask, int state)
{
	gpio_set_value(mask, (state == STATUS_LED_ON) ? 0 : 1);
}

void __led_toggle(led_id_t mask)
{
	gpio_set_value(mask, !gpio_get_value(mask));
}

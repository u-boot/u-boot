/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
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
#include <common.h>
#include <status_led.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

static unsigned int saved_state[2] = {STATUS_LED_OFF, STATUS_LED_OFF};

/*
 * GPIO LEDs
 * 173 red
 * 154 blue
 * 61  blue2
 */
#define ZOOM2_LED_RED	173
#define ZOOM2_LED_BLUE	154
#define ZOOM2_LED_BLUE2	61

void red_LED_off (void)
{
	/* red */
	if (!gpio_request(ZOOM2_LED_RED, "")) {
		gpio_direction_output(ZOOM2_LED_RED, 0);
		gpio_set_value(ZOOM2_LED_RED, 0);
	}
	saved_state[STATUS_LED_RED] = STATUS_LED_OFF;
}

void blue_LED_off (void)
{
	/* blue */
	if (!gpio_request(ZOOM2_LED_BLUE, "")) {
		gpio_direction_output(ZOOM2_LED_BLUE, 0);
		gpio_set_value(ZOOM2_LED_BLUE, 0);
	}

	/* blue 2 */
	if (!gpio_request(ZOOM2_LED_BLUE2, "")) {
		gpio_direction_output(ZOOM2_LED_BLUE2, 0);
		gpio_set_value(ZOOM2_LED_BLUE2, 0);
	}
	saved_state[STATUS_LED_BLUE] = STATUS_LED_OFF;
}

void red_LED_on (void)
{
	blue_LED_off ();

	/* red */
	if (!gpio_request(ZOOM2_LED_RED, "")) {
		gpio_direction_output(ZOOM2_LED_RED, 0);
		gpio_set_value(ZOOM2_LED_RED, 1);
	}
	saved_state[STATUS_LED_RED] = STATUS_LED_ON;
}

void blue_LED_on (void)
{
	red_LED_off ();

	/* blue */
	if (!gpio_request(ZOOM2_LED_BLUE, "")) {
		gpio_direction_output(ZOOM2_LED_BLUE, 0);
		gpio_set_value(ZOOM2_LED_BLUE, 1);
	}

	/* blue 2 */
	if (!gpio_request(ZOOM2_LED_BLUE2, "")) {
		gpio_direction_output(ZOOM2_LED_BLUE2, 0);
		gpio_set_value(ZOOM2_LED_BLUE2, 1);
	}

	saved_state[STATUS_LED_BLUE] = STATUS_LED_ON;
}

void __led_init (led_id_t mask, int state)
{
	__led_set (mask, state);
}

void __led_toggle (led_id_t mask)
{
	if (STATUS_LED_BLUE == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_BLUE])
			blue_LED_off ();
		else
			blue_LED_on ();
	} else if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_RED])
			red_LED_off ();
		else
			red_LED_on ();
	}
}

void __led_set (led_id_t mask, int state)
{
	if (STATUS_LED_BLUE == mask) {
		if (STATUS_LED_ON == state)
			blue_LED_on ();
		else
			blue_LED_off ();
	} else if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == state)
			red_LED_on ();
		else
			red_LED_off ();
	}
}

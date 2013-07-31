/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

void red_led_off(void)
{
	/* red */
	if (!gpio_request(ZOOM2_LED_RED, "")) {
		gpio_direction_output(ZOOM2_LED_RED, 0);
		gpio_set_value(ZOOM2_LED_RED, 0);
	}
	saved_state[STATUS_LED_RED] = STATUS_LED_OFF;
}

void blue_led_off(void)
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

void red_led_on(void)
{
	blue_led_off();

	/* red */
	if (!gpio_request(ZOOM2_LED_RED, "")) {
		gpio_direction_output(ZOOM2_LED_RED, 0);
		gpio_set_value(ZOOM2_LED_RED, 1);
	}
	saved_state[STATUS_LED_RED] = STATUS_LED_ON;
}

void blue_led_on(void)
{
	red_led_off();

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
			blue_led_off();
		else
			blue_led_on();
	} else if (STATUS_LED_RED == mask) {
		if (STATUS_LED_ON == saved_state[STATUS_LED_RED])
			red_led_off();
		else
			red_led_on();
	}
}

void __led_set (led_id_t mask, int state)
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
	}
}

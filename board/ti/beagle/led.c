/*
 * Copyright (c) 2010 Texas Instruments, Inc.
 * Jason Kridner <jkridner@beagleboard.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <status_led.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

/* GPIO pins for the LEDs */
#define BEAGLE_LED_USR0	150
#define BEAGLE_LED_USR1	149

#ifdef STATUS_LED_GREEN
void green_led_off(void)
{
	__led_set (STATUS_LED_GREEN, 0);
}

void green_led_on(void)
{
	__led_set (STATUS_LED_GREEN, 1);
}
#endif

void __led_init (led_id_t mask, int state)
{
	__led_set (mask, state);
}

void __led_toggle (led_id_t mask)
{
	int state, toggle_gpio = 0;
#ifdef STATUS_LED_BIT
	if (!toggle_gpio && STATUS_LED_BIT & mask)
		toggle_gpio = BEAGLE_LED_USR0;
#endif
#ifdef STATUS_LED_BIT1
	if (!toggle_gpio && STATUS_LED_BIT1 & mask)
		toggle_gpio = BEAGLE_LED_USR1;
#endif
	if (toggle_gpio) {
		if (!gpio_request(toggle_gpio, "")) {
			gpio_direction_output(toggle_gpio, 0);
			state = gpio_get_value(toggle_gpio);
			gpio_set_value(toggle_gpio, !state);
		}
	}
}

void __led_set (led_id_t mask, int state)
{
#ifdef STATUS_LED_BIT
	if (STATUS_LED_BIT & mask) {
		if (!gpio_request(BEAGLE_LED_USR0, "")) {
			gpio_direction_output(BEAGLE_LED_USR0, 0);
			gpio_set_value(BEAGLE_LED_USR0, state);
		}
	}
#endif
#ifdef STATUS_LED_BIT1
	if (STATUS_LED_BIT1 & mask) {
		if (!gpio_request(BEAGLE_LED_USR1, "")) {
			gpio_direction_output(BEAGLE_LED_USR1, 0);
			gpio_set_value(BEAGLE_LED_USR1, state);
		}
	}
#endif
}

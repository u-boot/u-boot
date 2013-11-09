/*
 * Copyright (c) 2013 Corscience GmbH & Co.KG
 * Andreas Bießmann <andreas.biessmann@corscience.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <status_led.h>
#include <twl4030.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>

#define TRICORDER_STATUS_LED_YELLOW 42
#define TRICORDER_STATUS_LED_GREEN  43

void __led_init(led_id_t mask, int state)
{
	__led_set(mask, state);
}

void __led_toggle(led_id_t mask)
{
	int toggle_gpio = 0;
#ifdef STATUS_LED_BIT
	if (!toggle_gpio && STATUS_LED_BIT & mask)
		toggle_gpio = TRICORDER_STATUS_LED_GREEN;
#endif
#ifdef STATUS_LED_BIT1
	if (!toggle_gpio && STATUS_LED_BIT1 & mask)
		toggle_gpio = TRICORDER_STATUS_LED_YELLOW;
#endif
#ifdef STATUS_LED_BIT2
	if (!toggle_gpio && STATUS_LED_BIT2 & mask) {
		uint8_t val;
		twl4030_i2c_read_u8(TWL4030_CHIP_LED, TWL4030_LED_LEDEN,
				    &val);
		val ^= (TWL4030_LED_LEDEN_LEDAON | TWL4030_LED_LEDEN_LEDAPWM);
		twl4030_i2c_write_u8(TWL4030_CHIP_LED, TWL4030_LED_LEDEN,
				     val);
	}
#endif
	if (toggle_gpio) {
		int state;
		gpio_request(toggle_gpio, "");
		state = gpio_get_value(toggle_gpio);
		gpio_set_value(toggle_gpio, !state);
	}
}

void __led_set(led_id_t mask, int state)
{
#ifdef STATUS_LED_BIT
	if (STATUS_LED_BIT & mask) {
		gpio_request(TRICORDER_STATUS_LED_GREEN, "");
		gpio_direction_output(TRICORDER_STATUS_LED_GREEN, 0);
		gpio_set_value(TRICORDER_STATUS_LED_GREEN, state);
	}
#endif
#ifdef STATUS_LED_BIT1
	if (STATUS_LED_BIT1 & mask) {
		gpio_request(TRICORDER_STATUS_LED_YELLOW, "");
		gpio_direction_output(TRICORDER_STATUS_LED_YELLOW, 0);
		gpio_set_value(TRICORDER_STATUS_LED_YELLOW, state);
	}
#endif
#ifdef STATUS_LED_BIT2
	if (STATUS_LED_BIT2 & mask) {
		if (STATUS_LED_OFF == state)
			twl4030_i2c_write_u8(TWL4030_CHIP_LED,
					     TWL4030_LED_LEDEN, 0);
		else
			twl4030_i2c_write_u8(TWL4030_CHIP_LED,
					     TWL4030_LED_LEDEN,
					     (TWL4030_LED_LEDEN_LEDAON |
					      TWL4030_LED_LEDEN_LEDAPWM));
	}
#endif
}

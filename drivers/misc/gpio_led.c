/*
 * Status LED driver based on GPIO access conventions of Linux
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <status_led.h>
#include <asm/gpio.h>

#ifndef CONFIG_GPIO_LED_INVERTED_TABLE
#define CONFIG_GPIO_LED_INVERTED_TABLE {}
#endif

static led_id_t gpio_led_inv[] = CONFIG_GPIO_LED_INVERTED_TABLE;

static int gpio_led_gpio_value(led_id_t mask, int state)
{
	int i, gpio_value = (state == STATUS_LED_ON);

	for (i = 0; i < ARRAY_SIZE(gpio_led_inv); i++) {
		if (gpio_led_inv[i] == mask)
			gpio_value = !gpio_value;
	}

	return gpio_value;
}

void __led_init(led_id_t mask, int state)
{
	int gpio_value;

	if (gpio_request(mask, "gpio_led") != 0) {
		printf("%s: failed requesting GPIO%lu!\n", __func__, mask);
		return;
	}

	gpio_value = gpio_led_gpio_value(mask, state);
	gpio_direction_output(mask, gpio_value);
}

void __led_set(led_id_t mask, int state)
{
	int gpio_value = gpio_led_gpio_value(mask, state);

	gpio_set_value(mask, gpio_value);
}

void __led_toggle(led_id_t mask)
{
	gpio_set_value(mask, !gpio_get_value(mask));
}

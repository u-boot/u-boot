/*
 * Status LED driver based on GPIO access conventions of Linux
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Licensed under the GPL-2 or later.
 */

#include <status_led.h>
#include <asm/gpio.h>

#ifndef CFG_GPIO_LED_INVERTED_TABLE
#define CFG_GPIO_LED_INVERTED_TABLE {}
#endif

static led_id_t gpio_led_inv[] = CFG_GPIO_LED_INVERTED_TABLE;

static int gpio_led_gpio_value(led_id_t mask, int state)
{
	int i, gpio_value = (state == CONFIG_LED_STATUS_ON);

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

/* 'generic' override of colored LED stubs, to use GPIO functions instead */

/* We support up to 6 LEDs, LED 0 STATUS BIT doesn't have the number suffix */
#define GPIO_BIT0	CONFIG_LED_STATUS_BIT
#define GPIO_BIT1	CONFIG_LED_STATUS_BIT1
#define GPIO_BIT2	CONFIG_LED_STATUS_BIT2
#define GPIO_BIT3	CONFIG_LED_STATUS_BIT3
#define GPIO_BIT4	CONFIG_LED_STATUS_BIT4
#define GPIO_BIT5	CONFIG_LED_STATUS_BIT5
/* C preprocessor magic way to generate a GPIO_LED<id> reference */
#define GPIO_BIT(id)	___PASTE(GPIO_BIT, id)

#ifdef CONFIG_LED_STATUS_RED

void red_led_on(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_RED), CONFIG_LED_STATUS_ON);
}

void red_led_off(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_RED), CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_GREEN
void green_led_on(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_GREEN), CONFIG_LED_STATUS_ON);
}

void green_led_off(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_GREEN), CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_YELLOW
void yellow_led_on(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_YELLOW), CONFIG_LED_STATUS_ON);
}

void yellow_led_off(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_YELLOW), CONFIG_LED_STATUS_OFF);
}
#endif

#ifdef CONFIG_LED_STATUS_BLUE
void blue_led_on(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_BLUE), CONFIG_LED_STATUS_ON);
}

void blue_led_off(void)
{
	__led_set(GPIO_BIT(CONFIG_LED_STATUS_BLUE), CONFIG_LED_STATUS_OFF);
}
#endif

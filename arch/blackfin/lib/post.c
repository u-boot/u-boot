/*
 * Blackfin POST code
 *
 * Copyright (c) 2005-2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <post.h>

#include <asm/gpio.h>

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC1
int led_post_test(int flags)
{
	unsigned leds[] = { CONFIG_POST_BSPEC1_GPIO_LEDS };
	int i;

	/* First turn them all off */
	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		if (gpio_request(leds[i], "post")) {
			printf("could not request gpio %u\n", leds[i]);
			continue;
		}
		gpio_direction_output(leds[i], 0);
	}

	/* Now turn them on one by one */
	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		printf("LED%i on", i + 1);
		gpio_set_value(leds[i], 1);
		udelay(1000000);
		printf("\b\b\b\b\b\b\b");
		gpio_free(leds[i]);
	}

	return 0;
}
#endif

#if CONFIG_POST & CONFIG_SYS_POST_BSPEC2
int button_post_test(int flags)
{
	unsigned buttons[] = { CONFIG_POST_BSPEC2_GPIO_BUTTONS };
	unsigned int sws[] = { CONFIG_POST_BSPEC2_GPIO_NAMES };
	int i, delay = 5;
	unsigned short value = 0;
	int result = 0;

	for (i = 0; i < ARRAY_SIZE(buttons); ++i) {
		if (gpio_request(buttons[i], "post")) {
			printf("could not request gpio %u\n", buttons[i]);
			continue;
		}
		gpio_direction_input(buttons[i]);

		delay = 5;
		printf("\n--------Press SW%i: %2d ", sws[i], delay);
		while (delay--) {
			int j;
			for (j = 0; j < 100; j++) {
				value = gpio_get_value(buttons[i]);
				if (value != 0)
					break;
				udelay(10000);
			}
			printf("\b\b\b%2d ", delay);
		}
		if (value != 0)
			puts("\b\bOK");
		else {
			result = -1;
			puts("\b\bfailed");
		}

		gpio_free(buttons[i]);
	}

	puts("\n");

	return result;
}
#endif

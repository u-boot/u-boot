/*
 * BF537-STAMP POST code
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (c) 2005-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <asm/blackfin.h>
#include <asm/gpio.h>

/* Using sw10-PF5 as the hotkey */
int post_hotkeys_pressed(void)
{
	int delay = 3;
	int i;
	unsigned short value;

	gpio_request(GPIO_PF5, "post");
	gpio_direction_input(GPIO_PF5);

	printf("########Press SW10 to enter Memory POST########: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = gpio_get_value(GPIO_PF5);
			if (value != 0) {
				break;
			}
			udelay(10000);
		}
		printf("\b\b\b%2d ", delay);
	}
	printf("\b\b\b 0");
	printf("\n");
	if (value == 0)
		return 0;
	else {
		printf("Hotkey has been pressed, Enter POST . . . . . .\n");
		return 1;
	}

	gpio_free(GPIO_PF5);
}

int uart_post_test(int flags)
{
	return 0;
}

#define BLOCK_SIZE 0x10000
#define VERIFY_ADDR 0x2000000
extern int erase_block_flash(int);
extern int write_data(long lStart, long lCount, uchar * pnData);
int flash_post_test(int flags)
{
	unsigned short *pbuf, *temp;
	int offset, n, i;
	int value = 0;
	int result = 0;
	printf("\n");
	pbuf = (unsigned short *)VERIFY_ADDR;
	temp = pbuf;
	for (n = FLASH_START_POST_BLOCK; n < FLASH_END_POST_BLOCK; n++) {
		offset = (n - 7) * BLOCK_SIZE;
		printf("--------Erase   block:%2d..", n);
		erase_block_flash(n);
		printf("OK\r");
		printf("--------Program block:%2d...", n);
		write_data(CONFIG_SYS_FLASH_BASE + offset, BLOCK_SIZE, pbuf);
		printf("OK\r");
		printf("--------Verify  block:%2d...", n);
		for (i = 0; i < BLOCK_SIZE; i += 2) {
			if (*(unsigned short *)(CONFIG_SYS_FLASH_BASE + offset + i) !=
			    *temp++) {
				value = 1;
				result = 1;
			}
		}
		if (value)
			printf("failed\n");
		else
			printf("OK		%3d%%\r",
			       (int)(
				     (n + 1 -
				      FLASH_START_POST_BLOCK) *
				     100 / (FLASH_END_POST_BLOCK -
					    FLASH_START_POST_BLOCK)));

		temp = pbuf;
		value = 0;
	}
	printf("\n");
	if (result)
		return -1;
	else
		return 0;
}

/****************************************************
 * LED1 ---- PF6	LED2 ---- PF7		    *
 * LED3 ---- PF8	LED4 ---- PF9		    *
 * LED5 ---- PF10	LED6 ---- PF11		    *
 ****************************************************/
int led_post_test(int flags)
{
	unsigned int leds[] = {
		GPIO_PF6, GPIO_PF7, GPIO_PF8,
		GPIO_PF9, GPIO_PF10, GPIO_PF11,
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		gpio_request(leds[i], "post");
		gpio_direction_output(leds[i], 0);

		printf("LED%i on", i + 1);
		gpio_set_value(leds[i], 1);
		udelay(1000000);
		printf("\b\b\b\b\b\b\b");

		gpio_free(leds[i]);
	}

	return 0;
}

/************************************************
 *  SW10 ---- PF5	SW11 ---- PF4		*
 *  SW12 ---- PF3	SW13 ---- PF2		*
 ************************************************/
int button_post_test(int flags)
{
	unsigned int buttons[] = {
		GPIO_PF2, GPIO_PF3, GPIO_PF4, GPIO_PF5,
	};
	unsigned int sws[] = { 13, 12, 11, 10, };
	int i, delay = 5;
	unsigned short value = 0;
	int result = 0;

	for (i = 0; i < ARRAY_SIZE(buttons); ++i) {
		gpio_request(buttons[i], "post");
		gpio_direction_input(buttons[i]);

		delay = 5;
		printf("\n--------Press SW%i: %2d ", sws[i], delay);
		while (delay--) {
			for (i = 0; i < 100; i++) {
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

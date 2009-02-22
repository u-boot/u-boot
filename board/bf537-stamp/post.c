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

#define POST_WORD_ADDR 0xFF903FFC

/* Using sw10-PF5 as the hotkey */
int post_hotkeys_pressed(void)
{
	int delay = 3;
	int i;
	unsigned short value;

	*pPORTF_FER &= ~PF5;
	*pPORTFIO_DIR &= ~PF5;
	*pPORTFIO_INEN |= PF5;

	printf("########Press SW10 to enter Memory POST########: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = *pPORTFIO & PF5;
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
}

void post_word_store(ulong a)
{
	volatile ulong *save_addr = (volatile ulong *)POST_WORD_ADDR;
	*save_addr = a;
}

ulong post_word_load(void)
{
	volatile ulong *save_addr = (volatile ulong *)POST_WORD_ADDR;
	return *save_addr;
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
	*pPORTF_FER &= ~(PF6 | PF7 | PF8 | PF9 | PF10 | PF11);
	*pPORTFIO_DIR |= PF6 | PF7 | PF8 | PF9 | PF10 | PF11;
	*pPORTFIO_INEN &= ~(PF6 | PF7 | PF8 | PF9 | PF10 | PF11);
	*pPORTFIO &= ~(PF6 | PF7 | PF8 | PF9 | PF10 | PF11);
	udelay(1000000);
	printf("LED1 on");
	*pPORTFIO |= PF6;
	udelay(1000000);
	printf("\b\b\b\b\b\b\b");
	printf("LED2 on");
	*pPORTFIO |= PF7;
	udelay(1000000);
	printf("\b\b\b\b\b\b\b");
	printf("LED3 on");
	*pPORTFIO |= PF8;
	udelay(1000000);
	printf("\b\b\b\b\b\b\b");
	printf("LED4 on");
	*pPORTFIO |= PF9;
	udelay(1000000);
	printf("\b\b\b\b\b\b\b");
	printf("LED5 on");
	*pPORTFIO |= PF10;
	udelay(1000000);
	printf("\b\b\b\b\b\b\b");
	printf("lED6 on");
	*pPORTFIO |= PF11;
	printf("\b\b\b\b\b\b\b ");
	return 0;
}

/************************************************
 *  SW10 ---- PF5	SW11 ---- PF4		*
 *  SW12 ---- PF3	SW13 ---- PF2		*
 ************************************************/
int button_post_test(int flags)
{
	int i, delay = 5;
	unsigned short value = 0;
	int result = 0;

	*pPORTF_FER &= ~(PF5 | PF4 | PF3 | PF2);
	*pPORTFIO_DIR &= ~(PF5 | PF4 | PF3 | PF2);
	*pPORTFIO_INEN |= (PF5 | PF4 | PF3 | PF2);

	printf("\n--------Press SW10: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = *pPORTFIO & PF5;
			if (value != 0) {
				break;
			}
			udelay(10000);
		}
		printf("\b\b\b%2d ", delay);
	}
	if (value != 0)
		printf("\b\bOK");
	else {
		result = -1;
		printf("\b\bfailed");
	}

	delay = 5;
	printf("\n--------Press SW11: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = *pPORTFIO & PF4;
			if (value != 0) {
				break;
			}
			udelay(10000);
		}
		printf("\b\b\b%2d ", delay);
	}
	if (value != 0)
		printf("\b\bOK");
	else {
		result = -1;
		printf("\b\bfailed");
	}

	delay = 5;
	printf("\n--------Press SW12: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = *pPORTFIO & PF3;
			if (value != 0) {
				break;
			}
			udelay(10000);
		}
		printf("\b\b\b%2d ", delay);
	}
	if (value != 0)
		printf("\b\bOK");
	else {
		result = -1;
		printf("\b\bfailed");
	}

	delay = 5;
	printf("\n--------Press SW13: %2d ", delay);
	while (delay--) {
		for (i = 0; i < 100; i++) {
			value = *pPORTFIO & PF2;
			if (value != 0) {
				break;
			}
			udelay(10000);
		}
		printf("\b\b\b%2d ", delay);
	}
	if (value != 0)
		printf("\b\bOK");
	else {
		result = -1;
		printf("\b\bfailed");
	}
	printf("\n");
	return result;
}

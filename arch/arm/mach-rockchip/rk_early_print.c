/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/uart.h>
#include <common.h>

static struct rk_uart *uart_ptr;

static void uart_wrtie_byte(char byte)
{
	writel(byte, &uart_ptr->rbr);
	while (!(readl(&uart_ptr->lsr) & 0x40))
		;
}

void print(char *s)
{
	while (*s) {
		if (*s == '\n')
			uart_wrtie_byte('\r');
	    uart_wrtie_byte(*s);
	    s++;
	}
}

void print_hex(unsigned int n)
{
	int i;
	int temp;

	uart_wrtie_byte('0');
	uart_wrtie_byte('x');

	for (i = 8; i > 0; i--) {
		temp = (n >> (i - 1) * 4) & 0x0f;
		if (temp < 10)
			uart_wrtie_byte((char)(temp + '0'));
		else
			uart_wrtie_byte((char)(temp - 10 + 'a'));
	}
	uart_wrtie_byte('\n');
	uart_wrtie_byte('\r');
}

/*
 * TODO: since rk3036 only 4K sram to use in SPL, for saving space,
 * we implement uart driver this way, we should convert this to use
 * ns16550 driver in future, which support DEBUG_UART in the standard way
 */
void rk_uart_init(void *base)
{
	uart_ptr = (struct rk_uart *)base;
	writel(0x83, &uart_ptr->lcr);
	writel(0x0d, &uart_ptr->rbr);
	writel(0x03, &uart_ptr->lcr);

	/* fifo enable, sfe is shadow register of FCR[0] */
	writel(0x01, &uart_ptr->sfe);
}

/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * Modified to support C structur SoC access by
 * Andreas Bießmann <biessmann@corscience.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <watchdog.h>
#include <serial.h>
#include <linux/compiler.h>

#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

#include "atmel_usart.h"

DECLARE_GLOBAL_DATA_PTR;

static void atmel_serial_setbrg(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;
	unsigned long divisor;
	unsigned long usart_hz;

	/*
	 *              Master Clock
	 * Baud Rate = --------------
	 *                16 * CD
	 */
	usart_hz = get_usart_clk_rate(CONFIG_USART_ID);
	divisor = (usart_hz / 16 + gd->baudrate / 2) / gd->baudrate;
	writel(USART3_BF(CD, divisor), &usart->brgr);
}

static int atmel_serial_init(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	/*
	 * Just in case: drain transmitter register
	 * 1000us is enough for baudrate >= 9600
	 */
	if (!(readl(&usart->csr) & USART3_BIT(TXEMPTY)))
		__udelay(1000);

	writel(USART3_BIT(RSTRX) | USART3_BIT(RSTTX), &usart->cr);

	serial_setbrg();

	writel((USART3_BF(USART_MODE, USART3_USART_MODE_NORMAL)
			   | USART3_BF(USCLKS, USART3_USCLKS_MCK)
			   | USART3_BF(CHRL, USART3_CHRL_8)
			   | USART3_BF(PAR, USART3_PAR_NONE)
			   | USART3_BF(NBSTOP, USART3_NBSTOP_1)),
			   &usart->mr);
	writel(USART3_BIT(RXEN) | USART3_BIT(TXEN), &usart->cr);
	/* 100us is enough for the new settings to be settled */
	__udelay(100);

	return 0;
}

static void atmel_serial_putc(char c)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	if (c == '\n')
		serial_putc('\r');

	while (!(readl(&usart->csr) & USART3_BIT(TXRDY)));
	writel(c, &usart->thr);
}

static int atmel_serial_getc(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;

	while (!(readl(&usart->csr) & USART3_BIT(RXRDY)))
		 WATCHDOG_RESET();
	return readl(&usart->rhr);
}

static int atmel_serial_tstc(void)
{
	atmel_usart3_t *usart = (atmel_usart3_t *)CONFIG_USART_BASE;
	return (readl(&usart->csr) & USART3_BIT(RXRDY)) != 0;
}

static struct serial_device atmel_serial_drv = {
	.name	= "atmel_serial",
	.start	= atmel_serial_init,
	.stop	= NULL,
	.setbrg	= atmel_serial_setbrg,
	.putc	= atmel_serial_putc,
	.puts	= default_serial_puts,
	.getc	= atmel_serial_getc,
	.tstc	= atmel_serial_tstc,
};

void atmel_serial_initialize(void)
{
	serial_register(&atmel_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &atmel_serial_drv;
}

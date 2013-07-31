/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define US1_TDRE        (1 << 7)
#define US1_RDRF        (1 << 5)
#define UC2_TE          (1 << 3)
#define UC2_RE          (1 << 2)

DECLARE_GLOBAL_DATA_PTR;

struct lpuart_fsl *base = (struct lpuart_fsl *)LPUART_BASE;

static void lpuart_serial_setbrg(void)
{
	u32 clk = mxc_get_clock(MXC_UART_CLK);
	u16 sbr;

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	sbr = (u16)(clk / (16 * gd->baudrate));
	/* place adjustment later - n/32 BRFA */

	__raw_writeb(sbr >> 8, &base->ubdh);
	__raw_writeb(sbr & 0xff, &base->ubdl);
}

static int lpuart_serial_getc(void)
{
	u8 status;

	while (!(__raw_readb(&base->us1) & US1_RDRF))
		WATCHDOG_RESET();

	status = __raw_readb(&base->us1);
	status |= US1_RDRF;
	__raw_writeb(status, &base->us1);

	return __raw_readb(&base->ud);
}

static void lpuart_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while (!(__raw_readb(&base->us1) & US1_TDRE))
		WATCHDOG_RESET();

	__raw_writeb(c, &base->ud);
}

/*
 * Test whether a character is in the RX buffer
 */
static int lpuart_serial_tstc(void)
{
	if (__raw_readb(&base->urcfifo) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int lpuart_serial_init(void)
{
	u8 ctrl;

	ctrl = __raw_readb(&base->uc2);
	ctrl &= ~UC2_RE;
	ctrl &= ~UC2_TE;
	__raw_writeb(ctrl, &base->uc2);

	__raw_writeb(0, &base->umodem);
	__raw_writeb(0, &base->uc1);

	/* provide data bits, parity, stop bit, etc */

	serial_setbrg();

	__raw_writeb(UC2_RE | UC2_TE, &base->uc2);

	return 0;
}

static struct serial_device lpuart_serial_drv = {
	.name = "lpuart_serial",
	.start = lpuart_serial_init,
	.stop = NULL,
	.setbrg = lpuart_serial_setbrg,
	.putc = lpuart_serial_putc,
	.puts = default_serial_puts,
	.getc = lpuart_serial_getc,
	.tstc = lpuart_serial_tstc,
};

void lpuart_serial_initialize(void)
{
	serial_register(&lpuart_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &lpuart_serial_drv;
}

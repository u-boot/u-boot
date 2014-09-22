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
#define US1_OR          (1 << 3)
#define UC2_TE          (1 << 3)
#define UC2_RE          (1 << 2)
#define CFIFO_TXFLUSH   (1 << 7)
#define CFIFO_RXFLUSH   (1 << 6)
#define SFIFO_RXOF      (1 << 2)
#define SFIFO_RXUF      (1 << 0)

#define STAT_LBKDIF	(1 << 31)
#define STAT_RXEDGIF	(1 << 30)
#define STAT_TDRE	(1 << 23)
#define STAT_RDRF	(1 << 21)
#define STAT_IDLE	(1 << 20)
#define STAT_OR		(1 << 19)
#define STAT_NF		(1 << 18)
#define STAT_FE		(1 << 17)
#define STAT_PF		(1 << 16)
#define STAT_MA1F	(1 << 15)
#define STAT_MA2F	(1 << 14)
#define STAT_FLAGS	(STAT_LBKDIF | STAT_RXEDGIF | STAT_IDLE | STAT_OR | \
			STAT_NF | STAT_FE | STAT_PF | STAT_MA1F | STAT_MA2F)

#define CTRL_TE		(1 << 19)
#define CTRL_RE		(1 << 18)

#define FIFO_TXFE		0x80
#define FIFO_RXFE		0x40

#define WATER_TXWATER_OFF	1
#define WATER_RXWATER_OFF	16

DECLARE_GLOBAL_DATA_PTR;

struct lpuart_fsl *base = (struct lpuart_fsl *)LPUART_BASE;

#ifndef CONFIG_LPUART_32B_REG
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
	while (!(__raw_readb(&base->us1) & (US1_RDRF | US1_OR)))
		WATCHDOG_RESET();

	barrier();

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

	/* Disable FIFO and flush buffer */
	__raw_writeb(0x0, &base->upfifo);
	__raw_writeb(0x0, &base->utwfifo);
	__raw_writeb(0x1, &base->urwfifo);
	__raw_writeb(CFIFO_TXFLUSH | CFIFO_RXFLUSH, &base->ucfifo);

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
#else
static void lpuart32_serial_setbrg(void)
{
	u32 clk = CONFIG_SYS_CLK_FREQ;
	u32 sbr;

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	sbr = (clk / (16 * gd->baudrate));
	/* place adjustment later - n/32 BRFA */

	out_be32(&base->baud, sbr);
}

static int lpuart32_serial_getc(void)
{
	u32 stat;

	while (((stat = in_be32(&base->stat)) & STAT_RDRF) == 0) {
		out_be32(&base->stat, STAT_FLAGS);
		WATCHDOG_RESET();
	}

	return in_be32(&base->data) & 0x3ff;
}

static void lpuart32_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	while (!(in_be32(&base->stat) & STAT_TDRE))
		WATCHDOG_RESET();

	out_be32(&base->data, c);
}

/*
 * Test whether a character is in the RX buffer
 */
static int lpuart32_serial_tstc(void)
{
	if ((in_be32(&base->water) >> 24) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int lpuart32_serial_init(void)
{
	u8 ctrl;

	ctrl = in_be32(&base->ctrl);
	ctrl &= ~CTRL_RE;
	ctrl &= ~CTRL_TE;
	out_be32(&base->ctrl, ctrl);

	out_be32(&base->modir, 0);
	out_be32(&base->fifo, ~(FIFO_TXFE | FIFO_RXFE));

	out_be32(&base->match, 0);
	/* provide data bits, parity, stop bit, etc */

	serial_setbrg();

	out_be32(&base->ctrl, CTRL_RE | CTRL_TE);

	return 0;
}

static struct serial_device lpuart32_serial_drv = {
	.name = "lpuart32_serial",
	.start = lpuart32_serial_init,
	.stop = NULL,
	.setbrg = lpuart32_serial_setbrg,
	.putc = lpuart32_serial_putc,
	.puts = default_serial_puts,
	.getc = lpuart32_serial_getc,
	.tstc = lpuart32_serial_tstc,
};
#endif

void lpuart_serial_initialize(void)
{
#ifdef CONFIG_LPUART_32B_REG
	serial_register(&lpuart32_serial_drv);
#else
	serial_register(&lpuart_serial_drv);
#endif
}

__weak struct serial_device *default_serial_console(void)
{
#ifdef CONFIG_LPUART_32B_REG
	return &lpuart32_serial_drv;
#else
	return &lpuart_serial_drv;
#endif
}

/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>

typedef volatile struct {
	unsigned	rxdata;		/* Rx data reg */
	unsigned	txdata;		/* Tx data reg */
	unsigned	status;		/* Status reg */
	unsigned	control;	/* Control reg */
	unsigned	divisor;	/* Baud rate divisor reg */
	unsigned	endofpacket;	/* End-of-packet reg */
} nios_uart_t;

/* status register */
#define NIOS_UART_PE		(1 << 0)	/* parity error */
#define NIOS_UART_FE		(1 << 1)	/* frame error */
#define NIOS_UART_BRK		(1 << 2)	/* break detect */
#define NIOS_UART_ROE		(1 << 3)	/* rx overrun */
#define NIOS_UART_TOE		(1 << 4)	/* tx overrun */
#define NIOS_UART_TMT		(1 << 5)	/* tx empty */
#define NIOS_UART_TRDY		(1 << 6)	/* tx ready */
#define NIOS_UART_RRDY		(1 << 7)	/* rx ready */
#define NIOS_UART_E		(1 << 8)	/* exception */
#define NIOS_UART_DCTS		(1 << 10)	/* cts change */
#define NIOS_UART_CTS		(1 << 11)	/* cts */
#define NIOS_UART_EOP		(1 << 12)	/* eop detected */

/* control register */
#define NIOS_UART_IPE		(1 << 0)	/* parity error int ena*/
#define NIOS_UART_IFE		(1 << 1)	/* frame error int ena */
#define NIOS_UART_IBRK		(1 << 2)	/* break detect int ena */
#define NIOS_UART_IROE		(1 << 3)	/* rx overrun int ena */
#define NIOS_UART_ITOE		(1 << 4)	/* tx overrun int ena */
#define NIOS_UART_ITMT		(1 << 5)	/* tx empty int ena */
#define NIOS_UART_ITRDY		(1 << 6)	/* tx ready int ena */
#define NIOS_UART_IRRDY		(1 << 7)	/* rx ready int ena */
#define NIOS_UART_IE		(1 << 8)	/* exception int ena */
#define NIOS_UART_TBRK		(1 << 9)	/* transmit break */
#define NIOS_UART_IDCTS		(1 << 10)	/* cts change int ena */
#define NIOS_UART_RTS		(1 << 11)	/* rts */
#define NIOS_UART_IEOP		(1 << 12)	/* eop detected int ena */

DECLARE_GLOBAL_DATA_PTR;

/*------------------------------------------------------------------
 * UART the serial port
 *-----------------------------------------------------------------*/

static nios_uart_t *uart = (nios_uart_t *) CONFIG_SYS_NIOS_CONSOLE;

#if defined(CONFIG_SYS_NIOS_FIXEDBAUD)

/*
 * Everything's already setup for fixed-baud PTF
 * assignment
 */
static void altera_serial_setbrg(void)
{
}

static int altera_serial_init(void)
{
	return 0;
}

#else

static void altera_serial_setbrg(void)
{
	unsigned div;

	div = (CONFIG_SYS_CLK_FREQ/gd->baudrate)-1;
	writel (div, &uart->divisor);
}

static int altera_serial_init(void)
{
	serial_setbrg();
	return 0;
}

#endif /* CONFIG_SYS_NIOS_FIXEDBAUD */

/*-----------------------------------------------------------------------
 * UART CONSOLE
 *---------------------------------------------------------------------*/
static void altera_serial_putc(char c)
{
	if (c == '\n')
		serial_putc ('\r');
	while ((readl (&uart->status) & NIOS_UART_TRDY) == 0)
		WATCHDOG_RESET ();
	writel ((unsigned char)c, &uart->txdata);
}

static int altera_serial_tstc(void)
{
	return (readl (&uart->status) & NIOS_UART_RRDY);
}

static int altera_serial_getc(void)
{
	while (serial_tstc () == 0)
		WATCHDOG_RESET ();
	return (readl (&uart->rxdata) & 0x00ff );
}

static struct serial_device altera_serial_drv = {
	.name	= "altera_serial",
	.start	= altera_serial_init,
	.stop	= NULL,
	.setbrg	= altera_serial_setbrg,
	.putc	= altera_serial_putc,
	.puts	= default_serial_puts,
	.getc	= altera_serial_getc,
	.tstc	= altera_serial_tstc,
};

void altera_serial_initialize(void)
{
	serial_register(&altera_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &altera_serial_drv;
}

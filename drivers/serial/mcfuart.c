/*
 * (C) Copyright 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
 *
 * Modified to add device model (DM) support
 * (C) Copyright 2015  Angelo Dureghello <angelo@sysam.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Minimal serial functions needed to use one of the uart ports
 * as serial console interface.
 */

#include <common.h>
#include <dm.h>
#include <dm/platform_data/serial_coldfire.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/immap.h>
#include <asm/uart.h>

DECLARE_GLOBAL_DATA_PTR;

extern void uart_port_conf(int port);

static int mcf_serial_init_common(uart_t *uart, int port_idx, int baudrate)
{
	u32 counter;

	uart_port_conf(port_idx);

	/* write to SICR: SIM2 = uart mode,dcd does not affect rx */
	writeb(UART_UCR_RESET_RX, &uart->ucr);
	writeb(UART_UCR_RESET_TX, &uart->ucr);
	writeb(UART_UCR_RESET_ERROR, &uart->ucr);
	writeb(UART_UCR_RESET_MR, &uart->ucr);
	__asm__("nop");

	writeb(0, &uart->uimr);

	/* write to CSR: RX/TX baud rate from timers */
	writeb(UART_UCSR_RCS_SYS_CLK | UART_UCSR_TCS_SYS_CLK, &uart->ucsr);

	writeb(UART_UMR_BC_8 | UART_UMR_PM_NONE, &uart->umr);
	writeb(UART_UMR_SB_STOP_BITS_1, &uart->umr);

	/* Setting up BaudRate */
	counter = (u32) ((gd->bus_clk / 32) + (baudrate / 2));
	counter = counter / baudrate;

	/* write to CTUR: divide counter upper byte */
	writeb((u8)((counter & 0xff00) >> 8), &uart->ubg1);
	/* write to CTLR: divide counter lower byte */
	writeb((u8)(counter & 0x00ff), &uart->ubg2);

	writeb(UART_UCR_RX_ENABLED | UART_UCR_TX_ENABLED, &uart->ucr);

	return (0);
}

static void mcf_serial_setbrg_common(uart_t *uart, int baudrate)
{
	u32 counter;

	/* Setting up BaudRate */
	counter = (u32) ((gd->bus_clk / 32) + (baudrate / 2));
	counter = counter / baudrate;

	/* write to CTUR: divide counter upper byte */
	writeb(((counter & 0xff00) >> 8), &uart->ubg1);
	/* write to CTLR: divide counter lower byte */
	writeb((counter & 0x00ff), &uart->ubg2);

	writeb(UART_UCR_RESET_RX, &uart->ucr);
	writeb(UART_UCR_RESET_TX, &uart->ucr);

	writeb(UART_UCR_RX_ENABLED | UART_UCR_TX_ENABLED, &uart->ucr);
}

#ifndef CONFIG_DM_SERIAL

static int mcf_serial_init(void)
{
	uart_t *uart_base;
	int port_idx;

	uart_base = (uart_t *)CONFIG_SYS_UART_BASE;
	port_idx = CONFIG_SYS_UART_PORT;

	return mcf_serial_init_common(uart_base, port_idx, gd->baudrate);
}

static void mcf_serial_putc(const char c)
{
	uart_t *uart = (uart_t *)CONFIG_SYS_UART_BASE;

	if (c == '\n')
		serial_putc('\r');

	/* Wait for last character to go. */
	while (!(readb(&uart->usr) & UART_USR_TXRDY))
		;

	writeb(c, &uart->utb);
}

static int mcf_serial_getc(void)
{
	uart_t *uart = (uart_t *)CONFIG_SYS_UART_BASE;

	/* Wait for a character to arrive. */
	while (!(readb(&uart->usr) & UART_USR_RXRDY))
		;

	return readb(&uart->urb);
}

static void mcf_serial_setbrg(void)
{
	uart_t *uart = (uart_t *)CONFIG_SYS_UART_BASE;

	mcf_serial_setbrg_common(uart, gd->baudrate);
}

static int mcf_serial_tstc(void)
{
	uart_t *uart = (uart_t *)CONFIG_SYS_UART_BASE;

	return readb(&uart->usr) & UART_USR_RXRDY;
}

static struct serial_device mcf_serial_drv = {
	.name	= "mcf_serial",
	.start	= mcf_serial_init,
	.stop	= NULL,
	.setbrg	= mcf_serial_setbrg,
	.putc	= mcf_serial_putc,
	.puts	= default_serial_puts,
	.getc	= mcf_serial_getc,
	.tstc	= mcf_serial_tstc,
};

void mcf_serial_initialize(void)
{
	serial_register(&mcf_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &mcf_serial_drv;
}

#endif

#ifdef CONFIG_DM_SERIAL

static int coldfire_serial_probe(struct udevice *dev)
{
	struct coldfire_serial_platdata *plat = dev->platdata;

	return mcf_serial_init_common((uart_t *)plat->base,
						plat->port, plat->baudrate);
}

static int coldfire_serial_putc(struct udevice *dev, const char ch)
{
	struct coldfire_serial_platdata *plat = dev->platdata;
	uart_t *uart = (uart_t *)plat->base;

	/* Wait for last character to go. */
	if (!(readb(&uart->usr) & UART_USR_TXRDY))
		return -EAGAIN;

	writeb(ch, &uart->utb);

	return 0;
}

static int coldfire_serial_getc(struct udevice *dev)
{
	struct coldfire_serial_platdata *plat = dev->platdata;
	uart_t *uart = (uart_t *)(plat->base);

	/* Wait for a character to arrive. */
	if (!(readb(&uart->usr) & UART_USR_RXRDY))
		return -EAGAIN;

	return readb(&uart->urb);
}

int coldfire_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct coldfire_serial_platdata *plat = dev->platdata;
	uart_t *uart = (uart_t *)(plat->base);

	mcf_serial_setbrg_common(uart, baudrate);

	return 0;
}

static int coldfire_serial_pending(struct udevice *dev, bool input)
{
	struct coldfire_serial_platdata *plat = dev->platdata;
	uart_t *uart = (uart_t *)(plat->base);

	if (input)
		return readb(&uart->usr) & UART_USR_RXRDY ? 1 : 0;
	else
		return readb(&uart->usr) & UART_USR_TXRDY ? 0 : 1;

	return 0;
}

static const struct dm_serial_ops coldfire_serial_ops = {
	.putc = coldfire_serial_putc,
	.pending = coldfire_serial_pending,
	.getc = coldfire_serial_getc,
	.setbrg = coldfire_serial_setbrg,
};

U_BOOT_DRIVER(serial_coldfire) = {
	.name = "serial_coldfire",
	.id = UCLASS_SERIAL,
	.probe = coldfire_serial_probe,
	.ops = &coldfire_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
#endif

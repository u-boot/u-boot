/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2011-2012 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <fdtdec.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

#define ZYNQ_UART_SR_TXFULL	0x00000010 /* TX FIFO full */
#define ZYNQ_UART_SR_RXEMPTY	0x00000002 /* RX FIFO empty */

#define ZYNQ_UART_CR_TX_EN	0x00000010 /* TX enabled */
#define ZYNQ_UART_CR_RX_EN	0x00000004 /* RX enabled */
#define ZYNQ_UART_CR_TXRST	0x00000002 /* TX logic reset */
#define ZYNQ_UART_CR_RXRST	0x00000001 /* RX logic reset */

#define ZYNQ_UART_MR_PARITY_NONE	0x00000020  /* No parity mode */

struct uart_zynq {
	u32 control; /* 0x0 - Control Register [8:0] */
	u32 mode; /* 0x4 - Mode Register [10:0] */
	u32 reserved1[4];
	u32 baud_rate_gen; /* 0x18 - Baud Rate Generator [15:0] */
	u32 reserved2[4];
	u32 channel_sts; /* 0x2c - Channel Status [11:0] */
	u32 tx_rx_fifo; /* 0x30 - FIFO [15:0] or [7:0] */
	u32 baud_rate_divider; /* 0x34 - Baud Rate Divider [7:0] */
};

static struct uart_zynq *uart_zynq_ports[2] = {
	[0] = (struct uart_zynq *)ZYNQ_SERIAL_BASEADDR0,
	[1] = (struct uart_zynq *)ZYNQ_SERIAL_BASEADDR1,
};

/* Set up the baud rate in gd struct */
static void uart_zynq_serial_setbrg(const int port)
{
	/* Calculation results. */
	unsigned int calc_bauderror, bdiv, bgen;
	unsigned long calc_baud = 0;
	unsigned long baud;
	unsigned long clock = get_uart_clk(port);
	struct uart_zynq *regs = uart_zynq_ports[port];

	/* Covering case where input clock is so slow */
	if (clock < 1000000 && gd->baudrate > 4800)
		gd->baudrate = 4800;

	baud = gd->baudrate;

	/*                master clock
	 * Baud rate = ------------------
	 *              bgen * (bdiv + 1)
	 *
	 * Find acceptable values for baud generation.
	 */
	for (bdiv = 4; bdiv < 255; bdiv++) {
		bgen = clock / (baud * (bdiv + 1));
		if (bgen < 2 || bgen > 65535)
			continue;

		calc_baud = clock / (bgen * (bdiv + 1));

		/*
		 * Use first calculated baudrate with
		 * an acceptable (<3%) error
		 */
		if (baud > calc_baud)
			calc_bauderror = baud - calc_baud;
		else
			calc_bauderror = calc_baud - baud;
		if (((calc_bauderror * 100) / baud) < 3)
			break;
	}

	writel(bdiv, &regs->baud_rate_divider);
	writel(bgen, &regs->baud_rate_gen);
}

/* Initialize the UART, with...some settings. */
static int uart_zynq_serial_init(const int port)
{
	struct uart_zynq *regs = uart_zynq_ports[port];

	if (!regs)
		return -1;

	/* RX/TX enabled & reset */
	writel(ZYNQ_UART_CR_TX_EN | ZYNQ_UART_CR_RX_EN | ZYNQ_UART_CR_TXRST | \
					ZYNQ_UART_CR_RXRST, &regs->control);
	writel(ZYNQ_UART_MR_PARITY_NONE, &regs->mode); /* 8 bit, no parity */
	uart_zynq_serial_setbrg(port);

	return 0;
}

static void uart_zynq_serial_putc(const char c, const int port)
{
	struct uart_zynq *regs = uart_zynq_ports[port];

	while ((readl(&regs->channel_sts) & ZYNQ_UART_SR_TXFULL) != 0)
		WATCHDOG_RESET();

	if (c == '\n') {
		writel('\r', &regs->tx_rx_fifo);
		while ((readl(&regs->channel_sts) & ZYNQ_UART_SR_TXFULL) != 0)
			WATCHDOG_RESET();
	}
	writel(c, &regs->tx_rx_fifo);
}

static void uart_zynq_serial_puts(const char *s, const int port)
{
	while (*s)
		uart_zynq_serial_putc(*s++, port);
}

static int uart_zynq_serial_tstc(const int port)
{
	struct uart_zynq *regs = uart_zynq_ports[port];

	return (readl(&regs->channel_sts) & ZYNQ_UART_SR_RXEMPTY) == 0;
}

static int uart_zynq_serial_getc(const int port)
{
	struct uart_zynq *regs = uart_zynq_ports[port];

	while (!uart_zynq_serial_tstc(port))
		WATCHDOG_RESET();
	return readl(&regs->tx_rx_fifo);
}

/* Multi serial device functions */
#define DECLARE_PSSERIAL_FUNCTIONS(port) \
	static int uart_zynq##port##_init(void) \
				{ return uart_zynq_serial_init(port); } \
	static void uart_zynq##port##_setbrg(void) \
				{ return uart_zynq_serial_setbrg(port); } \
	static int uart_zynq##port##_getc(void) \
				{ return uart_zynq_serial_getc(port); } \
	static int uart_zynq##port##_tstc(void) \
				{ return uart_zynq_serial_tstc(port); } \
	static void uart_zynq##port##_putc(const char c) \
				{ uart_zynq_serial_putc(c, port); } \
	static void uart_zynq##port##_puts(const char *s) \
				{ uart_zynq_serial_puts(s, port); }

/* Serial device descriptor */
#define INIT_PSSERIAL_STRUCTURE(port, __name) {	\
	  .name   = __name,			\
	  .start  = uart_zynq##port##_init,	\
	  .stop   = NULL,			\
	  .setbrg = uart_zynq##port##_setbrg,	\
	  .getc   = uart_zynq##port##_getc,	\
	  .tstc   = uart_zynq##port##_tstc,	\
	  .putc   = uart_zynq##port##_putc,	\
	  .puts   = uart_zynq##port##_puts,	\
}

DECLARE_PSSERIAL_FUNCTIONS(0);
static struct serial_device uart_zynq_serial0_device =
	INIT_PSSERIAL_STRUCTURE(0, "ttyPS0");
DECLARE_PSSERIAL_FUNCTIONS(1);
static struct serial_device uart_zynq_serial1_device =
	INIT_PSSERIAL_STRUCTURE(1, "ttyPS1");

#ifdef CONFIG_OF_CONTROL
__weak struct serial_device *default_serial_console(void)
{
	const void *blob = gd->fdt_blob;
	int node;
	unsigned int base_addr;

	node = fdt_path_offset(blob, "serial0");
	if (node < 0)
		return NULL;

	base_addr = fdtdec_get_addr(blob, node, "reg");
	if (base_addr == FDT_ADDR_T_NONE)
		return NULL;

	if (base_addr == ZYNQ_SERIAL_BASEADDR0)
		return &uart_zynq_serial0_device;

	if (base_addr == ZYNQ_SERIAL_BASEADDR1)
		return &uart_zynq_serial1_device;

	return NULL;
}
#else
__weak struct serial_device *default_serial_console(void)
{
#if defined(CONFIG_ZYNQ_SERIAL_UART0)
	if (uart_zynq_ports[0])
		return &uart_zynq_serial0_device;
#endif
#if defined(CONFIG_ZYNQ_SERIAL_UART1)
	if (uart_zynq_ports[1])
		return &uart_zynq_serial1_device;
#endif
	return NULL;
}
#endif

void zynq_serial_initialize(void)
{
	serial_register(&uart_zynq_serial0_device);
	serial_register(&uart_zynq_serial1_device);
}

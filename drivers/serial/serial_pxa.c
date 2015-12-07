/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/regs-uart.h>
#include <asm/io.h>
#include <linux/compiler.h>
#include <serial.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The numbering scheme differs here for PXA25x, PXA27x and PXA3xx so we can
 * easily handle enabling of clock.
 */
#ifdef	CONFIG_CPU_MONAHANS
#define	UART_CLK_BASE	CKENA_21_BTUART
#define	UART_CLK_REG	CKENA
#define	BTUART_INDEX	0
#define	FFUART_INDEX	1
#define	STUART_INDEX	2
#elif	CONFIG_CPU_PXA25X
#define	UART_CLK_BASE	(1 << 4)	/* HWUART */
#define	UART_CLK_REG	CKEN
#define	HWUART_INDEX	0
#define	STUART_INDEX	1
#define	FFUART_INDEX	2
#define	BTUART_INDEX	3
#else	/* PXA27x */
#define	UART_CLK_BASE	CKEN5_STUART
#define	UART_CLK_REG	CKEN
#define	STUART_INDEX	0
#define	FFUART_INDEX	1
#define	BTUART_INDEX	2
#endif

/*
 * Only PXA250 has HWUART, to avoid poluting the code with more macros,
 * artificially introduce this.
 */
#ifndef	CONFIG_CPU_PXA25X
#define	HWUART_INDEX	0xff
#endif

static uint32_t pxa_uart_get_baud_divider(void)
{
	if (gd->baudrate == 1200)
		return 768;
	else if (gd->baudrate == 9600)
		return 96;
	else if (gd->baudrate == 19200)
		return 48;
	else if (gd->baudrate == 38400)
		return 24;
	else if (gd->baudrate == 57600)
		return 16;
	else if (gd->baudrate == 115200)
		return 8;
	else	/* Unsupported baudrate */
		return 0;
}

static struct pxa_uart_regs *pxa_uart_index_to_regs(uint32_t uart_index)
{
	switch (uart_index) {
	case FFUART_INDEX: return (struct pxa_uart_regs *)FFUART_BASE;
	case BTUART_INDEX: return (struct pxa_uart_regs *)BTUART_BASE;
	case STUART_INDEX: return (struct pxa_uart_regs *)STUART_BASE;
	case HWUART_INDEX: return (struct pxa_uart_regs *)HWUART_BASE;
	default:
		return NULL;
	}
}

static void pxa_uart_toggle_clock(uint32_t uart_index, int enable)
{
	uint32_t clk_reg, clk_offset, reg;

	clk_reg = UART_CLK_REG;
	clk_offset = UART_CLK_BASE << uart_index;

	reg = readl(clk_reg);

	if (enable)
		reg |= clk_offset;
	else
		reg &= ~clk_offset;

	writel(reg, clk_reg);
}

/*
 * Enable clock and set baud rate, parity etc.
 */
void pxa_setbrg_dev(uint32_t uart_index)
{
	uint32_t divider = 0;
	struct pxa_uart_regs *uart_regs;

	divider = pxa_uart_get_baud_divider();
	if (!divider)
		hang();

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		hang();

	pxa_uart_toggle_clock(uart_index, 1);

	/* Disable interrupts and FIFOs */
	writel(0, &uart_regs->ier);
	writel(0, &uart_regs->fcr);

	/* Set baud rate */
	writel(LCR_WLS0 | LCR_WLS1 | LCR_DLAB, &uart_regs->lcr);
	writel(divider & 0xff, &uart_regs->dll);
	writel(divider >> 8, &uart_regs->dlh);
	writel(LCR_WLS0 | LCR_WLS1, &uart_regs->lcr);

	/* Enable UART */
	writel(IER_UUE, &uart_regs->ier);
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
int pxa_init_dev(unsigned int uart_index)
{
	pxa_setbrg_dev (uart_index);
	return 0;
}

/*
 * Output a single byte to the serial port.
 */
void pxa_putc_dev(unsigned int uart_index, const char c)
{
	struct pxa_uart_regs *uart_regs;

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		hang();

	while (!(readl(&uart_regs->lsr) & LSR_TEMT))
		WATCHDOG_RESET();
	writel(c, &uart_regs->thr);

	/* If \n, also do \r */
	if (c == '\n')
		pxa_putc_dev (uart_index,'\r');
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_tstc_dev(unsigned int uart_index)
{
	struct pxa_uart_regs *uart_regs;

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		return -1;

	return readl(&uart_regs->lsr) & LSR_DR;
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int pxa_getc_dev(unsigned int uart_index)
{
	struct pxa_uart_regs *uart_regs;

	uart_regs = pxa_uart_index_to_regs(uart_index);
	if (!uart_regs)
		return -1;

	while (!(readl(&uart_regs->lsr) & LSR_DR))
		WATCHDOG_RESET();
	return readl(&uart_regs->rbr) & 0xff;
}

void pxa_puts_dev(unsigned int uart_index, const char *s)
{
	while (*s)
		pxa_putc_dev(uart_index, *s++);
}

#define	pxa_uart(uart, UART)						\
	int uart##_init(void)						\
	{								\
		return pxa_init_dev(UART##_INDEX);			\
	}								\
									\
	void uart##_setbrg(void)					\
	{								\
		return pxa_setbrg_dev(UART##_INDEX);			\
	}								\
									\
	void uart##_putc(const char c)					\
	{								\
		return pxa_putc_dev(UART##_INDEX, c);			\
	}								\
									\
	void uart##_puts(const char *s)					\
	{								\
		return pxa_puts_dev(UART##_INDEX, s);			\
	}								\
									\
	int uart##_getc(void)						\
	{								\
		return pxa_getc_dev(UART##_INDEX);			\
	}								\
									\
	int uart##_tstc(void)						\
	{								\
		return pxa_tstc_dev(UART##_INDEX);			\
	}								\

#define	pxa_uart_desc(uart)						\
	struct serial_device serial_##uart##_device =			\
	{								\
		.name	= "serial_"#uart,				\
		.start	= uart##_init,					\
		.stop	= NULL,						\
		.setbrg	= uart##_setbrg,				\
		.getc	= uart##_getc,					\
		.tstc	= uart##_tstc,					\
		.putc	= uart##_putc,					\
		.puts	= uart##_puts,					\
	};

#define	pxa_uart_multi(uart, UART)					\
	pxa_uart(uart, UART)						\
	pxa_uart_desc(uart)

#if defined(CONFIG_HWUART)
	pxa_uart_multi(hwuart, HWUART)
#endif
#if defined(CONFIG_STUART)
	pxa_uart_multi(stuart, STUART)
#endif
#if defined(CONFIG_FFUART)
	pxa_uart_multi(ffuart, FFUART)
#endif
#if defined(CONFIG_BTUART)
	pxa_uart_multi(btuart, BTUART)
#endif

__weak struct serial_device *default_serial_console(void)
{
#if CONFIG_CONS_INDEX == 1
	return &serial_hwuart_device;
#elif CONFIG_CONS_INDEX == 2
	return &serial_stuart_device;
#elif CONFIG_CONS_INDEX == 3
	return &serial_ffuart_device;
#elif CONFIG_CONS_INDEX == 4
	return &serial_btuart_device;
#else
#error "Bad CONFIG_CONS_INDEX."
#endif
}

void pxa_serial_initialize(void)
{
#if defined(CONFIG_FFUART)
	serial_register(&serial_ffuart_device);
#endif
#if defined(CONFIG_BTUART)
	serial_register(&serial_btuart_device);
#endif
#if defined(CONFIG_STUART)
	serial_register(&serial_stuart_device);
#endif
}

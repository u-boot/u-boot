/* GRLIB APBUART Serial controller driver
 *
 * (C) Copyright 2007, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <ambapp.h>
#include <grlib/apbuart.h>
#include <serial.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

/* Select which UART that will become u-boot console */
#ifndef CONFIG_SYS_GRLIB_APBUART_INDEX
/* Try to use CONFIG_CONS_INDEX, if available, it is numbered from 1 */
#ifdef CONFIG_CONS_INDEX
#define CONFIG_SYS_GRLIB_APBUART_INDEX (CONFIG_CONS_INDEX - 1)
#else
#define CONFIG_SYS_GRLIB_APBUART_INDEX 0
#endif
#endif

static unsigned apbuart_calc_scaler(unsigned apbuart_freq, unsigned baud)
{
	return (((apbuart_freq * 10) / (baud * 8)) - 5) / 10;
}

static int leon3_serial_init(void)
{
	ambapp_dev_apbuart *uart;
	ambapp_apbdev apbdev;
	unsigned int tmp;

	/* find UART */
	if (ambapp_apb_find(&ambapp_plb, VENDOR_GAISLER, GAISLER_APBUART,
		CONFIG_SYS_GRLIB_APBUART_INDEX, &apbdev) != 1) {
		gd->flags &= ~GD_FLG_SERIAL_READY;
		panic("%s: apbuart not found!\n", __func__);
		return -1; /* didn't find hardware */
	}

	/* found apbuart, let's init .. */
	uart = (ambapp_dev_apbuart *) apbdev.address;

	/* APBUART Frequency is equal to bus frequency */
	gd->arch.uart_freq = ambapp_bus_freq(&ambapp_plb, apbdev.ahb_bus_index);

	/* Set scaler / baud rate */
	tmp = apbuart_calc_scaler(gd->arch.uart_freq, CONFIG_BAUDRATE);
	writel(tmp, &uart->scaler);

	/* Let bit 11 be unchanged (debug bit for GRMON) */
	tmp = readl(&uart->ctrl) & APBUART_CTRL_DBG;
	/* Receiver & transmitter enable */
	tmp |= APBUART_CTRL_RE | APBUART_CTRL_TE;
	writel(tmp, &uart->ctrl);

	gd->arch.uart = uart;
	return 0;
}

static inline ambapp_dev_apbuart *leon3_get_uart_regs(void)
{
	ambapp_dev_apbuart *uart = gd->arch.uart;
	return uart;
}

static void leon3_serial_putc_raw(const char c)
{
	ambapp_dev_apbuart * const uart = leon3_get_uart_regs();

	if (!uart)
		return;

	/* Wait for last character to go. */
	while (!(readl(&uart->status) & APBUART_STATUS_THE))
		WATCHDOG_RESET();

	/* Send data */
	writel(c, &uart->data);

#ifdef LEON_DEBUG
	/* Wait for data to be sent */
	while (!(readl(&uart->status) & APBUART_STATUS_TSE))
		WATCHDOG_RESET();
#endif
}

static void leon3_serial_putc(const char c)
{
	if (c == '\n')
		leon3_serial_putc_raw('\r');

	leon3_serial_putc_raw(c);
}

static int leon3_serial_getc(void)
{
	ambapp_dev_apbuart * const uart = leon3_get_uart_regs();

	if (!uart)
		return 0;

	/* Wait for a character to arrive. */
	while (!(readl(&uart->status) & APBUART_STATUS_DR))
		WATCHDOG_RESET();

	/* Read character data */
	return readl(&uart->data);
}

static int leon3_serial_tstc(void)
{
	ambapp_dev_apbuart * const uart = leon3_get_uart_regs();

	if (!uart)
		return 0;

	return readl(&uart->status) & APBUART_STATUS_DR;
}

/* set baud rate for uart */
static void leon3_serial_setbrg(void)
{
	ambapp_dev_apbuart * const uart = leon3_get_uart_regs();
	unsigned int scaler;

	if (!uart)
		return;

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	if (!gd->arch.uart_freq)
		gd->arch.uart_freq = CONFIG_SYS_CLK_FREQ;

	scaler = apbuart_calc_scaler(gd->arch.uart_freq, gd->baudrate);

	writel(scaler, &uart->scaler);
}

static struct serial_device leon3_serial_drv = {
	.name	= "leon3_serial",
	.start	= leon3_serial_init,
	.stop	= NULL,
	.setbrg	= leon3_serial_setbrg,
	.putc	= leon3_serial_putc,
	.puts	= default_serial_puts,
	.getc	= leon3_serial_getc,
	.tstc	= leon3_serial_tstc,
};

void leon3_serial_initialize(void)
{
	serial_register(&leon3_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &leon3_serial_drv;
}

#ifdef CONFIG_DEBUG_UART_APBUART

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	ambapp_dev_apbuart *uart = (ambapp_dev_apbuart *)CONFIG_DEBUG_UART_BASE;
	uart->scaler = apbuart_calc_scaler(CONFIG_DEBUG_UART_CLOCK, CONFIG_BAUDRATE);
	uart->ctrl = APBUART_CTRL_RE | APBUART_CTRL_TE;
}

static inline void _debug_uart_putc(int ch)
{
	ambapp_dev_apbuart *uart = (ambapp_dev_apbuart *)CONFIG_DEBUG_UART_BASE;
	while (!(readl(&uart->status) & APBUART_STATUS_THE))
		WATCHDOG_RESET();
	writel(ch, &uart->data);
}

DEBUG_UART_FUNCS

#endif

/* GRLIB APBUART Serial controller driver
 *
 * (C) Copyright 2008, 2015
 * Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <serial.h>
#include <watchdog.h>

DECLARE_GLOBAL_DATA_PTR;

static unsigned leon2_serial_calc_scaler(unsigned freq, unsigned baud)
{
	return (((freq*10) / (baud*8)) - 5) / 10;
}

static int leon2_serial_init(void)
{
	LEON2_regs *leon2 = (LEON2_regs *)LEON2_PREGS;
	LEON2_Uart_regs *regs;
	unsigned int tmp;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *)&leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *)&leon2->UART_Channel_2;
#endif

	/* Set scaler / baud rate */
	tmp = leon2_serial_calc_scaler(CONFIG_SYS_CLK_FREQ, CONFIG_BAUDRATE);
	writel(tmp, &regs->UART_Scaler);

	/* Let bit 11 be unchanged (debug bit for GRMON) */
	tmp = readl(&regs->UART_Control) & LEON2_UART_CTRL_DBG;
	tmp |= (LEON2_UART1_LOOPBACK_ENABLE << 7);
	tmp |= (LEON2_UART1_FLOWCTRL_ENABLE << 6);
	tmp |= (LEON2_UART1_PARITY_ENABLE << 5);
	tmp |= (LEON2_UART1_ODDPAR_ENABLE << 4);
	/* Receiver & transmitter enable */
	tmp |= (LEON2_UART_CTRL_RE | LEON2_UART_CTRL_TE);
	writel(tmp, &regs->UART_Control);

	gd->arch.uart = regs;
	return 0;
}

static inline LEON2_Uart_regs *leon2_get_uart_regs(void)
{
	LEON2_Uart_regs *uart = gd->arch.uart;

	return uart;
}

static void leon2_serial_putc_raw(const char c)
{
	LEON2_Uart_regs *uart = leon2_get_uart_regs();

	if (!uart)
		return;

	/* Wait for last character to go. */
	while (!(readl(&uart->UART_Status) & LEON2_UART_STAT_THE))
		WATCHDOG_RESET();

	/* Send data */
	writel(c, &uart->UART_Channel);

#ifdef LEON_DEBUG
	/* Wait for data to be sent */
	while (!(readl(&uart->UART_Status) & LEON2_UART_STAT_TSE))
		WATCHDOG_RESET();
#endif
}

static void leon2_serial_putc(const char c)
{
	if (c == '\n')
		leon2_serial_putc_raw('\r');

	leon2_serial_putc_raw(c);
}

static int leon2_serial_getc(void)
{
	LEON2_Uart_regs *uart = leon2_get_uart_regs();

	if (!uart)
		return 0;

	/* Wait for a character to arrive. */
	while (!(readl(&uart->UART_Status) & LEON2_UART_STAT_DR))
		WATCHDOG_RESET();

	/* Read character data */
	return readl(&uart->UART_Channel);
}

static int leon2_serial_tstc(void)
{
	LEON2_Uart_regs *uart = leon2_get_uart_regs();

	if (!uart)
		return 0;

	return readl(&uart->UART_Status) & LEON2_UART_STAT_DR;
}

static void leon2_serial_setbrg(void)
{
	LEON2_Uart_regs *uart = leon2_get_uart_regs();
	unsigned int scaler;

	if (!uart)
		return;

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	scaler = leon2_serial_calc_scaler(CONFIG_SYS_CLK_FREQ, gd->baudrate);

	writel(scaler, &uart->UART_Scaler);
}

static struct serial_device leon2_serial_drv = {
	.name	= "leon2_serial",
	.start	= leon2_serial_init,
	.stop	= NULL,
	.setbrg	= leon2_serial_setbrg,
	.putc	= leon2_serial_putc,
	.puts	= default_serial_puts,
	.getc	= leon2_serial_getc,
	.tstc	= leon2_serial_tstc,
};

void leon2_serial_initialize(void)
{
	serial_register(&leon2_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &leon2_serial_drv;
}

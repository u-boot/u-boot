/* GRLIB APBUART Serial controller driver
 *
 * (C) Copyright 2008
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/leon.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

static int leon2_serial_init(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;
	unsigned int tmp;

	/* Init LEON2 UART
	 *
	 * Set scaler / baud rate
	 *
	 * Receiver & transmitter enable
	 */
#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	regs->UART_Scaler = CONFIG_SYS_LEON2_UART1_SCALER;

	/* Let bit 11 be unchanged (debug bit for GRMON) */
	tmp = READ_WORD(regs->UART_Control);

	regs->UART_Control = ((tmp & LEON2_UART_CTRL_DBG) |
			      (LEON2_UART1_LOOPBACK_ENABLE << 7) |
			      (LEON2_UART1_FLOWCTRL_ENABLE << 6) |
			      (LEON2_UART1_PARITY_ENABLE << 5) |
			      (LEON2_UART1_ODDPAR_ENABLE << 4) |
			      LEON2_UART_CTRL_RE | LEON2_UART_CTRL_TE);

	return 0;
}

static void leon2_serial_putc_raw(const char c)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	/* Wait for last character to go. */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_THE)) ;

	/* Send data */
	regs->UART_Channel = c;

#ifdef LEON_DEBUG
	/* Wait for data to be sent */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_TSE)) ;
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
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	/* Wait for a character to arrive. */
	while (!(READ_WORD(regs->UART_Status) & LEON2_UART_STAT_DR)) ;

	/* read data */
	return READ_WORD(regs->UART_Channel);
}

static int leon2_serial_tstc(void)
{
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	return (READ_WORD(regs->UART_Status) & LEON2_UART_STAT_DR);
}

/* set baud rate for uart */
static void leon2_serial_setbrg(void)
{
	/* update baud rate settings, read it from gd->baudrate */
	unsigned int scaler;
	LEON2_regs *leon2 = (LEON2_regs *) LEON2_PREGS;
	LEON2_Uart_regs *regs;

#if LEON2_CONSOLE_SELECT == LEON_CONSOLE_UART1
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_1;
#else
	regs = (LEON2_Uart_regs *) & leon2->UART_Channel_2;
#endif

	if (gd->baudrate > 0) {
		scaler =
		    (((CONFIG_SYS_CLK_FREQ * 10) / (gd->baudrate * 8)) -
		     5) / 10;
		regs->UART_Scaler = scaler;
	}
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

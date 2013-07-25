/*
 * Jz4740 UART support
 * Copyright (c) 2011
 * Qi Hardware, Xiangfu Liu <xiangfu@sharism.cc>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/jz4740.h>
#include <serial.h>
#include <linux/compiler.h>

/*
 * serial_init - initialize a channel
 *
 * This routine initializes the number of data bits, parity
 * and set the selected baud rate. Interrupts are disabled.
 * Set the modem control signals if the option is selected.
 *
 * RETURNS: N/A
 */
struct jz4740_uart *uart = (struct jz4740_uart *)CONFIG_SYS_UART_BASE;

static int jz_serial_init(void)
{
	/* Disable port interrupts while changing hardware */
	writeb(0, &uart->dlhr_ier);

	/* Disable UART unit function */
	writeb(~UART_FCR_UUE, &uart->iir_fcr);

	/* Set both receiver and transmitter in UART mode (not SIR) */
	writeb(~(SIRCR_RSIRE | SIRCR_TSIRE), &uart->isr);

	/*
	 * Set databits, stopbits and parity.
	 * (8-bit data, 1 stopbit, no parity)
	 */
	writeb(UART_LCR_WLEN_8 | UART_LCR_STOP_1, &uart->lcr);

	/* Set baud rate */
	serial_setbrg();

	/* Enable UART unit, enable and clear FIFO */
	writeb(UART_FCR_UUE | UART_FCR_FE | UART_FCR_TFLS | UART_FCR_RFLS,
	       &uart->iir_fcr);

	return 0;
}

static void jz_serial_setbrg(void)
{
	u32 baud_div, tmp;

	baud_div = CONFIG_SYS_EXTAL / 16 / CONFIG_BAUDRATE;

	tmp = readb(&uart->lcr);
	tmp |= UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);

	writeb((baud_div >> 8) & 0xff, &uart->dlhr_ier);
	writeb(baud_div & 0xff, &uart->rbr_thr_dllr);

	tmp &= ~UART_LCR_DLAB;
	writeb(tmp, &uart->lcr);
}

static int jz_serial_tstc(void)
{
	if (readb(&uart->lsr) & UART_LSR_DR)
		return 1;

	return 0;
}

static void jz_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	/* Wait for fifo to shift out some bytes */
	while (!((readb(&uart->lsr) & (UART_LSR_TDRQ | UART_LSR_TEMT)) == 0x60))
		;

	writeb((u8)c, &uart->rbr_thr_dllr);
}

static int jz_serial_getc(void)
{
	while (!serial_tstc())
		;

	return readb(&uart->rbr_thr_dllr);
}

static struct serial_device jz_serial_drv = {
	.name	= "jz_serial",
	.start	= jz_serial_init,
	.stop	= NULL,
	.setbrg	= jz_serial_setbrg,
	.putc	= jz_serial_putc,
	.puts	= default_serial_puts,
	.getc	= jz_serial_getc,
	.tstc	= jz_serial_tstc,
};

void jz_serial_initialize(void)
{
	serial_register(&jz_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &jz_serial_drv;
}

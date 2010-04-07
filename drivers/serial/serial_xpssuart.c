/*
 * U-boot driver for Xilinx Dragonfire UART.
 * Based on existing U-boot serial drivers.
 */

#include <common.h>

#include "serial_xpssuart.h"

DECLARE_GLOBAL_DATA_PTR;

/* Set up the baud rate in gd struct */
void serial_setbrg(void)
{
	/*              master clock
	 * Baud rate = ---------------
	 *              bgen*(bdiv+1)
	 *
	 * master clock = 50MHz (I think?)
	 * bdiv remains at default (reset) 15
	 *
	 * Simplify RHS to base/bgen, where base == master/16
	 */
	long base = XDFUART_BASECLK;
	long baud = gd->baudrate;
	long bgen = base / baud;
	long base_err = base - (bgen*baud);
	long mod_bgen = bgen + (base_err >= 0 ? 1 : -1);
	long mod_err = base - (mod_bgen*baud);

	/* you'd think there'd be an abs() macro somewhere in include/... */
	mod_err = mod_err < 0 ? -mod_err : mod_err;
	base_err = base_err < 0 ? -base_err : base_err;

	bgen = mod_err < base_err ? mod_bgen : bgen;

	xdfuart_writel(BAUDDIV,XDFUART_BDIV); /* Ensure it's 15 */
	xdfuart_writel(BAUDGEN,bgen);
}

/* Initialize the UART, with...some settings. */
int serial_init(void)
{
	xdfuart_writel(CR,0x17); /* RX/TX enabled & reset */
	xdfuart_writel(MR,0x20); /* 8 bit, no parity */
	serial_setbrg();
	return 0;
}

/* Write a char to the Tx buffer */
void serial_putc(char c)
{
	while ((xdfuart_readl(SR) & XDFUART_SR_TXFULL) != 0);
	xdfuart_writel(FIFO,c);
}

/* Write a null-terminated string to the UART */
void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

/* Get a char from Rx buffer */
int serial_getc(void)
{
	while (!serial_tstc());
	return xdfuart_readl(FIFO);
}

/* Test character presence in Rx buffer */
int serial_tstc(void)
{
	return (xdfuart_readl(SR) & XDFUART_SR_RXEMPTY) == 0;
}

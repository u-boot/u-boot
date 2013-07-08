/* GRLIB APBUART Serial controller driver
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/leon.h>
#include <ambapp.h>
#include <serial.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ambapp_dev_apbuart *leon3_apbuart = NULL;

static int leon3_serial_init(void)
{
	ambapp_apbdev apbdev;
	unsigned int tmp;

	/* find UART */
	if (ambapp_apb_first(VENDOR_GAISLER, GAISLER_APBUART, &apbdev) == 1) {

		leon3_apbuart = (ambapp_dev_apbuart *) apbdev.address;

		/* found apbuart, let's init...
		 *
		 * Set scaler / baud rate
		 *
		 * Receiver & transmitter enable
		 */
		leon3_apbuart->scaler = CONFIG_SYS_GRLIB_APBUART_SCALER;

		/* Let bit 11 be unchanged (debug bit for GRMON) */
		tmp = READ_WORD(leon3_apbuart->ctrl);

		leon3_apbuart->ctrl = ((tmp & LEON_REG_UART_CTRL_DBG) |
				       LEON_REG_UART_CTRL_RE |
				       LEON_REG_UART_CTRL_TE);

		return 0;
	}
	return -1;		/* didn't find hardware */
}

static void leon3_serial_putc_raw(const char c)
{
	if (!leon3_apbuart)
		return;

	/* Wait for last character to go. */
	while (!(READ_WORD(leon3_apbuart->status) & LEON_REG_UART_STATUS_THE)) ;

	/* Send data */
	leon3_apbuart->data = c;

#ifdef LEON_DEBUG
	/* Wait for data to be sent */
	while (!(READ_WORD(leon3_apbuart->status) & LEON_REG_UART_STATUS_TSE)) ;
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
	if (!leon3_apbuart)
		return 0;

	/* Wait for a character to arrive. */
	while (!(READ_WORD(leon3_apbuart->status) & LEON_REG_UART_STATUS_DR)) ;

	/* read data */
	return READ_WORD(leon3_apbuart->data);
}

static int leon3_serial_tstc(void)
{
	if (leon3_apbuart)
		return (READ_WORD(leon3_apbuart->status) &
			LEON_REG_UART_STATUS_DR);
	return 0;
}

/* set baud rate for uart */
static void leon3_serial_setbrg(void)
{
	/* update baud rate settings, read it from gd->baudrate */
	unsigned int scaler;
	if (leon3_apbuart && (gd->baudrate > 0)) {
		scaler =
		    (((CONFIG_SYS_CLK_FREQ * 10) / (gd->baudrate * 8)) -
		     5) / 10;
		leon3_apbuart->scaler = scaler;
	}
	return;
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

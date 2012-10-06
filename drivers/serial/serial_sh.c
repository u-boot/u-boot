/*
 * SuperH SCIF device driver.
 * Copyright (C) 2007,2008,2010 Nobuhiro Iwamatsu
 * Copyright (C) 2002 - 2008  Paul Mundt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/processor.h>
#include "serial_sh.h"
#include <serial.h>
#include <linux/compiler.h>

#if defined(CONFIG_CONS_SCIF0)
# define SCIF_BASE	SCIF0_BASE
#elif defined(CONFIG_CONS_SCIF1)
# define SCIF_BASE	SCIF1_BASE
#elif defined(CONFIG_CONS_SCIF2)
# define SCIF_BASE	SCIF2_BASE
#elif defined(CONFIG_CONS_SCIF3)
# define SCIF_BASE	SCIF3_BASE
#elif defined(CONFIG_CONS_SCIF4)
# define SCIF_BASE	SCIF4_BASE
#elif defined(CONFIG_CONS_SCIF5)
# define SCIF_BASE	SCIF5_BASE
#elif defined(CONFIG_CONS_SCIF6)
# define SCIF_BASE	SCIF6_BASE
#elif defined(CONFIG_CONS_SCIF7)
# define SCIF_BASE	SCIF7_BASE
#else
# error "Default SCIF doesn't set....."
#endif

#if defined(CONFIG_SCIF_A)
	#define SCIF_BASE_PORT	PORT_SCIFA
#else
	#define SCIF_BASE_PORT	PORT_SCIF
#endif

static struct uart_port sh_sci = {
	.membase	= (unsigned char*)SCIF_BASE,
	.mapbase	= SCIF_BASE,
	.type		= SCIF_BASE_PORT,
};

static void sh_serial_setbrg(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	sci_out(&sh_sci, SCBRR, SCBRR_VALUE(gd->baudrate, CONFIG_SYS_CLK_FREQ));
}

static int sh_serial_init(void)
{
	sci_out(&sh_sci, SCSCR , SCSCR_INIT(&sh_sci));
	sci_out(&sh_sci, SCSCR , SCSCR_INIT(&sh_sci));
	sci_out(&sh_sci, SCSMR, 0);
	sci_out(&sh_sci, SCSMR, 0);
	sci_out(&sh_sci, SCFCR, SCFCR_RFRST|SCFCR_TFRST);
	sci_in(&sh_sci, SCFCR);
	sci_out(&sh_sci, SCFCR, 0);

	serial_setbrg();
	return 0;
}

#if defined(CONFIG_CPU_SH7760) || \
	defined(CONFIG_CPU_SH7780) || \
	defined(CONFIG_CPU_SH7785) || \
	defined(CONFIG_CPU_SH7786)
static int scif_rxfill(struct uart_port *port)
{
	return sci_in(port, SCRFDR) & 0xff;
}
#elif defined(CONFIG_CPU_SH7763)
static int scif_rxfill(struct uart_port *port)
{
	if ((port->mapbase == 0xffe00000) ||
		(port->mapbase == 0xffe08000)) {
		/* SCIF0/1*/
		return sci_in(port, SCRFDR) & 0xff;
	} else {
		/* SCIF2 */
		return sci_in(port, SCFDR) & SCIF2_RFDC_MASK;
	}
}
#elif defined(CONFIG_ARCH_SH7372)
static int scif_rxfill(struct uart_port *port)
{
	if (port->type == PORT_SCIFA)
		return sci_in(port, SCFDR) & SCIF_RFDC_MASK;
	else
		return sci_in(port, SCRFDR);
}
#else
static int scif_rxfill(struct uart_port *port)
{
	return sci_in(port, SCFDR) & SCIF_RFDC_MASK;
}
#endif

static int serial_rx_fifo_level(void)
{
	return scif_rxfill(&sh_sci);
}

void serial_raw_putc(const char c)
{
	while (1) {
		/* Tx fifo is empty */
		if (sci_in(&sh_sci, SCxSR) & SCxSR_TEND(&sh_sci))
			break;
	}

	sci_out(&sh_sci, SCxTDR, c);
	sci_out(&sh_sci, SCxSR, sci_in(&sh_sci, SCxSR) & ~SCxSR_TEND(&sh_sci));
}

static void sh_serial_putc(const char c)
{
	if (c == '\n')
		serial_raw_putc('\r');
	serial_raw_putc(c);
}

static int sh_serial_tstc(void)
{
	return serial_rx_fifo_level() ? 1 : 0;
}

void handle_error(void)
{
	sci_in(&sh_sci, SCxSR);
	sci_out(&sh_sci, SCxSR, SCxSR_ERROR_CLEAR(&sh_sci));
	sci_in(&sh_sci, SCLSR);
	sci_out(&sh_sci, SCLSR, 0x00);
}

int serial_getc_check(void)
{
	unsigned short status;

	status = sci_in(&sh_sci, SCxSR);

	if (status & SCIF_ERRORS)
		handle_error();
	if (sci_in(&sh_sci, SCLSR) & SCxSR_ORER(&sh_sci))
		handle_error();
	return status & (SCIF_DR | SCxSR_RDxF(&sh_sci));
}

static int sh_serial_getc(void)
{
	unsigned short status;
	char ch;

	while (!serial_getc_check())
		;

	ch = sci_in(&sh_sci, SCxRDR);
	status = sci_in(&sh_sci, SCxSR);

	sci_out(&sh_sci, SCxSR, SCxSR_RDxF_CLEAR(&sh_sci));

	if (status & SCIF_ERRORS)
			handle_error();

	if (sci_in(&sh_sci, SCLSR) & SCxSR_ORER(&sh_sci))
		handle_error();
	return ch;
}

static struct serial_device sh_serial_drv = {
	.name	= "sh_serial",
	.start	= sh_serial_init,
	.stop	= NULL,
	.setbrg	= sh_serial_setbrg,
	.putc	= sh_serial_putc,
	.puts	= default_serial_puts,
	.getc	= sh_serial_getc,
	.tstc	= sh_serial_tstc,
};

void sh_serial_initialize(void)
{
	serial_register(&sh_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &sh_serial_drv;
}

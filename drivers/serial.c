/*
 * (C) Copyright 2000
 * Rob Taylor, Flying Pig Systems. robt@flyingpig.com.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#ifdef CFG_NS16550_SERIAL

#include <ns16550.h>
#ifdef CFG_NS87308
#include <ns87308.h>
#endif

#if !defined(CONFIG_CONS_INDEX)
#error	"No console index specified."
#elif (CONFIG_CONS_INDEX < 1) || (CONFIG_CONS_INDEX > 4)
#error	"Invalid console index value."
#endif

#if CONFIG_CONS_INDEX == 1 && !defined(CFG_NS16550_COM1)
#error	"Console port 1 defined but not configured."
#elif CONFIG_CONS_INDEX == 2 && !defined(CFG_NS16550_COM2)
#error	"Console port 2 defined but not configured."
#elif CONFIG_CONS_INDEX == 3 && !defined(CFG_NS16550_COM3)
#error	"Console port 3 defined but not configured."
#elif CONFIG_CONS_INDEX == 4 && !defined(CFG_NS16550_COM4)
#error	"Console port 4 defined but not configured."
#endif

/* Note: The port number specified in the functions is 1 based.
 *	 the array is 0 based.
 */
static NS16550_t serial_ports[4] = {
#ifdef CFG_NS16550_COM1
	(NS16550_t)CFG_NS16550_COM1,
#else
	NULL,
#endif
#ifdef CFG_NS16550_COM2
	(NS16550_t)CFG_NS16550_COM2,
#else
	NULL,
#endif
#ifdef CFG_NS16550_COM3
	(NS16550_t)CFG_NS16550_COM3,
#else
	NULL,
#endif
#ifdef CFG_NS16550_COM4
	(NS16550_t)CFG_NS16550_COM4
#else
	NULL
#endif
};

#define PORT	serial_ports[port-1]
#define CONSOLE	(serial_ports[CONFIG_CONS_INDEX-1])

static int calc_divisor (NS16550_t port)
{
	DECLARE_GLOBAL_DATA_PTR;
#ifdef CONFIG_OMAP1510
	/* If can't cleanly clock 115200 set div to 1 */
	if ((CFG_NS16550_CLK == 12000000) && (gd->baudrate == 115200)) {
		port->osc_12m_sel = OSC_12M_SEL;	/* enable 6.5 * divisor */
		return (1);				/* return 1 for base divisor */
	}
	port->osc_12m_sel = 0;			/* clear if previsouly set */
#endif
#ifdef CONFIG_OMAP1610
	/* If can't cleanly clock 115200 set div to 1 */
	if ((CFG_NS16550_CLK == 48000000) && (gd->baudrate == 115200)) {
		return (26);		/* return 26 for base divisor */
	}
#endif

#ifdef CONFIG_APTIX
#define MODE_X_DIV 13
#else
#define MODE_X_DIV 16
#endif
	return (CFG_NS16550_CLK / MODE_X_DIV / gd->baudrate);

}

int serial_init (void)
{
	int clock_divisor;

#ifdef CFG_NS87308
	initialise_ns87308();
#endif

#ifdef CFG_NS16550_COM1
	clock_divisor = calc_divisor(serial_ports[0]);
	NS16550_init(serial_ports[0], clock_divisor);
#endif
#ifdef CFG_NS16550_COM2
	clock_divisor = calc_divisor(serial_ports[1]);
	NS16550_init(serial_ports[1], clock_divisor);
#endif
#ifdef CFG_NS16550_COM3
	clock_divisor = calc_divisor(serial_ports[2]);
	NS16550_init(serial_ports[2], clock_divisor);
#endif
#ifdef CFG_NS16550_COM4
	clock_divisor = calc_divisor(serial_ports[3]);
	NS16550_init(serial_ports[3], clock_divisor);
#endif

	return (0);
}

void
_serial_putc(const char c,const int port)
{
	if (c == '\n')
		NS16550_putc(PORT, '\r');

	NS16550_putc(PORT, c);
}

void
_serial_putc_raw(const char c,const int port)
{
	NS16550_putc(PORT, c);
}

void
_serial_puts (const char *s,const int port)
{
	while (*s) {
		_serial_putc (*s++,port);
	}
}


int
_serial_getc(const int port)
{
	return NS16550_getc(PORT);
}

int
_serial_tstc(const int port)
{
	return NS16550_tstc(PORT);
}

void
_serial_setbrg (const int port)
{
	int clock_divisor;

	clock_divisor = calc_divisor(PORT);
	NS16550_reinit(PORT, clock_divisor);
}

void
serial_putc(const char c)
{
	_serial_putc(c,CONFIG_CONS_INDEX);
}

void
serial_putc_raw(const char c)
{
	_serial_putc_raw(c,CONFIG_CONS_INDEX);
}

void
serial_puts(const char *s)
{
	_serial_puts(s,CONFIG_CONS_INDEX);
}

int
serial_getc(void)
{
	return _serial_getc(CONFIG_CONS_INDEX);
}

int
serial_tstc(void)
{
	return _serial_tstc(CONFIG_CONS_INDEX);
}

void
serial_setbrg(void)
{
	_serial_setbrg(CONFIG_CONS_INDEX);
}

#endif

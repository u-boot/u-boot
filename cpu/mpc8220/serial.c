/*
 * (C) Copyright 2004, Freescale, Inc
 * TsiChung Liew, Tsi-Chung.Liew@freescale.com.
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
 *
 */

/*
 * Minimal serial functions needed to use one of the PSC ports
 * as serial console interface.
 */

#include <common.h>
#include <mpc8220.h>

int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

#if defined (CONFIG_EXTUART_CONSOLE)
	volatile uchar *cpld = (volatile uchar *) CFG_CPLD_BASE;
#endif

	/* Check CPLD Switch 2 whether is external or internal */
#if defined (CONFIG_EXTUART_CONSOLE)
	if ((*cpld & 0x02) == 0x02) {
		gd->bExtUart = 1;
		return ext_serial_init ();
	} else
#endif
	{
#if defined(CONFIG_PSC_CONSOLE)
		gd->bExtUart = 0;
		return psc_serial_init ();
#endif
	}

	return (0);
}

void serial_putc (const char c)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bExtUart) {
#if defined (CONFIG_EXTUART_CONSOLE)
		ext_serial_putc (c);
#endif
	} else {
#if defined(CONFIG_PSC_CONSOLE)
		psc_serial_putc (c);
#endif
	}
}

void serial_puts (const char *s)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bExtUart) {
#if defined (CONFIG_EXTUART_CONSOLE)
		ext_serial_puts (s);
#endif
	} else {
#if defined(CONFIG_PSC_CONSOLE)
		psc_serial_puts (s);
#endif
	}
}

int serial_getc (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bExtUart) {
#if defined (CONFIG_EXTUART_CONSOLE)
		return ext_serial_getc ();
#endif
	} else {
#if defined(CONFIG_PSC_CONSOLE)
		return psc_serial_getc ();
#endif
	}
}

int serial_tstc (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bExtUart) {
#if defined (CONFIG_EXTUART_CONSOLE)
		return ext_serial_tstc ();
#endif
	} else {
#if defined(CONFIG_PSC_CONSOLE)
		return psc_serial_tstc ();
#endif
	}
}

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->bExtUart) {
#if defined (CONFIG_EXTUART_CONSOLE)
		ext_serial_setbrg ();
#endif
	} else {
#if defined(CONFIG_PSC_CONSOLE)
		psc_serial_setbrg ();
#endif
	}
}

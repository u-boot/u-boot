/*
 * Copyright (C) 2004-2007 ARM Limited.
 * Copyright (C) 2008 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * As a special exception, if other files instantiate templates or use macros
 * or inline functions from this file, or you compile this file and link it
 * with other works to produce a work based on this file, this file does not
 * by itself cause the resulting work to be covered by the GNU General Public
 * License. However the source code for this file must still be made available
 * in accordance with section (3) of the GNU General Public License.

 * This exception does not invalidate any other reasons why a work based on
 * this file might be covered by the GNU General Public License.
 */

#include <common.h>
#include <serial.h>

#if defined(CONFIG_CPU_V6)
/*
 * ARMV6
 */
#define DCC_RBIT	(1 << 30)
#define DCC_WBIT	(1 << 29)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c0, c5, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c5, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c1, 0\n" : "=r" (x))

#elif defined(CONFIG_CPU_XSCALE)
/*
 * XSCALE
 */
#define DCC_RBIT	(1 << 31)
#define DCC_WBIT	(1 << 28)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c8, c0, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c9, c0, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c14, c0, 0\n" : "=r" (x))

#else
#define DCC_RBIT	(1 << 0)
#define DCC_WBIT	(1 << 1)

#define write_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c1, c0, 0\n" : : "r" (x))

#define read_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c1, c0, 0\n" : "=r" (x))

#define status_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c0, 0\n" : "=r" (x))

#endif

#define can_read_dcc(x)	do {	\
		status_dcc(x);	\
		x &= DCC_RBIT;	\
		} while (0);

#define can_write_dcc(x) do {	\
		status_dcc(x);	\
		x &= DCC_WBIT;	\
		x = (x == 0);	\
		} while (0);

#define TIMEOUT_COUNT 0x4000000

static int arm_dcc_init(void)
{
	return 0;
}

static int arm_dcc_getc(void)
{
	int ch;
	register unsigned int reg;

	do {
		can_read_dcc(reg);
	} while (!reg);
	read_dcc(ch);

	return ch;
}

static void arm_dcc_putc(char ch)
{
	register unsigned int reg;
	unsigned int timeout_count = TIMEOUT_COUNT;

	while (--timeout_count) {
		can_write_dcc(reg);
		if (reg)
			break;
	}
	if (timeout_count == 0)
		return;
	else
		write_dcc(ch);
}

static int arm_dcc_tstc(void)
{
	register unsigned int reg;

	can_read_dcc(reg);

	return reg;
}

static void arm_dcc_setbrg(void)
{
}

static struct serial_device arm_dcc_drv = {
	.name	= "arm_dcc",
	.start	= arm_dcc_init,
	.stop	= NULL,
	.setbrg	= arm_dcc_setbrg,
	.putc	= arm_dcc_putc,
	.puts	= default_serial_puts,
	.getc	= arm_dcc_getc,
	.tstc	= arm_dcc_tstc,
};

void arm_dcc_initialize(void)
{
	serial_register(&arm_dcc_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &arm_dcc_drv;
}

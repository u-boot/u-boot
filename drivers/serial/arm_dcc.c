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
#include <devices.h>

#define DCC_ARM9_RBIT	(1 << 0)
#define DCC_ARM9_WBIT	(1 << 1)
#define DCC_ARM11_RBIT	(1 << 30)
#define DCC_ARM11_WBIT	(1 << 29)

#define read_core_id(x)	do {						\
		__asm__ ("mrc p15, 0, %0, c0, c0, 0\n" : "=r" (x));	\
		x = (x >> 4) & 0xFFF;					\
		} while (0);

/*
 * ARM9
 */
#define write_arm9_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c1, c0, 0\n" : : "r" (x))

#define read_arm9_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c1, c0, 0\n" : "=r" (x))

#define status_arm9_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c0, 0\n" : "=r" (x))

#define can_read_arm9_dcc(x)	do {	\
		status_arm9_dcc(x);	\
		x &= DCC_ARM9_RBIT;	\
		} while (0);

#define can_write_arm9_dcc(x)	do {	\
		status_arm9_dcc(x);	\
		x &= DCC_ARM9_WBIT;	\
		x = (x == 0);		\
		} while (0);

/*
 * ARM11
 */
#define write_arm11_dcc(x)	\
		__asm__ volatile ("mcr p14, 0, %0, c0, c5, 0\n" : : "r" (x))

#define read_arm11_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c5, 0\n" : "=r" (x))

#define status_arm11_dcc(x)	\
		__asm__ volatile ("mrc p14, 0, %0, c0, c1, 0\n" : "=r" (x))

#define can_read_arm11_dcc(x)	do {	\
		status_arm11_dcc(x);	\
		x &= DCC_ARM11_RBIT;	\
		} while (0);

#define can_write_arm11_dcc(x)	do {	\
		status_arm11_dcc(x);	\
		x &= DCC_ARM11_WBIT;	\
		x = (x == 0);		\
		} while (0);

#define TIMEOUT_COUNT 0x4000000

static enum {
	arm9_and_earlier,
	arm11_and_later
} arm_type = arm9_and_earlier;

#ifndef CONFIG_ARM_DCC_MULTI
#define arm_dcc_init serial_init
void serial_setbrg(void) {}
#define arm_dcc_getc serial_getc
#define arm_dcc_putc serial_putc
#define arm_dcc_puts serial_puts
#define arm_dcc_tstc serial_tstc
#endif

int arm_dcc_init(void)
{
	register unsigned int id;

	read_core_id(id);

	if (id >= 0xb00)
		arm_type = arm11_and_later;
	else
		arm_type = arm9_and_earlier;

	return 0;
}

int arm_dcc_getc(void)
{
	int ch;
	register unsigned int reg;

	switch (arm_type) {
	case arm11_and_later:
		do {
			can_read_arm11_dcc(reg);
		} while (!reg);
		read_arm11_dcc(ch);
		break;

	case arm9_and_earlier:
	default:
		do {
			can_read_arm9_dcc(reg);
		} while (!reg);
		read_arm9_dcc(ch);
		break;
	}

	return ch;
}

void arm_dcc_putc(char ch)
{
	register unsigned int reg;
	unsigned int timeout_count = TIMEOUT_COUNT;

	switch (arm_type) {
	case arm11_and_later:
		while (--timeout_count) {
			can_write_arm11_dcc(reg);
			if (reg)
				break;
		}
		if (timeout_count == 0)
			return;
		else
			write_arm11_dcc(ch);
		break;

	case arm9_and_earlier:
	default:
		while (--timeout_count) {
			can_write_arm9_dcc(reg);
			if (reg)
				break;
		}
		if (timeout_count == 0)
			return;
		else
			write_arm9_dcc(ch);
		break;
	}
}

void arm_dcc_puts(const char *s)
{
	while (*s)
		arm_dcc_putc(*s++);
}

int arm_dcc_tstc(void)
{
	register unsigned int reg;

	switch (arm_type) {
	case arm11_and_later:
		can_read_arm11_dcc(reg);
		break;
	case arm9_and_earlier:
	default:
		can_read_arm9_dcc(reg);
		break;
	}

	return reg;
}

#ifdef CONFIG_ARM_DCC_MULTI
static device_t arm_dcc_dev;

int drv_arm_dcc_init(void)
{
	int rc;

	/* Device initialization */
	memset(&arm_dcc_dev, 0, sizeof(arm_dcc_dev));

	strcpy(arm_dcc_dev.name, "dcc");
	arm_dcc_dev.ext = 0;	/* No extensions */
	arm_dcc_dev.flags = DEV_FLAGS_INPUT | DEV_FLAGS_OUTPUT;
	arm_dcc_dev.tstc = arm_dcc_tstc;	/* 'tstc' function */
	arm_dcc_dev.getc = arm_dcc_getc;	/* 'getc' function */
	arm_dcc_dev.putc = arm_dcc_putc;	/* 'putc' function */
	arm_dcc_dev.puts = arm_dcc_puts;	/* 'puts' function */

	rc = device_register(&arm_dcc_dev);

	if (rc == 0) {
		arm_dcc_init();
		return 1;
	}

	return 0;
}
#endif

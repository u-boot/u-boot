/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <watchdog.h>
#include <command.h>
#ifdef CONFIG_M5272
#include <asm/m5272.h>
#endif
#ifdef CONFIG_M5282
#include <asm/m5282.h>
#endif
#include <asm/cache.h>

int do_reset (cmd_tbl_t * cmdtp, bd_t * bd, int flag, int argc, char *argv[])
{
	return 0;
}

unsigned long get_tbclk (void)
{
	return CFG_HZ;
}

int checkcpu (void)
{
#ifdef CONFIG_M5272
	puts ("MOTOROLA Coldfire MCF5272\n");
#endif
#ifdef CONFIG_M5282
	puts ("MOTOROLA Coldfire MCF5282\n");
#endif
	return 0;
}

void do_exception (void)
{
	printf ("\n\n*** Unexpected exception ... Reset Board! ***\n");
	for (;;);
}

void do_buserror (void)
{
	printf ("\n\n*** Bus error ... Reset Board! ***\n");
	for (;;);
}

void do_addresserror (void)
{
	printf ("\n\n*** Address error ... Reset Board! ***\n");
	for (;;);
}

void trap_init (ulong value)
{
	extern void buserror_handler (void);
	extern void addresserror_handler (void);
	extern void exception_handler (void);
	unsigned long *vec = 0;
	int i;

	vec[2] = buserror_handler;
	vec[3] = addresserror_handler;
	for (i = 4; i < 256; i++) {
		vec[i] = exception_handler;
	}
}

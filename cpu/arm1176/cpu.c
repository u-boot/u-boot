/*
 * (C) Copyright 2004 Texas Insturments
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <s3c6400.h>
#include <asm/system.h>

static void cache_flush (void);

static void cp_delay (void)
{
	volatile int i;

	/* Many OMAP regs need at least 2 nops  */
	for (i = 0; i < 100; i++)
		__asm__ __volatile__("nop\n");
}

int cpu_init (void)
{
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * we turn off caches etc ...
	 */

	disable_interrupts ();

	/* turn off I/D-cache */
	icache_disable();
	dcache_disable();
	cache_flush();

	return 0;
}


/* * reset the cpu by setting up the watchdog timer and let him time out */
void reset_cpu (ulong ignored)
{
	printf("reset... \n\n\n");
	SW_RST_REG = 0x6400;
	/* loop forever and wait for reset to happen */
	while (1) {
		if (serial_tstc()) {
			serial_getc();
			break;
		}
	}
	/*NOTREACHED*/
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	disable_interrupts ();
	reset_cpu (0);
	/*NOTREACHED*/
	return 0;
}

void icache_enable (void)
{
	ulong reg;

	reg = get_cr ();	/* get control reg. */
	cp_delay ();
	set_cr (reg | CR_I);
}

void icache_disable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg & ~CR_I);
}

int icache_status (void)
{
	return (get_cr () & CR_I) != 0;
}

/* It makes no sense to use the dcache if the MMU is not enabled */
void dcache_enable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg | CR_C);
}

void dcache_disable (void)
{
	ulong reg;

	reg = get_cr ();
	cp_delay ();
	set_cr (reg & ~CR_C);
}

int dcache_status (void)
{
	return (get_cr () & CR_C) != 0;
}

/* flush I/D-cache */
static void cache_flush (void)
{
	/* invalidate both caches and flush btb */
	asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (0));
	/* mem barrier to sync things */
	asm ("mcr p15, 0, %0, c7, c10, 4": :"r" (0));
}

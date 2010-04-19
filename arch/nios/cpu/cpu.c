/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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
#include <nios.h>


int checkcpu (void)
{
	unsigned val;
	unsigned rev_major;
	unsigned rev_minor;
	short nregs, hi_limit, lo_limit;

	/* Get cpu version info */
	val = rdctl (CTL_CPU_ID);
	puts ("CPU:   ");
	printf ("%s", (val & 0x00008000) ? "Nios-16 " : "Nios-32 ");
	rev_major = (val>>12) & 0x07;
	rev_minor = (val>>4) & 0x0ff;
	printf ("Rev. %d.%d (0x%04x)", rev_major, rev_minor,
			val & 0xffff);
	if (rev_major == 0x08)
		printf (" [OpenCore (R) Plus]");
	printf ("\n");

	/* Check register file */
	val = rdctl (CTL_WVALID);
	lo_limit = val & 0x01f;
	hi_limit = (val>>5) & 0x1f;
	nregs = (hi_limit + 2) * 16;
	printf ("Reg file size: %d LO_LIMIT/HI_LIMIT: %d/%d\n",
		nregs, lo_limit, hi_limit);

	return (0);
}


int do_reset (void)
{
	/* trap 0 does the trick ... at least with the OCI debug
	 * present -- haven't tested without it yet (stm).
	 */
	disable_interrupts ();
	ipri (1);
	asm volatile ("trap 0\n");

	/* No return ;-) */

	return(0);
}


#if defined(CONFIG_WATCHDOG)
void watchdog_reset (void)
{
}
#endif /* CONFIG_WATCHDOG */

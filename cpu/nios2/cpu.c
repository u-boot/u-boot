/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
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
#include <nios2.h>
#include <nios2-io.h>

#if defined (CFG_NIOS_SYSID_BASE)
extern void display_sysid (void);
#endif /* CFG_NIOS_SYSID_BASE */

int checkcpu (void)
{
	printf ("CPU   : Nios-II\n");
#if !defined(CFG_NIOS_SYSID_BASE)
	printf ("SYSID : <unknown>\n");
#else
	display_sysid ();
#endif
	return (0);
}


int do_reset (void)
{
	void (*rst)(void) = (void(*)(void))CFG_RESET_ADDR;
	disable_interrupts ();
	rst();
	return(0);
}

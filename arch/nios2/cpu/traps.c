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

#include <asm/ptrace.h>
#include <common.h>

void trap_handler (struct pt_regs *regs)
{
	/* Just issue warning */
	printf ("\n\n*** WARNING: unimplemented trap @ %08x\n\n",
			regs->reg[29] - 4);
}

void soft_emulation (struct pt_regs *regs)
{
	/* TODO: Software emulation of mul/div etc. Until this is
	 * implemented, generate warning and hang.
	 */
	printf ("\n\n*** ERROR: unimplemented instruction @ %08x\n",
			regs->reg[29] - 4);
	hang ();
}

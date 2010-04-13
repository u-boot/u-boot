/*
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

#ifndef _M68K_PTRACE_H
#define _M68K_PTRACE_H

/*
 * This struct defines the way the registers are stored on the
 * kernel stack during an exception.
 */
#ifndef __ASSEMBLY__

struct pt_regs {
	ulong d0;
	ulong d1;
	ulong d2;
	ulong d3;
	ulong d4;
	ulong d5;
	ulong d6;
	ulong d7;
	ulong a0;
	ulong a1;
	ulong a2;
	ulong a3;
	ulong a4;
	ulong a5;
	ulong a6;
#if defined(__M68K__)
	unsigned format:4;	/* frame format specifier */
	unsigned vector:12;	/* vector offset */
	unsigned short sr;
	unsigned long pc;
#else
	unsigned short sr;
	unsigned long pc;
#endif
};

#endif				/* #ifndef __ASSEMBLY__ */

#endif				/* #ifndef _M68K_PTRACE_H */

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

#ifndef __NIOS2_H__
#define __NIOS2_H__

/*------------------------------------------------------------------------
 * Control registers -- use with wrctl() & rdctl()
 *----------------------------------------------------------------------*/
#define CTL_STATUS	0		/* Processor status reg		*/
#define CTL_ESTATUS	1		/* Exception status reg		*/
#define CTL_BSTATUS	2		/* Break status reg		*/
#define CTL_IENABLE	3		/* Interrut enable reg		*/
#define CTL_IPENDING	4		/* Interrut pending reg		*/

/*------------------------------------------------------------------------
 * Access to control regs
 *----------------------------------------------------------------------*/
#define _str_(x) #x

#define rdctl(reg)\
	({unsigned int val;\
	asm volatile( "rdctl %0, ctl" _str_(reg)\
		: "=r" (val) ); val;})

#define wrctl(reg,val)\
	asm volatile( "wrctl ctl" _str_(reg) ",%0"\
		: : "r" (val))

/*------------------------------------------------------------------------
 * Control reg bit masks
 *----------------------------------------------------------------------*/
#define STATUS_IE	(1<<0)		/* Interrupt enable		*/
#define STATUS_U	(1<<1)		/* User-mode			*/

/*------------------------------------------------------------------------
 * Bit-31 Cache bypass -- only valid for data access. When data cache
 * is not implemented, bit 31 is ignored for compatibility.
 *----------------------------------------------------------------------*/
#define CACHE_BYPASS(a) ((a) | 0x80000000)
#define CACHE_NO_BYPASS(a) ((a) & ~0x80000000)

#endif /* __NIOS2_H__ */

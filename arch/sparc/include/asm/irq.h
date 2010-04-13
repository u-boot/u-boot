/* IRQ functions
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#ifndef __SPARC_IRQ_H__
#define __SPARC_IRQ_H__

#include <asm/psr.h>

/* Set SPARC Processor Interrupt Level */
extern inline void set_pil(unsigned int level)
{
	unsigned int psr = get_psr();

	put_psr((psr & ~PSR_PIL) | ((level & 0xf) << PSR_PIL_OFS));
}

/* Get SPARC Processor Interrupt Level */
extern inline unsigned int get_pil(void)
{
	unsigned int psr = get_psr();
	return (psr & PSR_PIL) >> PSR_PIL_OFS;
}

/* Disables interrupts and return current PIL value */
extern int intLock(void);

/* Sets the PIL to oldLevel */
extern void intUnlock(int oldLevel);

#endif

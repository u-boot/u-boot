/*
 * Copyright (C) 2006 Atmel Corporation
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
#ifndef __ASM_AVR32_ADDRSPACE_H
#define __ASM_AVR32_ADDRSPACE_H

/* Memory segments when segmentation is enabled */
#define P0SEG		0x00000000
#define P1SEG		0x80000000
#define P2SEG		0xa0000000
#define P3SEG		0xc0000000
#define P4SEG		0xe0000000

/* Returns the privileged segment base of a given address */
#define PXSEG(a)	(((unsigned long)(a)) & 0xe0000000)

/* Returns the physical address of a PnSEG (n=1,2) address */
#define PHYSADDR(a)	(((unsigned long)(a)) & 0x1fffffff)

/*
 * Map an address to a certain privileged segment
 */
#define P1SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P1SEG))
#define P2SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P2SEG))
#define P3SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P3SEG))
#define P4SEGADDR(a) ((__typeof__(a))(((unsigned long)(a) & 0x1fffffff) | P4SEG))

#endif /* __ASM_AVR32_ADDRSPACE_H */

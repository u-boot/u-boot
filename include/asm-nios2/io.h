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

#ifndef __ASM_NIOS2_IO_H_
#define __ASM_NIOS2_IO_H_

#define sync() asm volatile ("sync" : : : "memory");

extern unsigned char inb (unsigned char *port);
extern unsigned short inw (unsigned short *port);
extern unsigned inl (unsigned port);

#define readb(addr)\
	({unsigned char val;\
	 asm volatile( "ldbio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readw(addr)\
	({unsigned short val;\
	 asm volatile( "ldhio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readl(addr)\
	({unsigned long val;\
	 asm volatile( "ldwio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define writeb(addr,val)\
	asm volatile ("stbio %0, 0(%1)" : : "r" (addr), "r" (val))
#define writew(addr,val)\
	asm volatile ("sthio %0, 0(%1)" : : "r" (addr), "r" (val))
#define writel(addr,val)\
	asm volatile ("stwio %0, 0(%1)" : : "r" (addr), "r" (val))

#define inb(addr)	readb(addr)
#define inw(addr)	readw(addr)
#define inl(addr)	readl(addr)
#define outb(addr,val)	writeb(addr,val)
#define outw(addr,val)	writew(addr,val)
#define outl(addr,val)	writel(addr,val)

static inline void insb (unsigned long port, void *dst, unsigned long count)
{
	unsigned char *p = dst;
	while (count--) *p++ = inb (port);
}
static inline void insw (unsigned long port, void *dst, unsigned long count)
{
	unsigned short *p = dst;
	while (count--) *p++ = inw (port);
}
static inline void insl (unsigned long port, void *dst, unsigned long count)
{
	unsigned long *p = dst;
	while (count--) *p++ = inl (port);
}

static inline void outsb (unsigned long port, const void *src, unsigned long count)
{
	const unsigned char *p = src;
	while (count--) outb (*p++, port);
}

static inline void outsw (unsigned long port, const void *src, unsigned long count)
{
	const unsigned short *p = src;
	while (count--) outw (*p++, port);
}
static inline void outsl (unsigned long port, const void *src, unsigned long count)
{
	const unsigned long *p = src;
	while (count--) outl (*p++, port);
}

#endif /* __ASM_NIOS2_IO_H_ */

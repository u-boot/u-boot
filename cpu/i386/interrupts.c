/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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
#include <asm/interrupt.h>


struct idt_entry {
	u16	base_low;
	u16	selector;
	u8	res;
	u8	access;
	u16	base_high;
} __attribute__ ((packed));


struct idt_entry idt[256];


asm (".globl irq_return\n"
     "irq_return:\n"
     "     addl  $4, %esp\n"
     "     popa\n"
     "     iret\n");

void __attribute__ ((regparm(0))) default_isr(void);
asm ("default_isr: iret\n");

asm ("idt_ptr:\n"
	".word	0x800\n" /* size of the table 8*256 bytes */
	".long	idt\n"	 /* offset */
	".word	0x18\n");/* data segment */

void set_vector(u8 intnum, void *routine)
{
	idt[intnum].base_high = (u16)((u32)(routine + gd->reloc_off) >> 16);
	idt[intnum].base_low = (u16)((u32)(routine + gd->reloc_off) & 0xffff);
}


int cpu_init_interrupts(void)
{
	int i;

	/* Just in case... */
	disable_interrupts();

	/* Setup the IDT */
	for (i=0;i<256;i++) {
		idt[i].access = 0x8e;
		idt[i].res = 0;
		idt[i].selector = 0x10;
		set_vector(i, default_isr);
	}

	asm ("cs lidt idt_ptr\n");

	/* It is now safe to enable interrupts */
	enable_interrupts();

	return 0;
}

void enable_interrupts(void)
{
	asm("sti\n");
}

int disable_interrupts(void)
{
	long flags;

	asm volatile ("pushfl ; popl %0 ; cli\n" : "=g" (flags) : );

	return (flags&0x200); /* IE flags is bit 9 */
}

/*
 * (C) Copyright 2009
 * Graeme Russ, graeme.russ@gmail.com
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

#ifndef __ASM_INTERRUPT_H_
#define __ASM_INTERRUPT_H_ 1

/* cpu/i386/interrupts.c */
void set_vector(u8 intnum, void *routine);

/* lib_i386/interupts.c */
void disable_irq(int irq);
void enable_irq(int irq);

/* Architecture specific functions */
void mask_irq(int irq);
void unmask_irq(int irq);
void specific_eoi(int irq);

extern char exception_stack[];

#define __isr__ void __attribute__ ((regparm(0)))

#endif

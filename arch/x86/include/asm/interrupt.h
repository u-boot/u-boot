/*
 * (C) Copyright 2009
 * Graeme Russ, graeme.russ@gmail.com
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_INTERRUPT_H_
#define __ASM_INTERRUPT_H_ 1

#include <asm/types.h>

/* arch/x86/cpu/interrupts.c */
void set_vector(u8 intnum, void *routine);

/* arch/x86/lib/interrupts.c */
void disable_irq(int irq);
void enable_irq(int irq);

/* Architecture specific functions */
void mask_irq(int irq);
void unmask_irq(int irq);
void specific_eoi(int irq);

extern char exception_stack[];

#endif

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

/**
 * configure_irq_trigger() - Configure IRQ triggering
 *
 * Switch the given interrupt to be level / edge triggered
 *
 * @param int_num legacy interrupt number (3-7, 9-15)
 * @param is_level_triggered true for level triggered interrupt, false for
 *	edge triggered interrupt
 */
void configure_irq_trigger(int int_num, bool is_level_triggered);

#endif

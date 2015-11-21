/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2003
 * Gleb Natapov <gnatapov@mrv.com>
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/irq.h>

/* Implemented by SPARC CPUs */
extern int interrupt_init_cpu(void);
extern void timer_interrupt_cpu(void *arg);
extern int timer_interrupt_init_cpu(void);

int intLock(void)
{
	unsigned int pil;

	pil = get_pil();

	/* set PIL to 15 ==> no pending interrupts will interrupt CPU */
	set_pil(15);

	return pil;
}

void intUnlock(int oldLevel)
{
	set_pil(oldLevel);
}

void enable_interrupts(void)
{
	set_pil(0);		/* enable all interrupts */
}

int disable_interrupts(void)
{
	return intLock();
}

int interrupt_is_enabled(void)
{
	if (get_pil() == 15)
		return 0;
	return 1;
}

int interrupt_init(void)
{
	int ret;

	/* call cpu specific function from $(CPU)/interrupts.c */
	ret = interrupt_init_cpu();

	/* enable global interrupts */
	enable_interrupts();

	return ret;
}

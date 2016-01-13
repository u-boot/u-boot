/* IRQ functions
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SPARC_IRQ_H__
#define __SPARC_IRQ_H__

#include <asm/psr.h>

/* Set SPARC Processor Interrupt Level */
static inline void set_pil(unsigned int level)
{
	unsigned int psr = get_psr();

	put_psr((psr & ~PSR_PIL) | ((level & 0xf) << PSR_PIL_OFS));
}

/* Get SPARC Processor Interrupt Level */
static inline unsigned int get_pil(void)
{
	unsigned int psr = get_psr();
	return (psr & PSR_PIL) >> PSR_PIL_OFS;
}

/* Disables interrupts and return current PIL value */
extern int intLock(void);

/* Sets the PIL to oldLevel */
extern void intUnlock(int oldLevel);

/* Return non-zero if interrupts are currently enabled */
extern int interrupt_is_enabled(void);

#endif

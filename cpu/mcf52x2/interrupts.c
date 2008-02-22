/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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
#include <watchdog.h>
#include <asm/processor.h>
#include <asm/immap.h>

#ifdef	CONFIG_M5272
int interrupt_init(void)
{
	volatile intctrl_t *intp = (intctrl_t *) (MMAP_INTC);

	/* disable all external interrupts */
	intp->int_icr1 = 0x88888888;
	intp->int_icr2 = 0x88888888;
	intp->int_icr3 = 0x88888888;
	intp->int_icr4 = 0x88888888;
	intp->int_pitr = 0x00000000;
	/* initialize vector register */
	intp->int_pivr = 0x40;

	enable_interrupts();

	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	volatile intctrl_t *intp = (intctrl_t *) (CFG_INTR_BASE);

	intp->int_icr1 &= ~INT_ICR1_TMR3MASK;
	intp->int_icr1 |= CFG_TMRINTR_PRI;
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5272 */

#if defined(CONFIG_M5282) || defined(CONFIG_M5271)
int interrupt_init(void)
{
	volatile int0_t *intp = (int0_t *) (CFG_INTR_BASE);

	/* Make sure all interrupts are disabled */
	intp->imrl0 |= 0x1;

	enable_interrupts();
	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	volatile int0_t *intp = (int0_t *) (CFG_INTR_BASE);

	intp->icr0[CFG_TMRINTR_NO] = CFG_TMRINTR_PRI;
	intp->imrl0 &= 0xFFFFFFFE;
	intp->imrl0 &= ~CFG_TMRINTR_MASK;
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5282 | CONFIG_M5271 */

#if defined(CONFIG_M5249) || defined(CONFIG_M5253)
int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

#if defined(CONFIG_MCFTMR)
void dtimer_intr_setup(void)
{
	mbar_writeLong(MCFSIM_IMR, mbar_readLong(MCFSIM_IMR) & ~0x00000400);
	mbar_writeByte(MCFSIM_TIMER2ICR, CFG_TMRINTR_PRI);
}
#endif				/* CONFIG_MCFTMR */
#endif				/* CONFIG_M5249 || CONFIG_M5253 */

/*
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#include <asm/timer.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

static ulong timestamp;

#if defined(CONFIG_SLTTMR)
#ifndef CONFIG_SYS_UDELAY_BASE
#	error	"uDelay base not defined!"
#endif

#if !defined(CONFIG_SYS_TMR_BASE) || !defined(CONFIG_SYS_INTR_BASE) || !defined(CONFIG_SYS_TMRINTR_NO) || !defined(CONFIG_SYS_TMRINTR_MASK)
#	error	"TMR_BASE, INTR_BASE, TMRINTR_NO or TMRINTR_MASk not defined!"
#endif
extern void dtimer_intr_setup(void);

void __udelay(unsigned long usec)
{
	volatile slt_t *timerp = (slt_t *) (CONFIG_SYS_UDELAY_BASE);
	u32 now, freq;

	/* 1 us period */
	freq = CONFIG_SYS_TIMER_PRESCALER;

	timerp->cr = 0;		/* Disable */
	timerp->tcnt = usec * freq;
	timerp->cr = SLT_CR_TEN;

	now = timerp->cnt;
	while (now != 0)
		now = timerp->cnt;

	timerp->sr |= SLT_SR_ST;
	timerp->cr = 0;
}

void dtimer_interrupt(void *not_used)
{
	volatile slt_t *timerp = (slt_t *) (CONFIG_SYS_TMR_BASE);

	/* check for timer interrupt asserted */
	if ((CONFIG_SYS_TMRPND_REG & CONFIG_SYS_TMRINTR_MASK) == CONFIG_SYS_TMRINTR_PEND) {
		timerp->sr |= SLT_SR_ST;
		timestamp++;
		return;
	}
}

void timer_init(void)
{
	volatile slt_t *timerp = (slt_t *) (CONFIG_SYS_TMR_BASE);

	timestamp = 0;

	timerp->cr = 0;		/* disable timer */
	timerp->tcnt = 0;
	timerp->sr = SLT_SR_BE | SLT_SR_ST;	/* clear status */

	/* initialize and enable timer interrupt */
	irq_install_handler(CONFIG_SYS_TMRINTR_NO, dtimer_interrupt, 0);

	/* Interrupt every ms */
	timerp->tcnt = 1000 * CONFIG_SYS_TIMER_PRESCALER;

	dtimer_intr_setup();

	/* set a period of 1us, set timer mode to restart and
	   enable timer and interrupt */
	timerp->cr = SLT_CR_RUN | SLT_CR_IEN | SLT_CR_TEN;
}

ulong get_timer(ulong base)
{
	return (timestamp - base);
}

#endif				/* CONFIG_SLTTMR */

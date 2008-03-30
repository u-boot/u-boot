/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
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
#include <asm/arch/mx31-regs.h>

#define TIMER_BASE 0x53f90000 /* General purpose timer 1 */

/* General purpose timers registers */
#define GPTCR   __REG(TIMER_BASE) /* Control register */
#define GPTPR  __REG(TIMER_BASE + 0x4) /* Prescaler register */
#define GPTSR   __REG(TIMER_BASE + 0x8) /* Status register */
#define GPTCNT __REG(TIMER_BASE + 0x24) /* Counter register */

/* General purpose timers bitfields */
#define GPTCR_SWR       (1<<15) /* Software reset */
#define GPTCR_FRR       (1<<9)  /* Freerun / restart */
#define GPTCR_CLKSOURCE_32 (4<<6)  /* Clock source */
#define GPTCR_TEN       (1)     /* Timer enable */

/*
 * nothing really to do with interrupts, just starts up a counter.
 */
int interrupt_init(void)
{
	int i;

	/* setup GP Timer 1 */
	GPTCR = GPTCR_SWR;
	for (i = 0; i < 100; i++) GPTCR = 0; /* We have no udelay by now */
	GPTPR = 0; /* 32Khz */
	/* Freerun Mode, PERCLK1 input */
	GPTCR |= GPTCR_CLKSOURCE_32 | GPTCR_TEN;

	return 0;
}

void reset_timer_masked(void)
{
	GPTCR = 0;
	/* Freerun Mode, PERCLK1 input*/
	GPTCR = GPTCR_CLKSOURCE_32 | GPTCR_TEN;
}

ulong get_timer_masked(void)
{
	ulong val = GPTCNT;
	return val;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void set_timer(ulong t)
{
}

/* delay x useconds AND perserve advance timstamp value */
void udelay(unsigned long usec)
{
	ulong tmo, tmp;

	if (usec >= 1000) {
	/* "big" number, spread normalization to seconds */
		/* start to normalize for usec to ticks per sec */
		tmo = usec / 1000;
		/* find number of "ticks" to wait to achieve target */
		tmo *= CFG_HZ;
		tmo /= 1000;	/* finish normalize. */
	} else {
		/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}

	tmp = get_timer(0);		/* get current timestamp */
	if ((tmo + tmp + 1) < tmp)
		/* setting this forward will roll time stamp */
		/* reset "advancing" timestamp to 0, set lastinc value */
		reset_timer_masked();
	else
		/* else, set advancing stamp wake up time */
		tmo	+= tmp;
	while (get_timer_masked() < tmo)/* loop till event */
		/*NOP*/;
}

void reset_cpu(ulong addr)
{
	__REG16(WDOG_BASE) = 4;
}

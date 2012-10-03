/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
#include <clps7111.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/hardware.h>

#ifndef CONFIG_NETARM
/* we always count down the max. */
#define TIMER_LOAD_VAL 0xffff
/* macro to read the 16 bit timer */
#define READ_TIMER (IO_TC1D & 0xffff)

#else
#define IRQEN	(*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_INTR_ENABLE))
#define TM2CTRL (*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_TIMER2_CONTROL))
#define TM2STAT (*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_TIMER2_STATUS))
#define TIMER_LOAD_VAL NETARM_GEN_TSTAT_CTC_MASK
#define READ_TIMER (TM2STAT & NETARM_GEN_TSTAT_CTC_MASK)
#endif

#ifdef CONFIG_USE_IRQ
void do_irq (struct pt_regs *pt_regs)
{
#if defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No do_irq() for IntegratorAP/CM720T as yet */
#else
#error do_irq() not defined for this CPU type
#endif
}
#endif

#if defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* Use IntegratorAP routines in board/integratorap.c */
#else

static ulong timestamp;
static ulong lastdec;

int timer_init (void)
{
#if defined(CONFIG_NETARM)
	/* disable all interrupts */
	IRQEN = 0;

	/* operate timer 2 in non-prescale mode */
	TM2CTRL = ( NETARM_GEN_TIMER_SET_HZ(CONFIG_SYS_HZ) |
		    NETARM_GEN_TCTL_ENABLE |
		    NETARM_GEN_TCTL_INIT_COUNT(TIMER_LOAD_VAL));

	/* set timer 2 counter */
	lastdec = TIMER_LOAD_VAL;
#elif defined(CONFIG_TEGRA)
	/* No timer routines for tegra as yet */
	lastdec = 0;
#else
#error No timer_init() defined for this CPU type
#endif
	timestamp = 0;

	return (0);
}

#endif /* ! IntegratorAP */

/*
 * timer without interrupts
 */


#if defined(CONFIG_NETARM)

ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void __udelay (unsigned long usec)
{
	ulong tmo;

	tmo = usec / 1000;
	tmo *= CONFIG_SYS_HZ;
	tmo /= 1000;

	tmo += get_timer (0);

	while (get_timer_masked () < tmo)
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CONFIG_SYS_HZ;
		tmo /= 1000;
	} else {
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	endtime = get_timer_masked () + tmo;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

#elif defined(CONFIG_INTEGRATOR) && defined(CONFIG_ARCH_INTEGRATOR)
	/* No timer routines for IntegratorAP/CM720T as yet */
#elif defined(CONFIG_TEGRA)
	/* No timer routines for tegra as yet */
#else
#error Timer routines not defined for this CPU type
#endif

/*
 * Copyright (C) 2010 Linaro Limited
 * John Rigby <john.rigby@linaro.org>
 *
 * Based on original from Linux kernel source and
 * internal ST-Ericsson U-Boot source.
 * (C) Copyright 2009 Alessandro Rubini
 * (C) Copyright 2010 ST-Ericsson
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The MTU device has some interrupt control registers
 * followed by 4 timers.
 */

/* The timers */
struct u8500_mtu_timer {
	u32 lr;			/* Load value */
	u32 cv;			/* Current value */
	u32 cr;			/* Control reg */
	u32 bglr;		/* ??? */
};

/* The MTU that contains the timers */
struct u8500_mtu {
	u32 imsc;		/* Interrupt mask set/clear */
	u32 ris;		/* Raw interrupt status */
	u32 mis;		/* Masked interrupt status */
	u32 icr;		/* Interrupt clear register */
	struct u8500_mtu_timer pt[4];
};

/* bits for the control register */
#define MTU_CR_ONESHOT		0x01	/* if 0 = wraps reloading from BGLR */
#define MTU_CR_32BITS		0x02

#define MTU_CR_PRESCALE_1	0x00
#define MTU_CR_PRESCALE_16	0x04
#define MTU_CR_PRESCALE_256	0x08
#define MTU_CR_PRESCALE_MASK	0x0c

#define MTU_CR_PERIODIC		0x40	/* if 0 = free-running */
#define MTU_CR_ENA		0x80

/*
 * The MTU is clocked at 133 MHz by default. (V1 and later)
 */
#define TIMER_CLOCK		(133 * 1000 * 1000 / 16)
#define COUNT_TO_USEC(x)	((x) * 16 / 133)
#define USEC_TO_COUNT(x)	((x) * 133 / 16)
#define TICKS_PER_HZ		(TIMER_CLOCK / CONFIG_SYS_HZ)
#define TICKS_TO_HZ(x)		((x) / TICKS_PER_HZ)
#define TIMER_LOAD_VAL		0xffffffff

/*
 * MTU timer to use (from 0 to 3).
 */
#define MTU_TIMER 2

static struct u8500_mtu_timer *timer_base =
	&((struct u8500_mtu *)U8500_MTU0_BASE_V1)->pt[MTU_TIMER];

/* macro to read the 32 bit timer: since it decrements, we invert read value */
#define READ_TIMER() (~readl(&timer_base->cv))

/* Configure a free-running, auto-wrap counter with /16 prescaler */
int timer_init(void)
{
	writel(MTU_CR_ENA | MTU_CR_PRESCALE_16 | MTU_CR_32BITS,
	       &timer_base->cr);
	return 0;
}

ulong get_timer_masked(void)
{
	/* current tick value */
	ulong now = TICKS_TO_HZ(READ_TIMER());

	if (now >= gd->arch.lastinc) {	/* normal (non rollover) */
		gd->arch.tbl += (now - gd->arch.lastinc);
	} else {			/* rollover */
		gd->arch.tbl += (TICKS_TO_HZ(TIMER_LOAD_VAL) -
					gd->arch.lastinc) + now;
	}
	gd->arch.lastinc = now;
	return gd->arch.tbl;
}

/* Delay x useconds */
void __udelay(ulong usec)
{
	long tmo = usec * (TIMER_CLOCK / 1000) / 1000;
	ulong now, last = READ_TIMER();

	while (tmo > 0) {
		now = READ_TIMER();
		if (now > last)	/* normal (non rollover) */
			tmo -= now - last;
		else		/* rollover */
			tmo -= TIMER_LOAD_VAL - last + now;
		last = now;
	}
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/*
 * Emulation of Power architecture long long timebase.
 *
 * TODO: Support gd->arch.tbu for real long long timebase.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * Emulation of Power architecture timebase.
 * NB: Low resolution compared to Power tbclk.
 */
ulong get_tbclk(void)
{
	return CONFIG_SYS_HZ;
}

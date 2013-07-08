/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/hardware.h>

/* we always count down the max. */
#define TIMER_LOAD_VAL 0xffff

/* macro to read the 16 bit timer */
#define READ_TIMER (TCNTO1 & 0xffff)

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ NOT supported
#endif

static ulong timestamp;
static ulong lastdec;

int timer_init (void)
{
	TCFG0 = 0x000000E9;
	TCFG1 = 0x00000004;
	TCON = 0x00000900;
	TCNTB1 = TIMER_LOAD_VAL;
	TCMPB1 = 0;
	TCON = 0x00000B00;
	TCON = 0x00000900;


	lastdec = TCNTB1 = TIMER_LOAD_VAL;
	timestamp = 0;
	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

void __udelay (unsigned long usec)
{
	ulong tmo;

	tmo = usec / 1000;
	tmo *= CONFIG_SYS_HZ;
	tmo /= 8;

	tmo += get_timer (0);

	while (get_timer_masked () < tmo)
		/*NOP*/;
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
		tmo /= 8;
	} else {
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*8);
	}

	endtime = get_timer(0) + tmo;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

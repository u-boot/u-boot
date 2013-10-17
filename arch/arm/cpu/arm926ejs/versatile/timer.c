/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#define TIMER_LOAD_VAL 0xffffffff

/* macro to read the 32 bit timer */
#define READ_TIMER (*(volatile ulong *)(CONFIG_SYS_TIMERBASE+4))

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->arch.tbl
#define lastdec gd->arch.lastinc

#define TIMER_ENABLE	(1 << 7)
#define TIMER_MODE_MSK	(1 << 6)
#define TIMER_MODE_FR	(0 << 6)
#define TIMER_MODE_PD	(1 << 6)

#define TIMER_INT_EN	(1 << 5)
#define TIMER_PRS_MSK	(3 << 2)
#define TIMER_PRS_8S	(1 << 3)
#define TIMER_SIZE_MSK	(1 << 2)
#define TIMER_ONE_SHT	(1 << 0)

int timer_init (void)
{
	ulong	tmr_ctrl_val;

	/* 1st disable the Timer */
	tmr_ctrl_val = *(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8);
	tmr_ctrl_val &= ~TIMER_ENABLE;
	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = tmr_ctrl_val;

	/*
	 * The Timer Control Register has one Undefined/Shouldn't Use Bit
	 * So we should do read/modify/write Operation
	 */

	/*
	 * Timer Mode : Free Running
	 * Interrupt : Disabled
	 * Prescale : 8 Stage, Clk/256
	 * Tmr Siz : 16 Bit Counter
	 * Tmr in Wrapping Mode
	 */
	tmr_ctrl_val = *(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8);
	tmr_ctrl_val &= ~(TIMER_MODE_MSK | TIMER_INT_EN | TIMER_PRS_MSK | TIMER_SIZE_MSK | TIMER_ONE_SHT );
	tmr_ctrl_val |= (TIMER_ENABLE | TIMER_PRS_8S);

	*(volatile ulong *)(CONFIG_SYS_TIMERBASE + 8) = tmr_ctrl_val;

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay (unsigned long usec)
{
	ulong tmo, tmp;

	if(usec >= 1000){		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize for usec to ticks per sec */
		tmo *= CONFIG_SYS_HZ;	/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;		/* finish normalize. */
	}else{				/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	tmp = get_timer (0);		/* get current timestamp */
	if( (tmo + tmp + 1) < tmp )	/* if setting this fordward will roll time stamp */
		reset_timer_masked ();	/* reset "advancing" timestamp to 0, set lastdec value */
	else
		tmo += tmp;		/* else, set advancing stamp wake up time */

	while (get_timer_masked () < tmo)/* loop till event */
		/*NOP*/;
}

void reset_timer_masked (void)
{
	/* reset time */
	lastdec = READ_TIMER;  /* capure current decrementer value time */
	timestamp = 0;	       /* start "advancing" time stamp from 0 */
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;		/* current tick value */

	if (lastdec >= now) {		/* normal mode (non roll) */
		/* normal mode */
		timestamp += lastdec - now; /* move stamp fordward with absoulte diff ticks */
	} else {			/* we have overflow of the count down timer */
		/* nts = ts + ld + (TLV - now)
		 * ts=old stamp, ld=time that passed before passing through -1
		 * (TLV-now) amount of time after passing though -1
		 * nts = new "advancing time stamp"...it could also roll and cause problems.
		 */
		timestamp += lastdec + TIMER_LOAD_VAL - now;
	}
	lastdec = now;

	return timestamp;
}

/* waits specified delay value and resets timestamp */
void udelay_masked (unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize for usec to ticks per sec */
		tmo *= CONFIG_SYS_HZ;		/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;		/* finish normalize. */
	} else {			/* else small number, don't kill it prior to HZ multiply */
		tmo = usec * CONFIG_SYS_HZ;
		tmo /= (1000*1000);
	}

	endtime = get_timer_masked () + tmo;

	do {
		ulong now = get_timer_masked ();
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;

	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}

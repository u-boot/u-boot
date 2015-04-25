/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>

#define UBOOT_CNTR	0	/* counter to use for U-Boot timer */

/*
 * ARM Timers Registers Map
 */
#define CNTMR_CTRL_REG			&tmr_regs->ctrl
#define CNTMR_RELOAD_REG(tmrnum)	&tmr_regs->tmr[tmrnum].reload
#define CNTMR_VAL_REG(tmrnum)		&tmr_regs->tmr[tmrnum].val

/*
 * ARM Timers Control Register
 * CPU_TIMERS_CTRL_REG (CTCR)
 */
#define CTCR_ARM_TIMER_EN_OFFS(cntr)	(cntr * 2)
#define CTCR_ARM_TIMER_EN(cntr)		(1 << CTCR_ARM_TIMER_EN_OFFS(cntr))

#define CTCR_ARM_TIMER_AUTO_OFFS(cntr)	((cntr * 2) + 1)
#define CTCR_ARM_TIMER_AUTO_EN(cntr)	(1 << CTCR_ARM_TIMER_AUTO_OFFS(cntr))

/* Only Armada XP have the 25MHz enable bit (Kirkwood doesn't) */
#if defined(CONFIG_ARMADA_XP)
#define CTCR_ARM_TIMER_25MHZ_OFFS(cntr)	(cntr + 11)
#define CTCR_ARM_TIMER_25MHZ(cntr)	(1 << CTCR_ARM_TIMER_25MHZ_OFFS(cntr))
#else
#define CTCR_ARM_TIMER_25MHZ(cntr)	0
#endif

#define TIMER_LOAD_VAL 			0xffffffff

#define timestamp			gd->arch.tbl
#define lastdec				gd->arch.lastinc

/* Timer reload and current value registers */
struct kwtmr_val {
	u32 reload;	/* Timer reload reg */
	u32 val;	/* Timer value reg */
};

/* Timer registers */
struct kwtmr_registers {
	u32 ctrl;	/* Timer control reg */
	u32 pad[3];
	struct kwtmr_val tmr[4];
	u32 wdt_reload;
	u32 wdt_val;
};

DECLARE_GLOBAL_DATA_PTR;

static struct kwtmr_registers *tmr_regs =
	(struct kwtmr_registers *)MVEBU_TIMER_BASE;

static inline ulong read_timer(void)
{
	return readl(CNTMR_VAL_REG(UBOOT_CNTR))	/ (CONFIG_SYS_TCLK / 1000);
}

ulong get_timer_masked(void)
{
	ulong now = read_timer();

	if (lastdec >= now) {
		/* normal mode */
		timestamp += lastdec - now;
	} else {
		/* we have an overflow ... */
		timestamp += lastdec +
			(TIMER_LOAD_VAL / (CONFIG_SYS_TCLK / 1000)) - now;
	}
	lastdec = now;

	return timestamp;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

void __udelay(unsigned long usec)
{
	uint current;
	ulong delayticks;

	current = readl(CNTMR_VAL_REG(UBOOT_CNTR));
	delayticks = (usec * (CONFIG_SYS_TCLK / 1000000));

	if (current < delayticks) {
		delayticks -= current;
		while (readl(CNTMR_VAL_REG(UBOOT_CNTR)) < current) ;
		while ((TIMER_LOAD_VAL - delayticks) <
			readl(CNTMR_VAL_REG(UBOOT_CNTR))) ;
	} else {
		while (readl(CNTMR_VAL_REG(UBOOT_CNTR)) >
			(current - delayticks)) ;
	}
}

/*
 * init the counter
 */
int timer_init(void)
{
	/* load value into timer */
	writel(TIMER_LOAD_VAL, CNTMR_RELOAD_REG(UBOOT_CNTR));
	writel(TIMER_LOAD_VAL, CNTMR_VAL_REG(UBOOT_CNTR));

	/* enable timer in auto reload mode */
	clrsetbits_le32(CNTMR_CTRL_REG, CTCR_ARM_TIMER_25MHZ(UBOOT_CNTR),
			CTCR_ARM_TIMER_EN(UBOOT_CNTR) |
			CTCR_ARM_TIMER_AUTO_EN(UBOOT_CNTR));

	/* init the timestamp and lastdec value */
	lastdec = read_timer();
	timestamp = 0;

	return 0;
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
	return (ulong)CONFIG_SYS_HZ;
}

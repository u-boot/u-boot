/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/arch/kirkwood.h>

#define UBOOT_CNTR	0	/* counter to use for uboot timer */

/* Timer reload and current value registers */
struct kwtmr_val {
	u32 reload;	/* Timer reload reg */
	u32 val;	/* Timer value reg */
};

/* Timer registers */
struct kwtmr_registers {
	u32 ctrl;	/* Timer control reg */
	u32 pad[3];
	struct kwtmr_val tmr[2];
	u32 wdt_reload;
	u32 wdt_val;
};

struct kwtmr_registers *kwtmr_regs = (struct kwtmr_registers *)KW_TIMER_BASE;

/*
 * ARM Timers Registers Map
 */
#define CNTMR_CTRL_REG			&kwtmr_regs->ctrl
#define CNTMR_RELOAD_REG(tmrnum)	&kwtmr_regs->tmr[tmrnum].reload
#define CNTMR_VAL_REG(tmrnum)		&kwtmr_regs->tmr[tmrnum].val

/*
 * ARM Timers Control Register
 * CPU_TIMERS_CTRL_REG (CTCR)
 */
#define CTCR_ARM_TIMER_EN_OFFS(cntr)	(cntr * 2)
#define CTCR_ARM_TIMER_EN_MASK(cntr)	(1 << CTCR_ARM_TIMER_EN_OFFS)
#define CTCR_ARM_TIMER_EN(cntr)		(1 << CTCR_ARM_TIMER_EN_OFFS(cntr))
#define CTCR_ARM_TIMER_DIS(cntr)	(0 << CTCR_ARM_TIMER_EN_OFFS(cntr))

#define CTCR_ARM_TIMER_AUTO_OFFS(cntr)	((cntr * 2) + 1)
#define CTCR_ARM_TIMER_AUTO_MASK(cntr)	(1 << 1)
#define CTCR_ARM_TIMER_AUTO_EN(cntr)	(1 << CTCR_ARM_TIMER_AUTO_OFFS(cntr))
#define CTCR_ARM_TIMER_AUTO_DIS(cntr)	(0 << CTCR_ARM_TIMER_AUTO_OFFS(cntr))

/*
 * ARM Timer\Watchdog Reload Register
 * CNTMR_RELOAD_REG (TRR)
 */
#define TRG_ARM_TIMER_REL_OFFS		0
#define TRG_ARM_TIMER_REL_MASK		0xffffffff

/*
 * ARM Timer\Watchdog Register
 * CNTMR_VAL_REG (TVRG)
 */
#define TVR_ARM_TIMER_OFFS		0
#define TVR_ARM_TIMER_MASK		0xffffffff
#define TVR_ARM_TIMER_MAX		0xffffffff
#define TIMER_LOAD_VAL 			0xffffffff

#define READ_TIMER			(readl(CNTMR_VAL_REG(UBOOT_CNTR)) /	\
					 (CONFIG_SYS_TCLK / 1000))

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->tbl
#define lastdec gd->lastinc

void reset_timer_masked(void)
{
	/* reset time */
	lastdec = READ_TIMER;
	timestamp = 0;
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER;

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
	unsigned int cntmrctrl;

	/* load value into timer */
	writel(TIMER_LOAD_VAL, CNTMR_RELOAD_REG(UBOOT_CNTR));
	writel(TIMER_LOAD_VAL, CNTMR_VAL_REG(UBOOT_CNTR));

	/* enable timer in auto reload mode */
	cntmrctrl = readl(CNTMR_CTRL_REG);
	cntmrctrl |= CTCR_ARM_TIMER_EN(UBOOT_CNTR);
	cntmrctrl |= CTCR_ARM_TIMER_AUTO_EN(UBOOT_CNTR);
	writel(cntmrctrl, CNTMR_CTRL_REG);

	/* init the timestamp and lastdec value */
	reset_timer_masked();

	return 0;
}

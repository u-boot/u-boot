/*
 * (C) Copyright 2011
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Lei Wen <leiwen@marvell.com>
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
#include <asm/arch/pantheon.h>

/*
 * Timer registers
 * Refer 6.2.9 in Datasheet
 */
struct panthtmr_registers {
	u32 clk_ctrl;	/* Timer clk control reg */
	u32 match[9];	/* Timer match registers */
	u32 count[3];	/* Timer count registers */
	u32 status[3];
	u32 ie[3];
	u32 preload[3];	/* Timer preload value */
	u32 preload_ctrl[3];
	u32 wdt_match_en;
	u32 wdt_match_r;
	u32 wdt_val;
	u32 wdt_sts;
	u32 icr[3];
	u32 wdt_icr;
	u32 cer;	/* Timer count enable reg */
	u32 cmr;
	u32 ilr[3];
	u32 wcr;
	u32 wfar;
	u32 wsar;
	u32 cvwr[3];
};

#define TIMER			0	/* Use TIMER 0 */
/* Each timer has 3 match registers */
#define MATCH_CMP(x)		((3 * TIMER) + x)
#define TIMER_LOAD_VAL 		0xffffffff
#define	COUNT_RD_REQ		0x1

DECLARE_GLOBAL_DATA_PTR;
/* Using gd->tbu from timestamp and gd->tbl for lastdec */

/*
 * For preventing risk of instability in reading counter value,
 * first set read request to register cvwr and then read same
 * register after it captures counter value.
 */
ulong read_timer(void)
{
	struct panthtmr_registers *panthtimers =
		(struct panthtmr_registers *) PANTHEON_TIMER_BASE;
	volatile int loop=100;
	ulong val;

	writel(COUNT_RD_REQ, &panthtimers->cvwr);
	while (loop--)
		val = readl(&panthtimers->cvwr);

	/*
	 * This stop gcc complain and prevent loop mistake init to 0
	 */
	val = readl(&panthtimers->cvwr);

	return val;
}

void reset_timer_masked(void)
{
	/* reset time */
	gd->tbl = read_timer();
	gd->tbu = 0;
}

ulong get_timer_masked(void)
{
	ulong now = read_timer();

	if (now >= gd->tbl) {
		/* normal mode */
		gd->tbu += now - gd->tbl;
	} else {
		/* we have an overflow ... */
		gd->tbu += now + TIMER_LOAD_VAL - gd->tbl;
	}
	gd->tbl = now;

	return gd->tbu;
}

ulong get_timer(ulong base)
{
	return ((get_timer_masked() / (CONFIG_SYS_HZ_CLOCK / 1000)) -
		base);
}

void __udelay(unsigned long usec)
{
	ulong delayticks;
	ulong endtime;

	delayticks = (usec * (CONFIG_SYS_HZ_CLOCK / 1000000));
	endtime = get_timer_masked() + delayticks;

	while (get_timer_masked() < endtime)
		;
}

/*
 * init the Timer
 */
int timer_init(void)
{
	struct panthapb_registers *apb1clkres =
		(struct panthapb_registers *) PANTHEON_APBC_BASE;
	struct panthtmr_registers *panthtimers =
		(struct panthtmr_registers *) PANTHEON_TIMER_BASE;

	/* Enable Timer clock at 3.25 MHZ */
	writel(APBC_APBCLK | APBC_FNCLK | APBC_FNCLKSEL(3), &apb1clkres->timers);

	/* load value into timer */
	writel(0x0, &panthtimers->clk_ctrl);
	/* Use Timer 0 Match Resiger 0 */
	writel(TIMER_LOAD_VAL, &panthtimers->match[MATCH_CMP(0)]);
	/* Preload value is 0 */
	writel(0x0, &panthtimers->preload[TIMER]);
	/* Enable match comparator 0 for Timer 0 */
	writel(0x1, &panthtimers->preload_ctrl[TIMER]);

	/* Enable timer 0 */
	writel(0x1, &panthtimers->cer);
	/* init the gd->tbu and gd->tbl value */
	reset_timer_masked();

	return 0;
}

#define MPMU_APRR_WDTR	(1<<4)
#define TMR_WFAR	0xbaba	/* WDT Register First key */
#define TMP_WSAR	0xeb10	/* WDT Register Second key */

/*
 * This function uses internal Watchdog Timer
 * based reset mechanism.
 * Steps to write watchdog registers (protected access)
 * 1. Write key value to TMR_WFAR reg.
 * 2. Write key value to TMP_WSAR reg.
 * 3. Perform write operation.
 */
void reset_cpu (unsigned long ignored)
{
	struct panthmpmu_registers *mpmu =
		(struct panthmpmu_registers *) PANTHEON_MPMU_BASE;
	struct panthtmr_registers *panthtimers =
		(struct panthtmr_registers *) PANTHEON_WD_TIMER_BASE;
	u32 val;

	/* negate hardware reset to the WDT after system reset */
	val = readl(&mpmu->aprr);
	val = val | MPMU_APRR_WDTR;
	writel(val, &mpmu->aprr);

	/* reset/enable WDT clock */
	writel(APBC_APBCLK, &mpmu->wdtpcr);

	/* clear previous WDT status */
	writel(TMR_WFAR, &panthtimers->wfar);
	writel(TMP_WSAR, &panthtimers->wsar);
	writel(0, &panthtimers->wdt_sts);

	/* set match counter */
	writel(TMR_WFAR, &panthtimers->wfar);
	writel(TMP_WSAR, &panthtimers->wsar);
	writel(0xf, &panthtimers->wdt_match_r);

	/* enable WDT reset */
	writel(TMR_WFAR, &panthtimers->wfar);
	writel(TMP_WSAR, &panthtimers->wsar);
	writel(0x3, &panthtimers->wdt_match_en);

	/*enable functional WDT clock */
	writel(APBC_APBCLK | APBC_FNCLK, &mpmu->wdtpcr);
}

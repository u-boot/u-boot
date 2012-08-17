/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
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
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/spr_gpt.h>
#include <asm/arch/spr_misc.h>

#define GPT_RESOLUTION	(CONFIG_SPEAR_HZ_CLOCK / CONFIG_SPEAR_HZ)
#define READ_TIMER()	(readl(&gpt_regs_p->count) & GPT_FREE_RUNNING)

static struct gpt_regs *const gpt_regs_p =
    (struct gpt_regs *)CONFIG_SPEAR_TIMERBASE;

static struct misc_regs *const misc_regs_p =
    (struct misc_regs *)CONFIG_SPEAR_MISCBASE;

DECLARE_GLOBAL_DATA_PTR;

#define timestamp gd->tbl
#define lastdec gd->lastinc

int timer_init(void)
{
	u32 synth;

	/* Prescaler setting */
#if defined(CONFIG_SPEAR3XX)
	writel(MISC_PRSC_CFG, &misc_regs_p->prsc2_clk_cfg);
	synth = MISC_GPT4SYNTH;
#elif defined(CONFIG_SPEAR600)
	writel(MISC_PRSC_CFG, &misc_regs_p->prsc1_clk_cfg);
	synth = MISC_GPT3SYNTH;
#else
# error Incorrect config. Can only be spear{600|300|310|320}
#endif

	writel(readl(&misc_regs_p->periph_clk_cfg) | synth,
	       &misc_regs_p->periph_clk_cfg);

	/* disable timers */
	writel(GPT_PRESCALER_1 | GPT_MODE_AUTO_RELOAD, &gpt_regs_p->control);

	/* load value for free running */
	writel(GPT_FREE_RUNNING, &gpt_regs_p->compare);

	/* auto reload, start timer */
	writel(readl(&gpt_regs_p->control) | GPT_ENABLE, &gpt_regs_p->control);

	/* Reset the timer */
	lastdec = READ_TIMER();
	timestamp = 0;

	return 0;
}

/*
 * timer without interrupts
 */
ulong get_timer(ulong base)
{
	return (get_timer_masked() / GPT_RESOLUTION) - base;
}

void __udelay(unsigned long usec)
{
	ulong tmo;
	ulong start = get_timer_masked();
	ulong tenudelcnt = CONFIG_SPEAR_HZ_CLOCK / (1000 * 100);
	ulong rndoff;

	rndoff = (usec % 10) ? 1 : 0;

	/* tenudelcnt timer tick gives 10 microsecconds delay */
	tmo = ((usec / 10) + rndoff) * tenudelcnt;

	while ((ulong) (get_timer_masked() - start) < tmo)
		;
}

ulong get_timer_masked(void)
{
	ulong now = READ_TIMER();

	if (now >= lastdec) {
		/* normal mode */
		timestamp += now - lastdec;
	} else {
		/* we have an overflow ... */
		timestamp += now + GPT_FREE_RUNNING - lastdec;
	}
	lastdec = now;

	return timestamp;
}

void udelay_masked(unsigned long usec)
{
	return udelay(usec);
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
ulong get_tbclk(void)
{
	return CONFIG_SPEAR_HZ;
}

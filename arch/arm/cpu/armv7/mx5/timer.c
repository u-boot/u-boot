/*
 * (C) Copyright 2007
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
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
#include <asm/arch/imx-regs.h>

/* General purpose timers registers */
struct mxc_gpt {
	unsigned int control;
	unsigned int prescaler;
	unsigned int status;
	unsigned int nouse[6];
	unsigned int counter;
};

static struct mxc_gpt *cur_gpt = (struct mxc_gpt *)GPT1_BASE_ADDR;

/* General purpose timers bitfields */
#define GPTCR_SWR       (1<<15)	/* Software reset */
#define GPTCR_FRR       (1<<9)	/* Freerun / restart */
#define GPTCR_CLKSOURCE_32 (4<<6)	/* Clock source */
#define GPTCR_TEN       (1)	/* Timer enable */

DECLARE_GLOBAL_DATA_PTR;

#define timestamp (gd->tbl)
#define lastinc (gd->lastinc)

int timer_init(void)
{
	int i;

	/* setup GP Timer 1 */
	__raw_writel(GPTCR_SWR, &cur_gpt->control);

	/* We have no udelay by now */
	for (i = 0; i < 100; i++)
		__raw_writel(0, &cur_gpt->control);

	__raw_writel(0, &cur_gpt->prescaler); /* 32Khz */

	/* Freerun Mode, PERCLK1 input */
	i = __raw_readl(&cur_gpt->control);
	__raw_writel(i | GPTCR_CLKSOURCE_32 | GPTCR_TEN, &cur_gpt->control);
	reset_timer_masked();
	return 0;
}

void reset_timer(void)
{
	reset_timer_masked();
}

void reset_timer_masked(void)
{
	ulong val = __raw_readl(&cur_gpt->counter);
	lastinc = val / (CONFIG_SYS_MX5_CLK32 / CONFIG_SYS_HZ);
	timestamp = 0;
}

ulong get_timer_masked(void)
{
	ulong val = __raw_readl(&cur_gpt->counter);
	val /= (CONFIG_SYS_MX5_CLK32 / CONFIG_SYS_HZ);
	if (val >= lastinc)
		timestamp += (val - lastinc);
	else
		timestamp += ((0xFFFFFFFF / (CONFIG_SYS_MX5_CLK32 / CONFIG_SYS_HZ))
				- lastinc) + val;
	lastinc = val;
	return timestamp;
}

ulong get_timer(ulong base)
{
	return get_timer_masked() - base;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay(unsigned long usec)
{
	unsigned long now, start, tmo;
	tmo = usec * (CONFIG_SYS_MX5_CLK32 / 1000) / 1000;

	if (!tmo)
		tmo = 1;

	now = start = readl(&cur_gpt->counter);

	while ((now - start) < tmo)
		now = readl(&cur_gpt->counter);

}

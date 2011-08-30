/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* Tegra2 Clock control functions */

#include <asm/io.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include <asm/arch/tegra2.h>
#include <common.h>

#ifdef DEBUG
#define assert(x)	\
	({ if (!(x)) printf("Assertion failure '%s' %s line %d\n", \
		#x, __FILE__, __LINE__); })
#else
#define assert(x)
#endif

/*
 * Get the oscillator frequency, from the corresponding hardware configuration
 * field.
 */
enum clock_osc_freq clock_get_osc_freq(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	return (reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
}

unsigned long clock_start_pll(enum clock_pll_id clkid, u32 divm, u32 divn,
		u32 divp, u32 cpcon, u32 lfcon)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 data;
	struct clk_pll *pll;

	assert(clock_pll_id_isvalid(clkid));
	pll = &clkrst->crc_pll[clkid];

	/*
	 * We cheat by treating all PLL (except PLLU) in the same fashion.
	 * This works only because:
	 * - same fields are always mapped at same offsets, except DCCON
	 * - DCCON is always 0, doesn't conflict
	 * - M,N, P of PLLP values are ignored for PLLP
	 */
	data = (cpcon << PLL_CPCON_SHIFT) | (lfcon << PLL_LFCON_SHIFT);
	writel(data, &pll->pll_misc);

	data = (divm << PLL_DIVM_SHIFT) | (divn << PLL_DIVN_SHIFT) |
			(0 << PLL_BYPASS_SHIFT) | (1 << PLL_ENABLE_SHIFT);

	if (clkid == CLOCK_PLL_ID_USB)
		data |= divp << PLLU_VCO_FREQ_SHIFT;
	else
		data |= divp << PLL_DIVP_SHIFT;
	writel(data, &pll->pll_base);

	/* calculate the stable time */
	return timer_get_us() + CLOCK_PLL_STABLE_DELAY_US;
}

void clock_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *clk = &clkrst->crc_clk_out_enb[PERIPH_REG(periph_id)];
	u32 reg;

	/* Enable/disable the clock to this peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	reg = readl(clk);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, clk);
}

void clock_enable(enum periph_id clkid)
{
	clock_set_enable(clkid, 1);
}

void clock_disable(enum periph_id clkid)
{
	clock_set_enable(clkid, 0);
}

void reset_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *reset = &clkrst->crc_rst_dev[PERIPH_REG(periph_id)];
	u32 reg;

	/* Enable/disable reset to the peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	reg = readl(reset);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, reset);
}

void reset_periph(enum periph_id periph_id, int us_delay)
{
	/* Put peripheral into reset */
	reset_set_enable(periph_id, 1);
	udelay(us_delay);

	/* Remove reset */
	reset_set_enable(periph_id, 0);

	udelay(us_delay);
}

void reset_cmplx_set_enable(int cpu, int which, int reset)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 mask;

	/* Form the mask, which depends on the cpu chosen. Tegra2 has 2 */
	assert(cpu >= 0 && cpu < 2);
	mask = which << cpu;

	/* either enable or disable those reset for that CPU */
	if (reset)
		writel(mask, &clkrst->crc_cpu_cmplx_set);
	else
		writel(mask, &clkrst->crc_cpu_cmplx_clr);
}

/*
 * Copyright (c) 2010-2014, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/scu.h>
#include "cpu.h"

int get_num_cpus(void)
{
	struct apb_misc_gp_ctlr *gp;
	uint rev;

	gp = (struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;
	rev = (readl(&gp->hidrev) & HIDREV_CHIPID_MASK) >> HIDREV_CHIPID_SHIFT;

	switch (rev) {
	case CHIPID_TEGRA20:
		return 2;
		break;
	case CHIPID_TEGRA30:
	case CHIPID_TEGRA114:
	default:
		return 4;
		break;
	}
}

/*
 * Timing tables for each SOC for all four oscillator options.
 */
struct clk_pll_table tegra_pll_x_table[TEGRA_SOC_CNT][CLOCK_OSC_FREQ_COUNT] = {
	/*
	 * T20: 1 GHz
	 *
	 * Register   Field  Bits   Width
	 * ------------------------------
	 * PLLX_BASE  p      22:20    3
	 * PLLX_BASE  n      17: 8   10
	 * PLLX_BASE  m       4: 0    5
	 * PLLX_MISC  cpcon  11: 8    4
	 */
	{
		{ .n = 1000, .m = 13, .p = 0, .cpcon = 12 }, /* OSC: 13.0 MHz */
		{ .n =  625, .m = 12, .p = 0, .cpcon =  8 }, /* OSC: 19.2 MHz */
		{ .n = 1000, .m = 12, .p = 0, .cpcon = 12 }, /* OSC: 12.0 MHz */
		{ .n = 1000, .m = 26, .p = 0, .cpcon = 12 }, /* OSC: 26.0 MHz */
	},
	/*
	 * T25: 1.2 GHz
	 *
	 * Register   Field  Bits   Width
	 * ------------------------------
	 * PLLX_BASE  p      22:20    3
	 * PLLX_BASE  n      17: 8   10
	 * PLLX_BASE  m       4: 0    5
	 * PLLX_MISC  cpcon  11: 8    4
	 */
	{
		{ .n = 923, .m = 10, .p = 0, .cpcon = 12 }, /* OSC: 13.0 MHz */
		{ .n = 750, .m = 12, .p = 0, .cpcon =  8 }, /* OSC: 19.2 MHz */
		{ .n = 600, .m =  6, .p = 0, .cpcon = 12 }, /* OSC: 12.0 MHz */
		{ .n = 600, .m = 13, .p = 0, .cpcon = 12 }, /* OSC: 26.0 MHz */
	},
	/*
	 * T30: 1.4 GHz
	 *
	 * Register   Field  Bits   Width
	 * ------------------------------
	 * PLLX_BASE  p      22:20    3
	 * PLLX_BASE  n      17: 8   10
	 * PLLX_BASE  m       4: 0    5
	 * PLLX_MISC  cpcon  11: 8    4
	 */
	{
		{ .n = 862, .m =  8, .p = 0, .cpcon = 8 }, /* OSC: 13.0 MHz */
		{ .n = 583, .m =  8, .p = 0, .cpcon = 4 }, /* OSC: 19.2 MHz */
		{ .n = 700, .m =  6, .p = 0, .cpcon = 8 }, /* OSC: 12.0 MHz */
		{ .n = 700, .m = 13, .p = 0, .cpcon = 8 }, /* OSC: 26.0 MHz */
	},
	/*
	 * T114: 700 MHz
	 *
	 * Register   Field  Bits   Width
	 * ------------------------------
	 * PLLX_BASE  p      23:20    4
	 * PLLX_BASE  n      15: 8    8
	 * PLLX_BASE  m       7: 0    8
	 */
	{
		{ .n = 108, .m = 1, .p = 1 }, /* OSC: 13.0 MHz */
		{ .n =  73, .m = 1, .p = 1 }, /* OSC: 19.2 MHz */
		{ .n = 116, .m = 1, .p = 1 }, /* OSC: 12.0 MHz */
		{ .n = 108, .m = 2, .p = 1 }, /* OSC: 26.0 MHz */
	},

	/*
	 * T124: 700 MHz
	 *
	 * Register   Field  Bits   Width
	 * ------------------------------
	 * PLLX_BASE  p      23:20    4
	 * PLLX_BASE  n      15: 8    8
	 * PLLX_BASE  m       7: 0    8
	 */
	{
		{ .n = 108, .m = 1, .p = 1 }, /* OSC: 13.0 MHz */
		{ .n =  73, .m = 1, .p = 1 }, /* OSC: 19.2 MHz */
		{ .n = 116, .m = 1, .p = 1 }, /* OSC: 12.0 MHz */
		{ .n = 108, .m = 2, .p = 1 }, /* OSC: 26.0 MHz */
	},
};

static inline void pllx_set_iddq(void)
{
#if defined(CONFIG_TEGRA124)
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	/* Disable IDDQ */
	reg = readl(&clkrst->crc_pllx_misc3);
	reg &= ~PLLX_IDDQ_MASK;
	writel(reg, &clkrst->crc_pllx_misc3);
	udelay(2);
	debug("%s: IDDQ: PLLX IDDQ = 0x%08X\n", __func__,
	      readl(&clkrst->crc_pllx_misc3));
#endif
}

int pllx_set_rate(struct clk_pll_simple *pll , u32 divn, u32 divm,
		u32 divp, u32 cpcon)
{
	int chip = tegra_get_chip();
	u32 reg;

	/* If PLLX is already enabled, just return */
	if (readl(&pll->pll_base) & PLL_ENABLE_MASK) {
		debug("pllx_set_rate: PLLX already enabled, returning\n");
		return 0;
	}

	debug(" pllx_set_rate entry\n");

	pllx_set_iddq();

	/* Set BYPASS, m, n and p to PLLX_BASE */
	reg = PLL_BYPASS_MASK | (divm << PLL_DIVM_SHIFT);
	reg |= ((divn << PLL_DIVN_SHIFT) | (divp << PLL_DIVP_SHIFT));
	writel(reg, &pll->pll_base);

	/* Set cpcon to PLLX_MISC */
	if (chip == CHIPID_TEGRA20 || chip == CHIPID_TEGRA30)
		reg = (cpcon << PLL_CPCON_SHIFT);
	else
		reg = 0;

	/* Set dccon to PLLX_MISC if freq > 600MHz */
	if (divn > 600)
		reg |= (1 << PLL_DCCON_SHIFT);
	writel(reg, &pll->pll_misc);

	/* Disable BYPASS */
	reg = readl(&pll->pll_base);
	reg &= ~PLL_BYPASS_MASK;
	writel(reg, &pll->pll_base);
	debug("pllx_set_rate: base = 0x%08X\n", reg);

	/* Set lock_enable to PLLX_MISC */
	reg = readl(&pll->pll_misc);
	reg |= PLL_LOCK_ENABLE_MASK;
	writel(reg, &pll->pll_misc);
	debug("pllx_set_rate: misc = 0x%08X\n", reg);

	/* Enable PLLX last, once it's all configured */
	reg = readl(&pll->pll_base);
	reg |= PLL_ENABLE_MASK;
	writel(reg, &pll->pll_base);
	debug("pllx_set_rate: base final = 0x%08X\n", reg);

	return 0;
}

void init_pllx(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct clk_pll_simple *pll = &clkrst->crc_pll_simple[SIMPLE_PLLX];
	int soc_type, sku_info, chip_sku;
	enum clock_osc_freq osc;
	struct clk_pll_table *sel;

	debug("init_pllx entry\n");

	/* get SOC (chip) type */
	soc_type = tegra_get_chip();
	debug(" init_pllx: SoC = 0x%02X\n", soc_type);

	/* get SKU info */
	sku_info = tegra_get_sku_info();
	debug(" init_pllx: SKU info byte = 0x%02X\n", sku_info);

	/* get chip SKU, combo of the above info */
	chip_sku = tegra_get_chip_sku();
	debug(" init_pllx: Chip SKU = %d\n", chip_sku);

	/* get osc freq */
	osc = clock_get_osc_freq();
	debug(" init_pllx: osc = %d\n", osc);

	/* set pllx */
	sel = &tegra_pll_x_table[chip_sku][osc];
	pllx_set_rate(pll, sel->n, sel->m, sel->p, sel->cpcon);
}

void enable_cpu_clock(int enable)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 clk;

	/*
	 * NOTE:
	 * Regardless of whether the request is to enable or disable the CPU
	 * clock, every processor in the CPU complex except the master (CPU 0)
	 * will have it's clock stopped because the AVP only talks to the
	 * master.
	 */

	if (enable) {
		/* Initialize PLLX */
		init_pllx();

		/* Wait until all clocks are stable */
		udelay(PLL_STABILIZATION_DELAY);

		writel(CCLK_BURST_POLICY, &clkrst->crc_cclk_brst_pol);
		writel(SUPER_CCLK_DIVIDER, &clkrst->crc_super_cclk_div);
	}

	/*
	 * Read the register containing the individual CPU clock enables and
	 * always stop the clocks to CPUs > 0.
	 */
	clk = readl(&clkrst->crc_clk_cpu_cmplx);
	clk |= 1 << CPU1_CLK_STP_SHIFT;
	if (get_num_cpus() == 4)
		clk |= (1 << CPU2_CLK_STP_SHIFT) + (1 << CPU3_CLK_STP_SHIFT);

	/* Stop/Unstop the CPU clock */
	clk &= ~CPU0_CLK_STP_MASK;
	clk |= !enable << CPU0_CLK_STP_SHIFT;
	writel(clk, &clkrst->crc_clk_cpu_cmplx);

	clock_enable(PERIPH_ID_CPU);
}

static int is_cpu_powered(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	return (readl(&pmc->pmc_pwrgate_status) & CPU_PWRED) ? 1 : 0;
}

static void remove_cpu_io_clamps(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	/* Remove the clamps on the CPU I/O signals */
	reg = readl(&pmc->pmc_remove_clamping);
	reg |= CPU_CLMP;
	writel(reg, &pmc->pmc_remove_clamping);

	/* Give I/O signals time to stabilize */
	udelay(IO_STABILIZATION_DELAY);
}

void powerup_cpu(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;
	int timeout = IO_STABILIZATION_DELAY;

	if (!is_cpu_powered()) {
		/* Toggle the CPU power state (OFF -> ON) */
		reg = readl(&pmc->pmc_pwrgate_toggle);
		reg &= PARTID_CP;
		reg |= START_CP;
		writel(reg, &pmc->pmc_pwrgate_toggle);

		/* Wait for the power to come up */
		while (!is_cpu_powered()) {
			if (timeout-- == 0)
				printf("CPU failed to power up!\n");
			else
				udelay(10);
		}

		/*
		 * Remove the I/O clamps from CPU power partition.
		 * Recommended only on a Warm boot, if the CPU partition gets
		 * power gated. Shouldn't cause any harm when called after a
		 * cold boot according to HW, probably just redundant.
		 */
		remove_cpu_io_clamps();
	}
}

void reset_A9_cpu(int reset)
{
	/*
	* NOTE:  Regardless of whether the request is to hold the CPU in reset
	*        or take it out of reset, every processor in the CPU complex
	*        except the master (CPU 0) will be held in reset because the
	*        AVP only talks to the master. The AVP does not know that there
	*        are multiple processors in the CPU complex.
	*/
	int mask = crc_rst_cpu | crc_rst_de | crc_rst_debug;
	int num_cpus = get_num_cpus();
	int cpu;

	debug("reset_a9_cpu entry\n");
	/* Hold CPUs 1 onwards in reset, and CPU 0 if asked */
	for (cpu = 1; cpu < num_cpus; cpu++)
		reset_cmplx_set_enable(cpu, mask, 1);
	reset_cmplx_set_enable(0, mask, reset);

	/* Enable/Disable master CPU reset */
	reset_set_enable(PERIPH_ID_CPU, reset);
}

void clock_enable_coresight(int enable)
{
	u32 rst, src = 2;

	debug("clock_enable_coresight entry\n");
	clock_set_enable(PERIPH_ID_CORESIGHT, enable);
	reset_set_enable(PERIPH_ID_CORESIGHT, !enable);

	if (enable) {
		/*
		 * Put CoreSight on PLLP_OUT0 and divide it down as per
		 * PLLP base frequency based on SoC type (T20/T30+).
		 * Clock divider request would setup CSITE clock as 144MHz
		 * for PLLP base 216MHz and 204MHz for PLLP base 408MHz
		 */
		src = CLK_DIVIDER(NVBL_PLLP_KHZ, CSITE_KHZ);
		clock_ll_set_source_divisor(PERIPH_ID_CSI, 0, src);

		/* Unlock the CPU CoreSight interfaces */
		rst = CORESIGHT_UNLOCK;
		writel(rst, CSITE_CPU_DBG0_LAR);
		writel(rst, CSITE_CPU_DBG1_LAR);
		if (get_num_cpus() == 4) {
			writel(rst, CSITE_CPU_DBG2_LAR);
			writel(rst, CSITE_CPU_DBG3_LAR);
		}
	}
}

void halt_avp(void)
{
	for (;;) {
		writel(HALT_COP_EVENT_JTAG | (FLOW_MODE_STOP << 29),
		       FLOW_CTLR_HALT_COP_EVENTS);
	}
}

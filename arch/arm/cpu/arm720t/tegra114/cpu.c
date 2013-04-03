/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
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
#include <asm/arch/flow.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include "../tegra-common/cpu.h"

/* Tegra114-specific CPU init code */
static void enable_cpu_power_rail(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	debug("enable_cpu_power_rail entry\n");

	/* un-tristate PWR_I2C SCL/SDA, rest of the defaults are correct */
	pinmux_tristate_disable(PINGRP_PWR_I2C_SCL);
	pinmux_tristate_disable(PINGRP_PWR_I2C_SDA);

	/*
	 * Set CPUPWRGOOD_TIMER - APB clock is 1/2 of SCLK (102MHz),
	 * set it for 25ms (102MHz * .025)
	 */
	reg = 0x26E8F0;
	writel(reg, &pmc->pmc_cpupwrgood_timer);

	/* Set polarity to 0 (normal) and enable CPUPWRREQ_OE */
	clrbits_le32(&pmc->pmc_cntrl, CPUPWRREQ_POL);
	setbits_le32(&pmc->pmc_cntrl, CPUPWRREQ_OE);

	/*
	 * Set CLK_RST_CONTROLLER_CPU_SOFTRST_CTRL2_0_CAR2PMC_CPU_ACK_WIDTH
	 * to 408 to satisfy the requirement of having at least 16 CPU clock
	 * cycles before clamp removal.
	 */

	clrbits_le32(&clkrst->crc_cpu_softrst_ctrl2, 0xFFF);
	setbits_le32(&clkrst->crc_cpu_softrst_ctrl2, 408);
}

static void enable_cpu_clocks(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	debug("enable_cpu_clocks entry\n");

	/* Wait for PLL-X to lock */
	do {
		reg = readl(&clkrst->crc_pll_simple[SIMPLE_PLLX].pll_base);
	} while ((reg & (1 << 27)) == 0);

	/* Wait until all clocks are stable */
	udelay(PLL_STABILIZATION_DELAY);

	writel(CCLK_BURST_POLICY, &clkrst->crc_cclk_brst_pol);
	writel(SUPER_CCLK_DIVIDER, &clkrst->crc_super_cclk_div);

	/* Always enable the main CPU complex clocks */
	clock_enable(PERIPH_ID_CPU);
	clock_enable(PERIPH_ID_CPULP);
	clock_enable(PERIPH_ID_CPUG);
}

static void remove_cpu_resets(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	debug("remove_cpu_resets entry\n");
	/* Take the slow non-CPU partition out of reset */
	reg = readl(&clkrst->crc_rst_cpulp_cmplx_clr);
	writel((reg | CLR_NONCPURESET), &clkrst->crc_rst_cpulp_cmplx_clr);

	/* Take the fast non-CPU partition out of reset */
	reg = readl(&clkrst->crc_rst_cpug_cmplx_clr);
	writel((reg | CLR_NONCPURESET), &clkrst->crc_rst_cpug_cmplx_clr);

	/* Clear the SW-controlled reset of the slow cluster */
	reg = readl(&clkrst->crc_rst_cpulp_cmplx_clr);
	reg |= (CLR_CPURESET0+CLR_DBGRESET0+CLR_CORERESET0+CLR_CXRESET0);
	writel(reg, &clkrst->crc_rst_cpulp_cmplx_clr);

	/* Clear the SW-controlled reset of the fast cluster */
	reg = readl(&clkrst->crc_rst_cpug_cmplx_clr);
	reg |= (CLR_CPURESET0+CLR_DBGRESET0+CLR_CORERESET0+CLR_CXRESET0);
	reg |= (CLR_CPURESET1+CLR_DBGRESET1+CLR_CORERESET1+CLR_CXRESET1);
	reg |= (CLR_CPURESET2+CLR_DBGRESET2+CLR_CORERESET2+CLR_CXRESET2);
	reg |= (CLR_CPURESET3+CLR_DBGRESET3+CLR_CORERESET3+CLR_CXRESET3);
	writel(reg, &clkrst->crc_rst_cpug_cmplx_clr);
}

/**
 * The T114 requires some special clock initialization, including setting up
 * the DVC I2C, turning on MSELECT and selecting the G CPU cluster
 */
void t114_init_clocks(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct flow_ctlr *flow = (struct flow_ctlr *)NV_PA_FLOW_BASE;
	u32 val;

	debug("t114_init_clocks entry\n");

	/* Set active CPU cluster to G */
	clrbits_le32(&flow->cluster_control, 1);

	/*
	 * Switch system clock to PLLP_OUT4 (108 MHz), AVP will now run
	 * at 108 MHz. This is glitch free as only the source is changed, no
	 * special precaution needed.
	 */
	val = (SCLK_SOURCE_PLLP_OUT4 << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT) |
		(SCLK_SOURCE_PLLP_OUT4 << SCLK_SWAKEUP_IRQ_SOURCE_SHIFT) |
		(SCLK_SOURCE_PLLP_OUT4 << SCLK_SWAKEUP_RUN_SOURCE_SHIFT) |
		(SCLK_SOURCE_PLLP_OUT4 << SCLK_SWAKEUP_IDLE_SOURCE_SHIFT) |
		(SCLK_SYS_STATE_RUN << SCLK_SYS_STATE_SHIFT);
	writel(val, &clkrst->crc_sclk_brst_pol);

	writel(SUPER_SCLK_ENB_MASK, &clkrst->crc_super_sclk_div);

	debug("Setting up PLLX\n");
	init_pllx();

	val = (1 << CLK_SYS_RATE_AHB_RATE_SHIFT);
	writel(val, &clkrst->crc_clk_sys_rate);

	/* Enable clocks to required peripherals. TBD - minimize this list */
	debug("Enabling clocks\n");

	clock_set_enable(PERIPH_ID_CACHE2, 1);
	clock_set_enable(PERIPH_ID_GPIO, 1);
	clock_set_enable(PERIPH_ID_TMR, 1);
	clock_set_enable(PERIPH_ID_RTC, 1);
	clock_set_enable(PERIPH_ID_CPU, 1);
	clock_set_enable(PERIPH_ID_EMC, 1);
	clock_set_enable(PERIPH_ID_I2C5, 1);
	clock_set_enable(PERIPH_ID_FUSE, 1);
	clock_set_enable(PERIPH_ID_PMC, 1);
	clock_set_enable(PERIPH_ID_APBDMA, 1);
	clock_set_enable(PERIPH_ID_MEM, 1);
	clock_set_enable(PERIPH_ID_IRAMA, 1);
	clock_set_enable(PERIPH_ID_IRAMB, 1);
	clock_set_enable(PERIPH_ID_IRAMC, 1);
	clock_set_enable(PERIPH_ID_IRAMD, 1);
	clock_set_enable(PERIPH_ID_CORESIGHT, 1);
	clock_set_enable(PERIPH_ID_MSELECT, 1);
	clock_set_enable(PERIPH_ID_EMC1, 1);
	clock_set_enable(PERIPH_ID_MC1, 1);
	clock_set_enable(PERIPH_ID_DVFS, 1);

	/*
	 * Set MSELECT clock source as PLLP (00), and ask for a clock
	 * divider that would set the MSELECT clock at 102MHz for a
	 * PLLP base of 408MHz.
	 */
	clock_ll_set_source_divisor(PERIPH_ID_MSELECT, 0,
		CLK_DIVIDER(NVBL_PLLP_KHZ, 102000));

	/* I2C5 (DVC) gets CLK_M and a divisor of 17 */
	clock_ll_set_source_divisor(PERIPH_ID_I2C5, 3, 16);

	/* Give clocks time to stabilize */
	udelay(1000);

	/* Take required peripherals out of reset */
	debug("Taking periphs out of reset\n");
	reset_set_enable(PERIPH_ID_CACHE2, 0);
	reset_set_enable(PERIPH_ID_GPIO, 0);
	reset_set_enable(PERIPH_ID_TMR, 0);
	reset_set_enable(PERIPH_ID_COP, 0);
	reset_set_enable(PERIPH_ID_EMC, 0);
	reset_set_enable(PERIPH_ID_I2C5, 0);
	reset_set_enable(PERIPH_ID_FUSE, 0);
	reset_set_enable(PERIPH_ID_APBDMA, 0);
	reset_set_enable(PERIPH_ID_MEM, 0);
	reset_set_enable(PERIPH_ID_CORESIGHT, 0);
	reset_set_enable(PERIPH_ID_MSELECT, 0);
	reset_set_enable(PERIPH_ID_EMC1, 0);
	reset_set_enable(PERIPH_ID_MC1, 0);
	reset_set_enable(PERIPH_ID_DVFS, 0);

	debug("t114_init_clocks exit\n");
}

static int is_partition_powered(u32 mask)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	/* Get power gate status */
	reg = readl(&pmc->pmc_pwrgate_status);
	return (reg & mask) == mask;
}

static int is_clamp_enabled(u32 mask)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	/* Get clamp status. TODO: Add pmc_clamp_status alias to pmc.h */
	reg = readl(&pmc->pmc_pwrgate_timer_on);
	return (reg & mask) == mask;
}

static void power_partition(u32 status, u32 partid)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;

	debug("%s: status = %08X, part ID = %08X\n", __func__, status, partid);
	/* Is the partition already on? */
	if (!is_partition_powered(status)) {
		/* No, toggle the partition power state (OFF -> ON) */
		debug("power_partition, toggling state\n");
		clrbits_le32(&pmc->pmc_pwrgate_toggle, 0x1F);
		setbits_le32(&pmc->pmc_pwrgate_toggle, partid);
		setbits_le32(&pmc->pmc_pwrgate_toggle, START_CP);

		/* Wait for the power to come up */
		while (!is_partition_powered(status))
			;

		/* Wait for the clamp status to be cleared */
		while (is_clamp_enabled(status))
			;

		/* Give I/O signals time to stabilize */
		udelay(IO_STABILIZATION_DELAY);
	}
}

void powerup_cpus(void)
{
	debug("powerup_cpus entry\n");

	/* We boot to the fast cluster */
	debug("powerup_cpus entry: G cluster\n");
	/* Power up the fast cluster rail partition */
	power_partition(CRAIL, CRAILID);

	/* Power up the fast cluster non-CPU partition */
	power_partition(C0NC, C0NCID);

	/* Power up the fast cluster CPU0 partition */
	power_partition(CE0, CE0ID);
}

void start_cpu(u32 reset_vector)
{
	u32 imme, inst;

	debug("start_cpu entry, reset_vector = %x\n", reset_vector);

	t114_init_clocks();

	/* Enable VDD_CPU */
	enable_cpu_power_rail();

	/* Get the CPU(s) running */
	enable_cpu_clocks();

	/* Enable CoreSight */
	clock_enable_coresight(1);

	/* Take CPU(s) out of reset */
	remove_cpu_resets();

	/* Set the entry point for CPU execution from reset */

	/*
	 * A01P with patched boot ROM; vector hard-coded to 0x4003fffc.
	 * See nvbug 1193357 for details.
	 */

	/* mov r0, #lsb(reset_vector) */
	imme = reset_vector & 0xffff;
	inst = imme & 0xfff;
	inst |= ((imme >> 12) << 16);
	inst |= 0xe3000000;
	writel(inst, 0x4003fff0);

	/* movt r0, #msb(reset_vector) */
	imme = (reset_vector >> 16) & 0xffff;
	inst = imme & 0xfff;
	inst |= ((imme >> 12) << 16);
	inst |= 0xe3400000;
	writel(inst, 0x4003fff4);

	/* bx r0 */
	writel(0xe12fff10, 0x4003fff8);

	/* b -12 */
	imme = (u32)-20;
	inst = (imme >> 2) & 0xffffff;
	inst |= 0xea000000;
	writel(inst, 0x4003fffc);

	/* Write to orignal location for compatibility */
	writel(reset_vector, EXCEP_VECTOR_CPU_RESET_VECTOR);

	/* If the CPU(s) don't already have power, power 'em up */
	powerup_cpus();
}

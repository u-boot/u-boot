/*
* (C) Copyright 2010-2011
* NVIDIA Corporation <www.nvidia.com>
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

#include "ap20.h"
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/pmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/scu.h>
#include <common.h>

u32 s_first_boot = 1;

void init_pllx(void)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct clk_pll *pll = &clkrst->crc_pll[CLOCK_PLL_ID_XCPU];
	u32 reg;

	/* If PLLX is already enabled, just return */
	reg = readl(&pll->pll_base);
	if (reg & PLL_ENABLE)
		return;

	/* Set PLLX_MISC */
	reg = CPCON;				/* CPCON[11:8]  = 0001 */
	writel(reg, &pll->pll_misc);

	/* Use 12MHz clock here */
	reg = (PLL_BYPASS | PLL_DIVM_VALUE);
	reg |= (1000 << 8);			/* DIVN = 0x3E8 */
	writel(reg, &pll->pll_base);

	reg |= PLL_ENABLE;
	writel(reg, &pll->pll_base);

	reg &= ~PLL_BYPASS;
	writel(reg, &pll->pll_base);
}

static void enable_cpu_clock(int enable)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 clk;

	/*
	 * NOTE:
	 * Regardless of whether the request is to enable or disable the CPU
	 * clock, every processor in the CPU complex except the master (CPU 0)
	 * will have it's clock stopped because the AVP only talks to the
	 * master. The AVP does not know (nor does it need to know) that there
	 * are multiple processors in the CPU complex.
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
	 * always stop the clock to CPU 1.
	 */
	clk = readl(&clkrst->crc_clk_cpu_cmplx);
	clk |= CPU1_CLK_STP;

	if (enable) {
		/* Unstop the CPU clock */
		clk &= ~CPU0_CLK_STP;
	} else {
		/* Stop the CPU clock */
		clk |= CPU0_CLK_STP;
	}

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

static void powerup_cpu(void)
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

static void enable_cpu_power_rail(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	u32 reg;

	reg = readl(&pmc->pmc_cntrl);
	reg |= CPUPWRREQ_OE;
	writel(reg, &pmc->pmc_cntrl);

	/*
	 * The TI PMU65861C needs a 3.75ms delay between enabling
	 * the power rail and enabling the CPU clock.  This delay
	 * between SM1EN and SM1 is for switching time + the ramp
	 * up of the voltage to the CPU (VDD_CPU from PMU).
	 */
	udelay(3750);
}

static void reset_A9_cpu(int reset)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 cpu;

	/*
	* NOTE:  Regardless of whether the request is to hold the CPU in reset
	*        or take it out of reset, every processor in the CPU complex
	*        except the master (CPU 0) will be held in reset because the
	*        AVP only talks to the master. The AVP does not know that there
	*        are multiple processors in the CPU complex.
	*/

	/* Hold CPU 1 in reset */
	cpu = SET_DBGRESET1 | SET_DERESET1 | SET_CPURESET1;
	writel(cpu, &clkrst->crc_cpu_cmplx_set);

	if (reset) {
		/* Now place CPU0 into reset */
		cpu |= SET_DBGRESET0 | SET_DERESET0 | SET_CPURESET0;
		writel(cpu, &clkrst->crc_cpu_cmplx_set);
	} else {
		/* Take CPU0 out of reset */
		cpu = CLR_DBGRESET0 | CLR_DERESET0 | CLR_CPURESET0;
		writel(cpu, &clkrst->crc_cpu_cmplx_clr);
	}

	/* Enable/Disable master CPU reset */
	reset_set_enable(PERIPH_ID_CPU, reset);
}

static void clock_enable_coresight(int enable)
{
	struct clk_rst_ctlr *clkrst = (struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 rst, src;

	clock_set_enable(PERIPH_ID_CORESIGHT, enable);
	reset_set_enable(PERIPH_ID_CORESIGHT, !enable);

	if (enable) {
		/*
		 * Put CoreSight on PLLP_OUT0 (216 MHz) and divide it down by
		 *  1.5, giving an effective frequency of 144MHz.
		 * Set PLLP_OUT0 [bits31:30 = 00], and use a 7.1 divisor
		 *  (bits 7:0), so 00000001b == 1.5 (n+1 + .5)
		 */
		src = CLK_DIVIDER(NVBL_PLLP_KHZ, 144000);
		writel(src, &clkrst->crc_clk_src_csite);

		/* Unlock the CPU CoreSight interfaces */
		rst = 0xC5ACCE55;
		writel(rst, CSITE_CPU_DBG0_LAR);
		writel(rst, CSITE_CPU_DBG1_LAR);
	}
}

void start_cpu(u32 reset_vector)
{
	/* Enable VDD_CPU */
	enable_cpu_power_rail();

	/* Hold the CPUs in reset */
	reset_A9_cpu(1);

	/* Disable the CPU clock */
	enable_cpu_clock(0);

	/* Enable CoreSight */
	clock_enable_coresight(1);

	/*
	 * Set the entry point for CPU execution from reset,
	 *  if it's a non-zero value.
	 */
	if (reset_vector)
		writel(reset_vector, EXCEP_VECTOR_CPU_RESET_VECTOR);

	/* Enable the CPU clock */
	enable_cpu_clock(1);

	/* If the CPU doesn't already have power, power it up */
	powerup_cpu();

	/* Take the CPU out of reset */
	reset_A9_cpu(0);
}


void halt_avp(void)
{
	for (;;) {
		writel((HALT_COP_EVENT_JTAG | HALT_COP_EVENT_IRQ_1 \
			| HALT_COP_EVENT_FIQ_1 | (FLOW_MODE_STOP<<29)),
			FLOW_CTLR_HALT_COP_EVENTS);
	}
}

void enable_scu(void)
{
	struct scu_ctlr *scu = (struct scu_ctlr *)NV_PA_ARM_PERIPHBASE;
	u32 reg;

	/* If SCU already setup/enabled, return */
	if (readl(&scu->scu_ctrl) & SCU_CTRL_ENABLE)
		return;

	/* Invalidate all ways for all processors */
	writel(0xFFFF, &scu->scu_inv_all);

	/* Enable SCU - bit 0 */
	reg = readl(&scu->scu_ctrl);
	reg |= SCU_CTRL_ENABLE;
	writel(reg, &scu->scu_ctrl);
}

void init_pmc_scratch(void)
{
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	int i;

	/* SCRATCH0 is initialized by the boot ROM and shouldn't be cleared */
	for (i = 0; i < 23; i++)
		writel(0, &pmc->pmc_scratch1+i);

	/* ODMDATA is for kernel use to determine RAM size, LP config, etc. */
	writel(CONFIG_SYS_BOARD_ODMDATA, &pmc->pmc_scratch20);
}

void cpu_start(void)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;

	/* enable JTAG */
	writel(0xC0, &pmt->pmt_cfg_ctl);

	if (s_first_boot) {
		/*
		 * Need to set this before cold-booting,
		 *  otherwise we'll end up in an infinite loop.
		 */
		s_first_boot = 0;
		cold_boot();
	}
}

void tegra2_start()
{
	if (s_first_boot) {
		/* Init Debug UART Port (115200 8n1) */
		uart_init();

		/* Init PMC scratch memory */
		init_pmc_scratch();
	}

#ifdef CONFIG_ENABLE_CORTEXA9
	/* take the mpcore out of reset */
	cpu_start();

	/* configure cache */
	cache_configure();
#endif
}

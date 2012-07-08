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

#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/ap20.h>
#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/fuse.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pmc.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/scu.h>
#include <asm/arch/warmboot.h>
#include <common.h>

int tegra_get_chip_type(void)
{
	struct apb_misc_gp_ctlr *gp;
	struct fuse_regs *fuse = (struct fuse_regs *)TEGRA2_FUSE_BASE;
	uint tegra_sku_id, rev;

	/*
	 * This is undocumented, Chip ID is bits 15:8 of the register
	 * APB_MISC + 0x804, and has value 0x20 for Tegra20, 0x30 for
	 * Tegra30
	 */
	gp = (struct apb_misc_gp_ctlr *)TEGRA2_APB_MISC_GP_BASE;
	rev = (readl(&gp->hidrev) & HIDREV_CHIPID_MASK) >> HIDREV_CHIPID_SHIFT;

	tegra_sku_id = readl(&fuse->sku_info) & 0xff;

	switch (rev) {
	case CHIPID_TEGRA2:
		switch (tegra_sku_id) {
		case SKU_ID_T20:
			return TEGRA_SOC_T20;
		case SKU_ID_T25SE:
		case SKU_ID_AP25:
		case SKU_ID_T25:
		case SKU_ID_AP25E:
		case SKU_ID_T25E:
			return TEGRA_SOC_T25;
		}
		break;
	}
	/* unknown sku id */
	return TEGRA_SOC_UNKNOWN;
}

/* Returns 1 if the current CPU executing is a Cortex-A9, else 0 */
static int ap20_cpu_is_cortexa9(void)
{
	u32 id = readb(NV_PA_PG_UP_BASE + PG_UP_TAG_0);
	return id == (PG_UP_TAG_0_PID_CPU & 0xff);
}

void init_pllx(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	struct clk_pll_simple *pll =
		&clkrst->crc_pll_simple[CLOCK_ID_XCPU - CLOCK_ID_FIRST_SIMPLE];
	u32 reg;

	/* If PLLX is already enabled, just return */
	if (readl(&pll->pll_base) & PLL_ENABLE_MASK)
		return;

	/* Set PLLX_MISC */
	writel(1 << PLL_CPCON_SHIFT, &pll->pll_misc);

	/* Use 12MHz clock here */
	reg = PLL_BYPASS_MASK | (12 << PLL_DIVM_SHIFT);
	reg |= 1000 << PLL_DIVN_SHIFT;
	writel(reg, &pll->pll_base);

	reg |= PLL_ENABLE_MASK;
	writel(reg, &pll->pll_base);

	reg &= ~PLL_BYPASS_MASK;
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
	clk |= 1 << CPU1_CLK_STP_SHIFT;

	/* Stop/Unstop the CPU clock */
	clk &= ~CPU0_CLK_STP_MASK;
	clk |= !enable << CPU0_CLK_STP_SHIFT;
	writel(clk, &clkrst->crc_clk_cpu_cmplx);

	clock_enable(PERIPH_ID_CPU);
}

static int is_cpu_powered(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;

	return (readl(&pmc->pmc_pwrgate_status) & CPU_PWRED) ? 1 : 0;
}

static void remove_cpu_io_clamps(void)
{
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;
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
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;
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
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;
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
	/*
	* NOTE:  Regardless of whether the request is to hold the CPU in reset
	*        or take it out of reset, every processor in the CPU complex
	*        except the master (CPU 0) will be held in reset because the
	*        AVP only talks to the master. The AVP does not know that there
	*        are multiple processors in the CPU complex.
	*/

	/* Hold CPU 1 in reset, and CPU 0 if asked */
	reset_cmplx_set_enable(1, crc_rst_cpu | crc_rst_de | crc_rst_debug, 1);
	reset_cmplx_set_enable(0, crc_rst_cpu | crc_rst_de | crc_rst_debug,
			       reset);

	/* Enable/Disable master CPU reset */
	reset_set_enable(PERIPH_ID_CPU, reset);
}

static void clock_enable_coresight(int enable)
{
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
		clock_ll_set_source_divisor(PERIPH_ID_CSI, 0, src);

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

static u32 get_odmdata(void)
{
	/*
	 * ODMDATA is stored in the BCT in IRAM by the BootROM.
	 * The BCT start and size are stored in the BIT in IRAM.
	 * Read the data @ bct_start + (bct_size - 12). This works
	 * on T20 and T30 BCTs, which are locked down. If this changes
	 * in new chips (T114, etc.), we can revisit this algorithm.
	 */

	u32 bct_start, odmdata;

	bct_start = readl(AP20_BASE_PA_SRAM + NVBOOTINFOTABLE_BCTPTR);
	odmdata = readl(bct_start + BCT_ODMDATA_OFFSET);

	return odmdata;
}

void init_pmc_scratch(void)
{
	struct pmc_ctlr *const pmc = (struct pmc_ctlr *)TEGRA2_PMC_BASE;
	u32 odmdata;
	int i;

	/* SCRATCH0 is initialized by the boot ROM and shouldn't be cleared */
	for (i = 0; i < 23; i++)
		writel(0, &pmc->pmc_scratch1+i);

	/* ODMDATA is for kernel use to determine RAM size, LP config, etc. */
	odmdata = get_odmdata();
	writel(odmdata, &pmc->pmc_scratch20);

#ifdef CONFIG_TEGRA2_LP0
	/* save Sdram params to PMC 2, 4, and 24 for WB0 */
	warmboot_save_sdram_params();
#endif
}

void tegra2_start(void)
{
	struct pmux_tri_ctlr *pmt = (struct pmux_tri_ctlr *)NV_PA_APB_MISC_BASE;

	/* If we are the AVP, start up the first Cortex-A9 */
	if (!ap20_cpu_is_cortexa9()) {
		/* enable JTAG */
		writel(0xC0, &pmt->pmt_cfg_ctl);

		/*
		 * If we are ARM7 - give it a different stack. We are about to
		 * start up the A9 which will want to use this one.
		 */
		asm volatile("mov	sp, %0\n"
			: : "r"(AVP_EARLY_BOOT_STACK_LIMIT));

		start_cpu((u32)_start);
		halt_avp();
		/* not reached */
	}

	/* Init PMC scratch memory */
	init_pmc_scratch();

	enable_scu();

	/* enable SMP mode and FW for CPU0, by writing to Auxiliary Ctl reg */
	asm volatile(
		"mrc	p15, 0, r0, c1, c0, 1\n"
		"orr	r0, r0, #0x41\n"
		"mcr	p15, 0, r0, c1, c0, 1\n");

	/* FIXME: should have ap20's L2 disabled too? */
}

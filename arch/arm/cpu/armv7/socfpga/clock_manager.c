/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock_manager.h>

static const struct socfpga_clock_manager *clock_manager_base =
		(void *)SOCFPGA_CLKMGR_ADDRESS;

#define CLKMGR_BYPASS_ENABLE	1
#define CLKMGR_BYPASS_DISABLE	0
#define CLKMGR_STAT_IDLE	0
#define CLKMGR_STAT_BUSY	1
#define CLKMGR_BYPASS_PERPLLSRC_SELECT_EOSC1		0
#define CLKMGR_BYPASS_PERPLLSRC_SELECT_INPUT_MUX	1
#define CLKMGR_BYPASS_SDRPLLSRC_SELECT_EOSC1		0
#define CLKMGR_BYPASS_SDRPLLSRC_SELECT_INPUT_MUX	1

#define CLEAR_BGP_EN_PWRDN \
	(CLKMGR_MAINPLLGRP_VCO_PWRDN_SET(0)| \
	CLKMGR_MAINPLLGRP_VCO_EN_SET(0)| \
	CLKMGR_MAINPLLGRP_VCO_BGPWRDN_SET(0))

#define VCO_EN_BASE \
	(CLKMGR_MAINPLLGRP_VCO_PWRDN_SET(0)| \
	CLKMGR_MAINPLLGRP_VCO_EN_SET(1)| \
	CLKMGR_MAINPLLGRP_VCO_BGPWRDN_SET(0))

static inline void cm_wait_for_lock(uint32_t mask)
{
	register uint32_t inter_val;
	do {
		inter_val = readl(&clock_manager_base->inter) & mask;
	} while (inter_val != mask);
}

/* function to poll in the fsm busy bit */
static inline void cm_wait_for_fsm(void)
{
	while (readl(&clock_manager_base->stat) & CLKMGR_STAT_BUSY)
		;
}

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static inline void cm_write_bypass(uint32_t val)
{
	writel(val, &clock_manager_base->bypass);
	cm_wait_for_fsm();
}

/* function to write the ctrl register which requires a poll of the busy bit */
static inline void cm_write_ctrl(uint32_t val)
{
	writel(val, &clock_manager_base->ctrl);
	cm_wait_for_fsm();
}

/* function to write a clock register that has phase information */
static inline void cm_write_with_phase(uint32_t value,
	uint32_t reg_address, uint32_t mask)
{
	/* poll until phase is zero */
	while (readl(reg_address) & mask)
		;

	writel(value, reg_address);

	while (readl(reg_address) & mask)
		;
}

/*
 * Setup clocks while making no assumptions about previous state of the clocks.
 *
 * Start by being paranoid and gate all sw managed clocks
 * Put all plls in bypass
 * Put all plls VCO registers back to reset value (bandgap power down).
 * Put peripheral and main pll src to reset value to avoid glitch.
 * Delay 5 us.
 * Deassert bandgap power down and set numerator and denominator
 * Start 7 us timer.
 * set internal dividers
 * Wait for 7 us timer.
 * Enable plls
 * Set external dividers while plls are locking
 * Wait for pll lock
 * Assert/deassert outreset all.
 * Take all pll's out of bypass
 * Clear safe mode
 * set source main and peripheral clocks
 * Ungate clocks
 */

void cm_basic_init(const cm_config_t *cfg)
{
	uint32_t start, timeout;

	/* Start by being paranoid and gate all sw managed clocks */

	/*
	 * We need to disable nandclk
	 * and then do another apb access before disabling
	 * gatting off the rest of the periperal clocks.
	 */
	writel(~CLKMGR_PERPLLGRP_EN_NANDCLK_MASK &
		readl(&clock_manager_base->per_pll_en),
		&clock_manager_base->per_pll_en);

	/* DO NOT GATE OFF DEBUG CLOCKS & BRIDGE CLOCKS */
	writel(CLKMGR_MAINPLLGRP_EN_DBGTIMERCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGTRACECLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_DBGATCLK_MASK |
		CLKMGR_MAINPLLGRP_EN_S2FUSER0CLK_MASK |
		CLKMGR_MAINPLLGRP_EN_L4MPCLK_MASK,
		&clock_manager_base->main_pll_en);

	writel(0, &clock_manager_base->sdr_pll_en);

	/* now we can gate off the rest of the peripheral clocks */
	writel(0, &clock_manager_base->per_pll_en);

	/* Put all plls in bypass */
	cm_write_bypass(
		CLKMGR_BYPASS_PERPLLSRC_SET(
		CLKMGR_BYPASS_PERPLLSRC_SELECT_EOSC1) |
		CLKMGR_BYPASS_SDRPLLSRC_SET(
		CLKMGR_BYPASS_SDRPLLSRC_SELECT_EOSC1) |
		CLKMGR_BYPASS_PERPLL_SET(CLKMGR_BYPASS_ENABLE) |
		CLKMGR_BYPASS_SDRPLL_SET(CLKMGR_BYPASS_ENABLE) |
		CLKMGR_BYPASS_MAINPLL_SET(CLKMGR_BYPASS_ENABLE));

	/*
	 * Put all plls VCO registers back to reset value.
	 * Some code might have messed with them.
	 */
	writel(CLKMGR_MAINPLLGRP_VCO_RESET_VALUE,
	       &clock_manager_base->main_pll_vco);
	writel(CLKMGR_PERPLLGRP_VCO_RESET_VALUE,
	       &clock_manager_base->per_pll_vco);
	writel(CLKMGR_SDRPLLGRP_VCO_RESET_VALUE,
	       &clock_manager_base->sdr_pll_vco);

	/*
	 * The clocks to the flash devices and the L4_MAIN clocks can
	 * glitch when coming out of safe mode if their source values
	 * are different from their reset value.  So the trick it to
	 * put them back to their reset state, and change input
	 * after exiting safe mode but before ungating the clocks.
	 */
	writel(CLKMGR_PERPLLGRP_SRC_RESET_VALUE,
	       &clock_manager_base->per_pll_src);
	writel(CLKMGR_MAINPLLGRP_L4SRC_RESET_VALUE,
	       &clock_manager_base->main_pll_l4src);

	/* read back for the required 5 us delay. */
	readl(&clock_manager_base->main_pll_vco);
	readl(&clock_manager_base->per_pll_vco);
	readl(&clock_manager_base->sdr_pll_vco);


	/*
	 * We made sure bgpwr down was assert for 5 us. Now deassert BG PWR DN
	 * with numerator and denominator.
	 */
	writel(cfg->main_vco_base | CLEAR_BGP_EN_PWRDN |
		CLKMGR_MAINPLLGRP_VCO_REGEXTSEL_MASK,
		&clock_manager_base->main_pll_vco);

	writel(cfg->peri_vco_base | CLEAR_BGP_EN_PWRDN |
		CLKMGR_PERPLLGRP_VCO_REGEXTSEL_MASK,
		&clock_manager_base->per_pll_vco);

	writel(CLKMGR_SDRPLLGRP_VCO_OUTRESET_SET(0) |
		CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(0) |
		cfg->sdram_vco_base | CLEAR_BGP_EN_PWRDN |
		CLKMGR_SDRPLLGRP_VCO_REGEXTSEL_MASK,
		&clock_manager_base->sdr_pll_vco);

	/*
	 * Time starts here
	 * must wait 7 us from BGPWRDN_SET(0) to VCO_ENABLE_SET(1)
	 */
	reset_timer();
	start = get_timer(0);
	/* timeout in unit of us as CONFIG_SYS_HZ = 1000*1000 */
	timeout = 7;

	/* main mpu */
	writel(cfg->mpuclk, &clock_manager_base->main_pll_mpuclk);

	/* main main clock */
	writel(cfg->mainclk, &clock_manager_base->main_pll_mainclk);

	/* main for dbg */
	writel(cfg->dbgatclk, &clock_manager_base->main_pll_dbgatclk);

	/* main for cfgs2fuser0clk */
	writel(cfg->cfg2fuser0clk,
	       &clock_manager_base->main_pll_cfgs2fuser0clk);

	/* Peri emac0 50 MHz default to RMII */
	writel(cfg->emac0clk, &clock_manager_base->per_pll_emac0clk);

	/* Peri emac1 50 MHz default to RMII */
	writel(cfg->emac1clk, &clock_manager_base->per_pll_emac1clk);

	/* Peri QSPI */
	writel(cfg->mainqspiclk, &clock_manager_base->main_pll_mainqspiclk);

	writel(cfg->perqspiclk, &clock_manager_base->per_pll_perqspiclk);

	/* Peri pernandsdmmcclk */
	writel(cfg->pernandsdmmcclk,
	       &clock_manager_base->per_pll_pernandsdmmcclk);

	/* Peri perbaseclk */
	writel(cfg->perbaseclk, &clock_manager_base->per_pll_perbaseclk);

	/* Peri s2fuser1clk */
	writel(cfg->s2fuser1clk, &clock_manager_base->per_pll_s2fuser1clk);

	/* 7 us must have elapsed before we can enable the VCO */
	while (get_timer(start) < timeout)
		;

	/* Enable vco */
	/* main pll vco */
	writel(cfg->main_vco_base | VCO_EN_BASE,
	       &clock_manager_base->main_pll_vco);

	/* periferal pll */
	writel(cfg->peri_vco_base | VCO_EN_BASE,
	       &clock_manager_base->per_pll_vco);

	/* sdram pll vco */
	writel(CLKMGR_SDRPLLGRP_VCO_OUTRESET_SET(0) |
		CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(0) |
		cfg->sdram_vco_base | VCO_EN_BASE,
		&clock_manager_base->sdr_pll_vco);

	/* L3 MP and L3 SP */
	writel(cfg->maindiv, &clock_manager_base->main_pll_maindiv);

	writel(cfg->dbgdiv, &clock_manager_base->main_pll_dbgdiv);

	writel(cfg->tracediv, &clock_manager_base->main_pll_tracediv);

	/* L4 MP, L4 SP, can0, and can1 */
	writel(cfg->perdiv, &clock_manager_base->per_pll_div);

	writel(cfg->gpiodiv, &clock_manager_base->per_pll_gpiodiv);

#define LOCKED_MASK \
	(CLKMGR_INTER_SDRPLLLOCKED_MASK  | \
	CLKMGR_INTER_PERPLLLOCKED_MASK  | \
	CLKMGR_INTER_MAINPLLLOCKED_MASK)

	cm_wait_for_lock(LOCKED_MASK);

	/* write the sdram clock counters before toggling outreset all */
	writel(cfg->ddrdqsclk & CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll_ddrdqsclk);

	writel(cfg->ddr2xdqsclk & CLKMGR_SDRPLLGRP_DDR2XDQSCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll_ddr2xdqsclk);

	writel(cfg->ddrdqclk & CLKMGR_SDRPLLGRP_DDRDQCLK_CNT_MASK,
	       &clock_manager_base->sdr_pll_ddrdqclk);

	writel(cfg->s2fuser2clk & CLKMGR_SDRPLLGRP_S2FUSER2CLK_CNT_MASK,
	       &clock_manager_base->sdr_pll_s2fuser2clk);

	/*
	 * after locking, but before taking out of bypass
	 * assert/deassert outresetall
	 */
	uint32_t mainvco = readl(&clock_manager_base->main_pll_vco);

	/* assert main outresetall */
	writel(mainvco | CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->main_pll_vco);

	uint32_t periphvco = readl(&clock_manager_base->per_pll_vco);

	/* assert pheriph outresetall */
	writel(periphvco | CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->per_pll_vco);

	/* assert sdram outresetall */
	writel(cfg->sdram_vco_base | VCO_EN_BASE|
		CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(1),
		&clock_manager_base->sdr_pll_vco);

	/* deassert main outresetall */
	writel(mainvco & ~CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->main_pll_vco);

	/* deassert pheriph outresetall */
	writel(periphvco & ~CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK,
	       &clock_manager_base->per_pll_vco);

	/* deassert sdram outresetall */
	writel(CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(0) |
		cfg->sdram_vco_base | VCO_EN_BASE,
		&clock_manager_base->sdr_pll_vco);

	/*
	 * now that we've toggled outreset all, all the clocks
	 * are aligned nicely; so we can change any phase.
	 */
	cm_write_with_phase(cfg->ddrdqsclk,
			    (uint32_t)&clock_manager_base->sdr_pll_ddrdqsclk,
			    CLKMGR_SDRPLLGRP_DDRDQSCLK_PHASE_MASK);

	/* SDRAM DDR2XDQSCLK */
	cm_write_with_phase(cfg->ddr2xdqsclk,
			    (uint32_t)&clock_manager_base->sdr_pll_ddr2xdqsclk,
			    CLKMGR_SDRPLLGRP_DDR2XDQSCLK_PHASE_MASK);

	cm_write_with_phase(cfg->ddrdqclk,
			    (uint32_t)&clock_manager_base->sdr_pll_ddrdqclk,
			    CLKMGR_SDRPLLGRP_DDRDQCLK_PHASE_MASK);

	cm_write_with_phase(cfg->s2fuser2clk,
			    (uint32_t)&clock_manager_base->sdr_pll_s2fuser2clk,
			    CLKMGR_SDRPLLGRP_S2FUSER2CLK_PHASE_MASK);

	/* Take all three PLLs out of bypass when safe mode is cleared. */
	cm_write_bypass(
		CLKMGR_BYPASS_PERPLLSRC_SET(
			CLKMGR_BYPASS_PERPLLSRC_SELECT_EOSC1) |
		CLKMGR_BYPASS_SDRPLLSRC_SET(
			CLKMGR_BYPASS_SDRPLLSRC_SELECT_EOSC1) |
		CLKMGR_BYPASS_PERPLL_SET(CLKMGR_BYPASS_DISABLE) |
		CLKMGR_BYPASS_SDRPLL_SET(CLKMGR_BYPASS_DISABLE) |
		CLKMGR_BYPASS_MAINPLL_SET(CLKMGR_BYPASS_DISABLE));

	/* clear safe mode */
	cm_write_ctrl(readl(&clock_manager_base->ctrl) |
			CLKMGR_CTRL_SAFEMODE_SET(CLKMGR_CTRL_SAFEMODE_MASK));

	/*
	 * now that safe mode is clear with clocks gated
	 * it safe to change the source mux for the flashes the the L4_MAIN
	 */
	writel(cfg->persrc, &clock_manager_base->per_pll_src);
	writel(cfg->l4src, &clock_manager_base->main_pll_l4src);

	/* Now ungate non-hw-managed clocks */
	writel(~0, &clock_manager_base->main_pll_en);
	writel(~0, &clock_manager_base->per_pll_en);
	writel(~0, &clock_manager_base->sdr_pll_en);
}

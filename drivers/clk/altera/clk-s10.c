// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2026 Altera Corporation <www.altera.com>
 *
 */

#include <log.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/system.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dm/lists.h>
#include <dm/util.h>
#include <dt-bindings/clock/stratix10-clock.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <asm/arch/clock_manager.h>

struct socfpga_clk_plat {
	void __iomem *regs;
	int pllgrp;
	int bitmask;
};

/*
 * function to write the bypass register which requires a poll of the
 * busy bit
 */
static void clk_write_bypass_mainpll(struct socfpga_clk_plat *plat, u32 val)
{
	void __iomem *base = plat->regs;

	CM_REG_WRITEL(plat, val, CLKMGR_MAINPLL_BYPASS);

	wait_for_bit_le32(base + CLKMGR_STAT,
			  CLKMGR_STAT_BUSY, false, 20000, false);
}

static void clk_write_bypass_perpll(struct socfpga_clk_plat *plat, u32 val)
{
	void __iomem *base = plat->regs;

	CM_REG_WRITEL(plat, val, CLKMGR_PERPLL_BYPASS);

	wait_for_bit_le32(base + CLKMGR_STAT,
			  CLKMGR_STAT_BUSY, false, 20000, false);
}

/* function to write the ctrl register which requires a poll of the busy bit */
static void clk_write_ctrl(struct socfpga_clk_plat *plat, u32 val)
{
	void __iomem *base = plat->regs;

	CM_REG_WRITEL(plat, val, CLKMGR_CTRL);

	wait_for_bit_le32(base + CLKMGR_STAT,
			  CLKMGR_STAT_BUSY, false, 20000, false);
}

/*
 * Setup clocks while making no assumptions about previous state of the clocks.
 */
static void clk_basic_init(struct udevice *dev,
			   const struct cm_config * const cfg)
{
	struct socfpga_clk_plat *plat = dev_get_plat(dev);
	u32 mdiv, refclkdiv, mscnt, hscnt, vcocalib;
	uintptr_t base_addr = (uintptr_t)plat->regs;

	if (!cfg)
		return;

	/* Put all plls in bypass */
	clk_write_bypass_mainpll(plat, CLKMGR_BYPASS_MAINPLL_ALL);
	clk_write_bypass_perpll(plat, CLKMGR_BYPASS_PERPLL_ALL);

	/* setup main PLL dividers where calculate the vcocalib value */
	mdiv = (cfg->main_pll_fdbck >> CLKMGR_FDBCK_MDIV_OFFSET) &
		CLKMGR_FDBCK_MDIV_MASK;
	refclkdiv = (cfg->main_pll_pllglob >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		     CLKMGR_PLLGLOB_REFCLKDIV_MASK;
	mscnt = CLKMGR_MSCNT_CONST / (CLKMGR_MDIV_CONST + mdiv) / refclkdiv;
	hscnt = (mdiv + CLKMGR_MDIV_CONST) * mscnt / refclkdiv -
		CLKMGR_HSCNT_CONST;
	vcocalib = (hscnt & CLKMGR_VCOCALIB_HSCNT_MASK) |
		   ((mscnt & CLKMGR_VCOCALIB_MSCNT_MASK) <<
		   CLKMGR_VCOCALIB_MSCNT_OFFSET);

	writel((cfg->main_pll_pllglob & ~CLKMGR_PLLGLOB_PD_MASK &
		~CLKMGR_PLLGLOB_RST_MASK),
		base_addr + CLKMGR_MAINPLL_PLLGLOB);
	writel(cfg->main_pll_fdbck,
	       base_addr + CLKMGR_MAINPLL_FDBCK);
	writel(vcocalib,
	       base_addr + CLKMGR_MAINPLL_VCOCALIB);
	writel(cfg->main_pll_pllc0,
	       base_addr + CLKMGR_MAINPLL_PLLC0);
	writel(cfg->main_pll_pllc1,
	       base_addr + CLKMGR_MAINPLL_PLLC1);
	writel(cfg->main_pll_nocdiv,
	       base_addr + CLKMGR_MAINPLL_NOCDIV);

	/* setup peripheral PLL dividers */
	/* calculate the vcocalib value */
	mdiv = (cfg->per_pll_fdbck >> CLKMGR_FDBCK_MDIV_OFFSET) &
		CLKMGR_FDBCK_MDIV_MASK;
	refclkdiv = (cfg->per_pll_pllglob >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		     CLKMGR_PLLGLOB_REFCLKDIV_MASK;
	mscnt = CLKMGR_MSCNT_CONST / (CLKMGR_MDIV_CONST + mdiv) / refclkdiv;
	hscnt = (mdiv + CLKMGR_MDIV_CONST) * mscnt / refclkdiv -
		CLKMGR_HSCNT_CONST;
	vcocalib = (hscnt & CLKMGR_VCOCALIB_HSCNT_MASK) |
		   ((mscnt & CLKMGR_VCOCALIB_MSCNT_MASK) <<
		   CLKMGR_VCOCALIB_MSCNT_OFFSET);

	writel((cfg->per_pll_pllglob & ~CLKMGR_PLLGLOB_PD_MASK &
		~CLKMGR_PLLGLOB_RST_MASK),
		base_addr + CLKMGR_PERPLL_PLLGLOB);
	writel(cfg->per_pll_fdbck,
	       base_addr + CLKMGR_PERPLL_FDBCK);
	writel(vcocalib,
	       base_addr + CLKMGR_PERPLL_VCOCALIB);
	writel(cfg->per_pll_pllc0,
	       base_addr + CLKMGR_PERPLL_PLLC0);
	writel(cfg->per_pll_pllc1,
	       base_addr + CLKMGR_PERPLL_PLLC1);
	writel(cfg->per_pll_emacctl,
	       base_addr + CLKMGR_PERPLL_EMACCTL);
	writel(cfg->per_pll_gpiodiv,
	       base_addr + CLKMGR_PERPLL_GPIODIV);

	/* Take both PLL out of reset and power up */
	setbits_le32(base_addr + CLKMGR_MAINPLL_PLLGLOB,
		     CLKMGR_PLLGLOB_PD_MASK | CLKMGR_PLLGLOB_RST_MASK);
	setbits_le32(base_addr + CLKMGR_PERPLL_PLLGLOB,
		     CLKMGR_PLLGLOB_PD_MASK | CLKMGR_PLLGLOB_RST_MASK);

	wait_for_bit_le32((const void *)(base_addr + CLKMGR_STAT),
			  CLKMGR_STAT_ALLPLL_LOCKED_MASK, true, 20000, false);

	/*
	 * Dividers for C2 to C9 only init after PLLs are lock. As dividers
	 * only take effect upon value change, we shall set a maximum value as
	 * default value.
	 */
	writel(0xff, base_addr + CLKMGR_MAINPLL_MPUCLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_NOCCLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR2CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR3CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR4CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR5CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR6CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR7CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR8CLK);
	writel(0xff, base_addr + CLKMGR_MAINPLL_CNTR9CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR2CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR3CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR4CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR5CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR6CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR7CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR8CLK);
	writel(0xff, base_addr + CLKMGR_PERPLL_CNTR9CLK);

	writel(cfg->main_pll_mpuclk,
	       base_addr + CLKMGR_MAINPLL_MPUCLK);
	writel(cfg->main_pll_nocclk,
	       base_addr + CLKMGR_MAINPLL_NOCCLK);
	writel(cfg->main_pll_cntr2clk,
	       base_addr + CLKMGR_MAINPLL_CNTR2CLK);
	writel(cfg->main_pll_cntr3clk,
	       base_addr + CLKMGR_MAINPLL_CNTR3CLK);
	writel(cfg->main_pll_cntr4clk,
	       base_addr + CLKMGR_MAINPLL_CNTR4CLK);
	writel(cfg->main_pll_cntr5clk,
	       base_addr + CLKMGR_MAINPLL_CNTR5CLK);
	writel(cfg->main_pll_cntr6clk,
	       base_addr + CLKMGR_MAINPLL_CNTR6CLK);
	writel(cfg->main_pll_cntr7clk,
	       base_addr + CLKMGR_MAINPLL_CNTR7CLK);
	writel(cfg->main_pll_cntr8clk,
	       base_addr + CLKMGR_MAINPLL_CNTR8CLK);
	writel(cfg->main_pll_cntr9clk,
	       base_addr + CLKMGR_MAINPLL_CNTR9CLK);
	writel(cfg->per_pll_cntr2clk,
	       base_addr + CLKMGR_PERPLL_CNTR2CLK);
	writel(cfg->per_pll_cntr3clk,
	       base_addr + CLKMGR_PERPLL_CNTR3CLK);
	writel(cfg->per_pll_cntr4clk,
	       base_addr + CLKMGR_PERPLL_CNTR4CLK);
	writel(cfg->per_pll_cntr5clk,
	       base_addr + CLKMGR_PERPLL_CNTR5CLK);
	writel(cfg->per_pll_cntr6clk,
	       base_addr + CLKMGR_PERPLL_CNTR6CLK);
	writel(cfg->per_pll_cntr7clk,
	       base_addr + CLKMGR_PERPLL_CNTR7CLK);
	writel(cfg->per_pll_cntr8clk,
	       base_addr + CLKMGR_PERPLL_CNTR8CLK);
	writel(cfg->per_pll_cntr9clk,
	       base_addr + CLKMGR_PERPLL_CNTR9CLK);

	/* Take all PLLs out of bypass */
	clk_write_bypass_mainpll(plat, 0);
	clk_write_bypass_perpll(plat, 0);

	/* clear safe mode / out of boot mode */
	clk_write_ctrl(plat, readl(base_addr + CLKMGR_CTRL) &
		      ~(CLKMGR_CTRL_SAFEMODE));

	/* Now ungate non-hw-managed clocks */
	writel(~0, base_addr + CLKMGR_MAINPLL_EN);
	writel(~0, base_addr + CLKMGR_PERPLL_EN);

	/* Clear the loss of lock bits (write 1 to clear) */
	writel(CLKMGR_INTER_PERPLLLOST_MASK |
		      CLKMGR_INTER_MAINPLLLOST_MASK,
		      base_addr + CLKMGR_INTRCLR);
}

static u64 clk_get_vco_clk_hz(struct socfpga_clk_plat *plat,
			      u32 pllglob_reg, u32 fdbck_reg)
{
	 u64 fref, refdiv, mdiv, reg, vco;

	reg = CM_REG_READL(plat, pllglob_reg);

	fref = (reg >> CLKMGR_PLLGLOB_VCO_PSRC_OFFSET) &
		CLKMGR_PLLGLOB_VCO_PSRC_MASK;

	switch (fref) {
	case CLKMGR_VCO_PSRC_EOSC1:
		fref = cm_get_osc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_INTOSC:
		fref = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_VCO_PSRC_F2S:
		fref = cm_get_fpga_clk_hz();
		break;
	}

	refdiv = (reg >> CLKMGR_PLLGLOB_REFCLKDIV_OFFSET) &
		  CLKMGR_PLLGLOB_REFCLKDIV_MASK;

	reg = CM_REG_READL(plat, fdbck_reg);
	mdiv = (reg >> CLKMGR_FDBCK_MDIV_OFFSET) & CLKMGR_FDBCK_MDIV_MASK;

	vco = fref / refdiv;
	vco = vco * (CLKMGR_MDIV_CONST + mdiv);

	return vco;
}

static u64 clk_get_main_vco_clk_hz(struct socfpga_clk_plat *plat)
{
	return clk_get_vco_clk_hz(plat, CLKMGR_MAINPLL_PLLGLOB,
				 CLKMGR_MAINPLL_FDBCK);
}

static u64 clk_get_per_vco_clk_hz(struct socfpga_clk_plat *plat)
{
	return clk_get_vco_clk_hz(plat, CLKMGR_PERPLL_PLLGLOB,
				 CLKMGR_PERPLL_FDBCK);
}

static u32 clk_get_5_1_clk_src(struct socfpga_clk_plat *plat, u64 reg)
{
	u32 clksrc = CM_REG_READL(plat, reg);

	return (clksrc >> CLKMGR_CLKSRC_OFFSET) & CLKMGR_CLKSRC_MASK;
}

static u64 clk_get_mpu_clk_hz(struct socfpga_clk_plat *plat)
{
	u64 clock;
	u32 clklsrc = clk_get_5_1_clk_src(plat, CLKMGR_MAINPLL_MPUCLK);

	switch (clklsrc) {
	case CLKMGR_CLKSRC_MAIN:
		clock = clk_get_main_vco_clk_hz(plat);
		clock /= (CM_REG_READL(plat, CLKMGR_MAINPLL_PLLC0) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;
	case CLKMGR_CLKSRC_PER:
		clock = clk_get_per_vco_clk_hz(plat);
		clock /= (CM_REG_READL(plat, CLKMGR_PERPLL_PLLC0) &
			  CLKMGR_CLKCNT_MSK);
		break;
	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;
	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	default:
		return 0;
	}

	clock /= 1 + (CM_REG_READL(plat, CLKMGR_MAINPLL_MPUCLK) &
		 CLKMGR_CLKCNT_MSK);

	return clock;
}

static u32 clk_get_l3_main_clk_hz(struct socfpga_clk_plat *plat)
{
	u64 clock;
	u32 clklsrc = clk_get_5_1_clk_src(plat, CLKMGR_MAINPLL_NOCCLK);

	switch (clklsrc) {
	case CLKMGR_CLKSRC_MAIN:
		clock = clk_get_main_vco_clk_hz(plat);
		clock /= (CM_REG_READL(plat, CLKMGR_MAINPLL_PLLC1) &
			  CLKMGR_PLLC0_DIV_MASK);
		break;
	case CLKMGR_CLKSRC_PER:
		clock = clk_get_per_vco_clk_hz(plat);
		clock /= (CM_REG_READL(plat, CLKMGR_PERPLL_PLLC1) &
			  CLKMGR_CLKCNT_MSK);
		break;
	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;
	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	default:
		return 0;
	}

	clock /= 1 + (CM_REG_READL(plat, CLKMGR_MAINPLL_NOCCLK) &
		 CLKMGR_CLKCNT_MSK);

	return clock;
}

static u32 clk_get_sdmmc_clk_hz(struct socfpga_clk_plat *plat)
{
	u32 clock;
	u32 clklsrc = clk_get_5_1_clk_src(plat, CLKMGR_PERPLL_CNTR6CLK);

	switch (clklsrc) {
	case CLKMGR_CLKSRC_MAIN:
		clock = clk_get_l3_main_clk_hz(plat);
		clock /= 1 + (CM_REG_READL(plat, CLKMGR_MAINPLL_CNTR6CLK) & CLKMGR_CLKCNT_MSK);
		break;
	case CLKMGR_CLKSRC_PER:
		clock = clk_get_l3_main_clk_hz(plat);
		clock /= 1 + (CM_REG_READL(plat, CLKMGR_PERPLL_CNTR6CLK) & CLKMGR_CLKCNT_MSK);
		break;
	case CLKMGR_CLKSRC_OSC1:
		clock = cm_get_osc_clk_hz();
		break;
	case CLKMGR_CLKSRC_INTOSC:
		clock = cm_get_intosc_clk_hz();
		break;
	case CLKMGR_CLKSRC_FPGA:
		clock = cm_get_fpga_clk_hz();
		break;
	default:
		return 0;
	}

	return clock / 4;
}

static u32 clk_get_l4_sp_clk_hz(struct socfpga_clk_plat *plat)
{
	u64 clock = clk_get_l3_main_clk_hz(plat);

	clock /= BIT((CM_REG_READL(plat, CLKMGR_MAINPLL_NOCDIV) >>
		      CLKMGR_NOCDIV_L4SPCLK_OFFSET) &
		      CLKMGR_CLKCNT_MSK);

	return clock;
}

static u32 clk_get_l4_sys_free_clk_hz(struct socfpga_clk_plat *plat)
{
	if (CM_REG_READL(plat, CLKMGR_STAT) & CLKMGR_STAT_BOOTMODE)
		return clk_get_l3_main_clk_hz(plat) / 2;

	return clk_get_l3_main_clk_hz(plat) / 4;
}

static ulong socfpga_clk_get_rate(struct clk *clk)
{
	struct socfpga_clk_plat *plat = dev_get_plat(clk->dev);

	switch (clk->id) {
	case STRATIX10_MPU_CLK:
		return clk_get_mpu_clk_hz(plat);
	case STRATIX10_NOC_CLK:
		return clk_get_l3_main_clk_hz(plat);
	case STRATIX10_MAIN_PLL_CLK:
		return clk_get_main_vco_clk_hz(plat);
	case STRATIX10_PERIPH_PLL_CLK:
		return clk_get_per_vco_clk_hz(plat);
	case STRATIX10_OSC1:
		return cm_get_osc_clk_hz();
	case STRATIX10_SDMMC_CLK:
		return clk_get_sdmmc_clk_hz(plat);
	case STRATIX10_L4_SP_CLK:
		return clk_get_l4_sp_clk_hz(plat);
	case STRATIX10_L4_SYS_FREE_CLK:
		return clk_get_l4_sys_free_clk_hz(plat);
	default:
		return -ENXIO;
	}
}

static int bitmask_from_clk_id(struct clk *clk)
{
	struct socfpga_clk_plat *plat = dev_get_plat(clk->dev);

	switch (clk->id) {
	case STRATIX10_MPU_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_MPUCLK_MASK;
		break;
	case STRATIX10_L4_MAIN_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_L4MAINCLK_MASK;
		break;
	case STRATIX10_L4_MP_CLK:
	case STRATIX10_NAND_X_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_L4MPCLK_MASK;
		break;
	case STRATIX10_L4_SP_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_L4SPCLK_MASK;
		break;
	case STRATIX10_CS_AT_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_CSCLK_MASK;
		break;
	case STRATIX10_CS_TRACE_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_CSCLK_MASK;
		break;
	case STRATIX10_CS_PDBG_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_CSCLK_MASK;
		break;
	case STRATIX10_CS_TIMER_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_CSTIMERCLK_MASK;
		break;
	case STRATIX10_S2F_USER0_CLK:
		plat->pllgrp = CLKMGR_MAINPLL_EN;
		plat->bitmask = CLKMGR_MAINPLLGRP_EN_S2FUSER0CLK_MASK;
		break;
	case STRATIX10_EMAC0_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_EMAC0CLK_MASK;
		break;
	case STRATIX10_EMAC1_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_EMAC1CLK_MASK;
		break;
	case STRATIX10_EMAC2_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_EMAC2CLK_MASK;
		break;
	case STRATIX10_EMAC_PTP_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_EMACPTPCLK_MASK;
		break;
	case STRATIX10_GPIO_DB_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_GPIODBCLK_MASK;
		break;
	case STRATIX10_SDMMC_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_SDMMCCLK_MASK;
		break;
	case STRATIX10_S2F_USER1_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_S2FUSER1CLK_MASK;
		break;
	case STRATIX10_PSI_REF_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_PSIREFCLK_MASK;
		break;
	case STRATIX10_USB_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_USBCLK_MASK;
		break;
	case STRATIX10_SPI_M_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_SPIMCLK_MASK;
		break;
	case STRATIX10_NAND_CLK:
		plat->pllgrp = CLKMGR_PERPLL_EN;
		plat->bitmask = CLKMGR_PERPLLGRP_EN_NANDCLK_MASK;
		break;
	case STRATIX10_L4_SYS_FREE_CLK:
		return -EOPNOTSUPP;
	default:
		return -ENXIO;
	}

	return 0;
}

static int socfpga_clk_enable(struct clk *clk)
{
	struct socfpga_clk_plat *plat = dev_get_plat(clk->dev);
	uintptr_t base_addr = (uintptr_t)plat->regs;
	int ret;

	ret = bitmask_from_clk_id(clk);
	if (ret == -EOPNOTSUPP)
		return 0;

	if (ret)
		return ret;

	setbits_le32(base_addr + plat->pllgrp, plat->bitmask);

	return 0;
}

static int socfpga_clk_disable(struct clk *clk)
{
	struct socfpga_clk_plat *plat = dev_get_plat(clk->dev);
	uintptr_t base_addr = (uintptr_t)plat->regs;
	int ret;

	ret = bitmask_from_clk_id(clk);
	if (ret == -EOPNOTSUPP)
		return 0;

	if (ret)
		return ret;

	clrbits_le32(base_addr + plat->pllgrp, plat->bitmask);

	return 0;
}

static int socfpga_clk_probe(struct udevice *dev)
{
	const struct cm_config *cm_default_cfg = cm_get_default_config();

	clk_basic_init(dev, cm_default_cfg);

	return 0;
}

static int socfpga_clk_of_to_plat(struct udevice *dev)
{
	struct socfpga_clk_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	plat->regs = (void __iomem *)addr;

	return 0;
}

static struct clk_ops socfpga_clk_ops = {
	.enable		= socfpga_clk_enable,
	.disable	= socfpga_clk_disable,
	.get_rate	= socfpga_clk_get_rate,
};

static const struct udevice_id socfpga_clk_match[] = {
	{ .compatible = "intel,stratix10-clkmgr" },
	{}
};

U_BOOT_DRIVER(socfpga_s10_clk) = {
	.name		= "clk-s10",
	.id		= UCLASS_CLK,
	.of_match	= socfpga_clk_match,
	.ops		= &socfpga_clk_ops,
	.probe		= socfpga_clk_probe,
	.of_to_plat = socfpga_clk_of_to_plat,
	.plat_auto	= sizeof(struct socfpga_clk_plat),
};

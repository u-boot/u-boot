/*
 * Memory setup for SMDK5250 board based on EXYNOS5
 *
 * Copyright (C) 2012 Samsung Electronics
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

#include <config.h>
#include <asm/io.h>
#include <asm/arch/dmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include "setup.h"

/* APLL : 1GHz */
/* MCLK_CDREX: MCLK_CDREX_533*/
/* LPDDR support: LPDDR2 */
static void reset_phy_ctrl(void);
static void config_zq(struct exynos5_phy_control *,
			struct exynos5_phy_control *);
static void update_reset_dll(struct exynos5_dmc *);
static void config_cdrex(void);
static void config_mrs(struct exynos5_dmc *);
static void sec_sdram_phy_init(struct exynos5_dmc *);
static void config_prech(struct exynos5_dmc *);
static void config_rdlvl(struct exynos5_dmc *,
			struct exynos5_phy_control *,
			struct exynos5_phy_control *);
static void config_memory(struct exynos5_dmc *);

static void config_offsets(unsigned int,
				struct exynos5_phy_control *,
				struct exynos5_phy_control *);

static void reset_phy_ctrl(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;

	writel(PHY_RESET_VAL, &clk->lpddr3phy_ctrl);
	sdelay(0x10000);
}

static void config_zq(struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val = 0;
	/*
	 * ZQ Calibration:
	 * Select Driver Strength,
	 * long calibration for manual calibration
	 */
	val = PHY_CON16_RESET_VAL;
	SET_ZQ_MODE_DDS_VAL(val);
	SET_ZQ_MODE_TERM_VAL(val);
	val |= ZQ_CLK_DIV_EN;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* Disable termination */
	val |= ZQ_MODE_NOTERM;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* ZQ_MANUAL_START: Enable */
	val |= ZQ_MANUAL_STR;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);
	sdelay(0x10000);

	/* ZQ_MANUAL_START: Disable */
	val &= ~ZQ_MANUAL_STR;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);
}

static void update_reset_dll(struct exynos5_dmc *dmc)
{
	unsigned long val;
	/*
	 * Update DLL Information:
	 * Force DLL Resyncronization
	 */
	val = readl(&dmc->phycontrol0);
	val |= FP_RSYNC;
	writel(val, &dmc->phycontrol0);

	/* Reset Force DLL Resyncronization */
	val = readl(&dmc->phycontrol0);
	val &= ~FP_RSYNC;
	writel(val, &dmc->phycontrol0);
}

static void config_mrs(struct exynos5_dmc *dmc)
{
	unsigned long channel, chip, mask = 0, val;

	for (channel = 0; channel < CONFIG_DMC_CHANNELS; channel++) {
		SET_CMD_CHANNEL(mask, channel);
		for (chip = 0; chip < CONFIG_CHIPS_PER_CHANNEL; chip++) {
			/*
			 * NOP CMD:
			 * Assert and hold CKE to logic high level
			 */
			SET_CMD_CHIP(mask, chip);
			val = DIRECT_CMD_NOP | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);

			/* EMRS, MRS Cmds(Mode Reg Settings) Using Direct Cmd */
			val = DIRECT_CMD_MRS1 | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);

			val = DIRECT_CMD_MRS2 | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);

			/* MCLK_CDREX_533 */
			val = DIRECT_CMD_MRS3 | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);

			val = DIRECT_CMD_MRS4 | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);
		}
	}
}

static void config_prech(struct exynos5_dmc *dmc)
{
	unsigned long channel, chip, mask = 0, val;

	for (channel = 0; channel < CONFIG_DMC_CHANNELS; channel++) {
		SET_CMD_CHANNEL(mask, channel);
		for (chip = 0; chip < CONFIG_CHIPS_PER_CHANNEL; chip++) {
			SET_CMD_CHIP(mask, chip);
			/* PALL (all banks precharge) CMD */
			val = DIRECT_CMD_PALL | mask;
			writel(val, &dmc->directcmd);
			sdelay(0x10000);
		}
	}
}

static void sec_sdram_phy_init(struct exynos5_dmc *dmc)
{
	unsigned long val;
	val = readl(&dmc->concontrol);
	val |= DFI_INIT_START;
	writel(val, &dmc->concontrol);
	sdelay(0x10000);

	val = readl(&dmc->concontrol);
	val &= ~DFI_INIT_START;
	writel(val, &dmc->concontrol);
}

static void config_offsets(unsigned int state,
				struct exynos5_phy_control *phy0_ctrl,
				struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	/* Set Offsets to read DQS */
	val = (state == SET) ? SET_DQS_OFFSET_VAL : RESET_DQS_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con4);
	writel(val, &phy1_ctrl->phy_con4);

	/* Set Offsets to read DQ */
	val = (state == SET) ? SET_DQ_OFFSET_VAL : RESET_DQ_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con6);
	writel(val, &phy1_ctrl->phy_con6);

	/* Debug Offset */
	val = (state == SET) ? SET_DEBUG_OFFSET_VAL : RESET_DEBUG_OFFSET_VAL;
	writel(val, &phy0_ctrl->phy_con10);
	writel(val, &phy1_ctrl->phy_con10);
}

static void config_cdrex(void)
{
	struct exynos5_clock *clk = (struct exynos5_clock *)EXYNOS5_CLOCK_BASE;
	writel(CLK_DIV_CDREX_VAL, &clk->div_cdrex);
	writel(CLK_SRC_CDREX_VAL, &clk->src_cdrex);
	sdelay(0x30000);
}

static void config_ctrl_dll_on(unsigned int state,
			unsigned int ctrl_force_val,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	val = readl(&phy0_ctrl->phy_con12);
	CONFIG_CTRL_DLL_ON(val, state);
	SET_CTRL_FORCE_VAL(val, ctrl_force_val);
	writel(val, &phy0_ctrl->phy_con12);

	val = readl(&phy1_ctrl->phy_con12);
	CONFIG_CTRL_DLL_ON(val, state);
	SET_CTRL_FORCE_VAL(val, ctrl_force_val);
	writel(val, &phy1_ctrl->phy_con12);
}

static void config_ctrl_start(unsigned int state,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;
	val = readl(&phy0_ctrl->phy_con12);
	CONFIG_CTRL_START(val, state);
	writel(val, &phy0_ctrl->phy_con12);

	val = readl(&phy1_ctrl->phy_con12);
	CONFIG_CTRL_START(val, state);
	writel(val, &phy1_ctrl->phy_con12);
}

#if defined(CONFIG_RD_LVL)
static void config_rdlvl(struct exynos5_dmc *dmc,
			struct exynos5_phy_control *phy0_ctrl,
			struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val;

	/* Disable CTRL_DLL_ON and set ctrl_force */
	config_ctrl_dll_on(RESET, 0x2D, phy0_ctrl, phy1_ctrl);

	/*
	 * Set ctrl_gateadj, ctrl_readadj
	 * ctrl_gateduradj, rdlvl_pass_adj
	 * rdlvl_rddataPadj
	 */
	val = SET_RDLVL_RDDATAPADJ;
	writel(val, &phy0_ctrl->phy_con1);
	writel(val, &phy1_ctrl->phy_con1);

	/* LPDDR2 Address */
	writel(LPDDR2_ADDR, &phy0_ctrl->phy_con22);
	writel(LPDDR2_ADDR, &phy1_ctrl->phy_con22);

	/* Enable Byte Read Leveling set ctrl_ddr_mode */
	val = readl(&phy0_ctrl->phy_con0);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con0);
	val = readl(&phy1_ctrl->phy_con0);
	val |= BYTE_RDLVL_EN;
	writel(val, &phy1_ctrl->phy_con0);

	/* rdlvl_en: Use levelling offset instead ctrl_shiftc */
	val = PHY_CON2_RESET_VAL | RDLVL_EN;
	writel(val, &phy0_ctrl->phy_con2);
	writel(val, &phy1_ctrl->phy_con2);
	sdelay(0x10000);

	/* Enable Data Eye Training */
	val = readl(&dmc->rdlvl_config);
	val |= CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);
	sdelay(0x10000);

	/* Disable Data Eye Training */
	val = readl(&dmc->rdlvl_config);
	val &= ~CTRL_RDLVL_DATA_EN;
	writel(val, &dmc->rdlvl_config);

	/* RdDeSkew_clear: Clear */
	val = readl(&phy0_ctrl->phy_con2);
	val |= RDDSKEW_CLEAR;
	writel(val, &phy0_ctrl->phy_con2);
	val = readl(&phy1_ctrl->phy_con2);
	val |= RDDSKEW_CLEAR;
	writel(val, &phy1_ctrl->phy_con2);

	/* Enable CTRL_DLL_ON */
	config_ctrl_dll_on(SET, 0x0, phy0_ctrl, phy1_ctrl);

	update_reset_dll(dmc);
	sdelay(0x10000);

	/* ctrl_atgte: ctrl_gate_p*, ctrl_read_p* generated by PHY */
	val = readl(&phy0_ctrl->phy_con0);
	val &= ~CTRL_ATGATE;
	writel(val, &phy0_ctrl->phy_con0);
	val = readl(&phy1_ctrl->phy_con0);
	val &= ~CTRL_ATGATE;
	writel(val, &phy1_ctrl->phy_con0);
}
#endif

static void config_memory(struct exynos5_dmc *dmc)
{
	/*
	 * Memory Configuration Chip 0
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	writel(DMC_MEMCONFIG0_VAL, &dmc->memconfig0);

	/*
	 * Memory Configuration Chip 1
	 * Address Mapping: Interleaved
	 * Number of Column address Bits: 10 bits
	 * Number of Rows Address Bits: 14
	 * Number of Banks: 8
	 */
	writel(DMC_MEMCONFIG1_VAL, &dmc->memconfig1);

	/*
	 * Chip0: AXI
	 * AXI Base Address: 0x40000000
	 * AXI Base Address Mask: 0x780
	 */
	writel(DMC_MEMBASECONFIG0_VAL, &dmc->membaseconfig0);

	/*
	 * Chip1: AXI
	 * AXI Base Address: 0x80000000
	 * AXI Base Address Mask: 0x780
	 */
	writel(DMC_MEMBASECONFIG1_VAL, &dmc->membaseconfig1);
}

void mem_ctrl_init()
{
	struct exynos5_phy_control *phy0_ctrl, *phy1_ctrl;
	struct exynos5_dmc *dmc;
	unsigned long val;

	phy0_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY0_BASE;
	phy1_ctrl = (struct exynos5_phy_control *)EXYNOS5_DMC_PHY1_BASE;
	dmc = (struct exynos5_dmc *)EXYNOS5_DMC_CTRL_BASE;

	/* Reset PHY Controllor: PHY_RESET[0] */
	reset_phy_ctrl();

	/*set Read Latancy and Burst Length for PHY0 and PHY1 */
	writel(PHY_CON42_VAL, &phy0_ctrl->phy_con42);
	writel(PHY_CON42_VAL, &phy1_ctrl->phy_con42);

	/* ZQ Cofiguration */
	config_zq(phy0_ctrl, phy1_ctrl);

	/* Operation Mode : LPDDR2 */
	val = PHY_CON0_RESET_VAL;
	SET_CTRL_DDR_MODE(val, DDR_MODE_LPDDR2);
	writel(val, &phy0_ctrl->phy_con0);
	writel(val, &phy1_ctrl->phy_con0);

	/* DQS, DQ: Signal, for LPDDR2: Always Set */
	val = CTRL_PULLD_DQ | CTRL_PULLD_DQS;
	writel(val, &phy0_ctrl->phy_con14);
	writel(val, &phy1_ctrl->phy_con14);

	/* Init SEC SDRAM PHY */
	sec_sdram_phy_init(dmc);
	sdelay(0x10000);

	update_reset_dll(dmc);

	/*
	 * Dynamic Clock: Always Running
	 * Memory Burst length: 4
	 * Number of chips: 2
	 * Memory Bus width: 32 bit
	 * Memory Type: LPDDR2-S4
	 * Additional Latancy for PLL: 1 Cycle
	 */
	writel(DMC_MEMCONTROL_VAL, &dmc->memcontrol);

	config_memory(dmc);

	/* Precharge Configuration */
	writel(DMC_PRECHCONFIG_VAL, &dmc->prechconfig);

	/* Power Down mode Configuration */
	writel(DMC_PWRDNCONFIG_VAL, &dmc->pwrdnconfig);

	/* Periodic Refrese Interval */
	writel(DMC_TIMINGREF_VAL, &dmc->timingref);

	/*
	 * TimingRow, TimingData, TimingPower Setting:
	 * Values as per Memory AC Parameters
	 */
	writel(DMC_TIMINGROW_VAL, &dmc->timingrow);

	writel(DMC_TIMINGDATA_VAL, &dmc->timingdata);

	writel(DMC_TIMINGPOWER_VAL, &dmc->timingpower);

	/* Memory Channel Inteleaving Size: 128 Bytes */
	writel(CONFIG_IV_SIZE, &dmc->ivcontrol);

	/* Set DQS, DQ and DEBUG offsets */
	config_offsets(SET, phy0_ctrl, phy1_ctrl);

	/* Disable CTRL_DLL_ON and set ctrl_force */
	config_ctrl_dll_on(RESET, 0x7F, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	update_reset_dll(dmc);

	/* Config MRS(Mode Register Settingg) */
	config_mrs(dmc);

	config_cdrex();

	/* Reset DQS DQ and DEBUG offsets */
	config_offsets(RESET, phy0_ctrl, phy1_ctrl);

	/* Enable CTRL_DLL_ON */
	config_ctrl_dll_on(SET, 0x0, phy0_ctrl, phy1_ctrl);

	/* Stop DLL Locking */
	config_ctrl_start(RESET, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	/* Start DLL Locking */
	config_ctrl_start(SET, phy0_ctrl, phy1_ctrl);
	sdelay(0x10000);

	update_reset_dll(dmc);

#if defined(CONFIG_RD_LVL)
	config_rdlvl(dmc, phy0_ctrl, phy1_ctrl);
#endif
	config_prech(dmc);

	/*
	 * Dynamic Clock: Stops During Idle Period
	 * Dynamic Power Down: Enable
	 * Dynamic Self refresh: Enable
	 */
	val = readl(&dmc->memcontrol);
	val |= CLK_STOP_EN | DPWRDN_EN | DSREF_EN;
	writel(val, &dmc->memcontrol);

	/* Start Auto refresh */
	val = readl(&dmc->concontrol);
	val |= AREF_EN;
	writel(val, &dmc->concontrol);
}

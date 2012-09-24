/*
 * Mem setup common file for different types of DDR present on SMDK5250 boards.
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

#include <common.h>
#include <asm/arch/spl.h>

#include "clock_init.h"
#include "setup.h"

#define ZQ_INIT_TIMEOUT	10000

int dmc_config_zq(struct mem_timings *mem,
		  struct exynos5_phy_control *phy0_ctrl,
		  struct exynos5_phy_control *phy1_ctrl)
{
	unsigned long val = 0;
	int i;

	/*
	 * ZQ Calibration:
	 * Select Driver Strength,
	 * long calibration for manual calibration
	 */
	val = PHY_CON16_RESET_VAL;
	val |= mem->zq_mode_dds << PHY_CON16_ZQ_MODE_DDS_SHIFT;
	val |= mem->zq_mode_term << PHY_CON16_ZQ_MODE_TERM_SHIFT;
	val |= ZQ_CLK_DIV_EN;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* Disable termination */
	if (mem->zq_mode_noterm)
		val |= PHY_CON16_ZQ_MODE_NOTERM_MASK;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* ZQ_MANUAL_START: Enable */
	val |= ZQ_MANUAL_STR;
	writel(val, &phy0_ctrl->phy_con16);
	writel(val, &phy1_ctrl->phy_con16);

	/* ZQ_MANUAL_START: Disable */
	val &= ~ZQ_MANUAL_STR;

	/*
	 * Since we are manaully calibrating the ZQ values,
	 * we are looping for the ZQ_init to complete.
	 */
	i = ZQ_INIT_TIMEOUT;
	while ((readl(&phy0_ctrl->phy_con17) & ZQ_DONE) != ZQ_DONE && i > 0) {
		sdelay(100);
		i--;
	}
	if (!i)
		return -1;
	writel(val, &phy0_ctrl->phy_con16);

	i = ZQ_INIT_TIMEOUT;
	while ((readl(&phy1_ctrl->phy_con17) & ZQ_DONE) != ZQ_DONE && i > 0) {
		sdelay(100);
		i--;
	}
	if (!i)
		return -1;
	writel(val, &phy1_ctrl->phy_con16);

	return 0;
}

void update_reset_dll(struct exynos5_dmc *dmc, enum ddr_mode mode)
{
	unsigned long val;

	if (mode == DDR_MODE_DDR3) {
		val = MEM_TERM_EN | PHY_TERM_EN | DMC_CTRL_SHGATE;
		writel(val, &dmc->phycontrol0);
	}

	/* Update DLL Information: Force DLL Resyncronization */
	val = readl(&dmc->phycontrol0);
	val |= FP_RSYNC;
	writel(val, &dmc->phycontrol0);

	/* Reset Force DLL Resyncronization */
	val = readl(&dmc->phycontrol0);
	val &= ~FP_RSYNC;
	writel(val, &dmc->phycontrol0);
}

void dmc_config_mrs(struct mem_timings *mem, struct exynos5_dmc *dmc)
{
	int channel, chip;

	for (channel = 0; channel < mem->dmc_channels; channel++) {
		unsigned long mask;

		mask = channel << DIRECT_CMD_CHANNEL_SHIFT;
		for (chip = 0; chip < mem->chips_to_configure; chip++) {
			int i;

			mask |= chip << DIRECT_CMD_CHIP_SHIFT;

			/* Sending NOP command */
			writel(DIRECT_CMD_NOP | mask, &dmc->directcmd);

			/*
			 * TODO(alim.akhtar@samsung.com): Do we need these
			 * delays? This one and the next were not there for
			 * DDR3.
			 */
			sdelay(0x10000);

			/* Sending EMRS/MRS commands */
			for (i = 0; i < MEM_TIMINGS_MSR_COUNT; i++) {
				writel(mem->direct_cmd_msr[i] | mask,
				       &dmc->directcmd);
				sdelay(0x10000);
			}

			if (mem->send_zq_init) {
				/* Sending ZQINIT command */
				writel(DIRECT_CMD_ZQINIT | mask,
				       &dmc->directcmd);

				sdelay(10000);
			}
		}
	}
}

void dmc_config_prech(struct mem_timings *mem, struct exynos5_dmc *dmc)
{
	int channel, chip;

	for (channel = 0; channel < mem->dmc_channels; channel++) {
		unsigned long mask;

		mask = channel << DIRECT_CMD_CHANNEL_SHIFT;
		for (chip = 0; chip < mem->chips_per_channel; chip++) {
			mask |= chip << DIRECT_CMD_CHIP_SHIFT;

			/* PALL (all banks precharge) CMD */
			writel(DIRECT_CMD_PALL | mask, &dmc->directcmd);
			sdelay(0x10000);
		}
	}
}

void dmc_config_memory(struct mem_timings *mem, struct exynos5_dmc *dmc)
{
	writel(mem->memconfig, &dmc->memconfig0);
	writel(mem->memconfig, &dmc->memconfig1);
	writel(DMC_MEMBASECONFIG0_VAL, &dmc->membaseconfig0);
	writel(DMC_MEMBASECONFIG1_VAL, &dmc->membaseconfig1);
}

void mem_ctrl_init()
{
	struct spl_machine_param *param = spl_get_machine_params();
	struct mem_timings *mem;
	int ret;

	mem = clock_get_mem_timings();

	/* If there are any other memory variant, add their init call below */
	if (param->mem_type == DDR_MODE_DDR3) {
		ret = ddr3_mem_ctrl_init(mem, param->mem_iv_size);
		if (ret) {
			/* will hang if failed to init memory control */
			while (1)
				;
		}
	} else {
		/* will hang if unknow memory type  */
		while (1)
			;
	}
}

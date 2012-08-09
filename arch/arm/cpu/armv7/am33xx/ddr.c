/*
 * DDR Configuration for AM33xx devices.
 *
 * Copyright (C) 2011 Texas Instruments Incorporated -
http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed .as is. WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

/**
 * Base address for EMIF instances
 */
static struct emif_reg_struct *emif_reg = {
				(struct emif_reg_struct *)EMIF4_0_CFG_BASE};

/**
 * Base address for DDR instance
 */
static struct ddr_regs *ddr_reg[2] = {
				(struct ddr_regs *)DDR_PHY_BASE_ADDR,
				(struct ddr_regs *)DDR_PHY_BASE_ADDR2};

/**
 * Base address for ddr io control instances
 */
static struct ddr_cmdtctrl *ioctrl_reg = {
			(struct ddr_cmdtctrl *)DDR_CONTROL_BASE_ADDR};

/**
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs)
{
	writel(regs->ref_ctrl, &emif_reg->emif_sdram_ref_ctrl);
	writel(regs->ref_ctrl, &emif_reg->emif_sdram_ref_ctrl_shdw);
	if (regs->zq_config){
		writel(regs->zq_config, &emif_reg->emif_zq_config);
		writel(regs->sdram_config, &cstat->secure_emif_sdram_config);
	}
	writel(regs->sdram_config, &emif_reg->emif_sdram_config);
}

/**
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs)
{
	writel(regs->sdram_tim1, &emif_reg->emif_sdram_tim_1);
	writel(regs->sdram_tim1, &emif_reg->emif_sdram_tim_1_shdw);
	writel(regs->sdram_tim2, &emif_reg->emif_sdram_tim_2);
	writel(regs->sdram_tim2, &emif_reg->emif_sdram_tim_2_shdw);
	writel(regs->sdram_tim3, &emif_reg->emif_sdram_tim_3);
	writel(regs->sdram_tim3, &emif_reg->emif_sdram_tim_3_shdw);
}

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs)
{
	writel(regs->emif_ddr_phy_ctlr_1, &emif_reg->emif_ddr_phy_ctrl_1);
	writel(regs->emif_ddr_phy_ctlr_1, &emif_reg->emif_ddr_phy_ctrl_1_shdw);
}

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd)
{
	writel(cmd->cmd0csratio, &ddr_reg[0]->cm0csratio);
	writel(cmd->cmd0dldiff, &ddr_reg[0]->cm0dldiff);
	writel(cmd->cmd0iclkout, &ddr_reg[0]->cm0iclkout);

	writel(cmd->cmd1csratio, &ddr_reg[0]->cm1csratio);
	writel(cmd->cmd1dldiff, &ddr_reg[0]->cm1dldiff);
	writel(cmd->cmd1iclkout, &ddr_reg[0]->cm1iclkout);

	writel(cmd->cmd2csratio, &ddr_reg[0]->cm2csratio);
	writel(cmd->cmd2dldiff, &ddr_reg[0]->cm2dldiff);
	writel(cmd->cmd2iclkout, &ddr_reg[0]->cm2iclkout);
}

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(int macrono, const struct ddr_data *data)
{
	writel(data->datardsratio0, &ddr_reg[macrono]->dt0rdsratio0);
	writel(data->datawdsratio0, &ddr_reg[macrono]->dt0wdsratio0);
	writel(data->datawiratio0, &ddr_reg[macrono]->dt0wiratio0);
	writel(data->datagiratio0, &ddr_reg[macrono]->dt0giratio0);
	writel(data->datafwsratio0, &ddr_reg[macrono]->dt0fwsratio0);
	writel(data->datawrsratio0, &ddr_reg[macrono]->dt0wrsratio0);
	writel(data->datauserank0delay, &ddr_reg[macrono]->dt0rdelays0);
	writel(data->datadldiff0, &ddr_reg[macrono]->dt0dldiff0);
}

void config_io_ctrl(unsigned long val)
{
	writel(val, &ioctrl_reg->cm0ioctl);
	writel(val, &ioctrl_reg->cm1ioctl);
	writel(val, &ioctrl_reg->cm2ioctl);
	writel(val, &ioctrl_reg->dt0ioctl);
	writel(val, &ioctrl_reg->dt1ioctl);
}

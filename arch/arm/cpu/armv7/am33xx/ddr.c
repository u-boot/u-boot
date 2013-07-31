/*
 * DDR Configuration for AM33xx devices.
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

/**
 * Base address for EMIF instances
 */
static struct emif_reg_struct *emif_reg[2] = {
				(struct emif_reg_struct *)EMIF4_0_CFG_BASE,
				(struct emif_reg_struct *)EMIF4_1_CFG_BASE};

/**
 * Base addresses for DDR PHY cmd/data regs
 */
static struct ddr_cmd_regs *ddr_cmd_reg[2] = {
				(struct ddr_cmd_regs *)DDR_PHY_CMD_ADDR,
				(struct ddr_cmd_regs *)DDR_PHY_CMD_ADDR2};

static struct ddr_data_regs *ddr_data_reg[2] = {
				(struct ddr_data_regs *)DDR_PHY_DATA_ADDR,
				(struct ddr_data_regs *)DDR_PHY_DATA_ADDR2};

/**
 * Base address for ddr io control instances
 */
static struct ddr_cmdtctrl *ioctrl_reg = {
			(struct ddr_cmdtctrl *)DDR_CONTROL_BASE_ADDR};

/**
 * Configure SDRAM
 */
void config_sdram(const struct emif_regs *regs, int nr)
{
	if (regs->zq_config) {
		/*
		 * A value of 0x2800 for the REF CTRL will give us
		 * about 570us for a delay, which will be long enough
		 * to configure things.
		 */
		writel(0x2800, &emif_reg[nr]->emif_sdram_ref_ctrl);
		writel(regs->zq_config, &emif_reg[nr]->emif_zq_config);
		writel(regs->sdram_config, &cstat->secure_emif_sdram_config);
		writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);
		writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
		writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);
	}
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);
	writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);
}

/**
 * Set SDRAM timings
 */
void set_sdram_timings(const struct emif_regs *regs, int nr)
{
	writel(regs->sdram_tim1, &emif_reg[nr]->emif_sdram_tim_1);
	writel(regs->sdram_tim1, &emif_reg[nr]->emif_sdram_tim_1_shdw);
	writel(regs->sdram_tim2, &emif_reg[nr]->emif_sdram_tim_2);
	writel(regs->sdram_tim2, &emif_reg[nr]->emif_sdram_tim_2_shdw);
	writel(regs->sdram_tim3, &emif_reg[nr]->emif_sdram_tim_3);
	writel(regs->sdram_tim3, &emif_reg[nr]->emif_sdram_tim_3_shdw);
}

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs, int nr)
{
	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1);
	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1_shdw);
}

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd, int nr)
{
	writel(cmd->cmd0csratio, &ddr_cmd_reg[nr]->cm0csratio);
	writel(cmd->cmd0dldiff, &ddr_cmd_reg[nr]->cm0dldiff);
	writel(cmd->cmd0iclkout, &ddr_cmd_reg[nr]->cm0iclkout);

	writel(cmd->cmd1csratio, &ddr_cmd_reg[nr]->cm1csratio);
	writel(cmd->cmd1dldiff, &ddr_cmd_reg[nr]->cm1dldiff);
	writel(cmd->cmd1iclkout, &ddr_cmd_reg[nr]->cm1iclkout);

	writel(cmd->cmd2csratio, &ddr_cmd_reg[nr]->cm2csratio);
	writel(cmd->cmd2dldiff, &ddr_cmd_reg[nr]->cm2dldiff);
	writel(cmd->cmd2iclkout, &ddr_cmd_reg[nr]->cm2iclkout);
}

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(const struct ddr_data *data, int nr)
{
	int i;

	for (i = 0; i < DDR_DATA_REGS_NR; i++) {
		writel(data->datardsratio0,
			&(ddr_data_reg[nr]+i)->dt0rdsratio0);
		writel(data->datawdsratio0,
			&(ddr_data_reg[nr]+i)->dt0wdsratio0);
		writel(data->datawiratio0,
			&(ddr_data_reg[nr]+i)->dt0wiratio0);
		writel(data->datagiratio0,
			&(ddr_data_reg[nr]+i)->dt0giratio0);
		writel(data->datafwsratio0,
			&(ddr_data_reg[nr]+i)->dt0fwsratio0);
		writel(data->datawrsratio0,
			&(ddr_data_reg[nr]+i)->dt0wrsratio0);
		writel(data->datauserank0delay,
			&(ddr_data_reg[nr]+i)->dt0rdelays0);
		writel(data->datadldiff0,
			&(ddr_data_reg[nr]+i)->dt0dldiff0);
	}
}

void config_io_ctrl(unsigned long val)
{
	writel(val, &ioctrl_reg->cm0ioctl);
	writel(val, &ioctrl_reg->cm1ioctl);
	writel(val, &ioctrl_reg->cm2ioctl);
	writel(val, &ioctrl_reg->dt0ioctl);
	writel(val, &ioctrl_reg->dt1ioctl);
}

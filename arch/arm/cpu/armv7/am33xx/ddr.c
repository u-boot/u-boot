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
#include <asm/io.h>

/**
 * Base address for EMIF instances
 */
static struct emif_regs *emif_reg = {
				(struct emif_regs *)EMIF4_0_CFG_BASE};

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
 * As a convention, all functions here return 0 on success
 * -1 on failure.
 */

/**
 * Configure SDRAM
 */
int config_sdram(struct sdram_config *cfg)
{
	writel(cfg->sdrcr, &emif_reg->sdrcr);
	writel(cfg->sdrcr2, &emif_reg->sdrcr2);
	writel(cfg->refresh, &emif_reg->sdrrcr);
	writel(cfg->refresh_sh, &emif_reg->sdrrcsr);

	return 0;
}

/**
 * Set SDRAM timings
 */
int set_sdram_timings(struct sdram_timing *t)
{
	writel(t->time1, &emif_reg->sdrtim1);
	writel(t->time1_sh, &emif_reg->sdrtim1sr);
	writel(t->time2, &emif_reg->sdrtim2);
	writel(t->time2_sh, &emif_reg->sdrtim2sr);
	writel(t->time3, &emif_reg->sdrtim3);
	writel(t->time3_sh, &emif_reg->sdrtim3sr);

	return 0;
}

/**
 * Configure DDR PHY
 */
int config_ddr_phy(struct ddr_phy_control *p)
{
	writel(p->reg, &emif_reg->ddrphycr);
	writel(p->reg_sh, &emif_reg->ddrphycsr);

	return 0;
}

/**
 * Configure DDR CMD control registers
 */
int config_cmd_ctrl(struct cmd_control *cmd)
{
	writel(cmd->cmd0csratio, &ddr_reg[0]->cm0csratio);
	writel(cmd->cmd0csforce, &ddr_reg[0]->cm0csforce);
	writel(cmd->cmd0csdelay, &ddr_reg[0]->cm0csdelay);
	writel(cmd->cmd0dldiff, &ddr_reg[0]->cm0dldiff);
	writel(cmd->cmd0iclkout, &ddr_reg[0]->cm0iclkout);

	writel(cmd->cmd1csratio, &ddr_reg[0]->cm1csratio);
	writel(cmd->cmd1csforce, &ddr_reg[0]->cm1csforce);
	writel(cmd->cmd1csdelay, &ddr_reg[0]->cm1csdelay);
	writel(cmd->cmd1dldiff, &ddr_reg[0]->cm1dldiff);
	writel(cmd->cmd1iclkout, &ddr_reg[0]->cm1iclkout);

	writel(cmd->cmd2csratio, &ddr_reg[0]->cm2csratio);
	writel(cmd->cmd2csforce, &ddr_reg[0]->cm2csforce);
	writel(cmd->cmd2csdelay, &ddr_reg[0]->cm2csdelay);
	writel(cmd->cmd2dldiff, &ddr_reg[0]->cm2dldiff);
	writel(cmd->cmd2iclkout, &ddr_reg[0]->cm2iclkout);

	return 0;
}

/**
 * Configure DDR DATA registers
 */
int config_ddr_data(int macrono, struct ddr_data *data)
{
	writel(data->datardsratio0, &ddr_reg[macrono]->dt0rdsratio0);
	writel(data->datardsratio1, &ddr_reg[macrono]->dt0rdsratio1);

	writel(data->datawdsratio0, &ddr_reg[macrono]->dt0wdsratio0);
	writel(data->datawdsratio1, &ddr_reg[macrono]->dt0wdsratio1);

	writel(data->datawiratio0, &ddr_reg[macrono]->dt0wiratio0);
	writel(data->datawiratio1, &ddr_reg[macrono]->dt0wiratio1);
	writel(data->datagiratio0, &ddr_reg[macrono]->dt0giratio0);
	writel(data->datagiratio1, &ddr_reg[macrono]->dt0giratio1);

	writel(data->datafwsratio0, &ddr_reg[macrono]->dt0fwsratio0);
	writel(data->datafwsratio1, &ddr_reg[macrono]->dt0fwsratio1);

	writel(data->datawrsratio0, &ddr_reg[macrono]->dt0wrsratio0);
	writel(data->datawrsratio1, &ddr_reg[macrono]->dt0wrsratio1);

	writel(data->datadldiff0, &ddr_reg[macrono]->dt0dldiff0);

	return 0;
}

int config_io_ctrl(struct ddr_ioctrl *ioctrl)
{
	writel(ioctrl->cmd1ctl, &ioctrl_reg->cm0ioctl);
	writel(ioctrl->cmd2ctl, &ioctrl_reg->cm1ioctl);
	writel(ioctrl->cmd3ctl, &ioctrl_reg->cm2ioctl);
	writel(ioctrl->data1ctl, &ioctrl_reg->dt0ioctl);
	writel(ioctrl->data2ctl, &ioctrl_reg->dt1ioctl);

	return 0;
}

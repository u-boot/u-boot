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

static inline u32 get_mr(int nr, u32 cs, u32 mr_addr)
{
	u32 mr;

	mr_addr |= cs << EMIF_REG_CS_SHIFT;
	writel(mr_addr, &emif_reg[nr]->emif_lpddr2_mode_reg_cfg);

	mr = readl(&emif_reg[nr]->emif_lpddr2_mode_reg_data);
	debug("get_mr: EMIF1 cs %d mr %08x val 0x%x\n", cs, mr_addr, mr);
	if (((mr & 0x0000ff00) >>  8) == (mr & 0xff) &&
	    ((mr & 0x00ff0000) >> 16) == (mr & 0xff) &&
	    ((mr & 0xff000000) >> 24) == (mr & 0xff))
		return mr & 0xff;
	else
		return mr;
}

static inline void set_mr(int nr, u32 cs, u32 mr_addr, u32 mr_val)
{
	mr_addr |= cs << EMIF_REG_CS_SHIFT;
	writel(mr_addr, &emif_reg[nr]->emif_lpddr2_mode_reg_cfg);
	writel(mr_val, &emif_reg[nr]->emif_lpddr2_mode_reg_data);
}

static void configure_mr(int nr, u32 cs)
{
	u32 mr_addr;

	while (get_mr(nr, cs, LPDDR2_MR0) & LPDDR2_MR0_DAI_MASK)
		;
	set_mr(nr, cs, LPDDR2_MR10, 0x56);

	set_mr(nr, cs, LPDDR2_MR1, 0x43);
	set_mr(nr, cs, LPDDR2_MR2, 0x2);

	mr_addr = LPDDR2_MR2 | EMIF_REG_REFRESH_EN_MASK;
	set_mr(nr, cs, mr_addr, 0x2);
}

/*
 * Configure EMIF4D5 registers and MR registers
 */
void config_sdram_emif4d5(const struct emif_regs *regs, int nr)
{
	writel(0xA0, &emif_reg[nr]->emif_pwr_mgmt_ctrl);
	writel(0xA0, &emif_reg[nr]->emif_pwr_mgmt_ctrl_shdw);
	writel(0x1, &emif_reg[nr]->emif_iodft_tlgc);
	writel(regs->zq_config, &emif_reg[nr]->emif_zq_config);

	writel(regs->temp_alert_config, &emif_reg[nr]->emif_temp_alert_config);
	writel(regs->emif_rd_wr_lvl_rmp_win,
	       &emif_reg[nr]->emif_rd_wr_lvl_rmp_win);
	writel(regs->emif_rd_wr_lvl_rmp_ctl,
	       &emif_reg[nr]->emif_rd_wr_lvl_rmp_ctl);
	writel(regs->emif_rd_wr_lvl_ctl, &emif_reg[nr]->emif_rd_wr_lvl_ctl);
	writel(regs->emif_rd_wr_exec_thresh,
	       &emif_reg[nr]->emif_rd_wr_exec_thresh);

	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl);
	writel(regs->ref_ctrl, &emif_reg[nr]->emif_sdram_ref_ctrl_shdw);
	writel(regs->sdram_config, &emif_reg[nr]->emif_sdram_config);
	writel(regs->sdram_config, &cstat->secure_emif_sdram_config);

	if (emif_sdram_type() == EMIF_SDRAM_TYPE_LPDDR2) {
		configure_mr(nr, 0);
		configure_mr(nr, 1);
	}
}

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

void __weak emif_get_ext_phy_ctrl_const_regs(const u32 **regs, u32 *size)
{
}

/*
 * Configure EXT PHY registers
 */
static void ext_phy_settings(const struct emif_regs *regs, int nr)
{
	u32 *ext_phy_ctrl_base = 0;
	u32 *emif_ext_phy_ctrl_base = 0;
	const u32 *ext_phy_ctrl_const_regs;
	u32 i = 0;
	u32 size;

	ext_phy_ctrl_base = (u32 *)&(regs->emif_ddr_ext_phy_ctrl_1);
	emif_ext_phy_ctrl_base =
			(u32 *)&(emif_reg[nr]->emif_ddr_ext_phy_ctrl_1);

	/* Configure external phy control timing registers */
	for (i = 0; i < EMIF_EXT_PHY_CTRL_TIMING_REG; i++) {
		writel(*ext_phy_ctrl_base, emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(*ext_phy_ctrl_base++, emif_ext_phy_ctrl_base++);
	}

	/*
	 * external phy 6-24 registers do not change with
	 * ddr frequency
	 */
	emif_get_ext_phy_ctrl_const_regs(&ext_phy_ctrl_const_regs, &size);

	if (!size)
		return;

	for (i = 0; i < size; i++) {
		writel(ext_phy_ctrl_const_regs[i], emif_ext_phy_ctrl_base++);
		/* Update shadow registers */
		writel(ext_phy_ctrl_const_regs[i], emif_ext_phy_ctrl_base++);
	}
}

/**
 * Configure DDR PHY
 */
void config_ddr_phy(const struct emif_regs *regs, int nr)
{
	/*
	 * disable initialization and refreshes for now until we
	 * finish programming EMIF regs.
	 */
	setbits_le32(&emif_reg[nr]->emif_sdram_ref_ctrl,
		     EMIF_REG_INITREF_DIS_MASK);

	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1);
	writel(regs->emif_ddr_phy_ctlr_1,
		&emif_reg[nr]->emif_ddr_phy_ctrl_1_shdw);

	if (get_emif_rev((u32)emif_reg[nr]) == EMIF_4D5)
		ext_phy_settings(regs, nr);
}

/**
 * Configure DDR CMD control registers
 */
void config_cmd_ctrl(const struct cmd_control *cmd, int nr)
{
	if (!cmd)
		return;

	writel(cmd->cmd0csratio, &ddr_cmd_reg[nr]->cm0csratio);
	writel(cmd->cmd0iclkout, &ddr_cmd_reg[nr]->cm0iclkout);

	writel(cmd->cmd1csratio, &ddr_cmd_reg[nr]->cm1csratio);
	writel(cmd->cmd1iclkout, &ddr_cmd_reg[nr]->cm1iclkout);

	writel(cmd->cmd2csratio, &ddr_cmd_reg[nr]->cm2csratio);
	writel(cmd->cmd2iclkout, &ddr_cmd_reg[nr]->cm2iclkout);
}

/**
 * Configure DDR DATA registers
 */
void config_ddr_data(const struct ddr_data *data, int nr)
{
	int i;

	if (!data)
		return;

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
	}
}

void config_io_ctrl(const struct ctrl_ioregs *ioregs)
{
	if (!ioregs)
		return;

	writel(ioregs->cm0ioctl, &ioctrl_reg->cm0ioctl);
	writel(ioregs->cm1ioctl, &ioctrl_reg->cm1ioctl);
	writel(ioregs->cm2ioctl, &ioctrl_reg->cm2ioctl);
	writel(ioregs->dt0ioctl, &ioctrl_reg->dt0ioctl);
	writel(ioregs->dt1ioctl, &ioctrl_reg->dt1ioctl);
#ifdef CONFIG_AM43XX
	writel(ioregs->dt2ioctrl, &ioctrl_reg->dt2ioctrl);
	writel(ioregs->dt3ioctrl, &ioctrl_reg->dt3ioctrl);
	writel(ioregs->emif_sdram_config_ext,
	       &ioctrl_reg->emif_sdram_config_ext);
#endif
}

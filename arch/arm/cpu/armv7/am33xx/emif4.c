/*
 * emif4.c
 *
 * AM33XX emif4 configuration file
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR /PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr_defs.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clock.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct ddr_regs *ddrregs = (struct ddr_regs *)DDR_PHY_BASE_ADDR;
struct vtp_reg *vtpreg = (struct vtp_reg *)VTP0_CTRL_ADDR;
struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;


int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size(
			(void *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_MAX_RAM_BANK_SIZE);
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = gd->ram_size;
}


#ifdef CONFIG_SPL_BUILD
static void data_macro_config(int dataMacroNum)
{
	struct ddr_data data;

	data.datardsratio0 = ((DDR2_RD_DQS<<30)|(DDR2_RD_DQS<<20)
				|(DDR2_RD_DQS<<10)|(DDR2_RD_DQS<<0));
	data.datardsratio1 = DDR2_RD_DQS>>2;
	data.datawdsratio0 = ((DDR2_WR_DQS<<30)|(DDR2_WR_DQS<<20)
				|(DDR2_WR_DQS<<10)|(DDR2_WR_DQS<<0));
	data.datawdsratio1 = DDR2_WR_DQS>>2;
	data.datawiratio0 = ((DDR2_PHY_WRLVL<<30)|(DDR2_PHY_WRLVL<<20)
				|(DDR2_PHY_WRLVL<<10)|(DDR2_PHY_WRLVL<<0));
	data.datawiratio1 = DDR2_PHY_WRLVL>>2;
	data.datagiratio0 = ((DDR2_PHY_GATELVL<<30)|(DDR2_PHY_GATELVL<<20)
				|(DDR2_PHY_GATELVL<<10)|(DDR2_PHY_GATELVL<<0));
	data.datagiratio1 = DDR2_PHY_GATELVL>>2;
	data.datafwsratio0 = ((DDR2_PHY_FIFO_WE<<30)|(DDR2_PHY_FIFO_WE<<20)
				|(DDR2_PHY_FIFO_WE<<10)|(DDR2_PHY_FIFO_WE<<0));
	data.datafwsratio1 = DDR2_PHY_FIFO_WE>>2;
	data.datawrsratio0 = ((DDR2_PHY_WR_DATA<<30)|(DDR2_PHY_WR_DATA<<20)
				|(DDR2_PHY_WR_DATA<<10)|(DDR2_PHY_WR_DATA<<0));
	data.datawrsratio1 = DDR2_PHY_WR_DATA>>2;
	data.datadldiff0 = PHY_DLL_LOCK_DIFF;

	config_ddr_data(dataMacroNum, &data);
}

static void cmd_macro_config(void)
{
	struct cmd_control cmd;

	cmd.cmd0csratio = DDR2_RATIO;
	cmd.cmd0csforce = CMD_FORCE;
	cmd.cmd0csdelay = CMD_DELAY;
	cmd.cmd0dldiff = DDR2_DLL_LOCK_DIFF;
	cmd.cmd0iclkout = DDR2_INVERT_CLKOUT;

	cmd.cmd1csratio = DDR2_RATIO;
	cmd.cmd1csforce = CMD_FORCE;
	cmd.cmd1csdelay = CMD_DELAY;
	cmd.cmd1dldiff = DDR2_DLL_LOCK_DIFF;
	cmd.cmd1iclkout = DDR2_INVERT_CLKOUT;

	cmd.cmd2csratio = DDR2_RATIO;
	cmd.cmd2csforce = CMD_FORCE;
	cmd.cmd2csdelay = CMD_DELAY;
	cmd.cmd2dldiff = DDR2_DLL_LOCK_DIFF;
	cmd.cmd2iclkout = DDR2_INVERT_CLKOUT;

	config_cmd_ctrl(&cmd);

}

static void config_vtp(void)
{
	writel(readl(&vtpreg->vtp0ctrlreg) | VTP_CTRL_ENABLE,
			&vtpreg->vtp0ctrlreg);
	writel(readl(&vtpreg->vtp0ctrlreg) & (~VTP_CTRL_START_EN),
			&vtpreg->vtp0ctrlreg);
	writel(readl(&vtpreg->vtp0ctrlreg) | VTP_CTRL_START_EN,
			&vtpreg->vtp0ctrlreg);

	/* Poll for READY */
	while ((readl(&vtpreg->vtp0ctrlreg) & VTP_CTRL_READY) !=
			VTP_CTRL_READY)
		;
}

static void config_emif_ddr2(void)
{
	int i;
	int ret;
	struct sdram_config cfg;
	struct sdram_timing tmg;
	struct ddr_phy_control phyc;

	/*Program EMIF0 CFG Registers*/
	phyc.reg = EMIF_READ_LATENCY;
	phyc.reg_sh = EMIF_READ_LATENCY;
	phyc.reg2 = EMIF_READ_LATENCY;

	tmg.time1 = EMIF_TIM1;
	tmg.time1_sh = EMIF_TIM1;
	tmg.time2 = EMIF_TIM2;
	tmg.time2_sh = EMIF_TIM2;
	tmg.time3 = EMIF_TIM3;
	tmg.time3_sh = EMIF_TIM3;

	cfg.sdrcr = EMIF_SDCFG;
	cfg.sdrcr2 = EMIF_SDCFG;
	cfg.refresh = 0x00004650;
	cfg.refresh_sh = 0x00004650;

	/* Program EMIF instance */
	ret = config_ddr_phy(&phyc);
	if (ret < 0)
		printf("Couldn't configure phyc\n");

	ret = config_sdram(&cfg);
	if (ret < 0)
		printf("Couldn't configure SDRAM\n");

	ret = set_sdram_timings(&tmg);
	if (ret < 0)
		printf("Couldn't configure timings\n");

	/* Delay */
	for (i = 0; i < 5000; i++)
		;

	cfg.refresh = EMIF_SDREF;
	cfg.refresh_sh = EMIF_SDREF;
	cfg.sdrcr = EMIF_SDCFG;
	cfg.sdrcr2 = EMIF_SDCFG;

	ret = config_sdram(&cfg);
	if (ret < 0)
		printf("Couldn't configure SDRAM\n");
}

void config_ddr(void)
{
	int data_macro_0 = 0;
	int data_macro_1 = 1;
	struct ddr_ioctrl ioctrl;

	enable_emif_clocks();

	config_vtp();

	cmd_macro_config();

	data_macro_config(data_macro_0);
	data_macro_config(data_macro_1);

	writel(PHY_RANK0_DELAY, &ddrregs->dt0rdelays0);
	writel(PHY_RANK0_DELAY, &ddrregs->dt1rdelays0);

	ioctrl.cmd1ctl = DDR_IOCTRL_VALUE;
	ioctrl.cmd2ctl = DDR_IOCTRL_VALUE;
	ioctrl.cmd3ctl = DDR_IOCTRL_VALUE;
	ioctrl.data1ctl = DDR_IOCTRL_VALUE;
	ioctrl.data2ctl = DDR_IOCTRL_VALUE;

	config_io_ctrl(&ioctrl);

	writel(readl(&ddrctrl->ddrioctrl) & 0xefffffff, &ddrctrl->ddrioctrl);
	writel(readl(&ddrctrl->ddrckectrl) | 0x00000001, &ddrctrl->ddrckectrl);

	config_emif_ddr2();
}
#endif

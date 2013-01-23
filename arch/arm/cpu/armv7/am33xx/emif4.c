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
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <asm/emif.h>

DECLARE_GLOBAL_DATA_PTR;

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
static struct vtp_reg *vtpreg = (struct vtp_reg *)VTP0_CTRL_ADDR;
static struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;

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

void config_ddr(unsigned int pll, unsigned int ioctrl,
		const struct ddr_data *data, const struct cmd_control *ctrl,
		const struct emif_regs *regs)
{
	enable_emif_clocks();
	ddr_pll_config(pll);
	config_vtp();
	config_cmd_ctrl(ctrl);

	config_ddr_data(0, data);
	config_ddr_data(1, data);

	config_io_ctrl(ioctrl);

	/* Set CKE to be controlled by EMIF/DDR PHY */
	writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

	/* Program EMIF instance */
	config_ddr_phy(regs);
	set_sdram_timings(regs);
	config_sdram(regs);
}
#endif

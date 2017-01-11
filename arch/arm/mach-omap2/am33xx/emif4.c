/*
 * emif4.c
 *
 * AM33XX emif4 configuration file
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	sdram_init();
#endif

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


#ifndef CONFIG_SKIP_LOWLEVEL_INIT
#ifdef CONFIG_TI81XX
static struct dmm_lisa_map_regs *hw_lisa_map_regs =
				(struct dmm_lisa_map_regs *)DMM_BASE;
#endif
#ifndef CONFIG_TI816X
static struct vtp_reg *vtpreg[2] = {
				(struct vtp_reg *)VTP0_CTRL_ADDR,
				(struct vtp_reg *)VTP1_CTRL_ADDR};
#endif
#ifdef CONFIG_AM33XX
static struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;
#endif
#ifdef CONFIG_AM43XX
static struct ddr_ctrl *ddrctrl = (struct ddr_ctrl *)DDR_CTRL_ADDR;
static struct cm_device_inst *cm_device =
				(struct cm_device_inst *)CM_DEVICE_INST;
#endif

#ifdef CONFIG_TI81XX
void config_dmm(const struct dmm_lisa_map_regs *regs)
{
	enable_dmm_clocks();

	writel(0, &hw_lisa_map_regs->dmm_lisa_map_3);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_2);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_1);
	writel(0, &hw_lisa_map_regs->dmm_lisa_map_0);

	writel(regs->dmm_lisa_map_3, &hw_lisa_map_regs->dmm_lisa_map_3);
	writel(regs->dmm_lisa_map_2, &hw_lisa_map_regs->dmm_lisa_map_2);
	writel(regs->dmm_lisa_map_1, &hw_lisa_map_regs->dmm_lisa_map_1);
	writel(regs->dmm_lisa_map_0, &hw_lisa_map_regs->dmm_lisa_map_0);
}
#endif

#ifndef CONFIG_TI816X
static void config_vtp(int nr)
{
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) | VTP_CTRL_ENABLE,
			&vtpreg[nr]->vtp0ctrlreg);
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) & (~VTP_CTRL_START_EN),
			&vtpreg[nr]->vtp0ctrlreg);
	writel(readl(&vtpreg[nr]->vtp0ctrlreg) | VTP_CTRL_START_EN,
			&vtpreg[nr]->vtp0ctrlreg);

	/* Poll for READY */
	while ((readl(&vtpreg[nr]->vtp0ctrlreg) & VTP_CTRL_READY) !=
			VTP_CTRL_READY)
		;
}
#endif

void __weak ddr_pll_config(unsigned int ddrpll_m)
{
}

void config_ddr(unsigned int pll, const struct ctrl_ioregs *ioregs,
		const struct ddr_data *data, const struct cmd_control *ctrl,
		const struct emif_regs *regs, int nr)
{
	ddr_pll_config(pll);
#ifndef CONFIG_TI816X
	config_vtp(nr);
#endif
	config_cmd_ctrl(ctrl, nr);

	config_ddr_data(data, nr);
#ifdef CONFIG_AM33XX
	config_io_ctrl(ioregs);

	/* Set CKE to be controlled by EMIF/DDR PHY */
	writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

#endif
#ifdef CONFIG_AM43XX
	writel(readl(&cm_device->cm_dll_ctrl) & ~0x1, &cm_device->cm_dll_ctrl);
	while ((readl(&cm_device->cm_dll_ctrl) & CM_DLL_READYST) == 0)
		;

	config_io_ctrl(ioregs);

	/* Set CKE to be controlled by EMIF/DDR PHY */
	writel(DDR_CKE_CTRL_NORMAL, &ddrctrl->ddrckectrl);

	if (emif_sdram_type(regs->sdram_config) == EMIF_SDRAM_TYPE_DDR3)
		/* Allow EMIF to control DDR_RESET */
		writel(0x00000000, &ddrctrl->ddrioctrl);
#endif

	/* Program EMIF instance */
	config_ddr_phy(regs, nr);
	set_sdram_timings(regs, nr);
	if (get_emif_rev(EMIF1_BASE) == EMIF_4D5)
		config_sdram_emif4d5(regs, nr);
	else
		config_sdram(regs, nr);
}
#endif

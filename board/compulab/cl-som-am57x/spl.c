// SPDX-License-Identifier: GPL-2.0+
/*
 * SPL data and initialization for CompuLab CL-SOM-AM57x board
 *
 * (C) Copyright 2016 CompuLab, Ltd. http://compulab.co.il/
 *
 * Author: Uri Mashiach <uri.mashiach@compulab.co.il>
 */

#include <asm/emif.h>
#include <asm/omap_common.h>
#include <asm/arch/sys_proto.h>

static const struct dmm_lisa_map_regs cl_som_am57x_lisa_regs = {
	.dmm_lisa_map_3 = 0x80740300,
	.is_ma_present  = 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	/* Disable SDRAM controller EMIF2 for single core SOC */
	*dmm_lisa_regs = &cl_som_am57x_lisa_regs;
	if (omap_revision() == DRA722_ES1_0) {
		((struct dmm_lisa_map_regs *) *dmm_lisa_regs)->dmm_lisa_map_3 =
		  0x80640100;
	}
}

static const struct emif_regs cl_som_am57x_emif1_ddr3_532mhz_emif_regs = {
	.sdram_config_init	= 0x61852332,
	.sdram_config		= 0x61852332,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000040f1,
	.ref_ctrl_final		= 0x00001040,
	.sdram_tim1		= 0xeeef36f3,
	.sdram_tim2		= 0x348f7fda,
	.sdram_tim3		= 0x027f88a8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x1007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x0034400b,
	.emif_ddr_phy_ctlr_1	= 0x0e34400b,
	.emif_ddr_ext_phy_ctrl_1 = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00740074,
	.emif_ddr_ext_phy_ctrl_3 = 0x00780078,
	.emif_ddr_ext_phy_ctrl_4 = 0x007c007c,
	.emif_ddr_ext_phy_ctrl_5 = 0x007b007b,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

/* Ext phy ctrl regs 1-35 */
static const u32 cl_som_am57x_emif1_ddr3_ext_phy_ctrl_regs[] = {
	0x10040100,
	0x00740074,
	0x00780078,
	0x007c007c,
	0x007b007b,
	0x00800080,
	0x00360036,
	0x00340034,
	0x00360036,
	0x00350035,
	0x00350035,

	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,

	0x00430043,
	0x003e003e,
	0x004a004a,
	0x00470047,
	0x00400040,

	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,

	0x00400040,
	0x00400040,
	0x00400040,
	0x00400040,
	0x00400040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

static const struct emif_regs cl_som_am57x_emif2_ddr3_532mhz_emif_regs = {
	.sdram_config_init	= 0x61852332,
	.sdram_config		= 0x61852332,
	.sdram_config2		= 0x00000000,
	.ref_ctrl		= 0x000040f1,
	.ref_ctrl_final		= 0x00001040,
	.sdram_tim1		= 0xeeef36f3,
	.sdram_tim2		= 0x348f7fda,
	.sdram_tim3		= 0x027f88a8,
	.read_idle_ctrl		= 0x00050000,
	.zq_config		= 0x1007190b,
	.temp_alert_config	= 0x00000000,
	.emif_ddr_phy_ctlr_1_init = 0x0034400b,
	.emif_ddr_phy_ctlr_1	= 0x0e34400b,
	.emif_ddr_ext_phy_ctrl_1 = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2 = 0x00740074,
	.emif_ddr_ext_phy_ctrl_3 = 0x00780078,
	.emif_ddr_ext_phy_ctrl_4 = 0x007c007c,
	.emif_ddr_ext_phy_ctrl_5 = 0x007b007b,
	.emif_rd_wr_lvl_rmp_win	= 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl	= 0x80000000,
	.emif_rd_wr_lvl_ctl	= 0x00000000,
	.emif_rd_wr_exec_thresh	= 0x00000305
};

static const u32 cl_som_am57x_emif2_ddr3_ext_phy_ctrl_regs[] = {
	0x10040100,
	0x00820082,
	0x008b008b,
	0x00800080,
	0x007e007e,
	0x00800080,
	0x00370037,
	0x00390039,
	0x00360036,
	0x00370037,
	0x00350035,
	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,
	0x01ff01ff,
	0x00540054,
	0x00540054,
	0x004e004e,
	0x004c004c,
	0x00400040,

	0x00000000,
	0x00600020,
	0x40011080,
	0x08102040,

	0x00400040,
	0x00400040,
	0x00400040,
	0x00400040,
	0x00400040,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0
};

static struct vcores_data cl_som_am57x_volts = {
	.mpu.value[OPP_NOM]	= VDD_MPU_DRA7_NOM,
	.mpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_MPU_NOM,
	.mpu.efuse.reg_bits     = DRA752_EFUSE_REGBITS,
	.mpu.addr		= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic		= &tps659038,

	.eve.value[OPP_NOM]	= VDD_EVE_DRA7_NOM,
	.eve.value[OPP_OD]	= VDD_EVE_DRA7_OD,
	.eve.value[OPP_HIGH]	= VDD_EVE_DRA7_HIGH,
	.eve.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_DSPEVE_NOM,
	.eve.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_DSPEVE_OD,
	.eve.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_DSPEVE_HIGH,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr		= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic		= &tps659038,

	.gpu.value[OPP_NOM]	= VDD_GPU_DRA7_NOM,
	.gpu.value[OPP_OD]	= VDD_GPU_DRA7_OD,
	.gpu.value[OPP_HIGH]	= VDD_GPU_DRA7_HIGH,
	.gpu.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_GPU_NOM,
	.gpu.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_GPU_OD,
	.gpu.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_GPU_HIGH,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr		= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic		= &tps659038,

	.core.value[OPP_NOM]	= VDD_CORE_DRA7_NOM,
	.core.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_CORE_NOM,
	.core.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.core.addr		= TPS659038_REG_ADDR_SMPS7,
	.core.pmic		= &tps659038,

	.iva.value[OPP_NOM]	= VDD_IVA_DRA7_NOM,
	.iva.value[OPP_OD]	= VDD_IVA_DRA7_OD,
	.iva.value[OPP_HIGH]	= VDD_IVA_DRA7_HIGH,
	.iva.efuse.reg[OPP_NOM]	= STD_FUSE_OPP_VMIN_IVA_NOM,
	.iva.efuse.reg[OPP_OD]	= STD_FUSE_OPP_VMIN_IVA_OD,
	.iva.efuse.reg[OPP_HIGH]	= STD_FUSE_OPP_VMIN_IVA_HIGH,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr		= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic		= &tps659038,
};

void hw_data_init(void)
{
	*prcm = &dra7xx_prcm;
	*dplls_data = &dra7xx_dplls;
	*omap_vcores = &cl_som_am57x_volts;
	*ctrl = &dra7xx_ctrl;
}

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	switch (emif_nr) {
	case 1:
		*regs = &cl_som_am57x_emif1_ddr3_532mhz_emif_regs;
		break;
	case 2:
		*regs = &cl_som_am57x_emif2_ddr3_532mhz_emif_regs;
		break;
	}
}

void emif_get_ext_phy_ctrl_const_regs(u32 emif_nr, const u32 **regs, u32 *size)
{
	switch (emif_nr) {
	case 1:
		*regs = cl_som_am57x_emif1_ddr3_ext_phy_ctrl_regs;
		*size = ARRAY_SIZE(cl_som_am57x_emif1_ddr3_ext_phy_ctrl_regs);
		break;
	case 2:
		*regs = cl_som_am57x_emif2_ddr3_ext_phy_ctrl_regs;
		*size = ARRAY_SIZE(cl_som_am57x_emif2_ddr3_ext_phy_ctrl_regs);
		break;
	}
}

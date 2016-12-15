/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated, <www.ti.com>
 *
 * Lokesh Vutla <lokeshvutla@ti.com>
 *
 * Based on previous work by:
 * Aneesh V       <aneesh@ti.com>
 * Steve Sakoman  <steve@sakoman.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <palmas.h>
#include <sata.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <usb.h>
#include <linux/usb/gadget.h>
#include <asm/omap_common.h>
#include <asm/omap_sec_common.h>
#include <asm/arch/gpio.h>
#include <asm/arch/dra7xx_iodelay.h>
#include <asm/emif.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sata.h>
#include <environment.h>
#include <dwc3-uboot.h>
#include <dwc3-omap-uboot.h>
#include <ti-usb-phy-uboot.h>
#include <miiphy.h>

#include "mux_data.h"
#include "../common/board_detect.h"

#define board_is_dra74x_evm()		board_ti_is("5777xCPU")
#define board_is_dra72x_evm()		board_ti_is("DRA72x-T")
#define board_is_dra74x_revh_or_later() board_is_dra74x_evm() &&	\
				(strncmp("H", board_ti_get_rev(), 1) <= 0)
#define board_is_dra72x_revc_or_later() board_is_dra72x_evm() &&	\
				(strncmp("C", board_ti_get_rev(), 1) <= 0)
#define board_ti_get_emif_size()	board_ti_get_emif1_size() +	\
					board_ti_get_emif2_size()

#ifdef CONFIG_DRIVER_TI_CPSW
#include <cpsw.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* GPIO 7_11 */
#define GPIO_DDR_VTT_EN 203

#define SYSINFO_BOARD_NAME_MAX_LEN	37

const struct omap_sysinfo sysinfo = {
	"Board: UNKNOWN(DRA7 EVM) REV UNKNOWN\n"
};

static const struct emif_regs emif1_ddr3_532_mhz_1cs = {
	.sdram_config_init              = 0x61851ab2,
	.sdram_config                   = 0x61851ab2,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const struct emif_regs emif2_ddr3_532_mhz_1cs = {
	.sdram_config_init              = 0x61851B32,
	.sdram_config                   = 0x61851B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

static const struct emif_regs emif_1_regs_ddr3_666_mhz_1cs_dra_es1 = {
	.sdram_config_init              = 0x61862B32,
	.sdram_config                   = 0x61862B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x0000514C,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD113781C,
	.sdram_tim2                     = 0x30717FE3,
	.sdram_tim3                     = 0x409F86A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400D,
	.emif_ddr_phy_ctlr_1            = 0x0E24400D,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00A400A4,
	.emif_ddr_ext_phy_ctrl_3        = 0x00A900A9,
	.emif_ddr_ext_phy_ctrl_4        = 0x00B000B0,
	.emif_ddr_ext_phy_ctrl_5        = 0x00B000B0,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif_1_regs_ddr3_666_mhz_1cs_dra_es2 = {
	.sdram_config_init              = 0x61862BB2,
	.sdram_config                   = 0x61862BB2,
	.sdram_config2			= 0x00000000,
	.ref_ctrl                       = 0x0000514D,
	.ref_ctrl_final			= 0x0000144A,
	.sdram_tim1                     = 0xD1137824,
	.sdram_tim2                     = 0x30B37FE3,
	.sdram_tim3                     = 0x409F8AD8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x5007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0824400E,
	.emif_ddr_phy_ctlr_1            = 0x0E24400E,
	.emif_ddr_ext_phy_ctrl_1        = 0x04040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x006B009F,
	.emif_ddr_ext_phy_ctrl_3        = 0x006B00A2,
	.emif_ddr_ext_phy_ctrl_4        = 0x006B00A8,
	.emif_ddr_ext_phy_ctrl_5        = 0x006B00A8,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif1_ddr3_532_mhz_1cs_2G = {
	.sdram_config_init              = 0x61851ab2,
	.sdram_config                   = 0x61851ab2,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x30BF7FDA,
	.sdram_tim3                     = 0x427F8BA8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

const struct emif_regs emif2_ddr3_532_mhz_1cs_2G = {
	.sdram_config_init              = 0x61851B32,
	.sdram_config                   = 0x61851B32,
	.sdram_config2			= 0x08000000,
	.ref_ctrl                       = 0x000040F1,
	.ref_ctrl_final			= 0x00001035,
	.sdram_tim1                     = 0xCCCF36B3,
	.sdram_tim2                     = 0x308F7FDA,
	.sdram_tim3                     = 0x427F88A8,
	.read_idle_ctrl                 = 0x00050000,
	.zq_config                      = 0x0007190B,
	.temp_alert_config              = 0x00000000,
	.emif_ddr_phy_ctlr_1_init       = 0x0024400B,
	.emif_ddr_phy_ctlr_1            = 0x0E24400B,
	.emif_ddr_ext_phy_ctrl_1        = 0x10040100,
	.emif_ddr_ext_phy_ctrl_2        = 0x00910091,
	.emif_ddr_ext_phy_ctrl_3        = 0x00950095,
	.emif_ddr_ext_phy_ctrl_4        = 0x009B009B,
	.emif_ddr_ext_phy_ctrl_5        = 0x009E009E,
	.emif_rd_wr_lvl_rmp_win         = 0x00000000,
	.emif_rd_wr_lvl_rmp_ctl         = 0x80000000,
	.emif_rd_wr_lvl_ctl             = 0x00000000,
	.emif_rd_wr_exec_thresh         = 0x00000305
};

void emif_get_reg_dump(u32 emif_nr, const struct emif_regs **regs)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	switch (omap_revision()) {
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
		switch (emif_nr) {
		case 1:
			if (ram_size > CONFIG_MAX_MEM_MAPPED)
				*regs = &emif1_ddr3_532_mhz_1cs_2G;
			else
				*regs = &emif1_ddr3_532_mhz_1cs;
			break;
		case 2:
			if (ram_size > CONFIG_MAX_MEM_MAPPED)
				*regs = &emif2_ddr3_532_mhz_1cs_2G;
			else
				*regs = &emif2_ddr3_532_mhz_1cs;
			break;
		}
		break;
	case DRA722_ES1_0:
	case DRA722_ES2_0:
		if (ram_size < CONFIG_MAX_MEM_MAPPED)
			*regs = &emif_1_regs_ddr3_666_mhz_1cs_dra_es1;
		else
			*regs = &emif_1_regs_ddr3_666_mhz_1cs_dra_es2;
		break;
	default:
		*regs = &emif1_ddr3_532_mhz_1cs;
	}
}

static const struct dmm_lisa_map_regs lisa_map_dra7_1536MB = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x80640300,
	.dmm_lisa_map_2 = 0xC0500220,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

static const struct dmm_lisa_map_regs lisa_map_2G_x_2 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80600100,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

const struct dmm_lisa_map_regs lisa_map_dra7_2GB = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80740300,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

/*
 * DRA722 EVM EMIF1 2GB CONFIGURATION
 * EMIF1 4 devices of 512Mb x 8 Micron
 */
const struct dmm_lisa_map_regs lisa_map_2G_x_4 = {
	.dmm_lisa_map_0 = 0x0,
	.dmm_lisa_map_1 = 0x0,
	.dmm_lisa_map_2 = 0x80700100,
	.dmm_lisa_map_3 = 0xFF020100,
	.is_ma_present	= 0x1
};

void emif_get_dmm_regs(const struct dmm_lisa_map_regs **dmm_lisa_regs)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	switch (omap_revision()) {
	case DRA752_ES1_0:
	case DRA752_ES1_1:
	case DRA752_ES2_0:
		if (ram_size > CONFIG_MAX_MEM_MAPPED)
			*dmm_lisa_regs = &lisa_map_dra7_2GB;
		else
			*dmm_lisa_regs = &lisa_map_dra7_1536MB;
		break;
	case DRA722_ES1_0:
	case DRA722_ES2_0:
	default:
		if (ram_size < CONFIG_MAX_MEM_MAPPED)
			*dmm_lisa_regs = &lisa_map_2G_x_2;
		else
			*dmm_lisa_regs = &lisa_map_2G_x_4;
		break;
	}
}

struct vcores_data dra752_volts = {
	.mpu.value	= VDD_MPU_DRA7,
	.mpu.efuse.reg	= STD_FUSE_OPP_VMIN_MPU,
	.mpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.mpu.addr	= TPS659038_REG_ADDR_SMPS12,
	.mpu.pmic	= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.eve.value	= VDD_EVE_DRA7,
	.eve.efuse.reg	= STD_FUSE_OPP_VMIN_DSPEVE,
	.eve.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.eve.addr	= TPS659038_REG_ADDR_SMPS45,
	.eve.pmic	= &tps659038,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.gpu.value	= VDD_GPU_DRA7,
	.gpu.efuse.reg	= STD_FUSE_OPP_VMIN_GPU,
	.gpu.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.gpu.addr	= TPS659038_REG_ADDR_SMPS6,
	.gpu.pmic	= &tps659038,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	.core.value	= VDD_CORE_DRA7,
	.core.efuse.reg	= STD_FUSE_OPP_VMIN_CORE,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= TPS659038_REG_ADDR_SMPS7,
	.core.pmic	= &tps659038,

	.iva.value	= VDD_IVA_DRA7,
	.iva.efuse.reg	= STD_FUSE_OPP_VMIN_IVA,
	.iva.efuse.reg_bits	= DRA752_EFUSE_REGBITS,
	.iva.addr	= TPS659038_REG_ADDR_SMPS8,
	.iva.pmic	= &tps659038,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

struct vcores_data dra722_volts = {
	.mpu.value	= VDD_MPU_DRA7,
	.mpu.efuse.reg	= STD_FUSE_OPP_VMIN_MPU,
	.mpu.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.mpu.addr	= TPS65917_REG_ADDR_SMPS1,
	.mpu.pmic	= &tps659038,
	.mpu.abb_tx_done_mask = OMAP_ABB_MPU_TXDONE_MASK,

	.core.value	= VDD_CORE_DRA7,
	.core.efuse.reg	= STD_FUSE_OPP_VMIN_CORE,
	.core.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.core.addr	= TPS65917_REG_ADDR_SMPS2,
	.core.pmic	= &tps659038,

	/*
	 * The DSPEVE, GPU and IVA rails are usually grouped on DRA72x
	 * designs and powered by TPS65917 SMPS3, as on the J6Eco EVM.
	 */
	.gpu.value	= VDD_GPU_DRA7,
	.gpu.efuse.reg	= STD_FUSE_OPP_VMIN_GPU,
	.gpu.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.gpu.addr	= TPS65917_REG_ADDR_SMPS3,
	.gpu.pmic	= &tps659038,
	.gpu.abb_tx_done_mask = OMAP_ABB_GPU_TXDONE_MASK,

	.eve.value	= VDD_EVE_DRA7,
	.eve.efuse.reg	= STD_FUSE_OPP_VMIN_DSPEVE,
	.eve.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.eve.addr	= TPS65917_REG_ADDR_SMPS3,
	.eve.pmic	= &tps659038,
	.eve.abb_tx_done_mask = OMAP_ABB_EVE_TXDONE_MASK,

	.iva.value	= VDD_IVA_DRA7,
	.iva.efuse.reg	= STD_FUSE_OPP_VMIN_IVA,
	.iva.efuse.reg_bits = DRA752_EFUSE_REGBITS,
	.iva.addr	= TPS65917_REG_ADDR_SMPS3,
	.iva.pmic	= &tps659038,
	.iva.abb_tx_done_mask = OMAP_ABB_IVA_TXDONE_MASK,
};

/**
 * @brief board_init
 *
 * @return 0
 */
int board_init(void)
{
	gpmc_init();
	gd->bd->bi_boot_params = (0x80000000 + 0x100); /* boot param addr */

	return 0;
}

void dram_init_banksize(void)
{
	u64 ram_size;

	ram_size = board_ti_get_emif_size();

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
	if (ram_size > CONFIG_MAX_MEM_MAPPED) {
		gd->bd->bi_dram[1].start = 0x200000000;
		gd->bd->bi_dram[1].size = ram_size - CONFIG_MAX_MEM_MAPPED;
	}
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char *name = "unknown";

	if (is_dra72x()) {
		if (board_is_dra72x_revc_or_later())
			name = "dra72x-revc";
		else
			name = "dra72x";
	} else {
		name = "dra7xx";
	}

	set_board_info_env(name);

	omap_die_id_serial();
#endif
	return 0;
}

#ifdef CONFIG_SPL_BUILD
void do_board_detect(void)
{
	int rc;

	rc = ti_i2c_eeprom_dra7_get(CONFIG_EEPROM_BUS_ADDRESS,
				    CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);
}

#else

void do_board_detect(void)
{
	char *bname = NULL;
	int rc;

	rc = ti_i2c_eeprom_dra7_get(CONFIG_EEPROM_BUS_ADDRESS,
				    CONFIG_EEPROM_CHIP_ADDRESS);
	if (rc)
		printf("ti_i2c_eeprom_init failed %d\n", rc);

	if (board_is_dra74x_evm()) {
		bname = "DRA74x EVM";
	} else if (board_is_dra72x_evm()) {
		bname = "DRA72x EVM";
	} else {
		/* If EEPROM is not populated */
		if (is_dra72x())
			bname = "DRA72x EVM";
		else
			bname = "DRA74x EVM";
	}

	if (bname)
		snprintf(sysinfo.board_string, SYSINFO_BOARD_NAME_MAX_LEN,
			 "Board: %s REV %s\n", bname, board_ti_get_rev());
}
#endif	/* CONFIG_SPL_BUILD */

void vcores_init(void)
{
	if (board_is_dra74x_evm()) {
		*omap_vcores = &dra752_volts;
	} else if (board_is_dra72x_evm()) {
		*omap_vcores = &dra722_volts;
	} else {
		/* If EEPROM is not populated */
		if (is_dra72x())
			*omap_vcores = &dra722_volts;
		else
			*omap_vcores = &dra752_volts;
	}
}

void set_muxconf_regs(void)
{
	do_set_mux32((*ctrl)->control_padconf_core_base,
		     early_padconf, ARRAY_SIZE(early_padconf));
}

#ifdef CONFIG_IODELAY_RECALIBRATION
void recalibrate_iodelay(void)
{
	struct pad_conf_entry const *pads, *delta_pads = NULL;
	struct iodelay_cfg_entry const *iodelay;
	int npads, niodelays, delta_npads = 0;
	int ret;

	switch (omap_revision()) {
	case DRA722_ES1_0:
	case DRA722_ES2_0:
		pads = dra72x_core_padconf_array_common;
		npads = ARRAY_SIZE(dra72x_core_padconf_array_common);
		if (board_is_dra72x_revc_or_later()) {
			delta_pads = dra72x_rgmii_padconf_array_revc;
			delta_npads =
				ARRAY_SIZE(dra72x_rgmii_padconf_array_revc);
			iodelay = dra72_iodelay_cfg_array_revc;
			niodelays = ARRAY_SIZE(dra72_iodelay_cfg_array_revc);
		} else {
			delta_pads = dra72x_rgmii_padconf_array_revb;
			delta_npads =
				ARRAY_SIZE(dra72x_rgmii_padconf_array_revb);
			iodelay = dra72_iodelay_cfg_array_revb;
			niodelays = ARRAY_SIZE(dra72_iodelay_cfg_array_revb);
		}
		break;
	case DRA752_ES1_0:
	case DRA752_ES1_1:
		pads = dra74x_core_padconf_array;
		npads = ARRAY_SIZE(dra74x_core_padconf_array);
		iodelay = dra742_es1_1_iodelay_cfg_array;
		niodelays = ARRAY_SIZE(dra742_es1_1_iodelay_cfg_array);
		break;
	default:
	case DRA752_ES2_0:
		pads = dra74x_core_padconf_array;
		npads = ARRAY_SIZE(dra74x_core_padconf_array);
		iodelay = dra742_es2_0_iodelay_cfg_array;
		niodelays = ARRAY_SIZE(dra742_es2_0_iodelay_cfg_array);
		/* Setup port1 and port2 for rgmii with 'no-id' mode */
		clrset_spare_register(1, 0, RGMII2_ID_MODE_N_MASK |
				      RGMII1_ID_MODE_N_MASK);
		break;
	}
	/* Setup I/O isolation */
	ret = __recalibrate_iodelay_start();
	if (ret)
		goto err;

	/* Do the muxing here */
	do_set_mux32((*ctrl)->control_padconf_core_base, pads, npads);

	/* Now do the weird minor deltas that should be safe */
	if (delta_npads)
		do_set_mux32((*ctrl)->control_padconf_core_base,
			     delta_pads, delta_npads);

	/* Setup IOdelay configuration */
	ret = do_set_iodelay((*ctrl)->iodelay_config_base, iodelay, niodelays);
err:
	/* Closeup.. remove isolation */
	__recalibrate_iodelay_end(ret);
}
#endif

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_GENERIC_MMC)
int board_mmc_init(bd_t *bis)
{
	omap_mmc_init(0, 0, 0, -1, -1);
	omap_mmc_init(1, 0, 0, -1, -1);
	return 0;
}
#endif

#ifdef CONFIG_USB_DWC3
static struct dwc3_device usb_otg_ss1 = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = DRA7_USB_OTG_SS1_BASE,
	.tx_fifo_resize = false,
	.index = 0,
};

static struct dwc3_omap_device usb_otg_ss1_glue = {
	.base = (void *)DRA7_USB_OTG_SS1_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 0,
};

static struct ti_usb_phy_device usb_phy1_device = {
	.pll_ctrl_base = (void *)DRA7_USB3_PHY1_PLL_CTRL,
	.usb2_phy_power = (void *)DRA7_USB2_PHY1_POWER,
	.usb3_phy_power = (void *)DRA7_USB3_PHY1_POWER,
	.index = 0,
};

static struct dwc3_device usb_otg_ss2 = {
	.maximum_speed = USB_SPEED_SUPER,
	.base = DRA7_USB_OTG_SS2_BASE,
	.tx_fifo_resize = false,
	.index = 1,
};

static struct dwc3_omap_device usb_otg_ss2_glue = {
	.base = (void *)DRA7_USB_OTG_SS2_GLUE_BASE,
	.utmi_mode = DWC3_OMAP_UTMI_MODE_SW,
	.index = 1,
};

static struct ti_usb_phy_device usb_phy2_device = {
	.usb2_phy_power = (void *)DRA7_USB2_PHY2_POWER,
	.index = 1,
};

int board_usb_init(int index, enum usb_init_type init)
{
	enable_usb_clocks(index);
	switch (index) {
	case 0:
		if (init == USB_INIT_DEVICE) {
			usb_otg_ss1.dr_mode = USB_DR_MODE_PERIPHERAL;
			usb_otg_ss1_glue.vbus_id_status = OMAP_DWC3_VBUS_VALID;
		} else {
			usb_otg_ss1.dr_mode = USB_DR_MODE_HOST;
			usb_otg_ss1_glue.vbus_id_status = OMAP_DWC3_ID_GROUND;
		}

		ti_usb_phy_uboot_init(&usb_phy1_device);
		dwc3_omap_uboot_init(&usb_otg_ss1_glue);
		dwc3_uboot_init(&usb_otg_ss1);
		break;
	case 1:
		if (init == USB_INIT_DEVICE) {
			usb_otg_ss2.dr_mode = USB_DR_MODE_PERIPHERAL;
			usb_otg_ss2_glue.vbus_id_status = OMAP_DWC3_VBUS_VALID;
		} else {
			usb_otg_ss2.dr_mode = USB_DR_MODE_HOST;
			usb_otg_ss2_glue.vbus_id_status = OMAP_DWC3_ID_GROUND;
		}

		ti_usb_phy_uboot_init(&usb_phy2_device);
		dwc3_omap_uboot_init(&usb_otg_ss2_glue);
		dwc3_uboot_init(&usb_otg_ss2);
		break;
	default:
		printf("Invalid Controller Index\n");
	}

	return 0;
}

int board_usb_cleanup(int index, enum usb_init_type init)
{
	switch (index) {
	case 0:
	case 1:
		ti_usb_phy_uboot_exit(index);
		dwc3_uboot_exit(index);
		dwc3_omap_uboot_exit(index);
		break;
	default:
		printf("Invalid Controller Index\n");
	}
	disable_usb_clocks(index);
	return 0;
}

int usb_gadget_handle_interrupts(int index)
{
	u32 status;

	status = dwc3_omap_uboot_interrupt_status(index);
	if (status)
		dwc3_uboot_handle_interrupt(index);

	return 0;
}
#endif

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_OS_BOOT)
int spl_start_uboot(void)
{
	/* break into full u-boot on 'c' */
	if (serial_tstc() && serial_getc() == 'c')
		return 1;

#ifdef CONFIG_SPL_ENV_SUPPORT
	env_init();
	env_relocate_spec();
	if (getenv_yesno("boot_os") != 1)
		return 1;
#endif

	return 0;
}
#endif

#ifdef CONFIG_DRIVER_TI_CPSW
extern u32 *const omap_si_rev;

static void cpsw_control(int enabled)
{
	/* VTP can be added here */

	return;
}

static struct cpsw_slave_data cpsw_slaves[] = {
	{
		.slave_reg_ofs	= 0x208,
		.sliver_reg_ofs	= 0xd80,
		.phy_addr	= 2,
	},
	{
		.slave_reg_ofs	= 0x308,
		.sliver_reg_ofs	= 0xdc0,
		.phy_addr	= 3,
	},
};

static struct cpsw_platform_data cpsw_data = {
	.mdio_base		= CPSW_MDIO_BASE,
	.cpsw_base		= CPSW_BASE,
	.mdio_div		= 0xff,
	.channels		= 8,
	.cpdma_reg_ofs		= 0x800,
	.slaves			= 2,
	.slave_data		= cpsw_slaves,
	.ale_reg_ofs		= 0xd00,
	.ale_entries		= 1024,
	.host_port_reg_ofs	= 0x108,
	.hw_stats_reg_ofs	= 0x900,
	.bd_ram_ofs		= 0x2000,
	.mac_control		= (1 << 5),
	.control		= cpsw_control,
	.host_port_num		= 0,
	.version		= CPSW_CTRL_VERSION_2,
};

int board_eth_init(bd_t *bis)
{
	int ret;
	uint8_t mac_addr[6];
	uint32_t mac_hi, mac_lo;
	uint32_t ctrl_val;

	/* try reading mac address from efuse */
	mac_lo = readl((*ctrl)->control_core_mac_id_0_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_0_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!getenv("ethaddr")) {
		printf("<ethaddr> not set. Validating first E-fuse MAC\n");

		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("ethaddr", mac_addr);
	}

	mac_lo = readl((*ctrl)->control_core_mac_id_1_lo);
	mac_hi = readl((*ctrl)->control_core_mac_id_1_hi);
	mac_addr[0] = (mac_hi & 0xFF0000) >> 16;
	mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	mac_addr[2] = mac_hi & 0xFF;
	mac_addr[3] = (mac_lo & 0xFF0000) >> 16;
	mac_addr[4] = (mac_lo & 0xFF00) >> 8;
	mac_addr[5] = mac_lo & 0xFF;

	if (!getenv("eth1addr")) {
		if (is_valid_ethaddr(mac_addr))
			eth_setenv_enetaddr("eth1addr", mac_addr);
	}

	ctrl_val = readl((*ctrl)->control_core_control_io1) & (~0x33);
	ctrl_val |= 0x22;
	writel(ctrl_val, (*ctrl)->control_core_control_io1);

	if (*omap_si_rev == DRA722_ES1_0)
		cpsw_data.active_slave = 1;

	if (board_is_dra72x_revc_or_later()) {
		cpsw_slaves[0].phy_if = PHY_INTERFACE_MODE_RGMII_ID;
		cpsw_slaves[1].phy_if = PHY_INTERFACE_MODE_RGMII_ID;
	}

	ret = cpsw_register(&cpsw_data);
	if (ret < 0)
		printf("Error %d registering CPSW switch\n", ret);

	return ret;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
/* VTT regulator enable */
static inline void vtt_regulator_enable(void)
{
	if (omap_hw_init_context() == OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL)
		return;

	/* Do not enable VTT for DRA722 */
	if (is_dra72x())
		return;

	/*
	 * EVM Rev G and later use gpio7_11 for DDR3 termination.
	 * This is safe enough to do on older revs.
	 */
	gpio_request(GPIO_DDR_VTT_EN, "ddr_vtt_en");
	gpio_direction_output(GPIO_DDR_VTT_EN, 1);
}

int board_early_init_f(void)
{
	vtt_regulator_enable();
	return 0;
}
#endif

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	if (is_dra72x() && !strcmp(name, "dra72-evm"))
		return 0;
	else if (!is_dra72x() && !strcmp(name, "dra7-evm"))
		return 0;
	else
		return -1;
}
#endif

#ifdef CONFIG_TI_SECURE_DEVICE
void board_fit_image_post_process(void **p_image, size_t *p_size)
{
	secure_boot_verify_image(p_image, p_size);
}
#endif

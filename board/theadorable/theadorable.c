/*
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_NET
#include <netdev.h>
#endif

#include "../drivers/ddr/marvell/axp/ddr3_hw_training.h"
#include "../arch/arm/mach-mvebu/serdes/axp/high_speed_env_spec.h"

DECLARE_GLOBAL_DATA_PTR;

#define THEADORABLE_GPP_OUT_ENA_LOW	0x00336780
#define THEADORABLE_GPP_OUT_ENA_MID	0x00003cf0
#define THEADORABLE_GPP_OUT_ENA_HIGH	(~(0x0))

#define THEADORABLE_GPP_OUT_VAL_LOW	0x2c0c983f
#define THEADORABLE_GPP_OUT_VAL_MID	0x0007000c
#define THEADORABLE_GPP_OUT_VAL_HIGH	0x00000000

/* DDR3 static configuration */
static MV_DRAM_MC_INIT ddr3_theadorable[MV_MAX_DDR3_STATIC_SIZE] = {
	{0x00001400, 0x7301ca28},	/* DDR SDRAM Configuration Register */
	{0x00001404, 0x30000800},	/* Dunit Control Low Register */
	{0x00001408, 0x44149887},	/* DDR SDRAM Timing (Low) Register */
	{0x0000140C, 0x38d93fc7},	/* DDR SDRAM Timing (High) Register */
	{0x00001410, 0x1b100001},	/* DDR SDRAM Address Control Register */
	{0x00001424, 0x0000f3ff},	/* Dunit Control High Register */
	{0x00001428, 0x000f8830},	/* ODT Timing (Low) Register */
	{0x0000142C, 0x014c50f4},	/* DDR3 Timing Register */
	{0x0000147C, 0x0000c671},	/* ODT Timing (High) Register */

	{0x00001494, 0x00010000},	/* DDR SDRAM ODT Control (Low) Reg */
	{0x0000149C, 0x00000001},	/* DDR Dunit ODT Control Register */
	{0x000014A0, 0x00000001},	/* DRAM FIFO Control Register */
	{0x000014A8, 0x00000101},	/* AXI Control Register */

	/*
	 * DO NOT Modify - Open Mbus Window - 2G - Mbus is required for the
	 * training sequence
	 */
	{0x000200e8, 0x3fff0e01},
	{0x00020184, 0x3fffffe0},	/* Close fast path Window to - 2G */

	{0x0001504, 0x7fffffe1},	/* CS0 Size */
	{0x000150C, 0x00000000},	/* CS1 Size */
	{0x0001514, 0x00000000},	/* CS2 Size */
	{0x000151C, 0x00000000},	/* CS3 Size */

	{0x00020220, 0x00000007},	/* Reserved */

	{0x00001538, 0x00000009},	/* Read Data Sample Delays Register */
	{0x0000153C, 0x00000009},	/* Read Data Ready Delay Register */

	{0x000015D0, 0x00000650},	/* MR0 */
	{0x000015D4, 0x00000044},	/* MR1 */
	{0x000015D8, 0x00000010},	/* MR2 */
	{0x000015DC, 0x00000000},	/* MR3 */
	{0x000015E0, 0x00000001},
	{0x000015E4, 0x00203c18},	/* ZQDS Configuration Register */
	{0x000015EC, 0xf800a225},	/* DDR PHY */

	/* Recommended Settings from Marvell for 4 x 16 bit devices: */
	{0x000014C0, 0x192424c9},	/* DRAM addr and Ctrl Driving Strenght*/
	{0x000014C4, 0x0aaa24c9},	/* DRAM Data and DQS Driving Strenght */

	{0x0, 0x0}
};

static MV_DRAM_MODES board_ddr_modes[MV_DDR3_MODES_NUMBER] = {
	{"theadorable_1333-667", 0x3, 0x5, 0x0, A0, ddr3_theadorable,  NULL},
};

extern MV_SERDES_CHANGE_M_PHY serdes_change_m_phy[];

/*
 * Lane0 - PCIE0.0 X1 (to WIFI Module)
 * Lane5 - SATA0
 * Lane6 - SATA1
 * Lane7 - SGMII0 (to Ethernet Phy)
 * Lane8-11 - PCIE2.0 X4 (to PEX Switch)
 * all other lanes are disabled
 */
MV_BIN_SERDES_CFG theadorable_serdes_cfg[] = {
	{ MV_PEX_ROOT_COMPLEX, 0x22200001, 0x00001111,
	  { PEX_BUS_MODE_X1, PEX_BUS_DISABLED, PEX_BUS_MODE_X4,
	    PEX_BUS_DISABLED },
	  0x0060, serdes_change_m_phy
	},
};

MV_DRAM_MODES *ddr3_get_static_ddr_mode(void)
{
	/* Only one mode supported for this board */
	return &board_ddr_modes[0];
}

MV_BIN_SERDES_CFG *board_serdes_cfg_get(u8 pex_mode)
{
	return &theadorable_serdes_cfg[0];
}

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x00000000, MVEBU_MPP_BASE + 0x00);
	writel(0x03300000, MVEBU_MPP_BASE + 0x04);
	writel(0x00000033, MVEBU_MPP_BASE + 0x08);
	writel(0x00000000, MVEBU_MPP_BASE + 0x0c);
	writel(0x11110000, MVEBU_MPP_BASE + 0x10);
	writel(0x00221100, MVEBU_MPP_BASE + 0x14);
	writel(0x00000000, MVEBU_MPP_BASE + 0x18);
	writel(0x00000000, MVEBU_MPP_BASE + 0x1c);
	writel(0x00000000, MVEBU_MPP_BASE + 0x20);

	/* Configure GPIO */
	writel(THEADORABLE_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(THEADORABLE_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
	writel(THEADORABLE_GPP_OUT_VAL_HIGH, MVEBU_GPIO2_BASE + 0x00);
	writel(THEADORABLE_GPP_OUT_ENA_HIGH, MVEBU_GPIO2_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: theadorable\n");

	return 0;
}

#ifdef CONFIG_NET
int board_eth_init(bd_t *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
#endif

int board_video_init(void)
{
	struct mvebu_lcd_info lcd_info;

	/* Reserved memory area via CONFIG_SYS_MEM_TOP_HIDE */
	lcd_info.fb_base	= gd->ram_size;
	lcd_info.x_res		= 240;
	lcd_info.x_fp		= 1;
	lcd_info.x_bp		= 45;
	lcd_info.y_res		= 320;
	lcd_info.y_fp		= 1;
	lcd_info.y_bp		= 3;

	return mvebu_lcd_register_init(&lcd_info);
}

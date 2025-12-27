// SPDX-License-Identifier: GPL-2.0+

#include "spl_mtypes.h"

static const struct dram_cfg_param ddr_ddrc_cfg_256mb[] = {
	/*  IOMUX */

	/* DDR IO TYPE: */
	{0x020E04B4, 0x000C0000}, /*  IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE */
	{0x020E04AC, 0x00000000}, /*  IOMUXC_SW_PAD_CTL_GRP_DDRPKE */

	/* CLOCK: */
	{0x020E027C, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK0_P */

	/* Control: */
	{0x020E0250, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS */
	{0x020E024C, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS */
	{0x020E0490, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_GRP_ADDDS */
	{0x020E0288, 0x000C0030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET */
	{0x020E0270, 0x00000000}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured */
				  /*  using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS */
	{0x020E0260, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_ODT0 */
	{0x020E0264, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_ODT1 */
	{0x020E04A0, 0x00000028}, /*  IOMUXC_SW_PAD_CTL_GRP_CTLDS */

	/* Data Strobes: */
	{0x020E0494, 0x00020000}, /*  IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL */
	{0x020E0280, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0_P */
	{0x020E0284, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1_P */

	/* Data: */
	{0x020E04B0, 0x00020000}, /*  IOMUXC_SW_PAD_CTL_GRP_DDRMODE */
	{0x020E0498, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_GRP_B0DS */
	{0x020E04A4, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_GRP_B1DS */
	{0x020E0244, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0 */
	{0x020E0248, 0x00000030}, /*  IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1 */

	/* ============================================================================= */
	/*  DDR Controller Registers */
	/* ============================================================================= */
	/*  Manufacturer:WINBOND */
	/*  Device Part Number:W632GU6RB-11 */
	/*  Clock Freq.: 400MHz */
	/*  Density per CS in Gb: 2 */
	/*  Chip Selects used:1 */
	/*  Total DRAM density (Gb)2 */
	/*  Number of Banks:8 */
	/*  Row address:    14 */
	/*  Column address: 10 */
	/*  Data bus width16 */
	/* ============================================================================= */
	{0x021B001C, 0x00008000}, /*  [MMDC_MDSCR] MMDC Core Special Command Register */

	/* ====================================================== */
	/* Calibrations: */
	/* ====================================================== */
	{0x021B0800, 0xA1390003}, /*  DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic */
				  /*  HW ZQ calibration. */

	{0x021B080C, 0x00070005}, /*  [MMDC_MPWLDECTRL0] MMDC PHY Write Leveling Delay Control */
				  /*  Register 0 */
	{0x021B083C, 0x414C0150}, /*  [MMDC_MPDGCTRL0] MMDC PHY Read DQS Gating Control */
				  /* Register 0 */
	{0x021B0848, 0x4040383E}, /*  [MMDC_MPRDDLCTL] MMDC PHY Read delay-lines Configuration */
				  /*  Register */
	{0x021B0850, 0x40402E2A}, /*  [MMDC_MPWRDLCTL] MMDC PHY Write delay-lines Configuration */
				  /*  Register */

	{0x021B081C, 0x33333333}, /*  [MMDC_MPRDDQBY0DL] MMDC PHY Read DQ Byte0 Delay Register */
	{0x021B0820, 0x33333333}, /*  [MMDC_MPRDDQBY1DL] MMDC PHY Read DQ Byte1 Delay Register */

	{0x021B082C, 0xf3333333}, /*  [MMDC_MPWRDQBY0DL] MMDC PHY Write DQ Byte0 Delay Register */
	{0x021B0830, 0xf3333333}, /*  [MMDC_MPWRDQBY1DL] MMDC PHY Write DQ Byte1 Delay Register */

	{0x021B08C0, 0x00944009}, /*  [MMDC_MPDCCR] MMDC Duty Cycle Control Register */

	/*  Complete calibration by forced measurement: */
	{0x021B08B8, 0x00000800}, /*  DDR_PHY_P0_MPMUR0, frc_msr */

	/* ====================================================== */
	/* MMDC init: */
	/* ====================================================== */
	{0x021B0004, 0x00020024}, /*  MMDC0_MDPDC */
	{0x021B0008, 0x1B333030}, /*  MMDC0_MDOTC */
	{0x021B000C, 0x3F4352D3}, /*  MMDC0_MDCFG0 */
	{0x021B0010, 0xB66D0A63}, /*  MMDC0_MDCFG1 */
	{0x021B0014, 0x01FF00DB}, /*  MMDC0_MDCFG2 */
	{0x021B0018, 0x00201740}, /*  MMDC0_MDMISC */
	{0x021B002C, 0x000026D2}, /*  MMDC0_MDRWD; recommend to maintain the default values */
	{0x021B0030, 0x00431023}, /*  MMDC0_MDOR */
	{0x021B0040, 0x00000047}, /*  CS0_END */
	{0x021B0000, 0x83180000}, /*  MMDC0_MDCTL */

	/*  Mode register writes for CS0 */
	{0x021B001C, 0x02808032}, /*  MMDC0_MDSCR, MR2 write, CS0 */
	{0x021B001C, 0x00008033}, /*  MMDC0_MDSCR, MR3 write, CS0 */
	{0x021B001C, 0x00048031}, /*  MMDC0_MDSCR, MR1 write, CS0 */
	{0x021B001C, 0x15208030}, /*  MMDC0_MDSCR, MR0 write, CS0 */
	{0x021B001C, 0x04008040}, /*  MMDC0_MDSCR, ZQ calibration command sent to device on CS0 */

	/*  Mode register writes for CS, commented out automatically */
	/*  if only one chip select used */
	/*     {0x021B001C, 0x0200803A},  */
	/*     {0x021B001C, 0x0000803B},  */
	/*     {0x021B001C, 0x00048039},  */
	/*     {0x021B001C, 0x15208038},  */
	/*     {0x021B001C, 0x04008048},  */

	/*  final DDR setup, before operation start: */
	{0x021B0020, 0x00000800}, /*  MMDC0_MDREF */
	{0x021B0818, 0x00000227}, /*  DDR_PHY_P0_MPODTCTRL */
	{0x021B0004, 0x00025564}, /*  MMDC0_MDPDC now SDCTL power down enabled */
	{0x021B0404, 0x00011006}, /*  MMDC0_MAPSR ADOPT power down enabled */
	{0x021B001C, 0x00000000}, /*  MMDC0_MDSCR, clear this register (especially the  */
				  /*  configuration bit as initialization is complete) */
};

struct dram_timing_info bsh_dram_timing_256mb = {
	.ddrc_cfg = ddr_ddrc_cfg_256mb,
	.ddrc_cfg_num = ARRAY_SIZE(ddr_ddrc_cfg_256mb),
	.dram_size = SZ_256M,
};

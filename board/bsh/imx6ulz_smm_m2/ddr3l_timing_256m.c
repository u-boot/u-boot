// SPDX-License-Identifier: GPL-2.0+

#include "spl_mtypes.h"

static const struct dram_cfg_param ddr_ddrc_cfg_256mb[] = {
	/* IOMUX */

	/* DDR IO Type: */
	{0x020e04b4, 0x000C0000},	/* IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE */
	{0x020e04ac, 0x00000000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRPKE */

	/* Clock: */
	{0x020e027c, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0 */

	/* Address: */
	{0x020e0250, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS */
	{0x020e024c, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS */
	{0x020e0490, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_ADDDS */

	/* Control: */
	{0x020e0288, 0x000C0028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET */
	{0x020e0270, 0x00000000},	/*
					 * IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured
					 * using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS
					 */

	{0x020e0260, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0 */
	{0x020e0264, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1 */
	{0x020e04a0, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_CTLDS */

	/* Data Strobes: */
	{0x020e0494, 0x00020000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL */
	{0x020e0280, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0 */
	{0x020e0284, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1 */

	/* Data: */
	{0x020e04b0, 0x00020000},	/* IOMUXC_SW_PAD_CTL_GRP_DDRMODE */
	{0x020e0498, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_B0DS */
	{0x020e04a4, 0x00000028},	/* IOMUXC_SW_PAD_CTL_GRP_B1DS */

	{0x020e0244, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0 */
	{0x020e0248, 0x00000028},	/* IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1 */

	/*
	 * =============================================================================
	 * DDR Controller Registers
	 * =============================================================================
	 * Manufacturer:ISSI
	 * Device Part Number:IS43TR16640BL-125JBLI
	 * Clock Freq.: 400MHz
	 * Density per CS in Gb: 2
	 * Chip Selects used:1
	 * Number of Banks:8
	 * Row address:    14
	 * Column address: 10
	 * Data bus width16
	 * =============================================================================
	 */
	{0x021b001c, 0x00008000},	/*
					 * MMDC0_MDSCR, set the Configuration request bit during
					 * MMDC set up
					 */

	/*
	 * =============================================================================
	 * Calibration setup.
	 * =============================================================================
	 */
	{0x021b0800, 0xA1390003},	/*
					 * DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic
					 * HW ZQ calibration
					 */

	/*
	 * For target board, may need to run write leveling calibration to fine tune these settings
	 */
	{0x021b080c, 0x00050005},

	/* Read DQS Gating calibration */
	{0x021b083c, 0x01480144},	/* MPDGCTRL0 PHY0 */

	/* Read calibration */
	{0x021b0848, 0x4040363A},	/* MPRDDLCTL PHY0 */

	/* Write calibration */
	{0x021b0850, 0x40402E2C},	/* MPWRDLCTL PHY0 */

	/*
	 * Read data bit delay: 3 is the reccommended default value, although out of reset value
	 * is 0
	 */
	{0x021b081c, 0x33333333},	/* MMDC_MPRDDQBY0DL */
	{0x021b0820, 0x33333333},	/* MMDC_MPRDDQBY1DL */

	/* Write data bit delay: */
	{0x021b082c, 0xF3333333},	/* MMDC_MPWRDQBY0DL */
	{0x021b0830, 0xF3333333},	/* MMDC_MPWRDQBY1DL */

	/* DQS&CLK Duty Cycle */
	{0x021b08c0, 0x00944009},	/* [MMDC_MPDCCR] MMDC Duty Cycle Control Register */

	/* Complete calibration by forced measurement: */
	{0x021b08b8, 0x00000800},	/* DDR_PHY_P0_MPMUR0, frc_msr */

	/*
	 * =============================================================================
	 * Calibration setup end
	 * =============================================================================
	 */

	/* MMDC init: */
	{0x021b0004, 0x0002002D},	/* MMDC0_MDPDC */
	{0x021b0008, 0x1B333030},	/* MMDC0_MDOTC */
	{0x021b000c, 0x3F435333},	/* MMDC0_MDCFG0 */
	{0x021b0010, 0xB68E0B63},	/* MMDC0_MDCFG1 */
	{0x021b0014, 0x01FF00DB},	/* MMDC0_MDCFG2 */

	/*
	 * MDMISC: RALAT kept to the high level of 5.
	 * MDMISC: consider reducing RALAT if your 528MHz board design allow that.
	 * Lower RALAT benefits:
	 * a. better operation at low frequency, for LPDDR2 freq < 100MHz, change RALAT to 3
	 * b. Small performence improvment
	 */
	{0x021b0018, 0x00211740},	/* MMDC0_MDMISC */
	{0x021b001c, 0x00008000},	/*
					 * MMDC0_MDSCR, set the Configuration request bit during
					 * MMDC set up
					 */
	{0x021b002c, 0x000026D2},	/* MMDC0_MDRWD */
	{0x021b0030, 0x00431023},	/* MMDC0_MDOR */
	{0x021b0040, 0x00000047},	/* Chan0 CS0_END */
	{0x021b0000, 0x83180000},	/* MMDC0_MDCTL */

	{0x021b0890, 0x00400000},	/* MPPDCMPR2 */

	/* Mode register writes */
	{0x021b001c, 0x02808032},	/* MMDC0_MDSCR, MR2 write, CS0 */
	{0x021b001c, 0x00008033},	/* MMDC0_MDSCR, MR3 write, CS0 */
	{0x021b001c, 0x00048031},	/* MMDC0_MDSCR, MR1 write, CS0 */
	{0x021b001c, 0x15208030},	/* MMDC0_MDSCR, MR0write, CS0 */
	{0x021b001c, 0x04008040},	/*
					 * MMDC0_MDSCR, ZQ calibration command sent to device
					 * on CS0
					 */

	{0x021b0020, 0x00007800},	/* MMDC0_MDREF */

	{0x021b0818, 0x00000227},	/* DDR_PHY_P0_MPODTCTRL */

	{0x021b0004, 0x0002552D},	/* MMDC0_MDPDC now SDCTL power down enabled */

	{0x021b0404, 0x00011006},	/*
					 * MMDC0_MAPSR ADOPT power down enabled, MMDC will enter
					 * automatically to self-refresh while the number of idle
					 * cycle reached
					 */

	{0x021b001c, 0x00000000},	/*
					 * MMDC0_MDSCR, clear this register (especially the
					 * configuration bit as initialization is complete)
					 */
};

struct dram_timing_info bsh_dram_timing_256mb = {
	.ddrc_cfg = ddr_ddrc_cfg_256mb,
	.ddrc_cfg_num = ARRAY_SIZE(ddr_ddrc_cfg_256mb),
	.dram_size = SZ_256M,
};

/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef	_CLOCK_MANAGER_H_
#define	_CLOCK_MANAGER_H_

typedef struct {
	/* main group */
	uint32_t main_vco_base;
	uint32_t mpuclk;
	uint32_t mainclk;
	uint32_t dbgatclk;
	uint32_t mainqspiclk;
	uint32_t mainnandsdmmcclk;
	uint32_t cfg2fuser0clk;
	uint32_t maindiv;
	uint32_t dbgdiv;
	uint32_t tracediv;
	uint32_t l4src;

	/* peripheral group */
	uint32_t peri_vco_base;
	uint32_t emac0clk;
	uint32_t emac1clk;
	uint32_t perqspiclk;
	uint32_t pernandsdmmcclk;
	uint32_t perbaseclk;
	uint32_t s2fuser1clk;
	uint32_t perdiv;
	uint32_t gpiodiv;
	uint32_t persrc;

	/* sdram pll group */
	uint32_t sdram_vco_base;
	uint32_t ddrdqsclk;
	uint32_t ddr2xdqsclk;
	uint32_t ddrdqclk;
	uint32_t s2fuser2clk;
} cm_config_t;

extern void cm_basic_init(const cm_config_t *cfg);

struct socfpga_clock_manager {
	u32	ctrl;
	u32	bypass;
	u32	inter;
	u32	intren;
	u32	dbctrl;
	u32	stat;
	u32	_pad_0x18_0x3f[10];
	u32	mainpllgrp;
	u32	perpllgrp;
	u32	sdrpllgrp;
	u32	_pad_0xe0_0x200[72];

	u32	main_pll_vco;
	u32	main_pll_misc;
	u32	main_pll_mpuclk;
	u32	main_pll_mainclk;
	u32	main_pll_dbgatclk;
	u32	main_pll_mainqspiclk;
	u32	main_pll_mainnandsdmmcclk;
	u32	main_pll_cfgs2fuser0clk;
	u32	main_pll_en;
	u32	main_pll_maindiv;
	u32	main_pll_dbgdiv;
	u32	main_pll_tracediv;
	u32	main_pll_l4src;
	u32	main_pll_stat;
	u32	main_pll__pad_0x38_0x40[2];

	u32	per_pll_vco;
	u32	per_pll_misc;
	u32	per_pll_emac0clk;
	u32	per_pll_emac1clk;
	u32	per_pll_perqspiclk;
	u32	per_pll_pernandsdmmcclk;
	u32	per_pll_perbaseclk;
	u32	per_pll_s2fuser1clk;
	u32	per_pll_en;
	u32	per_pll_div;
	u32	per_pll_gpiodiv;
	u32	per_pll_src;
	u32	per_pll_stat;
	u32	per_pll__pad_0x34_0x40[3];

	u32	sdr_pll_vco;
	u32	sdr_pll_ctrl;
	u32	sdr_pll_ddrdqsclk;
	u32	sdr_pll_ddr2xdqsclk;
	u32	sdr_pll_ddrdqclk;
	u32	sdr_pll_s2fuser2clk;
	u32	sdr_pll_en;
	u32	sdr_pll_stat;
};

#define CLKMGR_MAINPLLGRP_EN_S2FUSER0CLK_MASK 0x00000200
#define CLKMGR_MAINPLLGRP_EN_DBGTIMERCLK_MASK 0x00000080
#define CLKMGR_MAINPLLGRP_EN_DBGTRACECLK_MASK 0x00000040
#define CLKMGR_MAINPLLGRP_EN_DBGCLK_MASK 0x00000020
#define CLKMGR_MAINPLLGRP_EN_DBGATCLK_MASK 0x00000010
#define CLKMGR_MAINPLLGRP_EN_L4MPCLK_MASK 0x00000004
#define CLKMGR_MAINPLLGRP_VCO_RESET_VALUE 0x8001000d
#define CLKMGR_PERPLLGRP_VCO_RESET_VALUE 0x8001000d
#define CLKMGR_SDRPLLGRP_VCO_RESET_VALUE 0x8001000d
#define CLKMGR_MAINPLLGRP_MAINDIV_L4MPCLK_SET(x) (((x) << 4) & 0x00000070)
#define CLKMGR_MAINPLLGRP_MAINDIV_L4SPCLK_SET(x)  (((x) << 7) & 0x00000380)
#define CLKMGR_MAINPLLGRP_L4SRC_L4MP_SET(x) (((x) << 0) & 0x00000001)
#define CLKMGR_MAINPLLGRP_L4SRC_L4SP_SET(x) (((x) << 1) & 0x00000002)
#define CLKMGR_PERPLLGRP_SRC_QSPI_SET(x) (((x) << 4) & 0x00000030)
#define CLKMGR_PERPLLGRP_SRC_NAND_SET(x) (((x) << 2) & 0x0000000c)
#define CLKMGR_PERPLLGRP_SRC_SDMMC_SET(x) (((x) << 0) & 0x00000003)
#define CLKMGR_MAINPLLGRP_VCO_DENOM_SET(x) (((x) << 16) & 0x003f0000)
#define CLKMGR_MAINPLLGRP_VCO_NUMER_SET(x) (((x) << 3) & 0x0000fff8)
#define CLKMGR_MAINPLLGRP_VCO_PWRDN_SET(x) (((x) << 2) & 0x00000004)
#define CLKMGR_MAINPLLGRP_VCO_EN_SET(x) (((x) << 1) & 0x00000002)
#define CLKMGR_MAINPLLGRP_VCO_BGPWRDN_SET(x) (((x) << 0) & 0x00000001)
#define CLKMGR_PERPLLGRP_VCO_PSRC_SET(x) (((x) << 22) & 0x00c00000)
#define CLKMGR_PERPLLGRP_VCO_DENOM_SET(x) (((x) << 16) & 0x003f0000)
#define CLKMGR_PERPLLGRP_VCO_NUMER_SET(x) (((x) << 3) & 0x0000fff8)
#define CLKMGR_SDRPLLGRP_VCO_OUTRESET_SET(x) (((x) << 25) & 0x7e000000)
#define CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(x) (((x) << 24) & 0x01000000)
#define CLKMGR_SDRPLLGRP_VCO_SSRC_SET(x) (((x) << 22) & 0x00c00000)
#define CLKMGR_SDRPLLGRP_VCO_DENOM_SET(x) (((x) << 16) & 0x003f0000)
#define CLKMGR_SDRPLLGRP_VCO_NUMER_SET(x) (((x) << 3) & 0x0000fff8)
#define CLKMGR_MAINPLLGRP_MPUCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_MAINCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_DBGATCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_CFGS2FUSER0CLK_CNT_SET(x) \
	(((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_EMAC0CLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_EMAC1CLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_MAINQSPICLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_MAINNANDSDMMCCLK_CNT_SET(x) \
	(((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_PERBASECLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_S2FUSER1CLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_PERNANDSDMMCCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_SDRPLLGRP_DDRDQSCLK_PHASE_SET(x) (((x) << 9) & 0x00000e00)
#define CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_SDRPLLGRP_DDR2XDQSCLK_PHASE_SET(x) (((x) << 9) & 0x00000e00)
#define CLKMGR_SDRPLLGRP_DDR2XDQSCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_SDRPLLGRP_DDRDQCLK_PHASE_SET(x) (((x) << 9) & 0x00000e00)
#define CLKMGR_SDRPLLGRP_DDRDQCLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_SDRPLLGRP_S2FUSER2CLK_PHASE_SET(x) (((x) << 9) & 0x00000e00)
#define CLKMGR_SDRPLLGRP_S2FUSER2CLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_MAINPLLGRP_DBGDIV_DBGCLK_SET(x) (((x) << 2) & 0x0000000c)
#define CLKMGR_MAINPLLGRP_DBGDIV_DBGATCLK_SET(x) (((x) << 0) & 0x00000003)
#define CLKMGR_MAINPLLGRP_TRACEDIV_TRACECLK_SET(x) (((x) << 0) & 0x00000007)
#define CLKMGR_MAINPLLGRP_MAINDIV_L3MPCLK_SET(x) (((x) << 0) & 0x00000003)
#define CLKMGR_MAINPLLGRP_MAINDIV_L3SPCLK_SET(x) (((x) << 2) & 0x0000000c)
#define CLKMGR_BYPASS_PERPLL_SET(x) (((x) << 3) & 0x00000008)
#define CLKMGR_BYPASS_SDRPLL_SET(x) (((x) << 1) & 0x00000002)
#define CLKMGR_BYPASS_MAINPLL_SET(x) (((x) << 0) & 0x00000001)
#define CLKMGR_PERPLLGRP_DIV_USBCLK_SET(x) (((x) << 0) & 0x00000007)
#define CLKMGR_PERPLLGRP_DIV_SPIMCLK_SET(x) (((x) << 3) & 0x00000038)
#define CLKMGR_PERPLLGRP_DIV_CAN0CLK_SET(x) (((x) << 6) & 0x000001c0)
#define CLKMGR_PERPLLGRP_DIV_CAN1CLK_SET(x) (((x) << 9) & 0x00000e00)
#define CLKMGR_INTER_SDRPLLLOCKED_MASK 0x00000100
#define CLKMGR_INTER_PERPLLLOCKED_MASK 0x00000080
#define CLKMGR_INTER_MAINPLLLOCKED_MASK 0x00000040
#define CLKMGR_CTRL_SAFEMODE_MASK 0x00000001
#define CLKMGR_CTRL_SAFEMODE_SET(x) (((x) << 0) & 0x00000001)
#define CLKMGR_SDRPLLGRP_VCO_OUTRESET_MASK 0x7e000000
#define CLKMGR_SDRPLLGRP_VCO_OUTRESETALL_SET(x) (((x) << 24) & 0x01000000)
#define CLKMGR_PERPLLGRP_PERQSPICLK_CNT_SET(x) (((x) << 0) & 0x000001ff)
#define CLKMGR_PERPLLGRP_DIV_SPIMCLK_SET(x) (((x) << 3) & 0x00000038)
#define CLKMGR_PERPLLGRP_GPIODIV_GPIODBCLK_SET(x) (((x) << 0) & 0x00ffffff)
#define CLKMGR_BYPASS_PERPLLSRC_SET(x) (((x) << 4) & 0x00000010)
#define CLKMGR_BYPASS_SDRPLLSRC_SET(x) (((x) << 2) & 0x00000004)
#define CLKMGR_PERPLLGRP_SRC_RESET_VALUE 0x00000015
#define CLKMGR_MAINPLLGRP_L4SRC_RESET_VALUE 0x00000000
#define CLKMGR_MAINPLLGRP_VCO_REGEXTSEL_MASK 0x80000000
#define CLKMGR_PERPLLGRP_VCO_REGEXTSEL_MASK 0x80000000
#define CLKMGR_SDRPLLGRP_VCO_REGEXTSEL_MASK 0x80000000
#define CLKMGR_SDRPLLGRP_DDRDQSCLK_PHASE_MASK 0x001ffe00
#define CLKMGR_SDRPLLGRP_DDR2XDQSCLK_PHASE_MASK 0x001ffe00
#define CLKMGR_SDRPLLGRP_DDRDQCLK_PHASE_MASK 0x001ffe00
#define CLKMGR_SDRPLLGRP_S2FUSER2CLK_PHASE_MASK 0x001ffe00
#define CLKMGR_MAINPLLGRP_VCO_OUTRESETALL_MASK 0x01000000
#define CLKMGR_PERPLLGRP_VCO_OUTRESETALL_MASK 0x01000000
#define CLKMGR_PERPLLGRP_EN_NANDCLK_MASK 0x00000400
#define CLKMGR_SDRPLLGRP_DDRDQSCLK_CNT_MASK 0x000001ff
#define CLKMGR_SDRPLLGRP_DDR2XDQSCLK_CNT_MASK 0x000001ff
#define CLKMGR_SDRPLLGRP_DDRDQCLK_CNT_MASK 0x000001ff
#define CLKMGR_SDRPLLGRP_S2FUSER2CLK_CNT_MASK 0x000001ff

#define MAIN_VCO_BASE \
	(CLKMGR_MAINPLLGRP_VCO_DENOM_SET(CONFIG_HPS_MAINPLLGRP_VCO_DENOM) | \
	CLKMGR_MAINPLLGRP_VCO_NUMER_SET(CONFIG_HPS_MAINPLLGRP_VCO_NUMER))

#define PERI_VCO_BASE \
	(CLKMGR_PERPLLGRP_VCO_PSRC_SET(CONFIG_HPS_PERPLLGRP_VCO_PSRC) | \
	CLKMGR_PERPLLGRP_VCO_DENOM_SET(CONFIG_HPS_PERPLLGRP_VCO_DENOM) | \
	CLKMGR_PERPLLGRP_VCO_NUMER_SET(CONFIG_HPS_PERPLLGRP_VCO_NUMER))

#define SDR_VCO_BASE \
	(CLKMGR_SDRPLLGRP_VCO_SSRC_SET(CONFIG_HPS_SDRPLLGRP_VCO_SSRC) | \
	CLKMGR_SDRPLLGRP_VCO_DENOM_SET(CONFIG_HPS_SDRPLLGRP_VCO_DENOM) | \
	CLKMGR_SDRPLLGRP_VCO_NUMER_SET(CONFIG_HPS_SDRPLLGRP_VCO_NUMER))

#endif /* _CLOCK_MANAGER_H_ */

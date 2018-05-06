// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/ddrmc-vf610.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Default DDR pad settings in arch/arm/include/asm/arch-vf610/iomux-vf610.h
 * do not match our settings. Let us (re)define our own settings here.
 */

#define PCM052_VF610_DDR_PAD_CTRL	PAD_CTL_DSE_20ohm
#define PCM052_VF610_DDR_PAD_CTRL_1	(PAD_CTL_DSE_20ohm | \
					PAD_CTL_INPUT_DIFFERENTIAL)
#define PCM052_VF610_DDR_RESET_PAD_CTL	(PAD_CTL_DSE_150ohm | \
					PAD_CTL_PUS_100K_UP | \
					PAD_CTL_INPUT_DIFFERENTIAL)

enum {
	PCM052_VF610_PAD_DDR_RESETB			= IOMUX_PAD(0x021c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_RESET_PAD_CTL),
	PCM052_VF610_PAD_DDR_A15__DDR_A_15		= IOMUX_PAD(0x0220, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A14__DDR_A_14		= IOMUX_PAD(0x0224, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A13__DDR_A_13		= IOMUX_PAD(0x0228, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A12__DDR_A_12		= IOMUX_PAD(0x022c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A11__DDR_A_11		= IOMUX_PAD(0x0230, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A10__DDR_A_10		= IOMUX_PAD(0x0234, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A9__DDR_A_9		= IOMUX_PAD(0x0238, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A8__DDR_A_8		= IOMUX_PAD(0x023c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A7__DDR_A_7		= IOMUX_PAD(0x0240, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A6__DDR_A_6		= IOMUX_PAD(0x0244, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A5__DDR_A_5		= IOMUX_PAD(0x0248, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A4__DDR_A_4		= IOMUX_PAD(0x024c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A3__DDR_A_3		= IOMUX_PAD(0x0250, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A2__DDR_A_2		= IOMUX_PAD(0x0254, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A1__DDR_A_1		= IOMUX_PAD(0x0258, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_A0__DDR_A_0		= IOMUX_PAD(0x025c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_BA2__DDR_BA_2		= IOMUX_PAD(0x0260, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_BA1__DDR_BA_1		= IOMUX_PAD(0x0264, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_BA0__DDR_BA_0		= IOMUX_PAD(0x0268, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_CAS__DDR_CAS_B		= IOMUX_PAD(0x026c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_CKE__DDR_CKE_0		= IOMUX_PAD(0x0270, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_CLK__DDR_CLK_0		= IOMUX_PAD(0x0274, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL_1),
	PCM052_VF610_PAD_DDR_CS__DDR_CS_B_0		= IOMUX_PAD(0x0278, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D15__DDR_D_15		= IOMUX_PAD(0x027c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D14__DDR_D_14		= IOMUX_PAD(0x0280, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D13__DDR_D_13		= IOMUX_PAD(0x0284, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D12__DDR_D_12		= IOMUX_PAD(0x0288, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D11__DDR_D_11		= IOMUX_PAD(0x028c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D10__DDR_D_10		= IOMUX_PAD(0x0290, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D9__DDR_D_9		= IOMUX_PAD(0x0294, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D8__DDR_D_8		= IOMUX_PAD(0x0298, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D7__DDR_D_7		= IOMUX_PAD(0x029c, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D6__DDR_D_6		= IOMUX_PAD(0x02a0, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D5__DDR_D_5		= IOMUX_PAD(0x02a4, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D4__DDR_D_4		= IOMUX_PAD(0x02a8, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D3__DDR_D_3		= IOMUX_PAD(0x02ac, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D2__DDR_D_2		= IOMUX_PAD(0x02b0, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D1__DDR_D_1		= IOMUX_PAD(0x02b4, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_D0__DDR_D_0		= IOMUX_PAD(0x02b8, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_DQM1__DDR_DQM_1		= IOMUX_PAD(0x02bc, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_DQM0__DDR_DQM_0		= IOMUX_PAD(0x02c0, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_DQS1__DDR_DQS_1		= IOMUX_PAD(0x02c4, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL_1),
	PCM052_VF610_PAD_DDR_DQS0__DDR_DQS_0		= IOMUX_PAD(0x02c8, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL_1),
	PCM052_VF610_PAD_DDR_RAS__DDR_RAS_B		= IOMUX_PAD(0x02cc, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_WE__DDR_WE_B		= IOMUX_PAD(0x02d0, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_ODT1__DDR_ODT_0		= IOMUX_PAD(0x02d4, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_ODT0__DDR_ODT_1		= IOMUX_PAD(0x02d8, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_DDRBYTE1__DDR_DDRBYTE1	= IOMUX_PAD(0x02dc, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
	PCM052_VF610_PAD_DDR_DDRBYTE0__DDR_DDRBYTE0	= IOMUX_PAD(0x02e0, __NA_, 0, __NA_, 0, PCM052_VF610_DDR_PAD_CTRL),
};

static struct ddrmc_cr_setting pcm052_cr_settings[] = {
	/* not in the datasheets, but in the original code */
	{ 0x00002000, 105 },
	{ 0x00000020, 110 },
	/* AXI */
	{ DDRMC_CR117_AXI0_W_PRI(1) | DDRMC_CR117_AXI0_R_PRI(1), 117 },
	{ DDRMC_CR118_AXI1_W_PRI(1) | DDRMC_CR118_AXI1_R_PRI(1), 118 },
	{ DDRMC_CR120_AXI0_PRI1_RPRI(2) |
		   DDRMC_CR120_AXI0_PRI0_RPRI(2), 120 },
	{ DDRMC_CR121_AXI0_PRI3_RPRI(2) |
		   DDRMC_CR121_AXI0_PRI2_RPRI(2), 121 },
	{ DDRMC_CR122_AXI1_PRI1_RPRI(1) | DDRMC_CR122_AXI1_PRI0_RPRI(1) |
		   DDRMC_CR122_AXI0_PRIRLX(100), 122 },
	{ DDRMC_CR123_AXI1_P_ODR_EN | DDRMC_CR123_AXI1_PRI3_RPRI(1) |
		   DDRMC_CR123_AXI1_PRI2_RPRI(1), 123 },
	{ DDRMC_CR124_AXI1_PRIRLX(100), 124 },
	{ DDRMC_CR126_PHY_RDLAT(11), 126 },
	{ DDRMC_CR132_WRLAT_ADJ(5) | DDRMC_CR132_RDLAT_ADJ(6), 132 },
	{ DDRMC_CR137_PHYCTL_DL(2), 137 },
	{ DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
		   DDRMC_CR139_PHY_WRLV_DLL(3) |
		   DDRMC_CR139_PHY_WRLV_EN(3), 139 },
	{ DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
		   DDRMC_CR154_PAD_ZQ_MODE(1) |
		   DDRMC_CR154_DDR_SEL_PAD_CONTR(3) |
		   DDRMC_CR154_PAD_ZQ_HW_FOR(0), 154 },
	{ DDRMC_CR155_PAD_ODT_BYTE1(5) | DDRMC_CR155_PAD_ODT_BYTE0(5), 155 },
	{ DDRMC_CR158_TWR(6), 158 },
	{ DDRMC_CR161_ODT_EN(0) | DDRMC_CR161_TODTH_RD(0) |
		   DDRMC_CR161_TODTH_WR(6), 161 },
	/* end marker */
	{ 0, -1 }
};

/* PHY settings -- most of them differ from default in imx-regs.h */

#define PCM052_DDRMC_PHY_DQ_TIMING			0x00002213
#define PCM052_DDRMC_PHY_CTRL				0x00290000
#define PCM052_DDRMC_PHY_SLAVE_CTRL			0x00002c00
#define PCM052_DDRMC_PHY_PROC_PAD_ODT			0x00010020

static struct ddrmc_phy_setting pcm052_phy_settings[] = {
	{ PCM052_DDRMC_PHY_DQ_TIMING,  0 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 16 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 32 },
	{ PCM052_DDRMC_PHY_DQ_TIMING, 48 },
	{ DDRMC_PHY_DQS_TIMING,  1 },
	{ DDRMC_PHY_DQS_TIMING, 17 },
	{ DDRMC_PHY_DQS_TIMING, 33 },
	{ DDRMC_PHY_DQS_TIMING, 49 },
	{ PCM052_DDRMC_PHY_CTRL,  2 },
	{ PCM052_DDRMC_PHY_CTRL, 18 },
	{ PCM052_DDRMC_PHY_CTRL, 34 },
	{ DDRMC_PHY_MASTER_CTRL,  3 },
	{ DDRMC_PHY_MASTER_CTRL, 19 },
	{ DDRMC_PHY_MASTER_CTRL, 35 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL,  4 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL, 20 },
	{ PCM052_DDRMC_PHY_SLAVE_CTRL, 36 },
	{ DDRMC_PHY50_DDR3_MODE | DDRMC_PHY50_EN_SW_HALF_CYCLE, 50 },
	{ PCM052_DDRMC_PHY_PROC_PAD_ODT, 52 },

	/* end marker */
	{ 0, -1 }
};

int dram_init(void)
{
	static const iomux_v3_cfg_t pcm052_pads[] = {
		PCM052_VF610_PAD_DDR_A15__DDR_A_15,
		PCM052_VF610_PAD_DDR_A14__DDR_A_14,
		PCM052_VF610_PAD_DDR_A13__DDR_A_13,
		PCM052_VF610_PAD_DDR_A12__DDR_A_12,
		PCM052_VF610_PAD_DDR_A11__DDR_A_11,
		PCM052_VF610_PAD_DDR_A10__DDR_A_10,
		PCM052_VF610_PAD_DDR_A9__DDR_A_9,
		PCM052_VF610_PAD_DDR_A8__DDR_A_8,
		PCM052_VF610_PAD_DDR_A7__DDR_A_7,
		PCM052_VF610_PAD_DDR_A6__DDR_A_6,
		PCM052_VF610_PAD_DDR_A5__DDR_A_5,
		PCM052_VF610_PAD_DDR_A4__DDR_A_4,
		PCM052_VF610_PAD_DDR_A3__DDR_A_3,
		PCM052_VF610_PAD_DDR_A2__DDR_A_2,
		PCM052_VF610_PAD_DDR_A1__DDR_A_1,
		PCM052_VF610_PAD_DDR_A0__DDR_A_0,
		PCM052_VF610_PAD_DDR_BA2__DDR_BA_2,
		PCM052_VF610_PAD_DDR_BA1__DDR_BA_1,
		PCM052_VF610_PAD_DDR_BA0__DDR_BA_0,
		PCM052_VF610_PAD_DDR_CAS__DDR_CAS_B,
		PCM052_VF610_PAD_DDR_CKE__DDR_CKE_0,
		PCM052_VF610_PAD_DDR_CLK__DDR_CLK_0,
		PCM052_VF610_PAD_DDR_CS__DDR_CS_B_0,
		PCM052_VF610_PAD_DDR_D15__DDR_D_15,
		PCM052_VF610_PAD_DDR_D14__DDR_D_14,
		PCM052_VF610_PAD_DDR_D13__DDR_D_13,
		PCM052_VF610_PAD_DDR_D12__DDR_D_12,
		PCM052_VF610_PAD_DDR_D11__DDR_D_11,
		PCM052_VF610_PAD_DDR_D10__DDR_D_10,
		PCM052_VF610_PAD_DDR_D9__DDR_D_9,
		PCM052_VF610_PAD_DDR_D8__DDR_D_8,
		PCM052_VF610_PAD_DDR_D7__DDR_D_7,
		PCM052_VF610_PAD_DDR_D6__DDR_D_6,
		PCM052_VF610_PAD_DDR_D5__DDR_D_5,
		PCM052_VF610_PAD_DDR_D4__DDR_D_4,
		PCM052_VF610_PAD_DDR_D3__DDR_D_3,
		PCM052_VF610_PAD_DDR_D2__DDR_D_2,
		PCM052_VF610_PAD_DDR_D1__DDR_D_1,
		PCM052_VF610_PAD_DDR_D0__DDR_D_0,
		PCM052_VF610_PAD_DDR_DQM1__DDR_DQM_1,
		PCM052_VF610_PAD_DDR_DQM0__DDR_DQM_0,
		PCM052_VF610_PAD_DDR_DQS1__DDR_DQS_1,
		PCM052_VF610_PAD_DDR_DQS0__DDR_DQS_0,
		PCM052_VF610_PAD_DDR_RAS__DDR_RAS_B,
		PCM052_VF610_PAD_DDR_WE__DDR_WE_B,
		PCM052_VF610_PAD_DDR_ODT1__DDR_ODT_0,
		PCM052_VF610_PAD_DDR_ODT0__DDR_ODT_1,
		PCM052_VF610_PAD_DDR_DDRBYTE1__DDR_DDRBYTE1,
		PCM052_VF610_PAD_DDR_DDRBYTE0__DDR_DDRBYTE0,
		PCM052_VF610_PAD_DDR_RESETB,
	};

#if defined(CONFIG_TARGET_PCM052)

	static const struct ddr3_jedec_timings pcm052_ddr_timings = {
		.tinit             = 5,
		.trst_pwron        = 80000,
		.cke_inactive      = 200000,
		.wrlat             = 5,
		.caslat_lin        = 12,
		.trc               = 6,
		.trrd              = 4,
		.tccd              = 4,
		.tbst_int_interval = 4,
		.tfaw              = 18,
		.trp               = 6,
		.twtr              = 4,
		.tras_min          = 15,
		.tmrd              = 4,
		.trtp              = 4,
		.tras_max          = 14040,
		.tmod              = 12,
		.tckesr            = 4,
		.tcke              = 3,
		.trcd_int          = 6,
		.tras_lockout      = 1,
		.tdal              = 10,
		.bstlen            = 3,
		.tdll              = 512,
		.trp_ab            = 6,
		.tref              = 1542,
		.trfc              = 64,
		.tref_int          = 5,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 68,
		.txsr              = 506,
		.cksrx             = 5,
		.cksre             = 5,
		.freq_chg_en       = 1,
		.zqcl              = 256,
		.zqinit            = 512,
		.zqcs              = 64,
		.ref_per_zq        = 64,
		.zqcs_rotate       = 1,
		.aprebit           = 10,
		.cmd_age_cnt       = 255,
		.age_cnt           = 255,
		.q_fullness        = 0,
		.odt_rd_mapcs0     = 1,
		.odt_wr_mapcs0     = 1,
		.wlmrd             = 40,
		.wldqsen           = 25,
	};

    const int row_diff = 2;

#elif defined(CONFIG_TARGET_BK4R1)

	static const struct ddr3_jedec_timings pcm052_ddr_timings = {
		.tinit             = 5,
		.trst_pwron        = 80000,
		.cke_inactive      = 200000,
		.wrlat             = 5,
		.caslat_lin        = 12,
		.trc               = 6,
		.trrd              = 4,
		.tccd              = 4,
		.tbst_int_interval = 0,
		.tfaw              = 16,
		.trp               = 6,
		.twtr              = 4,
		.tras_min          = 15,
		.tmrd              = 4,
		.trtp              = 4,
		.tras_max          = 28080,
		.tmod              = 12,
		.tckesr            = 4,
		.tcke              = 3,
		.trcd_int          = 6,
		.tras_lockout      = 1,
		.tdal              = 12,
		.bstlen            = 3,
		.tdll              = 512,
		.trp_ab            = 6,
		.tref              = 3120,
		.trfc              = 104,
		.tref_int          = 0,
		.tpdex             = 3,
		.txpdll            = 10,
		.txsnr             = 108,
		.txsr              = 512,
		.cksrx             = 5,
		.cksre             = 5,
		.freq_chg_en       = 1,
		.zqcl              = 256,
		.zqinit            = 512,
		.zqcs              = 64,
		.ref_per_zq        = 64,
		.zqcs_rotate       = 1,
		.aprebit           = 10,
		.cmd_age_cnt       = 255,
		.age_cnt           = 255,
		.q_fullness        = 0,
		.odt_rd_mapcs0     = 1,
		.odt_wr_mapcs0     = 1,
		.wlmrd             = 40,
		.wldqsen           = 25,
	};

    const int row_diff = 1;

#else /* Unknown PCM052 variant */

#error DDR characteristics undefined for this target. Please define them.

#endif

	imx_iomux_v3_setup_multiple_pads(pcm052_pads, ARRAY_SIZE(pcm052_pads));

	ddrmc_ctrl_init_ddr3(&pcm052_ddr_timings, pcm052_cr_settings,
			     pcm052_phy_settings, 1, row_diff);

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart1_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTB4__UART1_TX, VF610_UART_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTB5__UART1_RX, VF610_UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

#define ENET_PAD_CTRL	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_50ohm | PAD_CTL_OBE_IBE_ENABLE)

static void setup_iomux_enet(void)
{
	static const iomux_v3_cfg_t enet0_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTA6__RMII0_CLKIN, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC1__RMII0_MDIO, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC0__RMII0_MDC, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC2__RMII0_CRS_DV, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC3__RMII0_RD1, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC4__RMII0_RD0, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC5__RMII0_RXER, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC6__RMII0_TD1, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC7__RMII0_TD0, ENET_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTC8__RMII0_TXEN, ENET_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(enet0_pads, ARRAY_SIZE(enet0_pads));
}

/*
 * I2C2 is the only I2C used, on pads PTA22/PTA23.
 */

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c_pads[] = {
		VF610_PAD_PTA22__I2C2_SCL,
		VF610_PAD_PTA23__I2C2_SDA,
	};

	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}

#ifdef CONFIG_NAND_VF610_NFC
static void setup_iomux_nfc(void)
{
	static const iomux_v3_cfg_t nfc_pads[] = {
		VF610_PAD_PTD31__NF_IO15,
		VF610_PAD_PTD30__NF_IO14,
		VF610_PAD_PTD29__NF_IO13,
		VF610_PAD_PTD28__NF_IO12,
		VF610_PAD_PTD27__NF_IO11,
		VF610_PAD_PTD26__NF_IO10,
		VF610_PAD_PTD25__NF_IO9,
		VF610_PAD_PTD24__NF_IO8,
		VF610_PAD_PTD23__NF_IO7,
		VF610_PAD_PTD22__NF_IO6,
		VF610_PAD_PTD21__NF_IO5,
		VF610_PAD_PTD20__NF_IO4,
		VF610_PAD_PTD19__NF_IO3,
		VF610_PAD_PTD18__NF_IO2,
		VF610_PAD_PTD17__NF_IO1,
		VF610_PAD_PTD16__NF_IO0,
		VF610_PAD_PTB24__NF_WE_B,
		VF610_PAD_PTB25__NF_CE0_B,
		VF610_PAD_PTB27__NF_RE_B,
		VF610_PAD_PTC26__NF_RB_B,
		VF610_PAD_PTC27__NF_ALE,
		VF610_PAD_PTC28__NF_CLE
	};

	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));
}
#endif

static void setup_iomux_qspi(void)
{
	static const iomux_v3_cfg_t qspi0_pads[] = {
		VF610_PAD_PTD0__QSPI0_A_QSCK,
		VF610_PAD_PTD1__QSPI0_A_CS0,
		VF610_PAD_PTD2__QSPI0_A_DATA3,
		VF610_PAD_PTD3__QSPI0_A_DATA2,
		VF610_PAD_PTD4__QSPI0_A_DATA1,
		VF610_PAD_PTD5__QSPI0_A_DATA0,
		VF610_PAD_PTD7__QSPI0_B_QSCK,
		VF610_PAD_PTD8__QSPI0_B_CS0,
		VF610_PAD_PTD9__QSPI0_B_DATA3,
		VF610_PAD_PTD10__QSPI0_B_DATA2,
		VF610_PAD_PTD11__QSPI0_B_DATA1,
		VF610_PAD_PTD12__QSPI0_B_DATA0,
	};

	imx_iomux_v3_setup_multiple_pads(qspi0_pads, ARRAY_SIZE(qspi0_pads));
}

#define ESDHC_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_20ohm | PAD_CTL_OBE_IBE_ENABLE)

struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{ESDHC1_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t esdhc1_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTA24__ESDHC1_CLK, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA25__ESDHC1_CMD, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA26__ESDHC1_DAT0, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA27__ESDHC1_DAT1, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA28__ESDHC1_DAT2, ESDHC_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTA29__ESDHC1_DAT3, ESDHC_PAD_CTRL),
	};

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(
		esdhc1_pads, ARRAY_SIZE(esdhc1_pads));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}

static void clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;

	clrsetbits_le32(&ccm->ccgr0, CCM_REG_CTRL_MASK,
			CCM_CCGR0_UART1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr1, CCM_REG_CTRL_MASK,
			CCM_CCGR1_PIT_CTRL_MASK | CCM_CCGR1_WDOGA5_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr2, CCM_REG_CTRL_MASK,
			CCM_CCGR2_IOMUXC_CTRL_MASK | CCM_CCGR2_PORTA_CTRL_MASK |
			CCM_CCGR2_PORTB_CTRL_MASK | CCM_CCGR2_PORTC_CTRL_MASK |
			CCM_CCGR2_PORTD_CTRL_MASK | CCM_CCGR2_PORTE_CTRL_MASK |
			CCM_CCGR2_QSPI0_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr3, CCM_REG_CTRL_MASK,
			CCM_CCGR3_ANADIG_CTRL_MASK | CCM_CCGR3_SCSC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr4, CCM_REG_CTRL_MASK,
			CCM_CCGR4_WKUP_CTRL_MASK | CCM_CCGR4_CCM_CTRL_MASK |
			CCM_CCGR4_GPC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr6, CCM_REG_CTRL_MASK,
			CCM_CCGR6_OCOTP_CTRL_MASK | CCM_CCGR6_DDRMC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr7, CCM_REG_CTRL_MASK,
			CCM_CCGR7_SDHC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr9, CCM_REG_CTRL_MASK,
			CCM_CCGR9_FEC0_CTRL_MASK | CCM_CCGR9_FEC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr10, CCM_REG_CTRL_MASK,
			CCM_CCGR10_NFC_CTRL_MASK | CCM_CCGR10_I2C2_CTRL_MASK);

	clrsetbits_le32(&anadig->pll2_ctrl, ANADIG_PLL2_CTRL_POWERDOWN,
			ANADIG_PLL2_CTRL_ENABLE | ANADIG_PLL2_CTRL_DIV_SELECT);
	clrsetbits_le32(&anadig->pll1_ctrl, ANADIG_PLL1_CTRL_POWERDOWN,
			ANADIG_PLL1_CTRL_ENABLE | ANADIG_PLL1_CTRL_DIV_SELECT);

	clrsetbits_le32(&ccm->ccr, CCM_CCR_OSCNT_MASK,
			CCM_CCR_FIRC_EN | CCM_CCR_OSCNT(5));
	clrsetbits_le32(&ccm->ccsr, CCM_REG_CTRL_MASK,
			CCM_CCSR_PLL1_PFD_CLK_SEL(3) | CCM_CCSR_PLL2_PFD4_EN |
			CCM_CCSR_PLL2_PFD3_EN | CCM_CCSR_PLL2_PFD2_EN |
			CCM_CCSR_PLL2_PFD1_EN | CCM_CCSR_PLL1_PFD4_EN |
			CCM_CCSR_PLL1_PFD3_EN | CCM_CCSR_PLL1_PFD2_EN |
			CCM_CCSR_PLL1_PFD1_EN | CCM_CCSR_DDRC_CLK_SEL(1) |
			CCM_CCSR_FAST_CLK_SEL(1) | CCM_CCSR_SYS_CLK_SEL(4));
	clrsetbits_le32(&ccm->cacrr, CCM_REG_CTRL_MASK,
			CCM_CACRR_IPG_CLK_DIV(1) | CCM_CACRR_BUS_CLK_DIV(2) |
			CCM_CACRR_ARM_CLK_DIV(0));
	clrsetbits_le32(&ccm->cscmr1, CCM_REG_CTRL_MASK,
			CCM_CSCMR1_ESDHC1_CLK_SEL(3) |
			CCM_CSCMR1_QSPI0_CLK_SEL(3) |
			CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
			CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
			CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
			CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
			CCM_CSCDR3_QSPI0_EN | CCM_CSCDR3_QSPI0_DIV(1) |
			CCM_CSCDR3_QSPI0_X2_DIV(1) |
			CCM_CSCDR3_QSPI0_X4_DIV(3) |
			CCM_CSCDR3_NFC_PRE_DIV(5));
	clrsetbits_le32(&ccm->cscmr2, CCM_REG_CTRL_MASK,
			CCM_CSCMR2_RMII_CLK_SEL(0));
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CP0_EN, &mscmir->irsprc[i]);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();
	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
	setup_iomux_qspi();
	setup_iomux_nfc();

	return 0;
}

int board_init(void)
{
	struct scsc_reg *scsc = (struct scsc_reg *)SCSC_BASE_ADDR;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	/*
	 * Enable external 32K Oscillator
	 *
	 * The internal clock experiences significant drift
	 * so we must use the external oscillator in order
	 * to maintain correct time in the hwclock
	 */
	setbits_le32(&scsc->sosc_ctr, SCSC_SOSC_CTR_SOSC_EN);

	return 0;
}

int checkboard(void)
{
	puts("Board: PCM-052\n");

	return 0;
}

static int do_m4go(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	ulong addr;

	/* Consume 'm4go' */
	argc--; argv++;

	/*
	 * Parse provided address - default to load_addr in case not provided.
	 */

	if (argc)
		addr = simple_strtoul(argv[0], NULL, 16);
	else
		addr = load_addr;

	/*
	 * Write boot address in PERSISTENT_ENTRY1[31:0] aka SRC_GPR2[31:0]
	 */
	writel(addr + 0x401, 0x4006E028);

	/*
	 * Start secondary processor by enabling its clock
	 */
	writel(0x15a5a, 0x4006B08C);

	return 1;
}

U_BOOT_CMD(
	m4go, 2 /* one arg max */, 1 /* repeatable */, do_m4go,
	"start the secondary Cortex-M4 from scatter file image",
	"[<addr>]\n"
	"    - start secondary Cortex-M4 core using a scatter file image\n"
	"The argument needs to be a scatter file\n"
);

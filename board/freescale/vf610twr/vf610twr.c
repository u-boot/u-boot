/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED | \
			PAD_CTL_DSE_25ohm | PAD_CTL_OBE_IBE_ENABLE)

#define ESDHC_PAD_CTRL	(PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_20ohm | PAD_CTL_OBE_IBE_ENABLE)

#define ENET_PAD_CTRL	(PAD_CTL_PUS_47K_UP | PAD_CTL_SPEED_HIGH | \
			PAD_CTL_DSE_50ohm | PAD_CTL_OBE_IBE_ENABLE)

void setup_iomux_ddr(void)
{
	static const iomux_v3_cfg_t ddr_pads[] = {
		VF610_PAD_DDR_A15__DDR_A_15,
		VF610_PAD_DDR_A14__DDR_A_14,
		VF610_PAD_DDR_A13__DDR_A_13,
		VF610_PAD_DDR_A12__DDR_A_12,
		VF610_PAD_DDR_A11__DDR_A_11,
		VF610_PAD_DDR_A10__DDR_A_10,
		VF610_PAD_DDR_A9__DDR_A_9,
		VF610_PAD_DDR_A8__DDR_A_8,
		VF610_PAD_DDR_A7__DDR_A_7,
		VF610_PAD_DDR_A6__DDR_A_6,
		VF610_PAD_DDR_A5__DDR_A_5,
		VF610_PAD_DDR_A4__DDR_A_4,
		VF610_PAD_DDR_A3__DDR_A_3,
		VF610_PAD_DDR_A2__DDR_A_2,
		VF610_PAD_DDR_A1__DDR_A_1,
		VF610_PAD_DDR_A0__DDR_A_0,
		VF610_PAD_DDR_BA2__DDR_BA_2,
		VF610_PAD_DDR_BA1__DDR_BA_1,
		VF610_PAD_DDR_BA0__DDR_BA_0,
		VF610_PAD_DDR_CAS__DDR_CAS_B,
		VF610_PAD_DDR_CKE__DDR_CKE_0,
		VF610_PAD_DDR_CLK__DDR_CLK_0,
		VF610_PAD_DDR_CS__DDR_CS_B_0,
		VF610_PAD_DDR_D15__DDR_D_15,
		VF610_PAD_DDR_D14__DDR_D_14,
		VF610_PAD_DDR_D13__DDR_D_13,
		VF610_PAD_DDR_D12__DDR_D_12,
		VF610_PAD_DDR_D11__DDR_D_11,
		VF610_PAD_DDR_D10__DDR_D_10,
		VF610_PAD_DDR_D9__DDR_D_9,
		VF610_PAD_DDR_D8__DDR_D_8,
		VF610_PAD_DDR_D7__DDR_D_7,
		VF610_PAD_DDR_D6__DDR_D_6,
		VF610_PAD_DDR_D5__DDR_D_5,
		VF610_PAD_DDR_D4__DDR_D_4,
		VF610_PAD_DDR_D3__DDR_D_3,
		VF610_PAD_DDR_D2__DDR_D_2,
		VF610_PAD_DDR_D1__DDR_D_1,
		VF610_PAD_DDR_D0__DDR_D_0,
		VF610_PAD_DDR_DQM1__DDR_DQM_1,
		VF610_PAD_DDR_DQM0__DDR_DQM_0,
		VF610_PAD_DDR_DQS1__DDR_DQS_1,
		VF610_PAD_DDR_DQS0__DDR_DQS_0,
		VF610_PAD_DDR_RAS__DDR_RAS_B,
		VF610_PAD_DDR_WE__DDR_WE_B,
		VF610_PAD_DDR_ODT1__DDR_ODT_0,
		VF610_PAD_DDR_ODT0__DDR_ODT_1,
		VF610_PAD_DDR_RESETB,
	};

	imx_iomux_v3_setup_multiple_pads(ddr_pads, ARRAY_SIZE(ddr_pads));
}

void ddr_phy_init(void)
{
	struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;

	writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[0]);
	writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[16]);
	writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[32]);

	writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[1]);
	writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[17]);

	writel(DDRMC_PHY_CTRL, &ddrmr->phy[2]);
	writel(DDRMC_PHY_CTRL, &ddrmr->phy[18]);
	writel(DDRMC_PHY_CTRL, &ddrmr->phy[34]);

	writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[3]);
	writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[19]);
	writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[35]);

	writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[4]);
	writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[20]);
	writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[36]);

	/* LPDDR2 only parameter */
	writel(DDRMC_PHY_OFF, &ddrmr->phy[49]);

	writel(DDRMC_PHY50_DDR3_MODE | DDRMC_PHY50_EN_SW_HALF_CYCLE,
		&ddrmr->phy[50]);

	/* Processor Pad ODT settings */
	writel(DDRMC_PHY_PROC_PAD_ODT, &ddrmr->phy[52]);
}

void ddr_ctrl_init(void)
{
	struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;

	writel(DDRMC_CR00_DRAM_CLASS_DDR3, &ddrmr->cr[0]);
	writel(DDRMC_CR02_DRAM_TINIT(32), &ddrmr->cr[2]);
	writel(DDRMC_CR10_TRST_PWRON(80000), &ddrmr->cr[10]);

	writel(DDRMC_CR11_CKE_INACTIVE(200000), &ddrmr->cr[11]);
	writel(DDRMC_CR12_WRLAT(5) | DDRMC_CR12_CASLAT_LIN(12), &ddrmr->cr[12]);
	writel(DDRMC_CR13_TRC(21) | DDRMC_CR13_TRRD(4) | DDRMC_CR13_TCCD(4),
		&ddrmr->cr[13]);
	writel(DDRMC_CR14_TFAW(20) | DDRMC_CR14_TRP(6) | DDRMC_CR14_TWTR(4) |
		DDRMC_CR14_TRAS_MIN(15), &ddrmr->cr[14]);
	writel(DDRMC_CR16_TMRD(4) | DDRMC_CR16_TRTP(4), &ddrmr->cr[16]);
	writel(DDRMC_CR17_TRAS_MAX(28080) | DDRMC_CR17_TMOD(12),
		&ddrmr->cr[17]);
	writel(DDRMC_CR18_TCKESR(4) | DDRMC_CR18_TCKE(3), &ddrmr->cr[18]);

	writel(DDRMC_CR20_AP_EN, &ddrmr->cr[20]);
	writel(DDRMC_CR21_TRCD_INT(6) | DDRMC_CR21_CCMAP_EN, &ddrmr->cr[21]);

	writel(DDRMC_CR22_TDAL(12), &ddrmr->cr[22]);
	writel(DDRMC_CR23_BSTLEN(3) | DDRMC_CR23_TDLL(512), &ddrmr->cr[23]);
	writel(DDRMC_CR24_TRP_AB(6), &ddrmr->cr[24]);

	writel(DDRMC_CR25_TREF_EN, &ddrmr->cr[25]);
	writel(DDRMC_CR26_TREF(3120) | DDRMC_CR26_TRFC(44), &ddrmr->cr[26]);
	writel(DDRMC_CR28_TREF_INT(0), &ddrmr->cr[28]);
	writel(DDRMC_CR29_TPDEX(3), &ddrmr->cr[29]);

	writel(DDRMC_CR30_TXPDLL(10), &ddrmr->cr[30]);
	writel(DDRMC_CR31_TXSNR(48) | DDRMC_CR31_TXSR(468), &ddrmr->cr[31]);
	writel(DDRMC_CR33_EN_QK_SREF, &ddrmr->cr[33]);
	writel(DDRMC_CR34_CKSRX(5) | DDRMC_CR34_CKSRE(5), &ddrmr->cr[34]);

	writel(DDRMC_CR38_FREQ_CHG_EN(0), &ddrmr->cr[38]);
	writel(DDRMC_CR39_PHY_INI_COM(1024) | DDRMC_CR39_PHY_INI_STA(16) |
		DDRMC_CR39_FRQ_CH_DLLOFF(2), &ddrmr->cr[39]);

	writel(DDRMC_CR41_PHY_INI_STRT_INI_DIS, &ddrmr->cr[41]);
	writel(DDRMC_CR48_MR1_DA_0(70) | DDRMC_CR48_MR0_DA_0(1056),
		&ddrmr->cr[48]);

	writel(DDRMC_CR66_ZQCL(256) | DDRMC_CR66_ZQINIT(512), &ddrmr->cr[66]);
	writel(DDRMC_CR67_ZQCS(64), &ddrmr->cr[67]);
	writel(DDRMC_CR69_ZQ_ON_SREF_EX(2), &ddrmr->cr[69]);

	writel(DDRMC_CR70_REF_PER_ZQ(64), &ddrmr->cr[70]);
	writel(DDRMC_CR72_ZQCS_ROTATE(0), &ddrmr->cr[72]);

	writel(DDRMC_CR73_APREBIT(10) | DDRMC_CR73_COL_DIFF(1) |
		DDRMC_CR73_ROW_DIFF(3), &ddrmr->cr[73]);
	writel(DDRMC_CR74_BANKSPLT_EN | DDRMC_CR74_ADDR_CMP_EN |
		DDRMC_CR74_CMD_AGE_CNT(64) | DDRMC_CR74_AGE_CNT(64),
		&ddrmr->cr[74]);
	writel(DDRMC_CR75_RW_PG_EN | DDRMC_CR75_RW_EN | DDRMC_CR75_PRI_EN |
		DDRMC_CR75_PLEN, &ddrmr->cr[75]);
	writel(DDRMC_CR76_NQENT_ACTDIS(3) | DDRMC_CR76_D_RW_G_BKCN(3) |
		DDRMC_CR76_W2R_SPLT_EN, &ddrmr->cr[76]);
	writel(DDRMC_CR77_CS_MAP | DDRMC_CR77_DI_RD_INTLEAVE |
		DDRMC_CR77_SWAP_EN, &ddrmr->cr[77]);
	writel(DDRMC_CR78_Q_FULLNESS(7) | DDRMC_CR78_BUR_ON_FLY_BIT(12),
		&ddrmr->cr[78]);
	writel(DDRMC_CR79_CTLUPD_AREF(0), &ddrmr->cr[79]);

	writel(DDRMC_CR82_INT_MASK, &ddrmr->cr[82]);

	writel(DDRMC_CR87_ODT_WR_MAPCS0, &ddrmr->cr[87]);
	writel(DDRMC_CR88_TODTL_CMD(4), &ddrmr->cr[88]);
	writel(DDRMC_CR89_AODT_RWSMCS(2), &ddrmr->cr[89]);

	writel(DDRMC_CR91_R2W_SMCSDL(2), &ddrmr->cr[91]);
	writel(DDRMC_CR96_WLMRD(40) | DDRMC_CR96_WLDQSEN(25), &ddrmr->cr[96]);
	writel(DDRMC_CR97_WRLVL_EN, &ddrmr->cr[97]);
	writel(DDRMC_CR98_WRLVL_DL_0, &ddrmr->cr[98]);
	writel(DDRMC_CR99_WRLVL_DL_1, &ddrmr->cr[99]);

	writel(DDRMC_CR102_RDLVL_GT_REGEN | DDRMC_CR102_RDLVL_REG_EN,
		&ddrmr->cr[102]);

	writel(DDRMC_CR105_RDLVL_DL_0(0), &ddrmr->cr[105]);
	writel(DDRMC_CR106_RDLVL_GTDL_0(4), &ddrmr->cr[106]);
	writel(DDRMC_CR110_RDLVL_GTDL_1(4), &ddrmr->cr[110]);
	writel(DDRMC_CR114_RDLVL_GTDL_2(0), &ddrmr->cr[114]);
	writel(DDRMC_CR115_RDLVL_GTDL_2(0), &ddrmr->cr[115]);

	writel(DDRMC_CR117_AXI0_W_PRI(0) | DDRMC_CR117_AXI0_R_PRI(0),
		&ddrmr->cr[117]);
	writel(DDRMC_CR118_AXI1_W_PRI(1) | DDRMC_CR118_AXI1_R_PRI(1),
		&ddrmr->cr[118]);

	writel(DDRMC_CR120_AXI0_PRI1_RPRI(2) | DDRMC_CR120_AXI0_PRI0_RPRI(2),
		&ddrmr->cr[120]);
	writel(DDRMC_CR121_AXI0_PRI3_RPRI(2) | DDRMC_CR121_AXI0_PRI2_RPRI(2),
		&ddrmr->cr[121]);
	writel(DDRMC_CR122_AXI1_PRI1_RPRI(1) | DDRMC_CR122_AXI1_PRI0_RPRI(1) |
		DDRMC_CR122_AXI0_PRIRLX(100), &ddrmr->cr[122]);
	writel(DDRMC_CR123_AXI1_P_ODR_EN | DDRMC_CR123_AXI1_PRI3_RPRI(1) |
		DDRMC_CR123_AXI1_PRI2_RPRI(1), &ddrmr->cr[123]);
	writel(DDRMC_CR124_AXI1_PRIRLX(100), &ddrmr->cr[124]);

	writel(DDRMC_CR126_PHY_RDLAT(8), &ddrmr->cr[126]);
	writel(DDRMC_CR132_WRLAT_ADJ(5) | DDRMC_CR132_RDLAT_ADJ(6),
		&ddrmr->cr[132]);
	writel(DDRMC_CR137_PHYCTL_DL(2), &ddrmr->cr[137]);
	writel(DDRMC_CR138_PHY_WRLV_MXDL(256) | DDRMC_CR138_PHYDRAM_CK_EN(1),
		&ddrmr->cr[138]);
	writel(DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
		DDRMC_CR139_PHY_WRLV_DLL(3) | DDRMC_CR139_PHY_WRLV_EN(3),
		&ddrmr->cr[139]);
	writel(DDRMC_CR140_PHY_WRLV_WW(64), &ddrmr->cr[140]);
	writel(DDRMC_CR143_RDLV_GAT_MXDL(1536) | DDRMC_CR143_RDLV_MXDL(128),
		&ddrmr->cr[143]);
	writel(DDRMC_CR144_PHY_RDLVL_RES(4) | DDRMC_CR144_PHY_RDLV_LOAD(7) |
		DDRMC_CR144_PHY_RDLV_DLL(3) | DDRMC_CR144_PHY_RDLV_EN(3),
		&ddrmr->cr[144]);
	writel(DDRMC_CR145_PHY_RDLV_RR(64), &ddrmr->cr[145]);
	writel(DDRMC_CR146_PHY_RDLVL_RESP(64), &ddrmr->cr[146]);
	writel(DDRMC_CR147_RDLV_RESP_MASK(983040), &ddrmr->cr[147]);
	writel(DDRMC_CR148_RDLV_GATE_RESP_MASK(983040), &ddrmr->cr[148]);
	writel(DDRMC_CR151_RDLV_GAT_DQ_ZERO_CNT(1) |
		DDRMC_CR151_RDLVL_DQ_ZERO_CNT(1), &ddrmr->cr[151]);

	writel(DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
		DDRMC_CR154_PAD_ZQ_MODE(1) | DDRMC_CR154_DDR_SEL_PAD_CONTR(3) |
		DDRMC_CR154_PAD_ZQ_HW_FOR(1), &ddrmr->cr[154]);
	writel(DDRMC_CR155_PAD_ODT_BYTE1(2) | DDRMC_CR155_PAD_ODT_BYTE0(2),
		&ddrmr->cr[155]);
	writel(DDRMC_CR158_TWR(6), &ddrmr->cr[158]);
	writel(DDRMC_CR161_ODT_EN(1) | DDRMC_CR161_TODTH_RD(2) |
		DDRMC_CR161_TODTH_WR(2), &ddrmr->cr[161]);

	ddr_phy_init();

	writel(DDRMC_CR00_DRAM_CLASS_DDR3 | DDRMC_CR00_START, &ddrmr->cr[0]);

	udelay(200);
}

int dram_init(void)
{
	setup_iomux_ddr();

	ddr_ctrl_init();
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart1_pads[] = {
		NEW_PAD_CTRL(VF610_PAD_PTB4__UART1_TX, UART_PAD_CTRL),
		NEW_PAD_CTRL(VF610_PAD_PTB5__UART1_RX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

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

static void setup_iomux_i2c(void)
{
	static const iomux_v3_cfg_t i2c0_pads[] = {
		VF610_PAD_PTB14__I2C0_SCL,
		VF610_PAD_PTB15__I2C0_SDA,
	};

	imx_iomux_v3_setup_multiple_pads(i2c0_pads, ARRAY_SIZE(i2c0_pads));
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

#ifdef CONFIG_FSL_ESDHC
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
#endif

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
		CCM_CCGR3_ANADIG_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr4, CCM_REG_CTRL_MASK,
		CCM_CCGR4_WKUP_CTRL_MASK | CCM_CCGR4_CCM_CTRL_MASK |
		CCM_CCGR4_GPC_CTRL_MASK | CCM_CCGR4_I2C0_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr6, CCM_REG_CTRL_MASK,
		CCM_CCGR6_OCOTP_CTRL_MASK | CCM_CCGR6_DDRMC_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr7, CCM_REG_CTRL_MASK,
		CCM_CCGR7_SDHC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr9, CCM_REG_CTRL_MASK,
		CCM_CCGR9_FEC0_CTRL_MASK | CCM_CCGR9_FEC1_CTRL_MASK);
	clrsetbits_le32(&ccm->ccgr10, CCM_REG_CTRL_MASK,
		CCM_CCGR10_NFC_CTRL_MASK);

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
		CCM_CSCMR1_ESDHC1_CLK_SEL(3) | CCM_CSCMR1_QSPI0_CLK_SEL(3) |
		CCM_CSCMR1_NFC_CLK_SEL(0));
	clrsetbits_le32(&ccm->cscdr1, CCM_REG_CTRL_MASK,
		CCM_CSCDR1_RMII_CLK_EN);
	clrsetbits_le32(&ccm->cscdr2, CCM_REG_CTRL_MASK,
		CCM_CSCDR2_ESDHC1_EN | CCM_CSCDR2_ESDHC1_CLK_DIV(0) |
		CCM_CSCDR2_NFC_EN);
	clrsetbits_le32(&ccm->cscdr3, CCM_REG_CTRL_MASK,
		CCM_CSCDR3_QSPI0_EN | CCM_CSCDR3_QSPI0_DIV(1) |
		CCM_CSCDR3_QSPI0_X2_DIV(1) | CCM_CSCDR3_QSPI0_X4_DIV(3) |
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
#ifdef CONFIG_NAND_VF610_NFC
	setup_iomux_nfc();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: vf610twr\n");

	return 0;
}

/*
 * Copyright 2015 Toradex, Inc.
 *
 * Based on vf610twr:
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-vf610.h>
#include <asm/arch/ddrmc-vf610.h>

void ddrmc_setup_iomux(void)
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

void ddrmc_phy_init(void)
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

	writel(DDRMC_PHY50_DDR3_MODE |
		   DDRMC_PHY50_EN_SW_HALF_CYCLE, &ddrmr->phy[50]);

	/* Processor Pad ODT settings */
	writel(DDRMC_PHY_PROC_PAD_ODT, &ddrmr->phy[52]);
}

static void ddrmc_ctrl_lvl_init(struct ddrmc_lvl_info *lvl)
{
	struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;
	u32 cr102 = 0, cr105 = 0, cr106 = 0, cr110 = 0;

	if (lvl->wrlvl_reg_en) {
		writel(DDRMC_CR97_WRLVL_EN, &ddrmr->cr[97]);
		writel(DDRMC_CR98_WRLVL_DL_0(lvl->wrlvl_dl_0), &ddrmr->cr[98]);
		writel(DDRMC_CR99_WRLVL_DL_1(lvl->wrlvl_dl_1), &ddrmr->cr[99]);
	}

	if (lvl->rdlvl_reg_en) {
		cr102 |= DDRMC_CR102_RDLVL_REG_EN;
		cr105 |= DDRMC_CR105_RDLVL_DL_0(lvl->rdlvl_dl_0);
		cr110 |= DDRMC_CR110_RDLVL_DL_1(lvl->rdlvl_dl_1);
	}

	if (lvl->rdlvl_gt_reg_en) {
		cr102 |= DDRMC_CR102_RDLVL_GT_REGEN;
		cr106 |= DDRMC_CR106_RDLVL_GTDL_0(lvl->rdlvl_gt_dl_0);
		cr110 |= DDRMC_CR110_RDLVL_GTDL_1(lvl->rdlvl_gt_dl_1);
	}

	writel(cr102, &ddrmr->cr[102]);
	writel(cr105, &ddrmr->cr[105]);
	writel(cr106, &ddrmr->cr[106]);
	writel(cr110, &ddrmr->cr[110]);
}

void ddrmc_ctrl_init_ddr3(struct ddr3_jedec_timings const *timings,
						  struct ddrmc_lvl_info *lvl,
						  int col_diff, int row_diff)
{
	struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;

	writel(DDRMC_CR00_DRAM_CLASS_DDR3, &ddrmr->cr[0]);
	writel(DDRMC_CR02_DRAM_TINIT(timings->tinit), &ddrmr->cr[2]);
	writel(DDRMC_CR10_TRST_PWRON(timings->trst_pwron), &ddrmr->cr[10]);

	writel(DDRMC_CR11_CKE_INACTIVE(timings->cke_inactive), &ddrmr->cr[11]);
	writel(DDRMC_CR12_WRLAT(timings->wrlat) |
		   DDRMC_CR12_CASLAT_LIN(timings->caslat_lin), &ddrmr->cr[12]);
	writel(DDRMC_CR13_TRC(timings->trc) | DDRMC_CR13_TRRD(timings->trrd) |
		   DDRMC_CR13_TCCD(timings->tccd), &ddrmr->cr[13]);
	writel(DDRMC_CR14_TFAW(timings->tfaw) | DDRMC_CR14_TRP(timings->trp) |
		   DDRMC_CR14_TWTR(timings->twtr) |
		   DDRMC_CR14_TRAS_MIN(timings->tras_min), &ddrmr->cr[14]);
	writel(DDRMC_CR16_TMRD(timings->tmrd) |
		   DDRMC_CR16_TRTP(timings->trtp), &ddrmr->cr[16]);
	writel(DDRMC_CR17_TRAS_MAX(timings->tras_max) |
		   DDRMC_CR17_TMOD(timings->tmod), &ddrmr->cr[17]);
	writel(DDRMC_CR18_TCKESR(timings->tckesr) |
		   DDRMC_CR18_TCKE(timings->tcke), &ddrmr->cr[18]);

	writel(DDRMC_CR20_AP_EN, &ddrmr->cr[20]);
	writel(DDRMC_CR21_TRCD_INT(timings->trcd_int) |
		   DDRMC_CR21_CCMAP_EN, &ddrmr->cr[21]);

	writel(DDRMC_CR22_TDAL(timings->tdal), &ddrmr->cr[22]);
	writel(DDRMC_CR23_BSTLEN(3) |
		   DDRMC_CR23_TDLL(timings->tdll), &ddrmr->cr[23]);
	writel(DDRMC_CR24_TRP_AB(timings->trp_ab), &ddrmr->cr[24]);

	writel(DDRMC_CR25_TREF_EN, &ddrmr->cr[25]);
	writel(DDRMC_CR26_TREF(timings->tref) |
		   DDRMC_CR26_TRFC(timings->trfc), &ddrmr->cr[26]);
	writel(DDRMC_CR28_TREF_INT(0), &ddrmr->cr[28]);
	writel(DDRMC_CR29_TPDEX(timings->tpdex), &ddrmr->cr[29]);

	writel(DDRMC_CR30_TXPDLL(timings->txpdll), &ddrmr->cr[30]);
	writel(DDRMC_CR31_TXSNR(timings->txsnr) |
		   DDRMC_CR31_TXSR(timings->txsr), &ddrmr->cr[31]);
	writel(DDRMC_CR33_EN_QK_SREF, &ddrmr->cr[33]);
	writel(DDRMC_CR34_CKSRX(timings->cksrx) |
		   DDRMC_CR34_CKSRE(timings->cksre), &ddrmr->cr[34]);

	writel(DDRMC_CR38_FREQ_CHG_EN(0), &ddrmr->cr[38]);
	writel(DDRMC_CR39_PHY_INI_COM(1024) | DDRMC_CR39_PHY_INI_STA(16) |
		   DDRMC_CR39_FRQ_CH_DLLOFF(2), &ddrmr->cr[39]);

	writel(DDRMC_CR41_PHY_INI_STRT_INI_DIS, &ddrmr->cr[41]);
	writel(DDRMC_CR48_MR1_DA_0(70) |
		   DDRMC_CR48_MR0_DA_0(1056), &ddrmr->cr[48]);

	writel(DDRMC_CR66_ZQCL(timings->zqcl) |
		   DDRMC_CR66_ZQINIT(timings->zqinit), &ddrmr->cr[66]);
	writel(DDRMC_CR67_ZQCS(timings->zqcs), &ddrmr->cr[67]);
	writel(DDRMC_CR69_ZQ_ON_SREF_EX(2), &ddrmr->cr[69]);

	writel(DDRMC_CR70_REF_PER_ZQ(timings->ref_per_zq), &ddrmr->cr[70]);
	writel(DDRMC_CR72_ZQCS_ROTATE(0), &ddrmr->cr[72]);

	writel(DDRMC_CR73_APREBIT(timings->aprebit) |
		   DDRMC_CR73_COL_DIFF(col_diff) |
		   DDRMC_CR73_ROW_DIFF(row_diff), &ddrmr->cr[73]);
	writel(DDRMC_CR74_BANKSPLT_EN | DDRMC_CR74_ADDR_CMP_EN |
		   DDRMC_CR74_CMD_AGE_CNT(64) | DDRMC_CR74_AGE_CNT(64),
		   &ddrmr->cr[74]);
	writel(DDRMC_CR75_RW_PG_EN | DDRMC_CR75_RW_EN | DDRMC_CR75_PRI_EN |
		   DDRMC_CR75_PLEN, &ddrmr->cr[75]);
	writel(DDRMC_CR76_NQENT_ACTDIS(3) | DDRMC_CR76_D_RW_G_BKCN(3) |
		   DDRMC_CR76_W2R_SPLT_EN, &ddrmr->cr[76]);
	writel(DDRMC_CR77_CS_MAP | DDRMC_CR77_DI_RD_INTLEAVE |
		   DDRMC_CR77_SWAP_EN, &ddrmr->cr[77]);
	writel(DDRMC_CR78_Q_FULLNESS(7) |
		   DDRMC_CR78_BUR_ON_FLY_BIT(12), &ddrmr->cr[78]);
	writel(DDRMC_CR79_CTLUPD_AREF(0), &ddrmr->cr[79]);

	writel(DDRMC_CR82_INT_MASK, &ddrmr->cr[82]);

	writel(DDRMC_CR87_ODT_WR_MAPCS0, &ddrmr->cr[87]);
	writel(DDRMC_CR88_TODTL_CMD(4), &ddrmr->cr[88]);
	writel(DDRMC_CR89_AODT_RWSMCS(2), &ddrmr->cr[89]);

	writel(DDRMC_CR91_R2W_SMCSDL(2), &ddrmr->cr[91]);
	writel(DDRMC_CR96_WLMRD(timings->wlmrd) |
		   DDRMC_CR96_WLDQSEN(timings->wldqsen), &ddrmr->cr[96]);

	if (lvl != NULL)
		ddrmc_ctrl_lvl_init(lvl);

	writel(DDRMC_CR117_AXI0_W_PRI(0) |
		   DDRMC_CR117_AXI0_R_PRI(0), &ddrmr->cr[117]);
	writel(DDRMC_CR118_AXI1_W_PRI(1) |
		   DDRMC_CR118_AXI1_R_PRI(1), &ddrmr->cr[118]);

	writel(DDRMC_CR120_AXI0_PRI1_RPRI(2) |
		   DDRMC_CR120_AXI0_PRI0_RPRI(2), &ddrmr->cr[120]);
	writel(DDRMC_CR121_AXI0_PRI3_RPRI(2) |
		   DDRMC_CR121_AXI0_PRI2_RPRI(2), &ddrmr->cr[121]);
	writel(DDRMC_CR122_AXI1_PRI1_RPRI(1) | DDRMC_CR122_AXI1_PRI0_RPRI(1) |
		   DDRMC_CR122_AXI0_PRIRLX(100), &ddrmr->cr[122]);
	writel(DDRMC_CR123_AXI1_P_ODR_EN | DDRMC_CR123_AXI1_PRI3_RPRI(1) |
		   DDRMC_CR123_AXI1_PRI2_RPRI(1), &ddrmr->cr[123]);
	writel(DDRMC_CR124_AXI1_PRIRLX(100), &ddrmr->cr[124]);

	writel(DDRMC_CR126_PHY_RDLAT(8), &ddrmr->cr[126]);
	writel(DDRMC_CR132_WRLAT_ADJ(5) |
		   DDRMC_CR132_RDLAT_ADJ(6), &ddrmr->cr[132]);
	writel(DDRMC_CR137_PHYCTL_DL(2), &ddrmr->cr[137]);
	writel(DDRMC_CR138_PHY_WRLV_MXDL(256) |
		   DDRMC_CR138_PHYDRAM_CK_EN(1), &ddrmr->cr[138]);
	writel(DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
		   DDRMC_CR139_PHY_WRLV_DLL(3) |
		   DDRMC_CR139_PHY_WRLV_EN(3), &ddrmr->cr[139]);
	writel(DDRMC_CR140_PHY_WRLV_WW(64), &ddrmr->cr[140]);
	writel(DDRMC_CR143_RDLV_GAT_MXDL(1536) |
		   DDRMC_CR143_RDLV_MXDL(128), &ddrmr->cr[143]);
	writel(DDRMC_CR144_PHY_RDLVL_RES(4) | DDRMC_CR144_PHY_RDLV_LOAD(7) |
		   DDRMC_CR144_PHY_RDLV_DLL(3) |
		   DDRMC_CR144_PHY_RDLV_EN(3), &ddrmr->cr[144]);
	writel(DDRMC_CR145_PHY_RDLV_RR(64), &ddrmr->cr[145]);
	writel(DDRMC_CR146_PHY_RDLVL_RESP(64), &ddrmr->cr[146]);
	writel(DDRMC_CR147_RDLV_RESP_MASK(983040), &ddrmr->cr[147]);
	writel(DDRMC_CR148_RDLV_GATE_RESP_MASK(983040), &ddrmr->cr[148]);
	writel(DDRMC_CR151_RDLV_GAT_DQ_ZERO_CNT(1) |
		   DDRMC_CR151_RDLVL_DQ_ZERO_CNT(1), &ddrmr->cr[151]);

	writel(DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
		   DDRMC_CR154_PAD_ZQ_MODE(1) |
		   DDRMC_CR154_DDR_SEL_PAD_CONTR(3) |
		   DDRMC_CR154_PAD_ZQ_HW_FOR(1), &ddrmr->cr[154]);
	writel(DDRMC_CR155_PAD_ODT_BYTE1(2) |
		   DDRMC_CR155_PAD_ODT_BYTE0(2), &ddrmr->cr[155]);
	writel(DDRMC_CR158_TWR(6), &ddrmr->cr[158]);
	writel(DDRMC_CR161_ODT_EN(1) | DDRMC_CR161_TODTH_RD(2) |
		   DDRMC_CR161_TODTH_WR(2), &ddrmr->cr[161]);

	ddrmc_phy_init();

	writel(DDRMC_CR00_DRAM_CLASS_DDR3 | DDRMC_CR00_START, &ddrmr->cr[0]);

	while (!(readl(&ddrmr->cr[80]) && 0x100))
		udelay(10);
}

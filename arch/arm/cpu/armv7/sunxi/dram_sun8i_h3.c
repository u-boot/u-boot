/*
 * sun8i H3 platform dram controller init
 *
 * (C) Copyright 2007-2015 Allwinner Technology Co.
 *                         Jerry Wang <wangflord@allwinnertech.com>
 * (C) Copyright 2015      Vishnu Patekar <vishnupatekar0510@gmail.com>
 * (C) Copyright 2015      Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2015      Jens Kuske <jenskuske@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/dram.h>
#include <linux/kconfig.h>

struct dram_para {
	u32 read_delays;
	u32 write_delays;
	u16 page_size;
	u8 bus_width;
	u8 dual_rank;
	u8 row_bits;
};

static inline int ns_to_t(int nanoseconds)
{
	const unsigned int ctrl_freq = CONFIG_DRAM_CLK / 2;

	return DIV_ROUND_UP(ctrl_freq * nanoseconds, 1000);
}

static u32 bin_to_mgray(int val)
{
	static const u8 lookup_table[32] = {
		0x00, 0x01, 0x02, 0x03, 0x06, 0x07, 0x04, 0x05,
		0x0c, 0x0d, 0x0e, 0x0f, 0x0a, 0x0b, 0x08, 0x09,
		0x18, 0x19, 0x1a, 0x1b, 0x1e, 0x1f, 0x1c, 0x1d,
		0x14, 0x15, 0x16, 0x17, 0x12, 0x13, 0x10, 0x11,
	};

	return lookup_table[clamp(val, 0, 31)];
}

static int mgray_to_bin(u32 val)
{
	static const u8 lookup_table[32] = {
		0x00, 0x01, 0x02, 0x03, 0x06, 0x07, 0x04, 0x05,
		0x0e, 0x0f, 0x0c, 0x0d, 0x08, 0x09, 0x0a, 0x0b,
		0x1e, 0x1f, 0x1c, 0x1d, 0x18, 0x19, 0x1a, 0x1b,
		0x10, 0x11, 0x12, 0x13, 0x16, 0x17, 0x14, 0x15,
	};

	return lookup_table[val & 0x1f];
}

static void mctl_phy_init(u32 val)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	writel(val | PIR_INIT, &mctl_ctl->pir);
	mctl_await_completion(&mctl_ctl->pgsr[0], PGSR_INIT_DONE, 0x1);
}

static void mctl_dq_delay(u32 read, u32 write)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	int i, j;
	u32 val;

	for (i = 0; i < 4; i++) {
		val = DATX_IOCR_WRITE_DELAY((write >> (i * 4)) & 0xf) |
		      DATX_IOCR_READ_DELAY(((read >> (i * 4)) & 0xf) * 2);

		for (j = DATX_IOCR_DQ(0); j <= DATX_IOCR_DM; j++)
			writel(val, &mctl_ctl->datx[i].iocr[j]);
	}

	clrbits_le32(&mctl_ctl->pgcr[0], 1 << 26);

	for (i = 0; i < 4; i++) {
		val = DATX_IOCR_WRITE_DELAY((write >> (16 + i * 4)) & 0xf) |
		      DATX_IOCR_READ_DELAY((read >> (16 + i * 4)) & 0xf);

		writel(val, &mctl_ctl->datx[i].iocr[DATX_IOCR_DQS]);
		writel(val, &mctl_ctl->datx[i].iocr[DATX_IOCR_DQSN]);
	}

	setbits_le32(&mctl_ctl->pgcr[0], 1 << 26);

	udelay(1);
}

static void mctl_set_master_priority(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	/* enable bandwidth limit windows and set windows size 1us */
	writel(0x00010190, &mctl_com->bwcr);

	/* set cpu high priority */
	writel(0x00000001, &mctl_com->mapr);

	writel(0x0200000d, &mctl_com->mcr[0][0]);
	writel(0x00800100, &mctl_com->mcr[0][1]);
	writel(0x06000009, &mctl_com->mcr[1][0]);
	writel(0x01000400, &mctl_com->mcr[1][1]);
	writel(0x0200000d, &mctl_com->mcr[2][0]);
	writel(0x00600100, &mctl_com->mcr[2][1]);
	writel(0x0100000d, &mctl_com->mcr[3][0]);
	writel(0x00200080, &mctl_com->mcr[3][1]);
	writel(0x07000009, &mctl_com->mcr[4][0]);
	writel(0x01000640, &mctl_com->mcr[4][1]);
	writel(0x0100000d, &mctl_com->mcr[5][0]);
	writel(0x00200080, &mctl_com->mcr[5][1]);
	writel(0x01000009, &mctl_com->mcr[6][0]);
	writel(0x00400080, &mctl_com->mcr[6][1]);
	writel(0x0100000d, &mctl_com->mcr[7][0]);
	writel(0x00400080, &mctl_com->mcr[7][1]);
	writel(0x0100000d, &mctl_com->mcr[8][0]);
	writel(0x00400080, &mctl_com->mcr[8][1]);
	writel(0x04000009, &mctl_com->mcr[9][0]);
	writel(0x00400100, &mctl_com->mcr[9][1]);
	writel(0x2000030d, &mctl_com->mcr[10][0]);
	writel(0x04001800, &mctl_com->mcr[10][1]);
	writel(0x04000009, &mctl_com->mcr[11][0]);
	writel(0x00400120, &mctl_com->mcr[11][1]);
}

static void mctl_set_timing_params(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	u8 tccd		= 2;
	u8 tfaw		= ns_to_t(50);
	u8 trrd		= max(ns_to_t(10), 4);
	u8 trcd		= ns_to_t(15);
	u8 trc		= ns_to_t(53);
	u8 txp		= max(ns_to_t(8), 3);
	u8 twtr		= max(ns_to_t(8), 4);
	u8 trtp		= max(ns_to_t(8), 4);
	u8 twr		= max(ns_to_t(15), 3);
	u8 trp		= ns_to_t(15);
	u8 tras		= ns_to_t(38);
	u16 trefi	= ns_to_t(7800) / 32;
	u16 trfc	= ns_to_t(350);

	u8 tmrw		= 0;
	u8 tmrd		= 4;
	u8 tmod		= 12;
	u8 tcke		= 3;
	u8 tcksrx	= 5;
	u8 tcksre	= 5;
	u8 tckesr	= 4;
	u8 trasmax	= 24;

	u8 tcl		= 6; /* CL 12 */
	u8 tcwl		= 4; /* CWL 8 */
	u8 t_rdata_en	= 4;
	u8 wr_latency	= 2;

	u32 tdinit0	= (500 * CONFIG_DRAM_CLK) + 1;		/* 500us */
	u32 tdinit1	= (360 * CONFIG_DRAM_CLK) / 1000 + 1;	/* 360ns */
	u32 tdinit2	= (200 * CONFIG_DRAM_CLK) + 1;		/* 200us */
	u32 tdinit3	= (1 * CONFIG_DRAM_CLK) + 1;		/* 1us */

	u8 twtp		= tcwl + 2 + twr;	/* WL + BL / 2 + tWR */
	u8 twr2rd	= tcwl + 2 + twtr;	/* WL + BL / 2 + tWTR */
	u8 trd2wr	= tcl + 2 + 1 - tcwl;	/* RL + BL / 2 + 2 - WL */

	/* set mode register */
	writel(0x1c70, &mctl_ctl->mr[0]);	/* CL=11, WR=12 */
	writel(0x40, &mctl_ctl->mr[1]);
	writel(0x18, &mctl_ctl->mr[2]);		/* CWL=8 */
	writel(0x0, &mctl_ctl->mr[3]);

	/* set DRAM timing */
	writel(DRAMTMG0_TWTP(twtp) | DRAMTMG0_TFAW(tfaw) |
	       DRAMTMG0_TRAS_MAX(trasmax) | DRAMTMG0_TRAS(tras),
	       &mctl_ctl->dramtmg[0]);
	writel(DRAMTMG1_TXP(txp) | DRAMTMG1_TRTP(trtp) | DRAMTMG1_TRC(trc),
	       &mctl_ctl->dramtmg[1]);
	writel(DRAMTMG2_TCWL(tcwl) | DRAMTMG2_TCL(tcl) |
	       DRAMTMG2_TRD2WR(trd2wr) | DRAMTMG2_TWR2RD(twr2rd),
	       &mctl_ctl->dramtmg[2]);
	writel(DRAMTMG3_TMRW(tmrw) | DRAMTMG3_TMRD(tmrd) | DRAMTMG3_TMOD(tmod),
	       &mctl_ctl->dramtmg[3]);
	writel(DRAMTMG4_TRCD(trcd) | DRAMTMG4_TCCD(tccd) | DRAMTMG4_TRRD(trrd) |
	       DRAMTMG4_TRP(trp), &mctl_ctl->dramtmg[4]);
	writel(DRAMTMG5_TCKSRX(tcksrx) | DRAMTMG5_TCKSRE(tcksre) |
	       DRAMTMG5_TCKESR(tckesr) | DRAMTMG5_TCKE(tcke),
	       &mctl_ctl->dramtmg[5]);

	/* set two rank timing */
	clrsetbits_le32(&mctl_ctl->dramtmg[8], (0xff << 8) | (0xff << 0),
			(0x66 << 8) | (0x10 << 0));

	/* set PHY interface timing, write latency and read latency configure */
	writel((0x2 << 24) | (t_rdata_en << 16) | (0x1 << 8) |
	       (wr_latency << 0), &mctl_ctl->pitmg[0]);

	/* set PHY timing, PTR0-2 use default */
	writel(PTR3_TDINIT0(tdinit0) | PTR3_TDINIT1(tdinit1), &mctl_ctl->ptr[3]);
	writel(PTR4_TDINIT2(tdinit2) | PTR4_TDINIT3(tdinit3), &mctl_ctl->ptr[4]);

	/* set refresh timing */
	writel(RFSHTMG_TREFI(trefi) | RFSHTMG_TRFC(trfc), &mctl_ctl->rfshtmg);
}

static void mctl_zq_calibration(struct dram_para *para)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	int i;
	u16 zq_val[6];
	u8 val;

	writel(0x0a0a0a0a, &mctl_ctl->zqdr[2]);

	for (i = 0; i < 6; i++) {
		u8 zq = (CONFIG_DRAM_ZQ >> (i * 4)) & 0xf;

		writel((zq << 20) | (zq << 16) | (zq << 12) |
				(zq << 8) | (zq << 4) | (zq << 0),
				&mctl_ctl->zqcr);

		writel(PIR_CLRSR, &mctl_ctl->pir);
		mctl_phy_init(PIR_ZCAL);

		zq_val[i] = readl(&mctl_ctl->zqdr[0]) & 0xff;
		writel(REPEAT_BYTE(zq_val[i]), &mctl_ctl->zqdr[2]);

		writel(PIR_CLRSR, &mctl_ctl->pir);
		mctl_phy_init(PIR_ZCAL);

		val = readl(&mctl_ctl->zqdr[0]) >> 24;
		zq_val[i] |= bin_to_mgray(mgray_to_bin(val) - 1) << 8;
	}

	writel((zq_val[1] << 16) | zq_val[0], &mctl_ctl->zqdr[0]);
	writel((zq_val[3] << 16) | zq_val[2], &mctl_ctl->zqdr[1]);
	writel((zq_val[5] << 16) | zq_val[4], &mctl_ctl->zqdr[2]);
}

static void mctl_set_cr(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;

	writel(MCTL_CR_BL8 | MCTL_CR_2T | MCTL_CR_DDR3 | MCTL_CR_INTERLEAVED |
	       MCTL_CR_EIGHT_BANKS | MCTL_CR_BUS_WIDTH(para->bus_width) |
	       (para->dual_rank ? MCTL_CR_DUAL_RANK : MCTL_CR_SINGLE_RANK) |
	       MCTL_CR_PAGE_SIZE(para->page_size) |
	       MCTL_CR_ROW_BITS(para->row_bits), &mctl_com->cr);
}

static void mctl_sys_init(struct dram_para *para)
{
	struct sunxi_ccm_reg * const ccm =
			(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	clrbits_le32(&ccm->mbus0_clk_cfg, MBUS_CLK_GATE);
	clrbits_le32(&ccm->mbus_reset, CCM_MBUS_RESET_RESET);
	clrbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	clrbits_le32(&ccm->pll5_cfg, CCM_PLL5_CTRL_EN);
	udelay(10);

	clrbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_RST);
	udelay(1000);

	clock_set_pll5(CONFIG_DRAM_CLK * 2 * 1000000, false);
	clrsetbits_le32(&ccm->dram_clk_cfg,
			CCM_DRAMCLK_CFG_DIV_MASK | CCM_DRAMCLK_CFG_SRC_MASK,
			CCM_DRAMCLK_CFG_DIV(1) | CCM_DRAMCLK_CFG_SRC_PLL5 |
			CCM_DRAMCLK_CFG_UPD);
	mctl_await_completion(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_UPD, 0);

	setbits_le32(&ccm->ahb_reset0_cfg, 1 << AHB_RESET_OFFSET_MCTL);
	setbits_le32(&ccm->ahb_gate0, 1 << AHB_GATE_OFFSET_MCTL);
	setbits_le32(&ccm->mbus_reset, CCM_MBUS_RESET_RESET);
	setbits_le32(&ccm->mbus0_clk_cfg, MBUS_CLK_GATE);

	setbits_le32(&ccm->dram_clk_cfg, CCM_DRAMCLK_CFG_RST);
	udelay(10);

	writel(0xc00e, &mctl_ctl->clken);
	udelay(500);
}

static int mctl_channel_init(struct dram_para *para)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	unsigned int i;

	mctl_set_cr(para);
	mctl_set_timing_params(para);
	mctl_set_master_priority();

	/* setting VTC, default disable all VT */
	clrbits_le32(&mctl_ctl->pgcr[0], (1 << 30) | 0x3f);
	clrsetbits_le32(&mctl_ctl->pgcr[1], 1 << 24, 1 << 26);

	/* increase DFI_PHY_UPD clock */
	writel(PROTECT_MAGIC, &mctl_com->protect);
	udelay(100);
	clrsetbits_le32(&mctl_ctl->upd2, 0xfff << 16, 0x50 << 16);
	writel(0x0, &mctl_com->protect);
	udelay(100);

	/* set dramc odt */
	for (i = 0; i < 4; i++)
		clrsetbits_le32(&mctl_ctl->datx[i].gcr, (0x3 << 4) |
				(0x1 << 1) | (0x3 << 2) | (0x3 << 12) |
				(0x3 << 14),
				IS_ENABLED(CONFIG_DRAM_ODT_EN) ? 0x0 : 0x2);

	/* AC PDR should always ON */
	setbits_le32(&mctl_ctl->aciocr, 0x1 << 1);

	/* set DQS auto gating PD mode */
	setbits_le32(&mctl_ctl->pgcr[2], 0x3 << 6);

	/* dx ddr_clk & hdr_clk dynamic mode */
	clrbits_le32(&mctl_ctl->pgcr[0], (0x3 << 14) | (0x3 << 12));

	/* dphy & aphy phase select 270 degree */
	clrsetbits_le32(&mctl_ctl->pgcr[2], (0x3 << 10) | (0x3 << 8),
			(0x1 << 10) | (0x2 << 8));

	/* set half DQ */
	if (para->bus_width != 32) {
		writel(0x0, &mctl_ctl->datx[2].gcr);
		writel(0x0, &mctl_ctl->datx[3].gcr);
	}

	/* data training configuration */
	clrsetbits_le32(&mctl_ctl->dtcr, 0xf << 24,
			(para->dual_rank ? 0x3 : 0x1) << 24);


	if (para->read_delays || para->write_delays) {
		mctl_dq_delay(para->read_delays, para->write_delays);
		udelay(50);
	}

	mctl_zq_calibration(para);

	mctl_phy_init(PIR_PLLINIT | PIR_DCAL | PIR_PHYRST | PIR_DRAMRST |
		      PIR_DRAMINIT | PIR_QSGATE);

	/* detect ranks and bus width */
	if (readl(&mctl_ctl->pgsr[0]) & (0xfe << 20)) {
		/* only one rank */
		if (((readl(&mctl_ctl->datx[0].gsr[0]) >> 24) & 0x2) ||
		    ((readl(&mctl_ctl->datx[1].gsr[0]) >> 24) & 0x2)) {
			clrsetbits_le32(&mctl_ctl->dtcr, 0xf << 24, 0x1 << 24);
			para->dual_rank = 0;
		}

		/* only half DQ width */
		if (((readl(&mctl_ctl->datx[2].gsr[0]) >> 24) & 0x1) ||
		    ((readl(&mctl_ctl->datx[3].gsr[0]) >> 24) & 0x1)) {
			writel(0x0, &mctl_ctl->datx[2].gcr);
			writel(0x0, &mctl_ctl->datx[3].gcr);
			para->bus_width = 16;
		}

		mctl_set_cr(para);
		udelay(20);

		/* re-train */
		mctl_phy_init(PIR_QSGATE);
		if (readl(&mctl_ctl->pgsr[0]) & (0xfe << 20))
			return 1;
	}

	/* check the dramc status */
	mctl_await_completion(&mctl_ctl->statr, 0x1, 0x1);

	/* liuke added for refresh debug */
	setbits_le32(&mctl_ctl->rfshctl0, 0x1 << 31);
	udelay(10);
	clrbits_le32(&mctl_ctl->rfshctl0, 0x1 << 31);
	udelay(10);

	/* set PGCR3, CKE polarity */
	writel(0x00aa0060, &mctl_ctl->pgcr[3]);

	/* power down zq calibration module for power save */
	setbits_le32(&mctl_ctl->zqcr, ZQCR_PWRDOWN);

	/* enable master access */
	writel(0xffffffff, &mctl_com->maer);

	return 0;
}

static void mctl_auto_detect_dram_size(struct dram_para *para)
{
	/* detect row address bits */
	para->page_size = 512;
	para->row_bits = 16;
	mctl_set_cr(para);

	for (para->row_bits = 11; para->row_bits < 16; para->row_bits++)
		if (mctl_mem_matches((1 << (para->row_bits + 3)) * para->page_size))
			break;

	/* detect page size */
	para->page_size = 8192;
	mctl_set_cr(para);

	for (para->page_size = 512; para->page_size < 8192; para->page_size *= 2)
		if (mctl_mem_matches(para->page_size))
			break;
}

unsigned long sunxi_dram_init(void)
{
	struct sunxi_mctl_com_reg * const mctl_com =
			(struct sunxi_mctl_com_reg *)SUNXI_DRAM_COM_BASE;
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	struct dram_para para = {
		.read_delays = 0x00007979,	/* dram_tpr12 */
		.write_delays = 0x6aaa0000,	/* dram_tpr11 */
		.dual_rank = 0,
		.bus_width = 32,
		.row_bits = 15,
		.page_size = 4096,
	};

	mctl_sys_init(&para);
	if (mctl_channel_init(&para))
		return 0;

	if (para.dual_rank)
		writel(0x00000303, &mctl_ctl->odtmap);
	else
		writel(0x00000201, &mctl_ctl->odtmap);
	udelay(1);

	/* odt delay */
	writel(0x0c000400, &mctl_ctl->odtcfg);

	/* clear credit value */
	setbits_le32(&mctl_com->cccr, 1 << 31);
	udelay(10);

	mctl_auto_detect_dram_size(&para);
	mctl_set_cr(&para);

	return (1 << (para.row_bits + 3)) * para.page_size *
						(para.dual_rank ? 2 : 1);
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */
#include <asm/io.h>
#include <asm/arch/fmc_hdr.h>
#include <asm/arch/scu.h>
#include <asm/arch/sdram.h>
#include <config.h>
#include <dm.h>
#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <ram.h>

enum ddr_type {
	DDR4_1600 = 0x0,
	DDR4_2400,
	DDR4_3200,
	DDR5_3200,

	DDR_TYPES
};

enum ddr_size {
	DDR_SIZE_256MB,
	DDR_SIZE_512MB,
	DDR_SIZE_1GB,
	DDR_SIZE_2GB,

	DDR_SIZE_MAX,
};

#define IS_DDR4(t) \
	(((t) <= DDR4_3200) ? 1 : 0)

struct sdrammc_ac_timing {
	u32 t_cl;
	u32 t_cwl;
	u32 t_bl;
	u32 t_rcd;	/* ACT-to-read/write command delay */
	u32 t_rp;	/* PRE command period */
	u32 t_ras;	/* ACT-to-PRE command delay */
	u32 t_rrd;	/* ACT-to-ACT delay for different BG */
	u32 t_rrd_l;	/* ACT-to-ACT delay for same BG */
	u32 t_faw;	/* Four active window */
	u32 t_rtp;	/* Read-to-PRE command delay */
	u32 t_wtr;	/* Minimum write to read command for different BG */
	u32 t_wtr_l;	/* Minimum write to read command for same BG */
	u32 t_wtr_a;	/* Write to read command for same BG with auto precharge */
	u32 t_wtp;	/* Minimum write to precharge command delay */
	u32 t_rtw;	/* minimum read to write command */
	u32 t_ccd_l;	/* CAS-to-CAS delay for same BG */
	u32 t_dllk;	/* DLL locking time */
	u32 t_cksre;	/* valid clock before after self-refresh or power-down entry/exit process */
	u32 t_pd;	/* power-down entry to exit minimum width */
	u32 t_xp;	/* exit power-down to valid command delay */
	u32 t_rfc;      /* refresh time period */
	u32 t_mrd;
	u32 t_refsbrd;
	u32 t_rfcsb;
	u32 t_cshsr;
	u32 t_zq;
};

static const struct sdrammc_ac_timing ac_table[] = {
	[DDR4_1600] = {
		.t_cl = 10,	.t_cwl = 9,	.t_bl = 8,	 .t_rcd = 10,
		.t_rp = 10,	.t_ras = 28,	.t_rrd = 5,	 .t_rrd_l = 6,
		.t_faw = 28,	.t_rtp = 6,	.t_wtr = 2,	 .t_wtr_l = 6,
		.t_wtr_a = 0,	.t_wtp = 12,	.t_rtw = 0,	 .t_ccd_l = 5,
		.t_dllk = 597,	.t_cksre = 8,	.t_pd = 4,	 .t_xp = 5,
		.t_rfc = 880,	.t_mrd = 24,	.t_refsbrd = 0,	 .t_rfcsb = 0,
		.t_cshsr = 0,	.t_zq = 80,
	},
	[DDR4_2400] = {
		.t_cl = 15,	.t_cwl = 12,	.t_bl = 8,	 .t_rcd = 16,
		.t_rp = 16,	.t_ras = 39,	.t_rrd = 7,	 .t_rrd_l = 8,
		.t_faw = 37,	.t_rtp = 10,	.t_wtr = 4,	 .t_wtr_l = 10,
		.t_wtr_a = 0,	.t_wtp = 19,	.t_rtw = 0,	 .t_ccd_l = 7,
		.t_dllk = 768,	.t_cksre = 13,	.t_pd = 7,	 .t_xp = 8,
		.t_rfc = 880,	.t_mrd = 24,	.t_refsbrd = 0,	 .t_rfcsb = 0,
		.t_cshsr = 0,	.t_zq = 80,
	},
	[DDR4_3200] = {
		.t_cl = 20,	.t_cwl = 16,	.t_bl = 8,	 .t_rcd = 20,
		.t_rp = 20,	.t_ras = 52,	.t_rrd = 9,	 .t_rrd_l = 11,
		.t_faw = 48,	.t_rtp = 12,	.t_wtr = 4,	 .t_wtr_l = 12,
		.t_wtr_a = 0,	.t_wtp = 24,	.t_rtw = 0,	 .t_ccd_l = 8,
		.t_dllk = 1023,	.t_cksre = 16,	.t_pd = 8,	 .t_xp = 10,
		.t_rfc = 880,	.t_mrd = 24,	.t_refsbrd = 0,	 .t_rfcsb = 0,
		.t_cshsr = 0,	.t_zq = 80,
	},
	[DDR5_3200] = {
		.t_cl = 26,	.t_cwl = 24,	.t_bl = 16,	 .t_rcd = 26,
		.t_rp = 26,	.t_ras = 52,	.t_rrd = 8,	 .t_rrd_l = 8,
		.t_faw = 40,	.t_rtp = 12,	.t_wtr = 4,	 .t_wtr_l = 16,
		.t_wtr_a = 36,	.t_wtp = 48,	.t_rtw = 0,	 .t_ccd_l = 8,
		.t_dllk = 1024,	.t_cksre = 9,	.t_pd = 13,	 .t_xp = 13,
		.t_rfc = 880,	.t_mrd = 23,	.t_refsbrd = 48, .t_rfcsb = 208,
		.t_cshsr = 30,
		.t_zq = 48,
	},
};

struct sdrammc {
	u32 type;
	void __iomem *regs;
	void __iomem *phy;
	void __iomem *scu0;
	void __iomem *scu1;
	const struct sdrammc_ac_timing *ac;
	struct ram_info info;
};

static size_t ast2700_sdrammc_get_vga_mem_size(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;
	void *scu0 = sdrammc->scu0;
	size_t vga_memsz[] = {
		SZ_32M,
		SZ_64M,
	};
	u32 reg, sel, dual = 0;

	sel = readl(&regs->gfmcfg) & 0x1;

	reg = readl(scu0 + SCU0_PCI_MISC70);
	if (reg & SCU0_PCI_MISC70_EN_PCIEVGA0) {
		debug("VGA0:%dMB\n", vga_memsz[sel] / SZ_1M);
		dual++;
	}

	reg = readl(scu0 + SCU0_PCI_MISC80);
	if (reg & SCU0_PCI_MISC80_EN_PCIEVGA1) {
		debug("VGA1:%dMB\n", vga_memsz[sel] / SZ_1M);
		dual++;
	}

	return vga_memsz[sel] * dual;
}

static int sdrammc_calc_size(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 val, test_pattern = 0xdeadbeef;
	size_t sz;

	struct {
		u32 size;
		int rfc[2];
	} ddr_capacity[] = {
		{ 0x10000000UL,	{208, 256} }, /* 256MB */
		{ 0x20000000UL,	{208, 416} }, /* 512MB */
		{ 0x40000000UL,	{208, 560} }, /* 1GB */
		{ 0x80000000UL,	{472, 880} }, /* 2GB */
	};

	/* Configure ram size to max to enable whole area */
	val = readl(&regs->mcfg);
	val &= ~(0x7 << 2);
	writel(val | (DDR_SIZE_2GB << 2), &regs->mcfg);

	/* Clear basement. */
	writel(0, (void *)CFG_SYS_SDRAM_BASE);

	for (sz = DDR_SIZE_2GB - 1; sz > DDR_SIZE_256MB; sz--) {
		test_pattern = (test_pattern << 4) + sz;
		writel(test_pattern, (void *)(CFG_SYS_SDRAM_BASE + ddr_capacity[sz].size));

		if (readl((void *)CFG_SYS_SDRAM_BASE) != test_pattern)
			break;
	}

	/* re-configure ram size to dramc. */
	val = readl(&regs->mcfg);
	val &= ~(0x7 << 2);
	writel(val | ((sz + 1) << 2), &regs->mcfg);

	/* update rfc in ac_timing5 register. */
	val = readl(&regs->actime5);
	val &= ~(0x3ff);
	val |= (ddr_capacity[sz + 1].rfc[IS_DDR4(sdrammc->type)] >> 1);
	writel(val, &regs->actime5);

	/* report actual ram base and size to kernel */
	sdrammc->info.base = CFG_SYS_SDRAM_BASE;
	sdrammc->info.size = ddr_capacity[sz + 1].size;

	/* reserve the VGA memory */
	sdrammc->info.size -= ast2700_sdrammc_get_vga_mem_size(sdrammc);

	return 0;
}

static int sdrammc_bist(struct sdrammc *sdrammc, u32 addr, u32 size, u32 cfg, u32 timeout)
{
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 val;
	u32 err = 0;

	writel(0, &regs->bistcfg);
	writel(cfg, &regs->bistcfg);
	writel(addr >> 4, &regs->bist_addr);
	writel(size >> 4, &regs->bist_size);
	writel(0x89abcdef, &regs->bist_patt);
	writel(cfg | DRAMC_BISTCFG_START, &regs->bistcfg);

	while (!(readl(&regs->intr_status) & DRAMC_IRQSTA_BIST_DONE))
		;

	writel(DRAMC_IRQSTA_BIST_DONE, &regs->intr_clear);

	val = readl(&regs->bist_res);

	if (val & DRAMC_BISTRES_DONE) {
		if (val & DRAMC_BISTRES_FAIL)
			err++;
	} else {
		err++;
	}

	return err;
}

static void sdrammc_enable_refresh(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;

	/* refresh update */
	clrbits_le32(&regs->refctl, 0x8000);
}

static void sdrammc_mr_send(struct sdrammc *sdrammc, u32 ctrl, u32 op)
{
	struct sdrammc_regs *regs = sdrammc->regs;

	writel(op, &regs->mrwr);
	writel(ctrl | DRAMC_MRCTL_CMD_START, &regs->mrctl);

	while (!(readl(&regs->intr_status) & DRAMC_IRQSTA_MR_DONE))
		;

	writel(DRAMC_IRQSTA_MR_DONE, &regs->intr_clear);
}

static void sdrammc_config_mrs(struct sdrammc *sdrammc)
{
	const struct sdrammc_ac_timing *ac = sdrammc->ac;
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 mr0_cas, mr0_rtp, mr0_val;
	u32 mr6_tccd_l, mr6_val;
	u32 mr2_cwl, mr2_val;
	u32 mr1_val;
	u32 mr3_val;
	u32 mr4_val;
	u32 mr5_val;

	if (!IS_DDR4(sdrammc->type))
		return;

	//-------------------------------------------------------------------
	// CAS Latency (Table-15)
	//-------------------------------------------------------------------
	switch (ac->t_cl) {
	case 9:
		mr0_cas = 0x00; //5'b00000;
		break;
	case 10:
		mr0_cas = 0x01; //5'b00001;
		break;
	case 11:
		mr0_cas = 0x02; //5'b00010;
		break;
	case 12:
		mr0_cas = 0x03; //5'b00011;
		break;
	case 13:
		mr0_cas = 0x04; //5'b00100;
		break;
	case 14:
		mr0_cas = 0x05; //5'b00101;
		break;
	case 15:
		mr0_cas = 0x06; //5'b00110;
		break;
	case 16:
		mr0_cas = 0x07; //5'b00111;
		break;
	case 18:
		mr0_cas = 0x08; //5'b01000;
		break;
	case 20:
		mr0_cas = 0x09; //5'b01001;
		break;
	case 22:
		mr0_cas = 0x0a; //5'b01010;
		break;
	case 24:
		mr0_cas = 0x0b; //5'b01011;
		break;
	case 23:
		mr0_cas = 0x0c; //5'b01100;
		break;
	case 17:
		mr0_cas = 0x0d; //5'b01101;
		break;
	case 19:
		mr0_cas = 0x0e; //5'b01110;
		break;
	case 21:
		mr0_cas = 0x0f; //5'b01111;
		break;
	case 25:
		mr0_cas = 0x10; //5'b10000;
		break;
	case 26:
		mr0_cas = 0x11; //5'b10001;
		break;
	case 27:
		mr0_cas = 0x12; //5'b10010;
		break;
	case 28:
		mr0_cas = 0x13; //5'b10011;
		break;
	case 30:
		mr0_cas = 0x15; //5'b10101;
		break;
	case 32:
		mr0_cas = 0x17; //5'b10111;
		break;
	}

	//-------------------------------------------------------------------
	// WR and RTP (Table-14)
	//-------------------------------------------------------------------
	switch (ac->t_rtp) {
	case 5:
		mr0_rtp = 0x0; //4'b0000;
		break;
	case 6:
		mr0_rtp = 0x1; //4'b0001;
		break;
	case 7:
		mr0_rtp = 0x2; //4'b0010;
		break;
	case 8:
		mr0_rtp = 0x3; //4'b0011;
		break;
	case 9:
		mr0_rtp = 0x4; //4'b0100;
		break;
	case 10:
		mr0_rtp = 0x5; //4'b0101;
		break;
	case 12:
		mr0_rtp = 0x6; //4'b0110;
		break;
	case 11:
		mr0_rtp = 0x7; //4'b0111;
		break;
	case 13:
		mr0_rtp = 0x8; //4'b1000;
		break;
	}

	//-------------------------------------------------------------------
	// CAS Write Latency (Table-21)
	//-------------------------------------------------------------------
	switch (ac->t_cwl)  {
	case 9:
		mr2_cwl = 0x0; // 3'b000; // 1600
		break;
	case 10:
		mr2_cwl = 0x1; // 3'b001; // 1866
		break;
	case 11:
		mr2_cwl = 0x2; // 3'b010; // 2133
		break;
	case 12:
		mr2_cwl = 0x3; // 3'b011; // 2400
		break;
	case 14:
		mr2_cwl = 0x4; // 3'b100; // 2666
		break;
	case 16:
		mr2_cwl = 0x5; // 3'b101; // 2933/3200
		break;
	case 18:
		mr2_cwl = 0x6; // 3'b110;
		break;
	case 20:
		mr2_cwl = 0x7; // 3'b111;
		break;
	}

	//-------------------------------------------------------------------
	// tCCD_L and tDLLK
	//-------------------------------------------------------------------
	switch (ac->t_ccd_l) {
	case 4:
		mr6_tccd_l = 0x0; //3'b000;  // rate <= 1333
		break;
	case 5:
		mr6_tccd_l = 0x1; //3'b001;  // 1333 < rate <= 1866
		break;
	case 6:
		mr6_tccd_l = 0x2; //3'b010;  // 1866 < rate <= 2400
		break;
	case 7:
		mr6_tccd_l = 0x3; //3'b011;  // 2400 < rate <= 2666
		break;
	case 8:
		mr6_tccd_l = 0x4; //3'b100;  // 2666 < rate <= 3200
		break;
	}

	/*
	 * mr0_val =
	 * mr0_rtp[3],		// 13
	 * mr0_cas[4],		// 12
	 * mr0_rtp[2:0],	// 13,11-9: WR and RTP
	 * 1'b0,		// 8: DLL reset
	 * 1'b0,		// 7: TM
	 * mr0_cas[3:1],	// 6-4,2: CAS latency
	 * 1'b0,		// 3: sequential
	 * mr0_cas[0],
	 * 2'b00		// 1-0: burst length
	 */
	mr0_val = ((mr0_cas & 0x1) << 2) |
		   (((mr0_cas >> 1) & 0x7) << 4) |
		   (((mr0_cas >> 4) & 0x1) << 12) |
		   ((mr0_rtp & 0x7) << 9) |
		   (((mr0_rtp >> 3) & 0x1) << 13);

	/*
	 * 3'b2 //[10:8]: rtt_nom, 000:disable,001:rzq/4,010:rzq/2,011:rzq/6,100:rzq/1,101:rzq/5,110:rzq/3,111:rzq/7
	 * 1'b0 //[7]: write leveling enable
	 * 2'b0 //[6:5]: reserved
	 * 2'b0 //[4:3]: additive latency
	 * 2'b0 //[2:1]: output driver impedance
	 * 1'b1 //[0]: enable dll
	 */
	mr1_val = 0x201;

	/*
	 * [10:9]: rtt_wr, 00:dynamic odt off, 01:rzq/2, 10:rzq/1, 11: hi-z
	 * [8]: 0
	 */
	mr2_val = ((mr2_cwl & 0x7) << 3) | 0x200;

	mr3_val = 0;

	mr4_val = 0;

	/*
	 * mr5_val = {
	 * 1'b0,		// 13: RFU
	 * 1'b0,		// 12: read DBI
	 * 1'b0,		// 11: write DBI
	 * 1'b1,		// 10: Data mask
	 * 1'b0,		// 9: C/A parity persistent error
	 * 3'b000,		// 8-6: RTT_PARK (disable)
	 * 1'b1,		// 5: ODT input buffer during power down mode
	 * 1'b0,		// 4: C/A parity status
	 * 1'b0,		// 3: CRC error clear
	 * 3'b0			// 2-0: C/A parity latency mode
	 * };
	 */
	mr5_val = 0x420;

	/*
	 * mr6_val = {
	 * 1'b0,		// 13, 9-8: RFU
	 * mr6_tccd_l[2:0],	// 12-10: tCCD_L
	 * 2'b0,		// 13, 9-8: RFU
	 * 1'b0,		// 7: VrefDQ training enable
	 * 1'b0,		// 6: VrefDQ training range
	 * 6'b0			// 5-0: VrefDQ training value
	 * };
	 */
	mr6_val = ((mr6_tccd_l & 0x7) << 10);

	writel((mr1_val << 16) + mr0_val, &regs->mr01);
	writel((mr3_val << 16) + mr2_val, &regs->mr23);
	writel((mr5_val << 16) + mr4_val, &regs->mr45);
	writel(mr6_val, &regs->mr67);

	/* Power-up initialization sequence */
	sdrammc_mr_send(sdrammc, MR_ADDR(3), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(6), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(5), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(4), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(2), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(1), 0);
	sdrammc_mr_send(sdrammc, MR_ADDR(0), 0);
}

static void sdrammc_exit_self_refresh(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;

	/* exit self-refresh after phy init */
	setbits_le32(&regs->mctl, DRAMC_MCTL_SELF_REF_START);

	/* query if self-ref done */
	while (!(readl(&regs->intr_status) & DRAMC_IRQSTA_REF_DONE))
		;

	/* clear status */
	writel(DRAMC_IRQSTA_REF_DONE, &regs->intr_clear);
	udelay(1);
}

/* user-customized functions for the vendor PHY init code */
#define DWC_PHY_IMEM_OFST		0x50000
#define DWC_PHY_DMEM_OFST		0x58000
#define DWC_PHY_MB_START_STREAM_MSG	0x8
#define DWC_PHY_MB_TRAIN_SUCCESS	0x7
#define DWC_PHY_MB_TRAIN_FAIL		0xff

#define dwc_ddrphy_apb_wr(addr, data) \
	writew((data), sdrammc->phy + ((addr) << 1))

#define dwc_ddrphy_apb_rd(addr) \
	readw(sdrammc->phy + ((addr) << 1))

#define dwc_ddrphy_apb_wr_32b(addr, data) \
	writel((data), sdrammc->phy + ((addr) << 1))

#define dwc_ddrphy_apb_rd_32b(addr) \
	readl(sdrammc->phy + ((addr) << 1))

void dwc_get_mailbox(struct sdrammc *sdrammc, const int mode, u32 *mbox)
{
	u32 val;

	/* 1. Poll the UctWriteProtShadow, looking for a 0 */
	while (dwc_ddrphy_apb_rd(0xd0004) & BIT(0))
		;

	/* 2. When a 0 is seen, read the UctWriteOnlyShadow register to get the major message number. */
	*mbox = dwc_ddrphy_apb_rd(0xd0032) & 0xffff;

	/* 3. If reading a streaming or SMBus message, also read the UctDatWriteOnlyShadow register. */
	if (mode) {
		val = (dwc_ddrphy_apb_rd(0xd0034)) & 0xffff;
		*mbox |= (val << 16);
	}

	/* 4. Write the DctWriteProt to 0 to acknowledge the reception of the message */
	dwc_ddrphy_apb_wr(0xd0031, 0);

	/* 5. Poll the UctWriteProtShadow, looking for a 1 */
	while (!(dwc_ddrphy_apb_rd(0xd0004) & BIT(0)))
		;

	/* 6. When a 1 is seen, write the DctWriteProt to 1 to complete the protocol */
	dwc_ddrphy_apb_wr(0xd0031, 1);
}

uint32_t dwc_readMsgBlock(struct sdrammc *sdrammc, const u32 addr_half)
{
	u32 data_word;

	data_word = dwc_ddrphy_apb_rd_32b((addr_half >> 1) << 1);

	if (addr_half & 0x1)
		data_word = data_word >> 16;
	else
		data_word &= 0xffff;

	return data_word;
}

int dwc_ddrphy_phyinit_userCustom_H_readMsgBlock(struct sdrammc *sdrammc, int train2D)
{
	u32 msg;

	if (IS_DDR4(sdrammc->type)) {
		/* DWC_PHY_DDR4_MB_RESULT */
		msg = dwc_readMsgBlock(sdrammc, 0x5800a);
		if (msg & 0xff)
			debug("%s: Training Failure index (0x%x)\n", __func__, msg);
		else
			debug("%s: %dD Training Passed\n", __func__, train2D ? 2 : 1);
	} else {
		/* DWC_PHY_DDR5_MB_RESULT */
		msg = dwc_readMsgBlock(sdrammc, 0x58007);
		if (msg & 0xff00)
			debug("%s: Training Failure index (0x%x)\n", __func__, msg);
		else
			debug("%s: DDR5 1D/2D Training Passed\n", __func__);

		/* DWC_PHY_DDR5_MB_RESULT_ADR */
		msg = dwc_readMsgBlock(sdrammc, 0x5800a);
		debug("%s: Result Address Offset (0x%x)\n", __func__, msg);
	}

	return 0;
}

void dwc_ddrphy_phyinit_userCustom_A_bringupPower(void)
{
    /* do nothing */
}

void dwc_ddrphy_phyinit_userCustom_B_startClockResetPhy(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;

	/*
	 * 1. Drive PwrOkIn to 0. Note: Reset, DfiClk, and APBCLK can be X.
	 * 2. Start DfiClk and APBCLK
	 * 3. Drive Reset to 1 and PRESETn_APB to 0.
	 * Note: The combination of PwrOkIn=0 and Reset=1 signals a cold reset to the PHY.
	 */
	writel(DRAMC_MCTL_PHY_RESET, &regs->mctl);
	udelay(2);

	/*
	 * 5. Drive PwrOkIn to 1. Once the PwrOkIn is asserted (and Reset is still asserted),
	 * DfiClk synchronously switches to any legal input frequency.
	 */
	writel(DRAMC_MCTL_PHY_RESET | DRAMC_MCTL_PHY_POWER_ON, &regs->mctl);
	udelay(2);

	/*
	 * 7. Drive Reset to 0. Note: All DFI and APB inputs must be driven at valid reset states
	 * before the deassertion of Reset.
	 */
	writel(DRAMC_MCTL_PHY_POWER_ON, &regs->mctl);
	udelay(2);

	/*
	 * 9. Drive PRESETn_APB to 1 to de-assert reset on the ABP bus.
	 * 10. The PHY is now in the reset state and is ready to accept APB transactions.
	 */
}

void dwc_ddrphy_phyinit_userCustom_overrideUserInput(void)
{
	/* do nothing */
}

void dwc_ddrphy_phyinit_userCustom_customPostTrain(void)
{
	/* do nothing */
}

void dwc_ddrphy_phyinit_userCustom_E_setDfiClk(struct sdrammc *sdrammc)
{
	dwc_ddrphy_apb_wr(0xd0031, 1);	/* DWC_DCTWRITEPROT */
	dwc_ddrphy_apb_wr(0xd0033, 1);	/* DWC_UCTWRITEPROT */
}

void dwc_ddrphy_phyinit_userCustom_G_waitFwDone(struct sdrammc *sdrammc)
{
	u32 mbox, msg = 0;

	while (msg != DWC_PHY_MB_TRAIN_SUCCESS && msg != DWC_PHY_MB_TRAIN_FAIL) {
		dwc_get_mailbox(sdrammc, 0, &mbox);
		msg = mbox & 0xffff;
	}
}

void dwc_ddrphy_phyinit_userCustom_J_enterMissionMode(struct sdrammc *sdrammc)
{
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 val;

	/*
	 * 1. Set the PHY input clocks to the desired frequency.
	 * 2. Initialize the PHY to mission mode by performing DFI Initialization.
	 * Please see the DFI specification for more information. See the DFI frequency bus encoding in section <XXX>.
	 * Note: The PHY training firmware initializes the DRAM state. if skip
	 * training is used, the DRAM state is not initialized.
	 */

	writel(0xffffffff, (void *)&regs->intr_mask);

	writel(0x0, (void *)&regs->dcfg);

	if (!IS_DDR4(sdrammc->type)) {
		dwc_ddrphy_apb_wr(0xd0000, 0);		/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
		dwc_ddrphy_apb_wr(0x20240, 0x3900);	/* DWC_DDRPHYA_MASTER0_base0_D5ACSMPtr0lat0 */
		dwc_ddrphy_apb_wr(0x900da, 8);		/* DWC_DDRPHYA_INITENG0_base0_SequenceReg0b59s0 */
		dwc_ddrphy_apb_wr(0xd0000, 1);		/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	}

	/* phy init start */
	val = readl((void *)&regs->mctl);
	val = val | DRAMC_MCTL_PHY_INIT_START;
	writel(val, (void *)&regs->mctl);

	/* wait phy complete */
	while (1) {
		val = readl(&regs->intr_status) & DRAMC_IRQSTA_PHY_INIT_DONE;
		if (val == DRAMC_IRQSTA_PHY_INIT_DONE)
			break;
	}

	writel(0xffff, (void *)&regs->intr_clear);

	while (readl((void *)&regs->intr_status))
		;

	if (!IS_DDR4(sdrammc->type)) {
		dwc_ddrphy_apb_wr(0xd0000, 0);		/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
		dwc_ddrphy_apb_wr(0x20240, 0x4300);	/* DWC_DDRPHYA_MASTER0_base0_D5ACSMPtr0lat0 */
		dwc_ddrphy_apb_wr(0x900da, 0);		/* DWC_DDRPHYA_INITENG0_base0_SequenceReg0b59s0 */
		dwc_ddrphy_apb_wr(0xd0000, 1);		/* DWC_DDRPHYA_APBONLY0_MicroContMuxSel */
	}
}

int dwc_ddrphy_phyinit_userCustom_D_loadIMEM(struct sdrammc *sdrammc, const int train2D)
{
	u32 imem_ofst, imem_size;
	u32 pb_type;

	if (IS_DDR4(sdrammc->type))
		pb_type = (train2D) ? PBT_DDR4_2D_PMU_TRAIN_IMEM : PBT_DDR4_PMU_TRAIN_IMEM;
	else
		pb_type = PBT_DDR5_PMU_TRAIN_IMEM;

	fmc_hdr_get_prebuilt(pb_type, &imem_ofst, &imem_size);

	memcpy(sdrammc->phy + (DWC_PHY_IMEM_OFST << 1),
	       (void *)(0x20000000 + imem_ofst), imem_size);

	return 0;
}

int dwc_ddrphy_phyinit_userCustom_F_loadDMEM(struct sdrammc *sdrammc,
					     const int pState, const int train2D)
{
	u32 dmem_ofst, dmem_size;
	u32 pb_type;

	if (IS_DDR4(sdrammc->type))
		pb_type = (train2D) ? PBT_DDR4_2D_PMU_TRAIN_DMEM : PBT_DDR4_PMU_TRAIN_DMEM;
	else
		pb_type = PBT_DDR5_PMU_TRAIN_DMEM;

	fmc_hdr_get_prebuilt(pb_type, &dmem_ofst, &dmem_size);

	memcpy(sdrammc->phy + (DWC_PHY_DMEM_OFST << 1),
	       (void *)(0x20000000 + dmem_ofst), dmem_size);

	return 0;
}

static void sdrammc_dwc_phy_init(struct sdrammc *sdrammc)
{
	/* enable ddr phy free-run clock */
	writel(SCU0_CLKGATE1_CLR_DDRPHY, sdrammc->scu0 + SCU0_CLKGATE1_CLR);

	/* include the vendor-provided PHY init code */
	if (IS_DDR4(sdrammc->type)) {
		#include "dwc_ddrphy_phyinit_ddr4-3200-nodimm-train2D.c"
	} else {
		#include "dwc_ddrphy_phyinit_ddr5-3200-nodimm-train2D.c"
	}
}

static void sdrammc_config_ac_timing(struct sdrammc *sdrammc)
{
	const struct sdrammc_ac_timing *ac = sdrammc->ac;
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 actime;

#define ACTIME1(ccd, rrd_l, rrd, mrd)	\
	(((ccd) << 24) |		\
	 (((rrd_l) >> 1) << 16) |	\
	 (((rrd) >> 1) << 8) |		\
	 ((mrd) >> 1))

#define ACTIME2(faw, rp, ras, rcd)	\
	((((faw) >> 1) << 24) |		\
	 (((rp) >> 1) << 16) |		\
	 (((ras) >> 1) << 8) |		\
	 ((rcd) >> 1))

#define ACTIME3(wtr, rtw, wtp, rtp)	\
	((((wtr) >> 1) << 24) |		\
	 (((rtw) >> 1) << 16) |		\
	 (((wtp) >> 1) << 8) |		\
	 ((rtp) >> 1))

#define ACTIME4(wtr_a, wtr_l)		\
	((((wtr_a) >> 1) << 8) |	\
	 ((wtr_l) >> 1))

#define ACTIME5(refsbrd, rfcsb, rfc)	\
	((((refsbrd) >> 1) << 20) |	\
	 (((rfcsb) >> 1) << 10) |	\
	 ((rfc) >> 1))

#define ACTIME6(cshsr, pd, xp, cksre)	\
	((((cshsr) >> 1) << 24) |	\
	 (((pd) >> 1) << 16) |		\
	 (((xp) >> 1) << 8) |		\
	 ((cksre) >> 1))

#define ACTIME7(zqcs, dllk)		\
	((((zqcs) >> 1) << 10) |	\
	 ((dllk) >> 1))

	actime = ACTIME1(ac->t_ccd_l, ac->t_rrd_l, ac->t_rrd, ac->t_mrd);
	writel(actime, &regs->actime1);

	actime = ACTIME2(ac->t_faw, ac->t_rp, ac->t_ras, ac->t_rcd);
	writel(actime, &regs->actime2);

	actime = ACTIME3(ac->t_cwl + ac->t_bl / 2 + ac->t_wtr,
			 ac->t_cl - ac->t_cwl + (ac->t_bl / 2) + 2,
			 ac->t_cwl + ac->t_bl / 2 + ac->t_wtp,
			 ac->t_rtp);
	writel(actime, &regs->actime3);

	actime = ACTIME4(ac->t_cwl + ac->t_bl / 2 + ac->t_wtr_a,
			 ac->t_cwl + ac->t_bl / 2 + ac->t_wtr_l);
	writel(actime, &regs->actime4);

	actime = ACTIME5(ac->t_refsbrd, ac->t_rfcsb, ac->t_rfc);
	writel(actime, &regs->actime5);

	actime = ACTIME6(ac->t_cshsr, ac->t_pd, ac->t_xp, ac->t_cksre);
	writel(actime, &regs->actime6);

	actime = ACTIME7(ac->t_zq, ac->t_dllk);
	writel(actime, &regs->actime7);
}

static void sdrammc_config_registers(struct sdrammc *sdrammc)
{
	const struct sdrammc_ac_timing *ac = sdrammc->ac;
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 reg;

	u32 dram_size = 5;
	u32 t_phy_wrdata;
	u32 t_phy_wrlat;
	u32 t_phy_rddata_en;
	u32 t_phy_odtlat;
	u32 t_phy_odtext;

	if (IS_DDR4(sdrammc->type)) {
		t_phy_wrlat = ac->t_cwl - 5 - 4;
		t_phy_rddata_en = ac->t_cl - 5 - 4;
		t_phy_wrdata = 2;
		t_phy_odtlat = ac->t_cwl - 5 - 4;
		t_phy_odtext = 0;
	} else {
		t_phy_wrlat = ac->t_cwl - 13 - 3;
		t_phy_rddata_en = ac->t_cl - 13 - 3;
		t_phy_wrdata = 6;
		t_phy_odtlat = 0;
		t_phy_odtext = 0;
	}

	writel(0x20 + (dram_size << 2) + !!!IS_DDR4(sdrammc->type), &regs->mcfg);

	reg = (t_phy_odtext << 20) + (t_phy_odtlat << 16) +
	      (t_phy_rddata_en << 10) + (t_phy_wrdata << 6) +
	      t_phy_wrlat;
	writel(reg, &regs->dfi_timing);
	writel(0, &regs->dctl);

	writel(0x40b48200, &regs->refctl);

	writel(0x42aa1800, &regs->zqctl);

	writel(0, &regs->arbctl);

	if (!IS_DDR4(sdrammc->type))
		writel(0, &regs->refmng_ctl);

	writel(0xffffffff, &regs->intr_mask);
}

static void sdrammc_init(struct sdrammc *sdrammc)
{
	u32 reg;

	reg = readl(sdrammc->scu1 + SCU1_HWSTRAP1);

	if (reg & SCU1_HWSTRAP1_DDR4) {
		if (IS_ENABLED(CONFIG_ASPEED_DDR_1600))
			sdrammc->type = DDR4_1600;
		else if (IS_ENABLED(CONFIG_ASPEED_DDR_2400))
			sdrammc->type = DDR4_2400;
		else if (IS_ENABLED(CONFIG_ASPEED_DDR_3200))
			sdrammc->type = DDR4_3200;
	} else {
		sdrammc->type = DDR5_3200;
	}

	sdrammc->ac = &ac_table[sdrammc->type];

	sdrammc_config_ac_timing(sdrammc);
	sdrammc_config_registers(sdrammc);
}

static int ast2700_sdrammc_probe(struct udevice *dev)
{
	struct sdrammc *sdrammc = dev_get_priv(dev);
	struct sdrammc_regs *regs = sdrammc->regs;
	u32 bistcfg;
	u32 reg;
	int rc;

	/* skip DRAM init if already done */
	reg = readl(sdrammc->scu0 + SCU0_VGA0_SCRATCH);
	if (reg & SCU0_VGA0_SCRATCH_DRAM_INIT)
		goto out;

	/* unlock DRAM controller */
	writel(DRAMC_UNLK_KEY, &regs->prot_key);

	sdrammc_init(sdrammc);

	sdrammc_dwc_phy_init(sdrammc);

	sdrammc_exit_self_refresh(sdrammc);

	sdrammc_config_mrs(sdrammc);

	sdrammc_enable_refresh(sdrammc);

	bistcfg = FIELD_PREP(DRAMC_BISTCFG_PMODE, BIST_PMODE_CRC) |
		  FIELD_PREP(DRAMC_BISTCFG_BMODE, BIST_BMODE_RW_SWITCH) |
		  DRAMC_BISTCFG_ENABLE;

	rc = sdrammc_bist(sdrammc, 0, 0x10000, bistcfg, 0x200000);
	if (rc) {
		debug("bist test failed, type=%d\n", sdrammc->type);
		return rc;
	}

	/* set DRAM init flag */
	reg |= SCU0_VGA0_SCRATCH_DRAM_INIT;
	writel(reg, sdrammc->scu0 + SCU0_VGA0_SCRATCH);

out:
	sdrammc_calc_size(sdrammc);

	return 0;
}

static int ast2700_sdrammc_of_to_plat(struct udevice *dev)
{
	struct sdrammc *sdrammc = dev_get_priv(dev);
	u32 phandle;
	ofnode node;
	int rc;

	sdrammc->regs = (struct sdrammc_regs *)devfdt_get_addr_index(dev, 0);
	if (sdrammc->regs == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map DRAM register\n");
		return -ENODEV;
	}

	sdrammc->phy = (void *)devfdt_get_addr_index(dev, 1);
	if (sdrammc->phy == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map PHY memory\n");
		return -ENODEV;
	}

	rc = ofnode_read_u32(dev_ofnode(dev), "aspeed,scu0", &phandle);
	if (rc) {
		debug("cannot find SCU0 handle\n");
		return -ENODEV;
	}

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node)) {
		debug("cannot get SCU0 node\n");
		return -ENODEV;
	}

	sdrammc->scu0 = (void *)ofnode_get_addr(node);
	if (sdrammc->scu0 == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map SCU0 register\n");
		return -ENODEV;
	}

	rc = ofnode_read_u32(dev_ofnode(dev), "aspeed,scu1", &phandle);
	if (rc) {
		debug("cannot find SCU1 handle\n");
		return -ENODEV;
	}

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node)) {
		debug("cannot get SCU1 node\n");
		return -ENODEV;
	}

	sdrammc->scu1 = (void *)ofnode_get_addr(node);
	if (sdrammc->scu1 == (void *)FDT_ADDR_T_NONE) {
		debug("cannot map SCU1 register\n");
		return -ENODEV;
	}

	return 0;
}

static int ast2700_sdrammc_get_info(struct udevice *dev, struct ram_info *info)
{
	struct sdrammc *sdrammc = dev_get_priv(dev);

	*info = sdrammc->info;

	return 0;
}

static struct ram_ops ast2700_sdrammc_ops = {
	.get_info = ast2700_sdrammc_get_info,
};

static const struct udevice_id ast2700_sdrammc_ids[] = {
	{ .compatible = "aspeed,ast2700-sdrammc" },
	{ }
};

U_BOOT_DRIVER(sdrammc_ast2700) = {
	.name = "aspeed_ast2700_sdrammc",
	.id = UCLASS_RAM,
	.of_match = ast2700_sdrammc_ids,
	.ops = &ast2700_sdrammc_ops,
	.of_to_plat = ast2700_sdrammc_of_to_plat,
	.probe = ast2700_sdrammc_probe,
	.priv_auto = sizeof(struct sdrammc),
};

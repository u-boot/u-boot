// SPDX-License-Identifier: GPL-2.0+
/*
 * sun55i A523 LPDDR4-2133 timings, as programmed by Allwinner's boot0
 *
 * (C) Copyright 2024 Jernej Skrabec <jernej.skrabec@gmail.com>
 * (C) Copyright 2023 Mikhail Kalashnikov <iuncuim@gmail.com>
 *
 */

#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

void mctl_set_timing_params(u32 clk)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;
	u8 tcl, tcwl, t_rdata_en, trtp, twr, tphy_wrlat;
	unsigned int mr1, mr2;

	u8 tccd		= 4;
	u8 tfaw		= ns_to_t(40, clk);
	u8 trrd		= max(ns_to_t(10, clk), 2);
	u8 twtr		= max(ns_to_t(10, clk), 4);
	u8 trcd		= max(ns_to_t(18, clk), 2);
	u8 trc		= ns_to_t(65, clk);
	u8 txp		= max(ns_to_t(8, clk), 2);
	u8 trp		= ns_to_t(21, clk);
	u8 tras		= ns_to_t(42, clk);
	u16 trefi	= ns_to_t(3904, clk) / 32;
	u16 trfc	= ns_to_t(280, clk);
	u16 txsr	= ns_to_t(290, clk);

	u8 tmrw		= max(ns_to_t(14, clk), 5);
	u8 tmod		= 12;
	u8 tcke		= max(ns_to_t(15, clk), 2);
	u8 tcksrx	= max(ns_to_t(2, clk), 2);
	u8 tcksre	= max(ns_to_t(5, clk), 2);
	u8 trasmax	= (trefi * 9) / 32;

	if (clk <= 936) {
		mr1 = 0x34;
		mr2 = 0x1b;
		tcl = 10;
		tcwl = 5;
		t_rdata_en = 17;
		trtp = 4;
		tphy_wrlat = 5;
		twr = 10;
	} else if (clk <= 1200) {
		mr1 = 0x54;
		mr2 = 0x2d;
		tcl = 14;
		tcwl = 7;
		t_rdata_en = 25;
		trtp = 6;
		tphy_wrlat = 9;
		twr = 15;
	} else {
		mr1 = 0x64;
		mr2 = 0x36;
		tcl = 16;
		tcwl = 8;
		t_rdata_en = 29;
		trtp = 7;
		tphy_wrlat = 11;
		twr = 17;
	}

	u8 tmrd		= tmrw;
	u8 tckesr	= tcke;
	u8 twtp		= twr + 9 + tcwl;
	u8 twr2rd	= twtr + 9 + tcwl;
	u8 trd2wr	= ns_to_t(4, clk) + 7 - ns_to_t(1, clk) + tcl;
	u8 txs		= 4;
	u8 txsdll	= 16;
	u8 txsabort	= 4;
	u8 txsfast	= 4;

	/* set DRAM timing */
	writel((twtp << 24) | (tfaw << 16) | (trasmax << 8) | tras,
	       &mctl_ctl->dramtmg[0]);
	writel((txp << 16) | (trtp << 8) | trc, &mctl_ctl->dramtmg[1]);
	writel((tcwl << 24) | (tcl << 16) | (trd2wr << 8) | twr2rd,
	       &mctl_ctl->dramtmg[2]);
	writel((tmrw << 20) | (tmrd << 12) | tmod, &mctl_ctl->dramtmg[3]);
	writel((trcd << 24) | (tccd << 16) | (trrd << 8) | trp,
	       &mctl_ctl->dramtmg[4]);
	writel((tcksrx << 24) | (tcksre << 16) | (tckesr << 8) | tcke,
	       &mctl_ctl->dramtmg[5]);
	/* Value suggested by ZynqMP manual and used by libdram */
	writel((txp + 2) | 0x02020000, &mctl_ctl->dramtmg[6]);
	writel((txsfast << 24) | (txsabort << 16) | (txsdll << 8) | txs,
	       &mctl_ctl->dramtmg[8]);
	writel(0x00020208, &mctl_ctl->dramtmg[9]);
	writel(0xE0C05, &mctl_ctl->dramtmg[10]);
	writel(0x440C021C, &mctl_ctl->dramtmg[11]);
	writel(8, &mctl_ctl->dramtmg[12]);
	writel(0xA100002, &mctl_ctl->dramtmg[13]);
	writel(txsr, &mctl_ctl->dramtmg[14]);

	clrsetbits_le32(&mctl_ctl->init[0], 0xC0000FFF, 0x558);
	writel(0x01f20000, &mctl_ctl->init[1]);
	writel(0x00001705, &mctl_ctl->init[2]);
	writel(0, &mctl_ctl->dfimisc);
	writel((mr1 << 16) | mr2, &mctl_ctl->init[3]);
	writel(0x00330000, &mctl_ctl->init[4]);
	writel(0x00040072, &mctl_ctl->init[6]);
	writel(0x00260008, &mctl_ctl->init[7]);

	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);

	/* Configure DFI timing */
	writel(tphy_wrlat | 0x2000000 | (t_rdata_en << 16) | 0x808000,
	       &mctl_ctl->dfitmg0);
	writel(0x100202, &mctl_ctl->dfitmg1);

	/* set refresh timing */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * sun55i A523 DDR3 timings, as programmed by Allwinner's boot0 on
 * the X96QPro+ TV box. As usual very conservative timings, but probably
 * the most compatible and reliable.
 *
 * (C) Copyright 2024 Mikhail Kalashnikov <iuncuim@gmail.com>
 *   Based on H616 DDR3 timings:
 *   (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 */

#include <asm/arch/dram.h>
#include <asm/arch/cpu.h>

void mctl_set_timing_params(u32 clk)
{
	struct sunxi_mctl_ctl_reg * const mctl_ctl =
			(struct sunxi_mctl_ctl_reg *)SUNXI_DRAM_CTL0_BASE;

	/*
	 *					formulas and constraints as of
	 *					JEDEC DDR3 specification, for
	 *					DDR3-1600, per JESD79-3F
	 */
	u8 tccd		= 2;				/* 4nCK */
	u8 tfaw		= ns_to_t(50, clk);
	u8 trrd		= max(ns_to_t(6, clk), 4);	/* max(6 ns, 4nCK) */
	u8 twtr		= max(ns_to_t(8, clk), 4);	/* max(7.5 ns, 4nCK) */
	u8 trcd		= ns_to_t(15, clk);		/* 13.5 ns */
	u8 trc		= ns_to_t(53, clk);
	u8 txp		= max(ns_to_t(8, clk), 2);	/* max(6 ns, 3nCK) */
	u8 trtp		= max(ns_to_t(8, clk), 4);	/* max(7.5 ns, 4nCK) */
	u8 trp		= ns_to_t(15, clk);		/* >= 13.75 ns */
	u8 tras		= ns_to_t(38, clk);
	u16 trefi	= ns_to_t(11350, clk) / 32;
	u16 trfc	= ns_to_t(360, clk);		/* 160 ns for 2Gb */
	u16 txsr	= 4;

	u8 tmrw		= 0;
	u8 tmrd		= 4;				/* 4nCK */

	u8 tmod		= max(ns_to_t(15, clk), 12);	/* max(15 ns, 12nCK) */
	u8 tcke		= max(ns_to_t(6, clk), 4);	/* max(5.625 ns, 3nCK)*/
	u8 tcksrx	= max(ns_to_t(10, clk), 4);	/* max(10 ns, 5nCK) */
	u8 tcksre	= max(ns_to_t(10, clk), 4);	/* max(10 ns, 5nCK) */
	u8 trasmax	= (clk / 2) / 15;		/* tREFI * 9 */

	/*
	 * TODO: support multiple DDR3 speed grades, these values below match
	 * the worst case for DDR3-2133, so should be good for all frequencies,
	 * but use the most conversative timings.
	 * DDR3-1866 (DRAM_CLK=912) should also work, or tcl=6 and tcwl=4 with
	 * DRAM_CLK=792. Maybe even the combination of both, depending on the
	 * particular device.
	 */
	u8 tcl		= 7;			/* CAS latency: 14 */
	u8 tcwl		= 5;			/* CAS write latency: 10 */
	u8 t_rdata_en	= 9;
	u8 tphy_wrlat	= 5;
	u8 twr		= 7;

	u8 tckesr	= tcke + 1;			/* tCKE(min) + 1nCK */

	u8 twtp		= twr + 2 + tcwl;
	u8 twr2rd	= twtr + 2 + tcwl;	/* (WL + BL / 2 + tWTR) / 2 */
	u8 trd2wr	= tcl + 3 - tcwl;
	u8 txs		= ns_to_t(360, clk) / 32;	/* max(5nCK,tRFC+10ns)*/
	u8 txsdll	= 512 / 32;			/* 512 nCK */
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
	writel(0xe0c05, &mctl_ctl->dramtmg[10]);
	writel(0x440c021c, &mctl_ctl->dramtmg[11]);
	writel(8, &mctl_ctl->dramtmg[12]);
	writel(0xa100002, &mctl_ctl->dramtmg[13]);
	writel(txsr, &mctl_ctl->dramtmg[14]);

	clrsetbits_le32(&mctl_ctl->init[0], 0xc0000fff, 0x156);
	writel(0x01f20000, &mctl_ctl->init[1]);
	writel(0x00001700, &mctl_ctl->init[2]);
	writel(0, &mctl_ctl->dfimisc);
	writel(0x1f140004, &mctl_ctl->init[3]);
	writel(0x00200000, &mctl_ctl->init[4]);
	writel(0, &mctl_ctl->init[6]);	/* ? */
	writel(0, &mctl_ctl->init[7]);	/* ? */

	clrsetbits_le32(&mctl_ctl->rankctl, 0xff0, 0x660);

	/* Configure DFI timing */
	writel(tphy_wrlat | 0x2000000 | (t_rdata_en << 16) | 0x808000,
	       &mctl_ctl->dfitmg0);
	writel(0x100202, &mctl_ctl->dfitmg1);

	/* set refresh timing */
	trfc = 0;	/* as written so by boot0 */
	writel((trefi << 16) | trfc, &mctl_ctl->rfshtmg);
}

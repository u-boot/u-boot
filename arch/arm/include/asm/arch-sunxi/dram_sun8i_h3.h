/*
 * sun8i H3 platform dram controller register and constant defines
 *
 * (C) Copyright 2007-2015 Allwinner Technology Co.
 *                         Jerry Wang <wangflord@allwinnertech.com>
 * (C) Copyright 2015      Vishnu Patekar <vishnupatekar0510@gmail.com>
 * (C) Copyright 2014-2015 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2015      Jens Kuske <jenskuske@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DRAM_SUN8I_H3_H
#define _SUNXI_DRAM_SUN8I_H3_H

struct sunxi_mctl_com_reg {
	u32 cr;			/* 0x00 control register */
	u8 res0[0xc];		/* 0x04 */
	u32 mcr[16][2];		/* 0x10 */
	u32 bwcr;		/* 0x90 bandwidth control register */
	u32 maer;		/* 0x94 master enable register */
	u32 mapr;		/* 0x98 master priority register */
	u32 mcgcr;		/* 0x9c */
	u32 cpu_bwcr;		/* 0xa0 */
	u32 gpu_bwcr;		/* 0xa4 */
	u32 ve_bwcr;		/* 0xa8 */
	u32 disp_bwcr;		/* 0xac */
	u32 other_bwcr;		/* 0xb0 */
	u32 total_bwcr;		/* 0xb4 */
	u8 res1[0x8];		/* 0xb8 */
	u32 swonr;		/* 0xc0 */
	u32 swoffr;		/* 0xc4 */
	u8 res2[0x8];		/* 0xc8 */
	u32 cccr;		/* 0xd0 */
	u8 res3[0x72c];		/* 0xd4 */
	u32 protect;		/* 0x800 */
};

#define MCTL_CR_BL8		(0x4 << 20)

#define MCTL_CR_1T		(0x1 << 19)
#define MCTL_CR_2T		(0x0 << 19)

#define MCTL_CR_LPDDR3		(0x7 << 16)
#define MCTL_CR_LPDDR2		(0x6 << 16)
#define MCTL_CR_DDR3		(0x3 << 16)
#define MCTL_CR_DDR2		(0x2 << 16)

#define MCTL_CR_SEQUENTIAL	(0x1 << 15)
#define MCTL_CR_INTERLEAVED	(0x0 << 15)

#define MCTL_CR_32BIT		(0x1 << 12)
#define MCTL_CR_16BIT		(0x0 << 12)
#define MCTL_CR_BUS_WIDTH(x)	((x) == 32 ? MCTL_CR_32BIT : MCTL_CR_16BIT)

#define MCTL_CR_PAGE_SIZE(x)	((fls(x) - 4) << 8)
#define MCTL_CR_ROW_BITS(x)	(((x) - 1) << 4)
#define MCTL_CR_EIGHT_BANKS	(0x1 << 2)
#define MCTL_CR_FOUR_BANKS	(0x0 << 2)
#define MCTL_CR_DUAL_RANK	(0x1 << 0)
#define MCTL_CR_SINGLE_RANK	(0x0 << 0)

#define PROTECT_MAGIC		(0x94be6fa3)

struct sunxi_mctl_ctl_reg {
	u32 pir;		/* 0x00 PHY initialization register */
	u32 pwrctl;		/* 0x04 */
	u32 mrctrl;		/* 0x08 */
	u32 clken;		/* 0x0c */
	u32 pgsr[2];		/* 0x10 PHY general status registers */
	u32 statr;		/* 0x18 */
	u8 res1[0x14];		/* 0x1c */
	u32 mr[4];		/* 0x30 mode registers */
	u32 pllgcr;		/* 0x40 */
	u32 ptr[5];		/* 0x44 PHY timing registers */
	u32 dramtmg[9];		/* 0x58 DRAM timing registers */
	u32 odtcfg;		/* 0x7c */
	u32 pitmg[2];		/* 0x80 PHY interface timing registers */
	u8 res2[0x4];		/* 0x88 */
	u32 rfshctl0;		/* 0x8c */
	u32 rfshtmg;		/* 0x90 refresh timing */
	u32 rfshctl1;		/* 0x94 */
	u32 pwrtmg;		/* 0x98 */
	u8  res3[0x20];		/* 0x9c */
	u32 dqsgmr;		/* 0xbc */
	u32 dtcr;		/* 0xc0 */
	u32 dtar[4];		/* 0xc4 */
	u32 dtdr[2];		/* 0xd4 */
	u32 dtmr[2];		/* 0xdc */
	u32 dtbmr;		/* 0xe4 */
	u32 catr[2];		/* 0xe8 */
	u32 dtedr[2];		/* 0xf0 */
	u8 res4[0x8];		/* 0xf8 */
	u32 pgcr[4];		/* 0x100 PHY general configuration registers */
	u32 iovcr[2];		/* 0x110 */
	u32 dqsdr;		/* 0x118 */
	u32 dxccr;		/* 0x11c */
	u32 odtmap;		/* 0x120 */
	u32 zqctl[2];		/* 0x124 */
	u8 res6[0x14];		/* 0x12c */
	u32 zqcr;		/* 0x140 ZQ control register */
	u32 zqsr;		/* 0x144 ZQ status register */
	u32 zqdr[3];		/* 0x148 ZQ data registers */
	u8 res7[0x6c];		/* 0x154 */
	u32 sched;		/* 0x1c0 */
	u32 perfhpr[2];		/* 0x1c4 */
	u32 perflpr[2];		/* 0x1cc */
	u32 perfwr[2];		/* 0x1d4 */
	u8 res8[0x2c];		/* 0x1dc */
	u32 aciocr;		/* 0x208 */
	u8 res9[0xf4];		/* 0x20c */
	struct {		/* 0x300 DATX8 modules*/
		u32 mdlr;		/* 0x00 */
		u32 lcdlr[3];		/* 0x04 */
		u32 iocr[11];		/* 0x10 IO configuration register */
		u32 bdlr6;		/* 0x3c */
		u32 gtr;		/* 0x40 */
		u32 gcr;		/* 0x44 */
		u32 gsr[3];		/* 0x48 */
		u8 res0[0x2c];		/* 0x54 */
	} datx[4];
	u8 res10[0x388];	/* 0x500 */
	u32 upd2;		/* 0x888 */
};

#define PTR3_TDINIT1(x)		((x) << 20)
#define PTR3_TDINIT0(x)		((x) <<  0)

#define PTR4_TDINIT3(x)		((x) << 20)
#define PTR4_TDINIT2(x)		((x) <<  0)

#define DRAMTMG0_TWTP(x)	((x) << 24)
#define DRAMTMG0_TFAW(x)	((x) << 16)
#define DRAMTMG0_TRAS_MAX(x)	((x) <<  8)
#define DRAMTMG0_TRAS(x)	((x) <<  0)

#define DRAMTMG1_TXP(x)		((x) << 16)
#define DRAMTMG1_TRTP(x)	((x) <<  8)
#define DRAMTMG1_TRC(x)		((x) <<  0)

#define DRAMTMG2_TCWL(x)	((x) << 24)
#define DRAMTMG2_TCL(x)		((x) << 16)
#define DRAMTMG2_TRD2WR(x)	((x) <<  8)
#define DRAMTMG2_TWR2RD(x)	((x) <<  0)

#define DRAMTMG3_TMRW(x)	((x) << 16)
#define DRAMTMG3_TMRD(x)	((x) << 12)
#define DRAMTMG3_TMOD(x)	((x) <<  0)

#define DRAMTMG4_TRCD(x)	((x) << 24)
#define DRAMTMG4_TCCD(x)	((x) << 16)
#define DRAMTMG4_TRRD(x)	((x) <<  8)
#define DRAMTMG4_TRP(x)		((x) <<  0)

#define DRAMTMG5_TCKSRX(x)	((x) << 24)
#define DRAMTMG5_TCKSRE(x)	((x) << 16)
#define DRAMTMG5_TCKESR(x)	((x) <<  8)
#define DRAMTMG5_TCKE(x)	((x) <<  0)

#define RFSHTMG_TREFI(x)	((x) << 16)
#define RFSHTMG_TRFC(x)		((x) <<  0)

#define PIR_CLRSR	(0x1 << 27)	/* clear status registers */
#define PIR_QSGATE	(0x1 << 10)	/* Read DQS gate training */
#define PIR_DRAMINIT	(0x1 << 8)	/* DRAM initialization */
#define PIR_DRAMRST	(0x1 << 7)	/* DRAM reset */
#define PIR_PHYRST	(0x1 << 6)	/* PHY reset */
#define PIR_DCAL	(0x1 << 5)	/* DDL calibration */
#define PIR_PLLINIT	(0x1 << 4)	/* PLL initialization */
#define PIR_ZCAL	(0x1 << 1)	/* ZQ calibration */
#define PIR_INIT	(0x1 << 0)	/* PHY initialization trigger */

#define PGSR_INIT_DONE	(0x1 << 0)	/* PHY init done */

#define ZQCR_PWRDOWN	(0x1 << 31)	/* ZQ power down */

#define DATX_IOCR_DQ(x)	(x)		/* DQ0-7 IOCR index */
#define DATX_IOCR_DM	(8)		/* DM IOCR index */
#define DATX_IOCR_DQS	(9)		/* DQS IOCR index */
#define DATX_IOCR_DQSN	(10)		/* DQSN IOCR index */

#define DATX_IOCR_WRITE_DELAY(x)	((x) << 8)
#define DATX_IOCR_READ_DELAY(x)		((x) << 0)

#endif /* _SUNXI_DRAM_SUN8I_H3_H */

/*
 * Copyright (C) 2016-2017 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#ifndef _ASM_ARCH_SDRAM_RK3328_H
#define _ASM_ARCH_SDRAM_RK3328_H

#define SR_IDLE		93
#define PD_IDLE		13
#define SDRAM_ADDR	0x00000000
#define PATTERN		(0x5aa5f00f)

/* ddr pctl registers define */
#define DDR_PCTL2_MSTR			0x0
#define DDR_PCTL2_STAT			0x4
#define DDR_PCTL2_MSTR1			0x8
#define DDR_PCTL2_MRCTRL0		0x10
#define DDR_PCTL2_MRCTRL1		0x14
#define DDR_PCTL2_MRSTAT		0x18
#define DDR_PCTL2_MRCTRL2		0x1c
#define DDR_PCTL2_DERATEEN		0x20
#define DDR_PCTL2_DERATEINT		0x24
#define DDR_PCTL2_PWRCTL		0x30
#define DDR_PCTL2_PWRTMG		0x34
#define DDR_PCTL2_HWLPCTL		0x38
#define DDR_PCTL2_RFSHCTL0		0x50
#define DDR_PCTL2_RFSHCTL1		0x54
#define DDR_PCTL2_RFSHCTL2		0x58
#define DDR_PCTL2_RFSHCTL4		0x5c
#define DDR_PCTL2_RFSHCTL3		0x60
#define DDR_PCTL2_RFSHTMG		0x64
#define DDR_PCTL2_RFSHTMG1		0x68
#define DDR_PCTL2_RFSHCTL5		0x6c
#define DDR_PCTL2_INIT0			0xd0
#define DDR_PCTL2_INIT1			0xd4
#define DDR_PCTL2_INIT2			0xd8
#define DDR_PCTL2_INIT3			0xdc
#define DDR_PCTL2_INIT4			0xe0
#define DDR_PCTL2_INIT5			0xe4
#define DDR_PCTL2_INIT6			0xe8
#define DDR_PCTL2_INIT7			0xec
#define DDR_PCTL2_DIMMCTL		0xf0
#define DDR_PCTL2_RANKCTL		0xf4
#define DDR_PCTL2_CHCTL			0xfc
#define DDR_PCTL2_DRAMTMG0		0x100
#define DDR_PCTL2_DRAMTMG1		0x104
#define DDR_PCTL2_DRAMTMG2		0x108
#define DDR_PCTL2_DRAMTMG3		0x10c
#define DDR_PCTL2_DRAMTMG4		0x110
#define DDR_PCTL2_DRAMTMG5		0x114
#define DDR_PCTL2_DRAMTMG6		0x118
#define DDR_PCTL2_DRAMTMG7		0x11c
#define DDR_PCTL2_DRAMTMG8		0x120
#define DDR_PCTL2_DRAMTMG9		0x124
#define DDR_PCTL2_DRAMTMG10		0x128
#define DDR_PCTL2_DRAMTMG11		0x12c
#define DDR_PCTL2_DRAMTMG12		0x130
#define DDR_PCTL2_DRAMTMG13		0x134
#define DDR_PCTL2_DRAMTMG14		0x138
#define DDR_PCTL2_DRAMTMG15		0x13c
#define DDR_PCTL2_DRAMTMG16		0x140
#define DDR_PCTL2_ZQCTL0		0x180
#define DDR_PCTL2_ZQCTL1		0x184
#define DDR_PCTL2_ZQCTL2		0x188
#define DDR_PCTL2_ZQSTAT		0x18c
#define DDR_PCTL2_DFITMG0		0x190
#define DDR_PCTL2_DFITMG1		0x194
#define DDR_PCTL2_DFILPCFG0		0x198
#define DDR_PCTL2_DFILPCFG1		0x19c
#define DDR_PCTL2_DFIUPD0		0x1a0
#define DDR_PCTL2_DFIUPD1		0x1a4
#define DDR_PCTL2_DFIUPD2		0x1a8
#define DDR_PCTL2_DFIMISC		0x1b0
#define DDR_PCTL2_DFITMG2		0x1b4
#define DDR_PCTL2_DFITMG3		0x1b8
#define DDR_PCTL2_DFISTAT		0x1bc
#define DDR_PCTL2_DBICTL		0x1c0
#define DDR_PCTL2_ADDRMAP0		0x200
#define DDR_PCTL2_ADDRMAP1		0x204
#define DDR_PCTL2_ADDRMAP2		0x208
#define DDR_PCTL2_ADDRMAP3		0x20c
#define DDR_PCTL2_ADDRMAP4		0x210
#define DDR_PCTL2_ADDRMAP5		0x214
#define DDR_PCTL2_ADDRMAP6		0x218
#define DDR_PCTL2_ADDRMAP7		0x21c
#define DDR_PCTL2_ADDRMAP8		0x220
#define DDR_PCTL2_ADDRMAP9		0x224
#define DDR_PCTL2_ADDRMAP10		0x228
#define DDR_PCTL2_ADDRMAP11		0x22c
#define DDR_PCTL2_ODTCFG		0x240
#define DDR_PCTL2_ODTMAP		0x244
#define DDR_PCTL2_SCHED			0x250
#define DDR_PCTL2_SCHED1		0x254
#define DDR_PCTL2_PERFHPR1		0x25c
#define DDR_PCTL2_PERFLPR1		0x264
#define DDR_PCTL2_PERFWR1		0x26c
#define DDR_PCTL2_DQMAP0		0x280
#define DDR_PCTL2_DQMAP1		0x284
#define DDR_PCTL2_DQMAP2		0x288
#define DDR_PCTL2_DQMAP3		0x28c
#define DDR_PCTL2_DQMAP4		0x290
#define DDR_PCTL2_DQMAP5		0x294
#define DDR_PCTL2_DBG0			0x300
#define DDR_PCTL2_DBG1			0x304
#define DDR_PCTL2_DBGCAM		0x308
#define DDR_PCTL2_DBGCMD		0x30c
#define DDR_PCTL2_DBGSTAT		0x310
#define DDR_PCTL2_SWCTL			0x320
#define DDR_PCTL2_SWSTAT		0x324
#define DDR_PCTL2_POISONCFG		0x36c
#define DDR_PCTL2_POISONSTAT		0x370
#define DDR_PCTL2_ADVECCINDEX		0x374
#define DDR_PCTL2_ADVECCSTAT		0x378
#define DDR_PCTL2_PSTAT			0x3fc
#define DDR_PCTL2_PCCFG			0x400
#define DDR_PCTL2_PCFGR_n		0x404
#define DDR_PCTL2_PCFGW_n		0x408
#define DDR_PCTL2_PCTRL_n		0x490

/* PCTL2_MRSTAT */
#define MR_WR_BUSY			BIT(0)

/* PHY_REG0 */
#define DIGITAL_DERESET			BIT(3)
#define ANALOG_DERESET			BIT(2)
#define DIGITAL_RESET			(0 << 3)
#define ANALOG_RESET			(0 << 2)

/* PHY_REG1 */
#define PHY_DDR2			(0)
#define PHY_LPDDR2			(1)
#define PHY_DDR3			(2)
#define PHY_LPDDR3			(3)
#define PHY_DDR4			(4)
#define PHY_BL_4			(0 << 2)
#define PHY_BL_8			BIT(2)

/* PHY_REG2 */
#define PHY_DTT_EN			BIT(0)
#define PHY_DTT_DISB			(0 << 0)
#define PHY_WRITE_LEVELING_EN		BIT(2)
#define PHY_WRITE_LEVELING_DISB		(0 << 2)
#define PHY_SELECT_CS0			(2)
#define PHY_SELECT_CS1			(1)
#define PHY_SELECT_CS0_1		(0)
#define PHY_WRITE_LEVELING_SELECTCS(n)	(n << 6)
#define PHY_DATA_TRAINING_SELECTCS(n)	(n << 4)

#define PHY_DDR3_RON_RTT_DISABLE	(0)
#define PHY_DDR3_RON_RTT_451ohm		(1)
#define PHY_DDR3_RON_RTT_225ohm		(2)
#define PHY_DDR3_RON_RTT_150ohm		(3)
#define PHY_DDR3_RON_RTT_112ohm		(4)
#define PHY_DDR3_RON_RTT_90ohm		(5)
#define PHY_DDR3_RON_RTT_75ohm		(6)
#define PHY_DDR3_RON_RTT_64ohm		(7)
#define PHY_DDR3_RON_RTT_56ohm		(16)
#define PHY_DDR3_RON_RTT_50ohm		(17)
#define PHY_DDR3_RON_RTT_45ohm		(18)
#define PHY_DDR3_RON_RTT_41ohm		(19)
#define PHY_DDR3_RON_RTT_37ohm		(20)
#define PHY_DDR3_RON_RTT_34ohm		(21)
#define PHY_DDR3_RON_RTT_33ohm		(22)
#define PHY_DDR3_RON_RTT_30ohm		(23)
#define PHY_DDR3_RON_RTT_28ohm		(24)
#define PHY_DDR3_RON_RTT_26ohm		(25)
#define PHY_DDR3_RON_RTT_25ohm		(26)
#define PHY_DDR3_RON_RTT_23ohm		(27)
#define PHY_DDR3_RON_RTT_22ohm		(28)
#define PHY_DDR3_RON_RTT_21ohm		(29)
#define PHY_DDR3_RON_RTT_20ohm		(30)
#define PHY_DDR3_RON_RTT_19ohm		(31)

#define PHY_DDR4_LPDDR3_RON_RTT_DISABLE	(0)
#define PHY_DDR4_LPDDR3_RON_RTT_480ohm	(1)
#define PHY_DDR4_LPDDR3_RON_RTT_240ohm	(2)
#define PHY_DDR4_LPDDR3_RON_RTT_160ohm	(3)
#define PHY_DDR4_LPDDR3_RON_RTT_120ohm	(4)
#define PHY_DDR4_LPDDR3_RON_RTT_96ohm	(5)
#define PHY_DDR4_LPDDR3_RON_RTT_80ohm	(6)
#define PHY_DDR4_LPDDR3_RON_RTT_68ohm	(7)
#define PHY_DDR4_LPDDR3_RON_RTT_60ohm	(16)
#define PHY_DDR4_LPDDR3_RON_RTT_53ohm	(17)
#define PHY_DDR4_LPDDR3_RON_RTT_48ohm	(18)
#define PHY_DDR4_LPDDR3_RON_RTT_43ohm	(19)
#define PHY_DDR4_LPDDR3_RON_RTT_40ohm	(20)
#define PHY_DDR4_LPDDR3_RON_RTT_37ohm	(21)
#define PHY_DDR4_LPDDR3_RON_RTT_34ohm	(22)
#define PHY_DDR4_LPDDR3_RON_RTT_32ohm	(23)
#define PHY_DDR4_LPDDR3_RON_RTT_30ohm	(24)
#define PHY_DDR4_LPDDR3_RON_RTT_28ohm	(25)
#define PHY_DDR4_LPDDR3_RON_RTT_26ohm	(26)
#define PHY_DDR4_LPDDR3_RON_RTT_25ohm	(27)
#define PHY_DDR4_LPDDR3_RON_RTT_24ohm	(28)
#define PHY_DDR4_LPDDR3_RON_RTT_22ohm	(29)
#define PHY_DDR4_LPDDR3_RON_RTT_21ohm	(30)
#define PHY_DDR4_LPDDR3_RON_RTT_20ohm	(31)

/* noc registers define */
#define DDRCONF				0x8
#define DDRTIMING			0xc
#define DDRMODE				0x10
#define READLATENCY			0x14
#define AGING0				0x18
#define AGING1				0x1c
#define AGING2				0x20
#define AGING3				0x24
#define AGING4				0x28
#define AGING5				0x2c
#define ACTIVATE			0x38
#define DEVTODEV			0x3c
#define DDR4TIMING			0x40

/* DDR GRF */
#define DDR_GRF_CON(n)		(0 + (n) * 4)
#define DDR_GRF_STATUS_BASE	(0X100)
#define DDR_GRF_STATUS(n)	(DDR_GRF_STATUS_BASE + (n) * 4)

/* CRU_SOFTRESET_CON5 */
#define ddrphy_psrstn_req(n)    (((0x1 << 15) << 16) | (n << 15))
#define ddrphy_srstn_req(n)     (((0x1 << 14) << 16) | (n << 14))
#define ddrctrl_psrstn_req(n)	(((0x1 << 13) << 16) | (n << 13))
#define ddrctrl_srstn_req(n)	(((0x1 << 12) << 16) | (n << 12))
#define ddrmsch_srstn_req(n)	(((0x1 << 11) << 16) | (n << 11))
#define msch_srstn_req(n)		(((0x1 << 9) << 16) | (n << 9))
#define dfimon_srstn_req(n)		(((0x1 << 8) << 16) | (n << 8))
#define grf_ddr_srstn_req(n)	(((0x1 << 7) << 16) | (n << 7))
/* CRU_SOFTRESET_CON9 */
#define ddrctrl_asrstn_req(n)		(((0x1 << 9) << 16) | (n << 9))

/* CRU register */
#define CRU_PLL_CON(pll_id, n)	((pll_id)  * 0x20 + (n) * 4)
#define CRU_MODE				(0x80)
#define CRU_GLB_CNT_TH			(0x90)
#define CRU_CLKSEL_CON_BASE		0x100
#define CRU_CLKSELS_CON(i)		(CRU_CLKSEL_CON_BASE + ((i) * 4))
#define CRU_CLKGATE_CON_BASE		0x200
#define CRU_CLKGATE_CON(i)		(CRU_CLKGATE_CON_BASE + ((i) * 4))
#define CRU_CLKSFTRST_CON_BASE	0x300
#define CRU_CLKSFTRST_CON(i)	(CRU_CLKSFTRST_CON_BASE + ((i) * 4))

/* CRU_PLL_CON0 */
#define PB(n)         ((0x1 << (15 + 16)) | ((n) << 15))
#define POSTDIV1(n)   ((0x7 << (12 + 16)) | ((n) << 12))
#define FBDIV(n)      ((0xFFF << 16) | (n))

/* CRU_PLL_CON1 */
#define RSTMODE(n)    ((0x1 << (15 + 16)) | ((n) << 15))
#define RST(n)        ((0x1 << (14 + 16)) | ((n) << 14))
#define PD(n)         ((0x1 << (13 + 16)) | ((n) << 13))
#define DSMPD(n)      ((0x1 << (12 + 16)) | ((n) << 12))
#define LOCK(n)       (((n) >> 10) & 0x1)
#define POSTDIV2(n)   ((0x7 << (6 + 16)) | ((n) << 6))
#define REFDIV(n)     ((0x3F << 16) | (n))

union noc_ddrtiming {
	u32 d32;
	struct {
		unsigned acttoact:6;
		unsigned rdtomiss:6;
		unsigned wrtomiss:6;
		unsigned burstlen:3;
		unsigned rdtowr:5;
		unsigned wrtord:5;
		unsigned bwratio:1;
	} b;
} NOC_TIMING_T;

union noc_activate {
	u32 d32;
	struct {
		unsigned rrd:4;
		unsigned faw:6;
		unsigned fawbank:1;
		unsigned reserved1:21;
	} b;
};

union noc_devtodev {
	u32 d32;
	struct {
		unsigned busrdtord:2;
		unsigned busrdtowr:2;
		unsigned buswrtord:2;
		unsigned reserved2:26;
	} b;
};

union noc_ddr4timing {
	u32 d32;
	struct {
		unsigned ccdl:3;
		unsigned wrtordl:5;
		unsigned rrdl:4;
		unsigned reserved2:20;
	} b;
};

union noc_ddrmode {
	u32 d32;
	struct {
		unsigned autoprecharge:1;
		unsigned bwratioextended:1;
		unsigned reserved3:30;
	} b;
};

u32 addrmap[21][9] = {
	/* map0  map1  map2  map3  map4  map5  map6  map7  map8 */
	{22, 0x00070707, 0x00000000, 0x1f000000, 0x00001f1f, 0x06060606,
		0x06060606, 0x00000f0f, 0x3f3f},
	{23, 0x00080808, 0x00000000, 0x00000000, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f0f, 0x3f3f},
	{23, 0x00090909, 0x00000000, 0x00000000, 0x00001f00, 0x08080808,
		0x0f080808, 0x00000f0f, 0x3f3f},
	{24, 0x00090909, 0x00000000, 0x00000000, 0x00001f00, 0x08080808,
		0x08080808, 0x00000f0f, 0x3f3f},
	{24, 0x000a0a0a, 0x00000000, 0x00000000, 0x00000000, 0x09090909,
		0x0f090909, 0x00000f0f, 0x3f3f},
	{6, 0x00070707, 0x00000000, 0x1f000000, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f0f, 0x3f3f},
	{7, 0x00080808, 0x00000000, 0x00000000, 0x00001f1f, 0x08080808,
		0x08080808, 0x00000f0f, 0x3f3f},
	{8, 0x00090909, 0x00000000, 0x00000000, 0x00001f00, 0x09090909,
		0x0f090909, 0x00000f0f, 0x3f3f},
	{22, 0x001f0808, 0x00000000, 0x00000000, 0x00001f1f, 0x06060606,
		0x06060606, 0x00000f0f, 0x3f3f},
	{23, 0x00080808, 0x00000000, 0x00000000, 0x00001f1f, 0x07070707,
		0x0f070707, 0x00000f0f, 0x3f3f},

	{24, 0x003f0a0a, 0x01010100, 0x01010101, 0x00001f1f, 0x08080808,
		0x08080808, 0x00000f0f, 0x0801},
	{23, 0x003f0a0a, 0x01010100, 0x01010101, 0x00001f1f, 0x08080808,
		0x0f080808, 0x00000f0f, 0x0801},
	{24, 0x003f0909, 0x00000007, 0x1f000000, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f07, 0x0700},
	{23, 0x003f0909, 0x00000007, 0x1f000000, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f0f, 0x0700},
	{24, 0x003f0909, 0x01010100, 0x01010101, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f07, 0x3f01},
	{23, 0x003f0909, 0x01010100, 0x01010101, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f0f, 0x3f01},
	{24, 0x003f0808, 0x00000007, 0x1f000000, 0x00001f1f, 0x06060606,
		0x06060606, 0x00000f06, 0x3f00},
	{8, 0x003f0a0a, 0x01010100, 0x01010101, 0x00001f1f, 0x09090909,
		0x0f090909, 0x00000f0f, 0x0801},
	{7, 0x003f0909, 0x00000007, 0x1f000000, 0x00001f1f, 0x08080808,
		0x08080808, 0x00000f0f, 0x0700},
	{7, 0x003f0909, 0x01010100, 0x01010101, 0x00001f1f, 0x08080808,
		0x08080808, 0x00000f0f, 0x3f01},

	{6, 0x003f0808, 0x00000007, 0x1f000000, 0x00001f1f, 0x07070707,
		0x07070707, 0x00000f07, 0x3f00}
};

struct rk3328_msch_timings {
	union noc_ddrtiming ddrtiming;
	union noc_ddrmode ddrmode;
	u32 readlatency;
	union noc_activate activate;
	union noc_devtodev devtodev;
	union noc_ddr4timing ddr4timing;
	u32 agingx0;
};

struct rk3328_msch_regs {
	u32 coreid;
	u32 revisionid;
	u32 ddrconf;
	u32 ddrtiming;
	u32 ddrmode;
	u32 readlatency;
	u32 aging0;
	u32 aging1;
	u32 aging2;
	u32 aging3;
	u32 aging4;
	u32 aging5;
	u32 reserved[2];
	u32 activate;
	u32 devtodev;
	u32 ddr4_timing;
};

struct rk3328_ddr_grf_regs {
	u32 ddr_grf_con[4];
	u32 reserved[(0x100 - 0x10) / 4];
	u32 ddr_grf_status[11];
};

struct rk3328_ddr_pctl_regs {
	u32 pctl[30][2];
};

struct rk3328_ddr_phy_regs {
	u32 phy[5][2];
};

struct rk3328_ddr_skew {
	u32 a0_a1_skew[15];
	u32 cs0_dm0_skew[11];
	u32 cs0_dm1_skew[11];
	u32 cs0_dm2_skew[11];
	u32 cs0_dm3_skew[11];
	u32 cs1_dm0_skew[11];
	u32 cs1_dm1_skew[11];
	u32 cs1_dm2_skew[11];
	u32 cs1_dm3_skew[11];
};

struct rk3328_sdram_channel {
	unsigned int rank;
	unsigned int col;
	/* 3:8bank, 2:4bank */
	unsigned int bk;
	/* channel buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int bw;
	/* die buswidth, 2:32bit, 1:16bit, 0:8bit */
	unsigned int dbw;
	unsigned int row_3_4;
	unsigned int cs0_row;
	unsigned int cs1_row;
	unsigned int ddrconfig;
	struct rk3328_msch_timings noc_timings;
};

struct rk3328_sdram_params {
	struct rk3328_sdram_channel ch;
	unsigned int ddr_freq;
	unsigned int dramtype;
	unsigned int odt;
	struct rk3328_ddr_pctl_regs pctl_regs;
	struct rk3328_ddr_phy_regs phy_regs;
	struct rk3328_ddr_skew skew;
};

#define PHY_REG(base, n)		(base + 4 * (n))

#endif

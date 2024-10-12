/*
 * A133 dram controller register and constant defines
 * libdram version
 *
 * Based on H616 one, which is:
 * (C) Copyright 2020  Jernej Skrabec <jernej.skrabec@siol.net>
 *
 * Based on H6 one, which is:
 * (C) Copyright 2017  Icenowy Zheng <icenowy@aosc.io>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DRAM_SUN50I_A100_H
#define _SUNXI_DRAM_SUN50I_A100_H

#include <stdbool.h>
#include <inttypes.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

enum sunxi_dram_type
{
	SUNXI_DRAM_TYPE_DDR3 = 3,
	SUNXI_DRAM_TYPE_DDR4,
	SUNXI_DRAM_TYPE_LPDDR3 = 7,
	SUNXI_DRAM_TYPE_LPDDR4
};

/* MBUS part is largely the same as in H6, except for one special register */
struct sunxi_mctl_com_reg
{
	u32 cr;					/* 0x000 control register */
	u8 reserved_0x004[4];	/* 0x004 */
	u32 unk_0x008;			/* 0x008 */
	u32 tmr;				/* 0x00c timer register */
	u8 reserved_0x010[4];	/* 0x010 */
	u32 unk_0x014;			/* 0x014 */
	u8 reserved_0x018[8];	/* 0x018 */
	u32 maer0;				/* 0x020 master enable register 0 */
	u32 maer1;				/* 0x024 master enable register 1 */
	u32 maer2;				/* 0x028 master enable register 2 */
	u8 reserved_0x02c[468]; /* 0x02c */
	u32 bwcr;				/* 0x200 bandwidth control register */
	u8 reserved_0x204[12];	/* 0x204 */
	/*
	 * The last master configured by BSP libdram is at 0x49x, so the
	 * size of this struct array is set to 41 (0x29) now.
	 */
	struct
	{
		u32 cfg0;			/* 0x0 */
		u32 cfg1;			/* 0x4 */
		u8 reserved_0x8[8]; /* 0x8 */
	} master[41];			/* 0x210 + index * 0x10 */
	u8 reserved_0x4a0[96];	/* 0x4a0 */
	u32 unk_0x500;			/* 0x500 */
};
check_member(sunxi_mctl_com_reg, unk_0x500, 0x500);

/*
 * Controller registers seems to be the same or at least very similar
 * to those in H6.
 */
struct sunxi_mctl_ctl_reg
{
	u32 mstr;				  /* 0x000 */
	u32 statr;				  /* 0x004 unused */
	u32 mstr1;				  /* 0x008 unused */
	u32 clken;				  /* 0x00c */
	u32 mrctrl0;			  /* 0x010 unused */
	u32 mrctrl1;			  /* 0x014 unused */
	u32 mrstatr;			  /* 0x018 unused */
	u32 mrctrl2;			  /* 0x01c unused */
	u32 derateen;			  /* 0x020 unused */
	u32 derateint;			  /* 0x024 unused */
	u8 reserved_0x028[8];	  /* 0x028 */
	u32 pwrctl;				  /* 0x030 unused */
	u32 pwrtmg;				  /* 0x034 unused */
	u32 hwlpctl;			  /* 0x038 unused */
	u8 reserved_0x03c[20];	  /* 0x03c */
	u32 rfshctl0;			  /* 0x050 unused */
	u32 rfshctl1;			  /* 0x054 unused */
	u8 reserved_0x058[8];	  /* 0x05c */
	u32 rfshctl3;			  /* 0x060 */
	u32 rfshtmg;			  /* 0x064 */
	u8 reserved_0x068[104];	  /* 0x068 */
	u32 init[8];			  /* 0x0d0 */
	u32 dimmctl;			  /* 0x0f0 unused */
	u32 rankctl;			  /* 0x0f4 */
	u8 reserved_0x0f8[8];	  /* 0x0f8 */
	u32 dramtmg[17];		  /* 0x100 */
	u8 reserved_0x144[60];	  /* 0x144 */
	u32 zqctl[3];			  /* 0x180 */
	u32 zqstat;				  /* 0x18c unused */
	u32 dfitmg0;			  /* 0x190 */
	u32 dfitmg1;			  /* 0x194 */
	u32 dfilpcfg[2];		  /* 0x198 unused */
	u32 dfiupd[3];			  /* 0x1a0 */
	u32 reserved_0x1ac;		  /* 0x1ac */
	u32 dfimisc;			  /* 0x1b0 */
	u32 dfitmg2;			  /* 0x1b4 unused */
	u32 dfitmg3;			  /* 0x1b8 unused */
	u32 dfistat;			  /* 0x1bc */
	u32 dbictl;				  /* 0x1c0 */
	u8 reserved_0x1c4[60];	  /* 0x1c4 */
	u32 addrmap[12];		  /* 0x200 */
	u8 reserved_0x230[16];	  /* 0x230 */
	u32 odtcfg;				  /* 0x240 */
	u32 odtmap;				  /* 0x244 */
	u8 reserved_0x248[8];	  /* 0x248 */
	u32 sched[2];			  /* 0x250 */
	u8 reserved_0x258[180];	  /* 0x258 */
	u32 dbgcmd;				  /* 0x30c unused */
	u32 dbgstat;			  /* 0x310 unused */
	u8 reserved_0x314[12];	  /* 0x314 */
	u32 swctl;				  /* 0x320 */
	u32 swstat;				  /* 0x324 */
	u8 reserved_0x328[7768];  /* 0x328 */
	u32 unk_0x2180;			  /* 0x2180 */
	u8 reserved_0x2184[188];  /* 0x2184 */
	u32 unk_0x2240;			  /* 0x2240 */
	u8 reserved_0x2244[3900]; /* 0x2244 */
	u32 unk_0x3180;			  /* 0x3180 */
	u8 reserved_0x3184[188];  /* 0x3184 */
	u32 unk_0x3240;			  /* 0x3240 */
	u8 reserved_0x3244[3900]; /* 0x3244 */
	u32 unk_0x4180;			  /* 0x4180 */
	u8 reserved_0x4184[188];  /* 0x4184 */
	u32 unk_0x4240;			  /* 0x4240 */
};
check_member(sunxi_mctl_ctl_reg, swstat, 0x324);
check_member(sunxi_mctl_ctl_reg, unk_0x4240, 0x4240);

#define MSTR_DEVICETYPE_DDR3 BIT(0)
#define MSTR_DEVICETYPE_LPDDR2 BIT(2)
#define MSTR_DEVICETYPE_LPDDR3 BIT(3)
#define MSTR_DEVICETYPE_DDR4 BIT(4)
#define MSTR_DEVICETYPE_LPDDR4 BIT(5)
#define MSTR_DEVICETYPE_MASK GENMASK(5, 0)
#define MSTR_2TMODE BIT(10)
#define MSTR_BUSWIDTH_FULL (0 << 12)
#define MSTR_BUSWIDTH_HALF (1 << 12)
#define MSTR_ACTIVE_RANKS(x) (((x == 2) ? 3 : 1) << 24)
#define MSTR_BURST_LENGTH(x) (((x) >> 1) << 16)
#define CFG_SYS_DDR_SDRAM_BASE 0x40000000

struct dram_para
{
	uint32_t clk;
	enum sunxi_dram_type type;
	uint32_t dx_odt;
	uint32_t dx_dri;
	uint32_t ca_dri;
	uint32_t para0;
	uint32_t para1;
	uint32_t para2;
	uint32_t mr0;
	uint32_t mr1;
	uint32_t mr2;
	uint32_t mr3;
	uint32_t mr4;
	uint32_t mr5;
	uint32_t mr6;
	uint32_t mr11;
	uint32_t mr12;
	uint32_t mr13;
	uint32_t mr14;
	uint32_t mr16;
	uint32_t mr17;
	uint32_t mr22;
	uint32_t tpr0;
	uint32_t tpr1;
	uint32_t tpr2;
	uint32_t tpr3;
	uint32_t tpr6;
	uint32_t tpr10;
	uint32_t tpr11;
	uint32_t tpr12;
	uint32_t tpr13;
	uint32_t tpr14;
};

struct dram_timing
{
	uint8_t tccd;
	uint8_t tfaw;
	uint8_t trrd;
	uint8_t trcd;
	uint8_t trc;
	uint8_t txp;
	uint8_t trtp;
	uint8_t trp;
	uint8_t tras;
	uint16_t trefi;
	uint16_t trfc;
	uint16_t txsr;

	uint8_t tmrw;
	uint8_t tmrd;
	uint8_t tmod;
	uint8_t tcke;
	uint8_t tcksrx;
	uint8_t tcksre;
	uint8_t tckesr;
	uint8_t trasmax;
	uint8_t txs;
	uint8_t txsdll;
	uint8_t txsabort;
	uint8_t txsfas;
	uint8_t tcl;
	uint8_t tcwl;

	uint8_t twtp;
	uint8_t twr2rd;
	uint8_t trd2wr;

	uint8_t unk_4;
	uint8_t unk_42;
	uint8_t unk_43;
	uint8_t unk_44;
	uint8_t unk_50;
	uint8_t unk_63;
	uint8_t unk_66;
	uint8_t unk_69;
};

#endif /* _SUNXI_DRAM_SUN50I_A133_H */

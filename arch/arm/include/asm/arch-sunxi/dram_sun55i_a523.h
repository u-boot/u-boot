/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * t527 dram controller register and constant defines
 *
 * (C) Copyright 2024  Jernej Skrabec <jernej.skrabec@gmail.com>
 */

#ifndef _SUNXI_DRAM_SUN55I_A523_H
#define _SUNXI_DRAM_SUN55I_A523_H

#include <linux/bitops.h>

enum sunxi_dram_type {
	SUNXI_DRAM_TYPE_DDR3 = 3,
	SUNXI_DRAM_TYPE_DDR4,
	SUNXI_DRAM_TYPE_LPDDR3 = 7,
	SUNXI_DRAM_TYPE_LPDDR4
};

#define MCTL_COM_UNK_008        0x008
#define MCTL_COM_MAER0          0x020

/*
 * Controller registers seems to be the same or at least very similar
 * to those in H6.
 */
struct sunxi_mctl_ctl_reg {
	u32 mstr;		/* 0x000 */
	u32 statr;		/* 0x004 unused */
	u32 mstr1;		/* 0x008 unused */
	u32 clken;		/* 0x00c */
	u32 mrctrl0;		/* 0x010 unused */
	u32 mrctrl1;		/* 0x014 unused */
	u32 mrstatr;		/* 0x018 unused */
	u32 mrctrl2;		/* 0x01c unused */
	u32 derateen;		/* 0x020 unused */
	u32 derateint;		/* 0x024 unused */
	u8 reserved_0x028[8];	/* 0x028 */
	u32 pwrctl;		/* 0x030 */
	u32 pwrtmg;		/* 0x034 unused */
	u32 hwlpctl;		/* 0x038 */
	u8 reserved_0x03c[20];	/* 0x03c */
	u32 rfshctl0;		/* 0x050 unused */
	u32 rfshctl1;		/* 0x054 unused */
	u8 reserved_0x058[8];	/* 0x058 */
	u32 rfshctl3;		/* 0x060 */
	u32 rfshtmg;		/* 0x064 */
	u8 reserved_0x068[104];	/* 0x068 */
	u32 init[8];		/* 0x0d0 */
	u32 dimmctl;		/* 0x0f0 unused */
	u32 rankctl;		/* 0x0f4 */
	u8 reserved_0x0f8[8];	/* 0x0f8 */
	u32 dramtmg[17];	/* 0x100 */
	u8 reserved_0x144[60];	/* 0x144 */
	u32 zqctl[3];		/* 0x180 */
	u32 zqstat;		/* 0x18c unused */
	u32 dfitmg0;		/* 0x190 */
	u32 dfitmg1;		/* 0x194 */
	u32 dfilpcfg[2];	/* 0x198 unused */
	u32 dfiupd[3];		/* 0x1a0 */
	u32 reserved_0x1ac;	/* 0x1ac */
	u32 dfimisc;		/* 0x1b0 */
	u32 dfitmg2;		/* 0x1b4 unused */
	u32 dfitmg3;		/* 0x1b8 unused */
	u32 dfistat;		/* 0x1bc */
	u32 dbictl;		/* 0x1c0 */
	u8 reserved_0x1c4[60];	/* 0x1c4 */
	u32 addrmap[12];	/* 0x200 */
	u8 reserved_0x230[16];	/* 0x230 */
	u32 odtcfg;		/* 0x240 */
	u32 odtmap;		/* 0x244 */
	u8 reserved_0x248[8];	/* 0x248 */
	u32 sched[2];		/* 0x250 */
	u8 reserved_0x258[12];	/* 0x258 */
	u32 unk_0x264;		/* 0x264 */
	u8 reserved_0x268[8];	/* 0x268 */
	u32 unk_0x270;		/* 0x270 */
	u8 reserved_0x274[152];	/* 0x274 */
	u32 dbgcmd;		/* 0x30c unused */
	u32 dbgstat;		/* 0x310 unused */
	u8 reserved_0x314[12];	/* 0x314 */
	u32 swctl;		/* 0x320 */
	u32 swstat;		/* 0x324 */
	u8 reserved_0x328[7768];/* 0x328 */
	u32 unk_0x2180;		/* 0x2180 */
	u8 reserved_0x2184[188];/* 0x2184 */
	u32 unk_0x2240;		/* 0x2240 */
	u8 reserved_0x2244[3900];/* 0x2244 */
	u32 unk_0x3180;		/* 0x3180 */
	u8 reserved_0x3184[188];/* 0x3184 */
	u32 unk_0x3240;		/* 0x3240 */
	u8 reserved_0x3244[3900];/* 0x3244 */
	u32 unk_0x4180;		/* 0x4180 */
	u8 reserved_0x4184[188];/* 0x4184 */
	u32 unk_0x4240;		/* 0x4240 */
};
check_member(sunxi_mctl_ctl_reg, swstat, 0x324);
check_member(sunxi_mctl_ctl_reg, unk_0x4240, 0x4240);

#define MSTR_DEVICETYPE_DDR3	BIT(0)
#define MSTR_DEVICETYPE_LPDDR2	BIT(2)
#define MSTR_DEVICETYPE_LPDDR3	BIT(3)
#define MSTR_DEVICETYPE_DDR4	BIT(4)
#define MSTR_DEVICETYPE_LPDDR4	BIT(5)
#define MSTR_DEVICETYPE_MASK	GENMASK(5, 0)
#define MSTR_2TMODE		BIT(10)
#define MSTR_BUSWIDTH_FULL	(0 << 12)
#define MSTR_BUSWIDTH_HALF	(1 << 12)
#define MSTR_ACTIVE_RANKS(x)	((((x) == 2) ? 3 : 1) << 24)
#define MSTR_BURST_LENGTH(x)	(((x) >> 1) << 16)

#define TPR10_CA_BIT_DELAY	0xffff0000
#define TPR10_DX_BIT_DELAY0	BIT(17)
#define TPR10_DX_BIT_DELAY1	BIT(18)
#define TPR10_WRITE_LEVELING	BIT(20)
#define TPR10_READ_CALIBRATION	BIT(21)
#define TPR10_READ_TRAINING	BIT(22)
#define TPR10_WRITE_TRAINING	BIT(23)

struct dram_para {
	enum sunxi_dram_type type;
	u32 dx_odt;
	u32 dx_dri;
	u32 ca_dri;
	u32 tpr0;
	u32 tpr1;
	u32 tpr2;
	u32 tpr6;
	u32 tpr10;
};

struct dram_config {
	u8 cols;
	u8 rows;
	u8 ranks;
	u8 bus_full_width;
	u32 clk;
	u32 odt_en;
	u32 tpr11;
	u32 tpr12;
	u32 tpr14;
};

static inline int ns_to_t(int nanoseconds, u32 clk)
{
	const unsigned int ctrl_freq = clk / 2;

	return DIV_ROUND_UP(ctrl_freq * nanoseconds, 1000);
}

void mctl_set_timing_params(u32 clk);

#endif /* _SUNXI_DRAM_SUN55I_T527_H */

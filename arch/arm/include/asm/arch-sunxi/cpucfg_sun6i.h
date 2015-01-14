/*
 * Sunxi A31 CPUCFG register definition.
 *
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CPUCFG_H
#define _SUNXI_CPUCFG_H

#ifndef __ASSEMBLY__

struct sunxi_cpucfg_reg {
	u8 res0[0x40];		/* 0x000 */
	u32 cpu0_rst;		/* 0x040 */
	u32 cpu0_ctrl;		/* 0x044 */
	u32 cpu0_status;	/* 0x048 */
	u8 res1[0x34];		/* 0x04c */
	u32 cpu1_rst;		/* 0x080 */
	u32 cpu1_ctrl;		/* 0x084 */
	u32 cpu1_status;	/* 0x088 */
	u8 res2[0x34];		/* 0x08c */
	u32 cpu2_rst;		/* 0x0c0 */
	u32 cpu2_ctrl;		/* 0x0c4 */
	u32 cpu2_status;	/* 0x0c8 */
	u8 res3[0x34];		/* 0x0cc */
	u32 cpu3_rst;		/* 0x100 */
	u32 cpu3_ctrl;		/* 0x104 */
	u32 cpu3_status;	/* 0x108 */
	u8 res4[0x78];		/* 0x10c */
	u32 gen_ctrl;		/* 0x184 */
	u32 l2_status;		/* 0x188 */
	u8 res5[0x4];		/* 0x18c */
	u32 event_in;		/* 0x190 */
	u8 res6[0xc];		/* 0x194 */
	u32 super_standy_flag;	/* 0x1a0 */
	u32 priv0;		/* 0x1a4 */
	u32 priv1;		/* 0x1a8 */
	u8 res7[0x54];		/* 0x1ac */
	u32 idle_cnt0_low;	/* 0x200 */
	u32 idle_cnt0_high;	/* 0x204 */
	u32 idle_cnt0_ctrl;	/* 0x208 */
	u8 res8[0x4];		/* 0x20c */
	u32 idle_cnt1_low;	/* 0x210 */
	u32 idle_cnt1_high;	/* 0x214 */
	u32 idle_cnt1_ctrl;	/* 0x218 */
	u8 res9[0x4];		/* 0x21c */
	u32 idle_cnt2_low;	/* 0x220 */
	u32 idle_cnt2_high;	/* 0x224 */
	u32 idle_cnt2_ctrl;	/* 0x228 */
	u8 res10[0x4];		/* 0x22c */
	u32 idle_cnt3_low;	/* 0x230 */
	u32 idle_cnt3_high;	/* 0x234 */
	u32 idle_cnt3_ctrl;	/* 0x238 */
	u8 res11[0x4];		/* 0x23c */
	u32 idle_cnt4_low;	/* 0x240 */
	u32 idle_cnt4_high;	/* 0x244 */
	u32 idle_cnt4_ctrl;	/* 0x248 */
	u8 res12[0x34];		/* 0x24c */
	u32 cnt64_ctrl;		/* 0x280 */
	u32 cnt64_low;		/* 0x284 */
	u32 cnt64_high;		/* 0x288 */
};

#endif /* __ASSEMBLY__ */
#endif /* _SUNXI_CPUCFG_H */

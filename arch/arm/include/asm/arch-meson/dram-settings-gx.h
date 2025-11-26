/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2023-2024, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#ifndef DRAM_SETTINGS_GX_H
#define DRAM_SETTINGS_GX_H
#include <linux/bitops.h>
#include <asm/arch/dram-gx.h>

/*
 * These registers are pretty similar to other DRAM registers found in
 * Allwinner A31/sun6i. Some of these registers also exist in some Rockchip
 * SoCs and the TI KeyStone3.
 */
/* DMC control register */
#if defined(CONFIG_DRAM_TWO_IDENTICAL_RANKS) || defined(CONFIG_DRAM_ONE_RANK)
#define DMC_DRAM_SIZE_SHIFT 6
#else
#define DMC_DRAM_SIZE_SHIFT 7
#endif
#define DMC_CTRL_CHANNEL	BIT(6) /* Channel 0 only */
#if defined(CONFIG_DRAM_DDR4)
#define DMC_CTRL_DDR_TYPE	BIT(22) | BIT(20) /* DDR4 */
#else
#define DMC_CTRL_DDR_TYPE	0
#endif
#if defined(CONFIG_DRAM_ONE_RANK) || defined(CONFIG_DRAM_TWO_DIFF_RANKS)
#define DMC_CTRL_RANK	BIT(21) /* Enable rank 1 */
#elif defined(CONFIG_DRAM_TWO_IDENTICAL_RANKS)
#define DMC_CTRL_RANK	BIT(22) /* Rank 0 and 1 are identical */
#elif defined(CONFIG_DRAM_16BIT_RANK)
#define DMC_CTRL_RANK	BIT(16) /* 16-bit Rank 0 */
#endif
#define DMC_CTRL	DMC_CTRL_CHANNEL | DMC_CTRL_RANK | DMC_CTRL_DDR_TYPE

/* Mode Register */
#if defined(CONFIG_MESON_GXL) && defined(CONFIG_DRAM_DDR4)
#define PUB_MR0			4 | (((((timings.cl - 9) >> 1) & 7) << 4)) | \
	((((timings.wr - 10) >> 1) & 7) << 9)
#define PUB_MR1			(timings.odt << 8) | (timings.drv << 1) | 0x81
#define PUB_MR2			(((timings.cwl - 6) >> 1) & 7) << 3 | 0xc0
#define PUB_MR3			0
#define PUB_MR4			8
#else
#define PUB_MR0			(((timings.cl - 4) & 8) >> 1) | \
	(((timings.cl - 4) & 7) << 4) | \
	(((timings.wr <= 8 ? (timings.wr - 4) : (timings.wr >> 1)) & 7) << 9) | 0x1c00
#define PUB_MR1			(timings.drv << 1) | \
	((timings.odt & 1) << 2)        | \
	(((timings.odt >> 1) & 1) << 6) | \
	(((timings.odt >> 2) & 1) << 9) | \
	BIT(7)			        | \
	((timings.al ? ((timings.cl - timings.al) & 3) : 0) << 3)
#define PUB_MR2			BIT(6) | (((timings.cwl - 5) & 7) << 3)
#endif
#define PUB_MR3			0
#if defined(CONFIG_MESON_GXL)
#if defined(CONFIG_DRAM_DDR3)
#define PUB_MR4			0
#define PUB_MR5			0x420
#elif defined(CONFIG_DRAM_DDR4)
#define PUB_MR5			0x400
#endif
#define PUB_MR6			0x800
#endif

/* ODT Configuration Register */
#if defined(CONFIG_MESON_GXBB)
#define PUB_ODTCR	0x210000
#elif defined(CONFIG_MESON_GXL)
#define PUB_ODTCR	0x30000
#endif

/* DDR Timing Parameter */
#if defined(CONFIG_MESON_GXBB)
#define PUB_DTPR0	timings.rtp | \
	(timings.wtr << 4)  | \
	(timings.rp  << 8)  | \
	(timings.ras << 16) | \
	(timings.rrd << 22) | \
	(timings.rcd << 26)
#define PUB_DTPR1	(timings.mod << 2) | \
	(timings.faw << 5)    | \
	(timings.rfc << 11)   | \
	(timings.wlmrd << 20) | \
	(timings.wlo << 26)
#define PUB_DTPR2	timings.xs | \
	(timings.xp << 10)   | \
	(timings.dllk << 19)
#define PUB_DTPR3	0 | \
	(0 << 3)            | \
	(timings.rc << 6)   | \
	(timings.cke << 13) | \
	(timings.mrd << 18) | \
	(0 << 29)
#elif defined(CONFIG_MESON_GXL)
#define PUB_DTPR0	timings.rtp | \
	(timings.rp  << 8)  | \
	(timings.ras << 16) | \
	(timings.rrd << 24)
#define PUB_DTPR1	(timings.wlmrd << 24) | \
	(timings.faw << 16) | \
	timings.mrd
#define PUB_DTPR2	timings.xs | \
	(timings.cke << 16)
#define PUB_DTPR3	(timings.dllk << 16) | (4 << 28)
#define PUB_DTPR4	timings.xp | BIT(11) | (timings.rfc << 0x10)
#define PUB_DTPR5	(timings.rc << 16) | (timings.rcd << 8) | \
	timings.wtr
#endif

#if defined(CONFIG_MESON_GXBB)
#define PUB_PGCR0	0x7D81E3F
#define PUB_PGCR1	0x380C6A0
#define PUB_PGCR2	(0x1F12480 & 0xefffffff)
#define PUB_PGCR3	0xC0AAFE60
#elif defined(CONFIG_MESON_GXL)
#define PUB_PGCR0	0x7d81e3f
#define PUB_PGCR1	0x2004620
#define PUB_PGCR2	(0xf05f97 & 0xefffffff)
#if defined(CONFIG_DRAM_DDR3)
#define PUB_PGCR3	0xc0aae860
#elif defined(CONFIG_DRAM_DDR4)
#define PUB_PGCR3	0xc0aae860 | 0x4000000
#endif
#endif

#if defined(CONFIG_MESON_GXBB)
#define PUB_DXCCR	0x181884
#define PUB_DTCR	0x4300308f
#define PUB_DSGCR	0x20645A

#define PUB_ZQ0PR	0x69
#define PUB_ZQ1PR	0x69
#define PUB_ZQ2PR	0x69
#define PUB_ZQ3PR	0x69
#elif defined(CONFIG_MESON_GXL)
#define PUB_DXCCR	0x20c01204

#if defined(CONFIG_DRAM_DDR4)
#define PUB_DTCR	0x80003187 | 0x40
#else
#define PUB_DTCR	0x80003187
#endif

#define PUB_DTCR1	0x00010237 /* XXX: Needed? */
#define PUB_DSGCR	(0x20641b | 0x800004) /* Works on DDR4 too? */

#if defined(CONFIG_DRAM_DDR3)
#define PUB_ZQ0PR	0x5d95d
#define PUB_ZQ1PR	0x5d95d
#define PUB_ZQ2PR	0x5d95d
#define PUB_ZQ3PR	0x1dd1d
#elif defined(CONFIG_DRAM_DDR4)
#define PUB_ZQ0PR	0x775d
#define PUB_ZQ1PR	0x6fc5d
#define PUB_ZQ2PR	0x6fc5d
#define PUB_ZQ3PR	0x1dd1d
#endif
#endif

#if defined(CONFIG_DRAM_DDR3)
#define PUB_DCR	0xb
#elif defined(CONFIG_DRAM_DDR4)
#define PUB_DCR	0x1800040c
#endif
#define PUB_DTAR	(0 | (0 << 12) | (0 << 28)) /* Uh? */

#define PCTL0_1US_PCK	0x1C8
#define PCTL0_100NS_PCK	0x2D
#define PCTL0_INIT_US	0x2
#define PCTL0_RSTH_US	0x2

/* Mode Config(?) */
#if defined(CONFIG_MESON_GXBB)
#define PCTL0_MCFG	((((timings.faw + timings.rrd - 1) / timings.rrd) & 3) << 0x12) | \
	(0xa2f21 & 0xfff3ffff)
#define PCTL0_MCFG1	(((timings.rrd - ((timings.faw - (timings.faw / timings.rrd) * \
	timings.rrd) & 0xff)) & 7) << 8) | \
	(0x80200000 & 0xfffffcff)
#elif defined(CONFIG_MESON_GXL)
#if defined(CONFIG_DRAM_DDR3)
#define PCTL0_MCFG_DDRTYPE	0
#elif defined(CONFIG_DRAM_DDR4)
#define PCTL0_MCFG_DDRTYPE	BIT(4)
#endif

#define PCTL0_MCFG	(0xa2f21 & 0xffffff8f) | PCTL0_MCFG_DDRTYPE
/* XXX: What is this?           ↓ ??? */
#define PCTL0_MCFG1	0
#endif

#define PCTL0_SCFG	0xF01

#if defined(CONFIG_MESON_GXL) && defined(CONFIG_DRAM_16BIT_RANK)
#define PCTL0_PPCFG	0x1fd
#else
#define PCTL0_PPCFG	0x1e0
#endif

#define PCTL0_DFISTCFG0	0x4
#define PCTL0_DFISTCFG1	0x1

#define PCTL0_DFITCTRLDELAY	0x2

#if defined(CONFIG_MESON_GXBB)
#define PCTL0_DFITPHYWRDATA	0x1
#else
#define PCTL0_DFITPHYWRDATA	0x2
#endif

#if defined(CONFIG_MESON_GXBB)
#define PCTL0_DFITPHYWRLTA	(timings.cwl + timings.al - \
	(((timings.cwl + timings.al) % 2) ? 3 : 4)) / 2
#define PCTL0_DFITRDDATAEN	(timings.cl + timings.al - \
	(((timings.cl + timings.al) % 2) ? 3 : 4)) / 2
#define PCTL0_DFITPHYRDLAT	((timings.cl + timings.al) % 2) ? 14 : 16
#elif defined(CONFIG_MESON_GXL)
#define PCTL0_DFITPHYWRLTA	((timings.cwl + timings.al) - 2)
#define PCTL0_DFITRDDATAEN	((timings.cl + timings.al) - 4)
#define PCTL0_DFITPHYRDLAT	0x16
#endif

#define PCTL0_DFITDRAMCLKDIS	1
#define PCTL0_DFITDRAMCLKEN	1
#if defined(CONFIG_MESON_GXBB)
#define PCTL0_DFITPHYUPDTYPE1	0x200
#else
#define PCTL0_DFITPHYUPDTYPE0	16
#define PCTL0_DFITPHYUPDTYPE1	16
#define PCTL0_DFITCTRLUPDMAX	64
#define PCTL0_DFIUPDCFG		3
#endif
#define PCTL0_DFITCTRLUPDMIN	16

#define PCTL0_CMDTSTATEN	1

#if defined(CONFIG_DRAM_ONE_RANK) || defined(CONFIG_DRAM_16BIT_RANK)
#define PCTL0_DFIODTCFG	0x808
#elif defined(CONFIG_DRAM_TWO_DIFF_RANKS)
#define PCTL0_DFIODTCFG	0xc0c
#elif defined(CONFIG_DRAM_TWO_IDENTICAL_RANKS)
#define PCTL0_DFIODTCFG	0x8
#endif

#if defined(CONFIG_MESON_GXBB)
#define PCTL0_DFIODTCFG1	(0 | (6 << 16))
#elif defined(CONFIG_MESON_GXL)
#if defined(CONFIG_DRAM_16BIT_RANK)
#define PCTL0_DFIODTCFG1	((6 << 16) | (8 << 16))
#else
#define PCTL0_DFIODTCFG1	((6 << 16) | (3 << 25) | (8 << 16))
#endif
#endif
#define PCTL0_DFILPCFG0		(1 | (3 << 4) | BIT(8) | (3 << 12) | \
	(7 << 16) | BIT(24) | (3 << 28))

#if defined(CONFIG_MESON_GXBB)
#define PUB_ACBDLR0		0x10
#elif defined(CONFIG_MESON_GXL)
#if defined(CONFIG_DRAM_DDR3)
#define PUB_ACBDLR0		0
#define PUB_ACBDLR3		0
#define PUB_ACLCDLR		48
#elif defined(CONFIG_DRAM_DDR4)
#define PUB_ACBDLR0		0x3f
#define PUB_ACBDLR3		0x10
#define PUB_ACLCDLR		0x28
#else
#define PUB_ACBDLR0		0
#define PUB_ACBDLR3		0
#define PUB_ACLCDLR		48
#endif
#endif

#define LPDDR3_CA0	2
#define LPDDR3_CA1	0
#define LPDDR3_REMAP	3
#define LPDDR3_WL	1

/* PLL */
#if defined(CONFIG_MESON_GXBB)
#define DDR_PLL_CNTL1	0x69c80000
#define DDR_PLL_CNTL2	0xca463823
#define DDR_PLL_CNTL3	0xc00023
#define DDR_PLL_CNTL4	0x303500
#define DDR_PLL_CNTL5	0 /* Unused */
#elif defined(CONFIG_MESON_GXL)
#define DDR_PLL_CNTL1	0xaa203
#define DDR_PLL_CNTL2	0x2919a288
#define DDR_PLL_CNTL3	0x3e3b744
#define DDR_PLL_CNTL4	0xc0101
#define DDR_PLL_CNTL5	0xe600001e
#endif

#endif /* DRAM_SETTINGS_GX_H */

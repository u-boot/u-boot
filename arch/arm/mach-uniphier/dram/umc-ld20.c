/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * based on commit 5e1cb0f1caeabc6c99469dd997cb6b4f46834443 of Diag
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <asm/processor.h>

#include "../init.h"
#include "ddruqphy-regs.h"
#include "umc64-regs.h"

#define DRAM_CH_NR	3

enum dram_freq {
	DRAM_FREQ_1866M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

enum dram_board {		/* board type */
	DRAM_BOARD_LD20_REF,	/* LD20 reference */
	DRAM_BOARD_LD20_GLOBAL,	/* LD20 TV */
	DRAM_BOARD_LD20_C1,	/* LD20 TV C1 */
	DRAM_BOARD_LD21_REF,	/* LD21 reference */
	DRAM_BOARD_LD21_GLOBAL,	/* LD21 TV */
	DRAM_BOARD_NR,
};

/* PHY */
static const int ddrphy_adrctrl[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 reference */
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 TV */
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 TV C1 */
	{268 - 212, 268 - 268, /* No CH2 */},	/* LD21 reference */
	{268 - 212, 268 - 268, /* No CH2 */},	/* LD21 TV */
};

static const int ddrphy_dlltrimclk[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268, 268, 268},			/* LD20 reference */
	{268, 268, 268},			/* LD20 TV */
	{189, 189, 189},			/* LD20 TV C1 */
	{268, 268 + 252, /* No CH2 */},		/* LD21 reference */
	{268, 268 + 202, /* No CH2 */},		/* LD21 TV */
};

static const int ddrphy_dllrecalib[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 reference */
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 TV */
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 TV C1 */
	{268 - 212, 268 - 536, /* No CH2 */},	/* LD21 reference */
	{268 - 212, 268 - 536, /* No CH2 */},	/* LD21 TV */
};

static const u32 ddrphy_phy_pad_ctrl[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{0x50B840B1, 0x50B840B1, 0x50B840B1},	/* LD20 reference */
	{0x50BB40B1, 0x50BB40B1, 0x50BB40B1},	/* LD20 TV */
	{0x50BB40B1, 0x50BB40B1, 0x50BB40B1},	/* LD20 TV C1 */
	{0x50BB40B4, 0x50B840B1, /* No CH2 */},	/* LD21 reference */
	{0x50BB40B4, 0x50B840B1, /* No CH2 */},	/* LD21 TV */
};

static const u32 ddrphy_scl_gate_timing[DRAM_CH_NR] = {
	0x00000140, 0x00000180, 0x00000140
};

static const int ddrphy_op_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD20 TV */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD20 TV C1 */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD21 reference */
		{
			1, 1, 0, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0,
			1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0,
			1, 1, 1, 1, 0, 1, 0, 0,
		},
		/* No CH2 */
	},
	{ /* LD21 TV */
		{
			1, 1, 0, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0,
			1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0,
			1, 1, 1, 1, 0, 1, 0, 0,
		},
		/* No CH2 */
	},
};

static int ddrphy_ip_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD20 TV */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD20 TV C1 */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD21 reference */
		{
			2, 2, 2, 2, 1, 2, 2, 2,
			2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1,
			2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1,
			1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0,
			0, 1, 0, 0, 1, 1, 0, 1,
		},
		/* No CH2 */
	},
	{ /* LD21 TV */
		{
			2, 2, 2, 2, 1, 2, 2, 2,
			2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1,
			2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1,
			1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0,
			0, 1, 0, 0, 1, 1, 0, 1,
		},
		/* No CH2 */
	},
};

/* DDR PHY */
static void ddrphy_select_lane(void __iomem *phy_base, unsigned int lane,
			       unsigned int bit)
{
	WARN_ON(lane >= 1 << PHY_LANE_SEL_LANE_WIDTH);
	WARN_ON(bit >= 1 << PHY_LANE_SEL_BIT_WIDTH);

	writel((bit << PHY_LANE_SEL_BIT_SHIFT) |
	       (lane << PHY_LANE_SEL_LANE_SHIFT),
	       phy_base + PHY_LANE_SEL);
}

static void ddrphy_init(void __iomem *phy_base, enum dram_board board, int ch)
{
	writel(0x0C001001, phy_base + PHY_UNIQUIFY_TSMC_IO_1);
	while (!(readl(phy_base + PHY_UNIQUIFY_TSMC_IO_1) & BIT(1)))
		cpu_relax();
	writel(0x0C001000, phy_base + PHY_UNIQUIFY_TSMC_IO_1);

	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_3);
	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_1);
	ddrphy_select_lane(phy_base, 0, 0);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	ddrphy_select_lane(phy_base, 6, 0);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	ddrphy_select_lane(phy_base, 12, 0);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	ddrphy_select_lane(phy_base, 18, 0);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000001, phy_base + PHY_SCL_WINDOW_TRIM);
	writel(0x00000000, phy_base + PHY_UNQ_ANALOG_DLL_1);
	writel(ddrphy_phy_pad_ctrl[board][ch], phy_base + PHY_PAD_CTRL);
	writel(0x00000070, phy_base + PHY_VREF_TRAINING);
	writel(0x01000075, phy_base + PHY_SCL_CONFIG_1);
	writel(0x00000501, phy_base + PHY_SCL_CONFIG_2);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_3);
	writel(0x000261c0, phy_base + PHY_DYNAMIC_WRITE_BIT_LVL);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_4);
	writel(ddrphy_scl_gate_timing[ch], phy_base + PHY_SCL_GATE_TIMING);
	writel(0x02a000a0, phy_base + PHY_WRLVL_DYN_ODT);
	writel(0x00840004, phy_base + PHY_WRLVL_ON_OFF);
	writel(0x0000020d, phy_base + PHY_DLL_ADRCTRL);
	ddrphy_select_lane(phy_base, 0, 0);
	writel(0x0000008d, phy_base + PHY_DLL_TRIM_CLK);
	writel(0xa800100d, phy_base + PHY_DLL_RECALIB);
	writel(0x00005076, phy_base + PHY_SCL_LATENCY);
}

static int ddrphy_to_dly_step(void __iomem *phy_base, unsigned int freq,
			      int delay)
{
	int mdl;

	mdl = (readl(phy_base + PHY_DLL_ADRCTRL) & PHY_DLL_ADRCTRL_MDL_MASK) >>
						PHY_DLL_ADRCTRL_MDL_SHIFT;

	return DIV_ROUND_CLOSEST((long)freq * delay * mdl, 2 * 1000000L);
}

static void ddrphy_set_delay(void __iomem *phy_base, unsigned int reg,
			     u32 mask, u32 incr, int dly_step)
{
	u32 tmp;

	tmp = readl(phy_base + reg);
	tmp &= ~mask;
	tmp |= min_t(u32, abs(dly_step), mask);

	if (dly_step >= 0)
		tmp |= incr;
	else
		tmp &= ~incr;

	writel(tmp, phy_base + reg);
}

static void ddrphy_set_dll_recalib(void __iomem *phy_base, int dly_step)
{
	ddrphy_set_delay(phy_base, PHY_DLL_RECALIB,
			 PHY_DLL_RECALIB_TRIM_MASK, PHY_DLL_RECALIB_INCR,
			 dly_step);
}

static void ddrphy_set_dll_adrctrl(void __iomem *phy_base, int dly_step)
{
	ddrphy_set_delay(phy_base, PHY_DLL_ADRCTRL,
			 PHY_DLL_ADRCTRL_TRIM_MASK, PHY_DLL_ADRCTRL_INCR,
			 dly_step);
}

static void ddrphy_set_dll_trim_clk(void __iomem *phy_base, int dly_step)
{
	ddrphy_select_lane(phy_base, 0, 0);

	ddrphy_set_delay(phy_base, PHY_DLL_TRIM_CLK,
			 PHY_DLL_TRIM_CLK_MASK, PHY_DLL_TRIM_CLK_INCR,
			 dly_step);
}

static void ddrphy_init_tail(void __iomem *phy_base, enum dram_board board,
			     unsigned int freq, int ch)
{
	int step;

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_adrctrl[board][ch]);
	ddrphy_set_dll_adrctrl(phy_base, step);

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_dlltrimclk[board][ch]);
	ddrphy_set_dll_trim_clk(phy_base, step);

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_dllrecalib[board][ch]);
	ddrphy_set_dll_recalib(phy_base, step);
}

static void ddrphy_shift_one_dq(void __iomem *phy_base, unsigned int reg,
				u32 mask, u32 incr, int shift_val)
{
	u32 tmp;
	int val;

	tmp = readl(phy_base + reg);

	val = tmp & mask;
	if (!(tmp & incr))
		val = -val;

	val += shift_val;

	tmp &= ~(incr | mask);
	tmp |= min_t(u32, abs(val), mask);
	if (val >= 0)
		tmp |= incr;

	writel(tmp, phy_base + reg);
}

static void ddrphy_shift_dq(void __iomem *phy_base, unsigned int reg,
			    u32 mask, u32 incr, u32 override,
			    const int *shift_val_array)
{
	u32 tmp;
	int dx, bit;

	tmp = readl(phy_base + reg);
	tmp |= override;
	writel(tmp, phy_base + reg);

	for (dx = 0; dx < 4; dx++) {
		for (bit = 0; bit < 8; bit++) {
			ddrphy_select_lane(phy_base,
					   (PHY_BITLVL_DLY_WIDTH + 1) * dx,
					   bit);

			ddrphy_shift_one_dq(phy_base, reg, mask, incr,
					    shift_val_array[dx * 8 + bit]);
		}
	}

	ddrphy_select_lane(phy_base, 0, 0);
}

static int ddrphy_training(void __iomem *phy_base, enum dram_board board,
			   int ch)
{
	writel(0x0000000f, phy_base + PHY_WRLVL_AUTOINC_TRIM);
	writel(0x00010000, phy_base + PHY_DLL_TRIM_2);
	writel(0x50000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		cpu_relax();

	writel(0x00000000, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_0);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_1);
	writel(0xFBF8FFFF, phy_base + PHY_SCL_START_ADDR);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		cpu_relax();

	writel(0xFBF0FFFF, phy_base + PHY_SCL_START_ADDR);
	writel(0x30500000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		cpu_relax();

	writel(0x00000001, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0x00000010, phy_base + PHY_SCL_MAIN_CLK_DELTA);
	writel(0x789b3de0, phy_base + PHY_SCL_DATA_0);
	writel(0xf10e4a56, phy_base + PHY_SCL_DATA_1);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		cpu_relax();

	writel(0x34000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		cpu_relax();

	writel(0x00000003, phy_base + PHY_DISABLE_GATING_FOR_SCL);

	writel(0x000261c0, phy_base + PHY_DYNAMIC_WRITE_BIT_LVL);
	writel(0x00003270, phy_base + PHY_DYNAMIC_BIT_LVL);
	writel(0x011BD0C4, phy_base + PHY_DSCL_CNT);

	/* shift ip_dq trim */
	ddrphy_shift_dq(phy_base,
			PHY_IP_DQ_DQS_BITWISE_TRIM,
			PHY_IP_DQ_DQS_BITWISE_TRIM_MASK,
			PHY_IP_DQ_DQS_BITWISE_TRIM_INC,
			PHY_IP_DQ_DQS_BITWISE_TRIM_OVERRIDE,
			ddrphy_ip_dq_shift_val[board][ch]);

	/* shift op_dq trim */
	ddrphy_shift_dq(phy_base,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_MASK,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_INC,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_OVERRIDE,
			ddrphy_op_dq_shift_val[board][ch]);

	return 0;
}

/* UMC */
static const u32 umc_initctla[DRAM_FREQ_NR] = {0x71016D11};
static const u32 umc_initctlb[DRAM_FREQ_NR] = {0x07E390AC};
static const u32 umc_initctlc[DRAM_FREQ_NR] = {0x00FF00FF};
static const u32 umc_drmmr0[DRAM_FREQ_NR] = {0x00000114};
static const u32 umc_drmmr2[DRAM_FREQ_NR] = {0x000002a0};

static const u32 umc_memconf0a[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000601, 0x00000801},	/* 1866 MHz */
};

static const u32 umc_memconf0b[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000120, 0x00000130},	/* 1866 MHz */
};

static const u32 umc_memconfch[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00033603, 0x00033803},	/* 1866 MHz */
};

static const u32 umc_cmdctla[DRAM_FREQ_NR] = {0x060D0D20};
static const u32 umc_cmdctlb[DRAM_FREQ_NR] = {0x2D211C08};
static const u32 umc_cmdctlc[DRAM_FREQ_NR] = {0x00150C04};
static const u32 umc_cmdctle[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x0049071D, 0x0078071D},	/* 1866 MHz */
};

static const u32 umc_rdatactl[DRAM_FREQ_NR] = {0x00000610};
static const u32 umc_wdatactl[DRAM_FREQ_NR] = {0x00000204};
static const u32 umc_odtctl[DRAM_FREQ_NR] = {0x02000002};
static const u32 umc_dataset[DRAM_FREQ_NR] = {0x04000000};

static const u32 umc_flowctla[DRAM_FREQ_NR] = {0x0081E01E};
static const u32 umc_directbusctrla[DRAM_CH_NR] = {
	0x00000000, 0x00000001, 0x00000001
};

static void umc_poll_phy_init_complete(void __iomem *dc_base)
{
	/* Wait for PHY Init Complete */
	while (!(readl(dc_base + UMC_DFISTCTLC) & BIT(0)))
		cpu_relax();
}

static int umc_dc_init(void __iomem *dc_base, unsigned int freq,
		       unsigned long size, int ch)
{
	enum dram_freq freq_e;
	enum dram_size size_e;

	switch (freq) {
	case 1866:
		freq_e = DRAM_FREQ_1866M;
		break;
	default:
		pr_err("unsupported DRAM frequency %ud MHz\n", freq);
		return -EINVAL;
	}

	switch (size) {
	case 0:
		return 0;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	case SZ_512M:
		size_e = DRAM_SZ_512M;
		break;
	default:
		pr_err("unsupported DRAM size 0x%08lx (per 16bit) for ch%d\n",
		       size, ch);
		return -EINVAL;
	}

	writel(0x00000001, dc_base + UMC_DFICSOVRRD);
	writel(0x00000000, dc_base + UMC_DFITURNOFF);

	writel(umc_initctla[freq_e], dc_base + UMC_INITCTLA);
	writel(umc_initctlb[freq_e], dc_base + UMC_INITCTLB);
	writel(umc_initctlc[freq_e], dc_base + UMC_INITCTLC);

	writel(umc_drmmr0[freq_e], dc_base + UMC_DRMMR0);
	writel(0x00000004, dc_base + UMC_DRMMR1);
	writel(umc_drmmr2[freq_e], dc_base + UMC_DRMMR2);
	writel(0x00000000, dc_base + UMC_DRMMR3);

	writel(umc_memconf0a[freq_e][size_e], dc_base + UMC_MEMCONF0A);
	writel(umc_memconf0b[freq_e][size_e], dc_base + UMC_MEMCONF0B);
	writel(umc_memconfch[freq_e][size_e], dc_base + UMC_MEMCONFCH);
	writel(0x00000008, dc_base + UMC_MEMMAPSET);

	writel(umc_cmdctla[freq_e], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb[freq_e], dc_base + UMC_CMDCTLB);
	writel(umc_cmdctlc[freq_e], dc_base + UMC_CMDCTLC);
	writel(umc_cmdctle[freq_e][size_e], dc_base + UMC_CMDCTLE);

	writel(umc_rdatactl[freq_e], dc_base + UMC_RDATACTL_D0);
	writel(umc_rdatactl[freq_e], dc_base + UMC_RDATACTL_D1);

	writel(umc_wdatactl[freq_e], dc_base + UMC_WDATACTL_D0);
	writel(umc_wdatactl[freq_e], dc_base + UMC_WDATACTL_D1);
	writel(umc_odtctl[freq_e], dc_base + UMC_ODTCTL_D0);
	writel(umc_odtctl[freq_e], dc_base + UMC_ODTCTL_D1);
	writel(umc_dataset[freq_e], dc_base + UMC_DATASET);

	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x00000003, dc_base + UMC_ACSSETA);
	writel(0x00000103, dc_base + UMC_FLOWCTLG);
	writel(0x00010200, dc_base + UMC_ACSSETB);

	writel(umc_flowctla[freq_e], dc_base + UMC_FLOWCTLA);
	writel(0x00004444, dc_base + UMC_FLOWCTLC);
	writel(0x00000000, dc_base + UMC_DFICUPDCTLA);

	writel(0x00202000, dc_base + UMC_FLOWCTLB);
	writel(0x00000000, dc_base + UMC_BSICMAPSET);
	writel(0x00000000, dc_base + UMC_ERRMASKA);
	writel(0x00000000, dc_base + UMC_ERRMASKB);

	writel(umc_directbusctrla[ch], dc_base + UMC_DIRECTBUSCTRLA);

	writel(0x00000001, dc_base + UMC_INITSET);
	/* Wait for PHY Init Complete */
	while (readl(dc_base + UMC_INITSTAT) & BIT(0))
		cpu_relax();

	writel(0x2A0A0A00, dc_base + UMC_SPCSETB);
	writel(0x00000000, dc_base + UMC_DFICSOVRRD);

	return 0;
}

static int umc_ch_init(void __iomem *umc_ch_base, void __iomem *phy_ch_base,
		       enum dram_board board, unsigned int freq,
		       unsigned long size, int ch)
{
	void __iomem *dc_base = umc_ch_base + 0x00011000;
	void __iomem *phy_base = phy_ch_base;
	int ret;

	/* PHY Update Mode (ON) */
	writel(0x8000003f, dc_base + UMC_DFIPUPDCTLA);

	/* deassert PHY reset signals */
	writel(UMC_DIOCTLA_CTL_NRST | UMC_DIOCTLA_CFG_NRST,
	       dc_base + UMC_DIOCTLA);

	ddrphy_init(phy_base, board, ch);

	umc_poll_phy_init_complete(dc_base);

	ddrphy_init_tail(phy_base, board, freq, ch);

	ret = umc_dc_init(dc_base, freq, size, ch);
	if (ret)
		return ret;

	ret = ddrphy_training(phy_base, board, ch);
	if (ret)
		return ret;

	return 0;
}

static void um_init(void __iomem *um_base)
{
	writel(0x000000ff, um_base + UMC_MBUS0);
	writel(0x000000ff, um_base + UMC_MBUS1);
	writel(0x000000ff, um_base + UMC_MBUS2);
	writel(0x00000001, um_base + UMC_MBUS3);
	writel(0x00000001, um_base + UMC_MBUS4);
	writel(0x00000001, um_base + UMC_MBUS5);
	writel(0x00000001, um_base + UMC_MBUS6);
	writel(0x00000001, um_base + UMC_MBUS7);
	writel(0x00000001, um_base + UMC_MBUS8);
	writel(0x00000001, um_base + UMC_MBUS9);
	writel(0x00000001, um_base + UMC_MBUS10);
}

int uniphier_ld20_umc_init(const struct uniphier_board_data *bd)
{
	void __iomem *um_base = (void __iomem *)0x5b600000;
	void __iomem *umc_ch_base = (void __iomem *)0x5b800000;
	void __iomem *phy_ch_base = (void __iomem *)0x6e200000;
	enum dram_board board;
	int ch, ret;

	switch (UNIPHIER_BD_BOARD_GET_TYPE(bd->flags)) {
	case UNIPHIER_BD_BOARD_LD20_REF:
		board = DRAM_BOARD_LD20_REF;
		break;
	case UNIPHIER_BD_BOARD_LD20_GLOBAL:
		board = DRAM_BOARD_LD20_GLOBAL;
		break;
	case UNIPHIER_BD_BOARD_LD20_C1:
		board = DRAM_BOARD_LD20_C1;
		break;
	case UNIPHIER_BD_BOARD_LD21_REF:
		board = DRAM_BOARD_LD21_REF;
		break;
	case UNIPHIER_BD_BOARD_LD21_GLOBAL:
		board = DRAM_BOARD_LD21_GLOBAL;
		break;
	default:
		pr_err("unsupported board type %d\n",
		       UNIPHIER_BD_BOARD_GET_TYPE(bd->flags));
		return -EINVAL;
	}

	for (ch = 0; ch < bd->dram_nr_ch; ch++) {
		unsigned long size = bd->dram_ch[ch].size;
		unsigned int width = bd->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, phy_ch_base, board,
				  bd->dram_freq, size / (width / 16), ch);
		if (ret) {
			pr_err("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
		phy_ch_base += 0x00004000;
	}

	um_init(um_base);

	return 0;
}

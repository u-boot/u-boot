/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * based on commit a3c28918e86ad57127cf07bf8b32950cab20c03c of Diag
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
#include "ddrphy-ld20-regs.h"
#include "umc64-regs.h"

#define DRAM_CH_NR	3
#define CONFIG_DDR_FREQ		1866

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
	DRAM_BOARD_LD21_REF,	/* LD21 reference */
	DRAM_BOARD_LD21_GLOBAL,	/* LD21 TV */
	DRAM_BOARD_NR,
};

#define MSK_PHY_LANE_SEL		0x000000FF
#define MSK_BIT_SEL			0x00000F00
#define MSK_DLL_MAS_DLY			0xFF000000
#define MSK_MAS_DLY			0x7F000000
#define MSK_DLLS_TRIM_CLK		0x000000FF

#define PHY_DLL_MAS_DLY_WIDTH		8
#define PHY_SLV_DLY_WIDTH		6

static void ddrphy_maskwritel(u32 data, u32 mask, void *addr)
{
	u32 value;

	value = (readl(addr) & ~mask) | (data & mask);
	writel(value, addr);
}

static u32 ddrphy_maskreadl(u32 mask, void *addr)
{
	return readl(addr) & mask;
}

/* set phy_lane_sel.phy_lane_sel */
static void ddrphy_set_phy_lane_sel(int val, void __iomem *phy_base)
{
	ddrphy_maskwritel(val, MSK_PHY_LANE_SEL, phy_base + PHY_LANE_SEL);
}

/* set phy_lane_sel.bit_sel */
static void ddrphy_set_bit_sel(int bit, void __iomem *phy_base)
{
	ddrphy_maskwritel(bit << 8, MSK_BIT_SEL, phy_base + PHY_LANE_SEL);
}

/* Calculating step for PUB-byte */
static int ddrphy_hpstep(int delay, void __iomem *phy_base)
{
	int mdl, freq;

	freq = CONFIG_DDR_FREQ; /* FIXME */
	mdl = ddrphy_maskreadl(MSK_DLL_MAS_DLY, phy_base + PHY_DLL_ADRCTRL) >> 24;

	return DIV_ROUND_CLOSEST(freq * delay * mdl, 2 * 1000000);
}

static void ddrphy_set_dll_trim_clk(int delay_ckoffset,  void __iomem *phy_base)
{
	u8 ck_step;	/* ckoffset_step for clock */
	u32 ck_step_all;

	/* CK-Offset */
	if (delay_ckoffset >= 0) {
		/* shift + direction */
		ck_step = min(ddrphy_hpstep(delay_ckoffset, phy_base), 127);
		ck_step_all = ((0x1<<(PHY_SLV_DLY_WIDTH + 1))|ck_step);
	} else{
		/* shift - direction */
		ck_step = min(ddrphy_hpstep(-1*delay_ckoffset, phy_base), 127);
		ck_step_all = ck_step;
	}

	ddrphy_set_phy_lane_sel(0, phy_base);
	ddrphy_maskwritel(ck_step_all, MSK_DLLS_TRIM_CLK, phy_base + PHY_DLL_TRIM_CLK);
}

static void ddrphy_set_dll_recalib(int delay_qoffset, u32 recalib_cnt,
				   u8 disable_recalib, u8 ctr_start_val,
				   void __iomem *phy_base)
{
	u8 dlls_trim_adrctrl_ma, incr_dly_adrctrl_ma; /* qoffset_step and flag for inc/dec */
	u32 recalib_all;	/* all fields of register dll_recalib */

	/* Q-Offset */
	if (delay_qoffset >= 0) {
		dlls_trim_adrctrl_ma = min(ddrphy_hpstep(delay_qoffset, phy_base), 63);
		incr_dly_adrctrl_ma = 0x1;
	} else {
		dlls_trim_adrctrl_ma = min(ddrphy_hpstep(-1*delay_qoffset, phy_base), 63);
		incr_dly_adrctrl_ma = 0x0;
	}

	recalib_all = ((ctr_start_val & 0xf) << 28) |
			(incr_dly_adrctrl_ma << 27) |
			((disable_recalib & 0x1) << 26) |
			((recalib_cnt & 0x3ffff) << 8) |
			(dlls_trim_adrctrl_ma & 0x3f);

	/* write value for all bits other than bit[7:6] */
	ddrphy_maskwritel(recalib_all, ~0xc0, phy_base + PHY_DLL_RECALIB);
}

static void ddrphy_set_dll_adrctrl(int delay_qoffset, u8 override_adrctrl,
				   void __iomem *phy_base)
{
	u8 dlls_trim_adrctrl, incr_dly_adrctrl; /* qoffset_step for clock */
	u32 adrctrl_all;

	if (delay_qoffset >= 0) {
		dlls_trim_adrctrl = min(ddrphy_hpstep(delay_qoffset, phy_base), 63);
		incr_dly_adrctrl = 0x1;
	} else {
		dlls_trim_adrctrl = min(ddrphy_hpstep(-delay_qoffset, phy_base), 63);
		incr_dly_adrctrl = 0x0;
	}

	adrctrl_all = (incr_dly_adrctrl << 9) |
			((override_adrctrl & 0x1) << 8) |
			dlls_trim_adrctrl;

	ddrphy_maskwritel(adrctrl_all, 0x33f, phy_base + PHY_DLL_ADRCTRL);
}

/* dio */
static int dio_adrctrl_0[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268-262, 268-263, 268-378},		/* LD20 reference */
	{268-262, 268-263, 268-378},		/* LD20 TV */
	{268-212, 268-268, 0},			/* LD21 reference */
	{268-212, 268-268, 0},			/* LD21 TV */
};
static int dio_dlltrimclk_0[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268, 268, 268},			/* LD20 reference */
	{268, 268, 268},			/* LD20 TV */
	{268, 268+252, 0},			/* LD21 reference */
	{268, 268+202, 0},			/* LD21 TV */
};
static int dio_dllrecalib_0[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268-378, 268-263, 268-378},		/* LD20 reference */
	{268-378, 268-263, 268-378},		/* LD20 TV */
	{268-212, 268-536, 0},			/* LD21 reference */
	{268-212, 268-536, 0},			/* LD21 TV */
};

static u32 dio_phy_pad_ctrl[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{0x50B840B1, 0x50B840B1, 0x50B840B1},	/* LD20 reference */
	{0x50BB40B1, 0x50BB40B1, 0x50BB40B1},	/* LD20 TV */
	{0x50BB40B4, 0x50B840B1, 0x50BB40B1},	/* LD21 reference */
	{0x50BB40B4, 0x50B840B1, 0x50BB40B1},	/* LD21 TV */
};

static u32 dio_scl_gate_timing[DRAM_CH_NR] = {0x00000140, 0x00000180, 0x00000140};

static int dio_op_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			2, 1, 0, 1, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1, 2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1, 1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD20 TV */
		{
			2, 1, 0, 1, 2, 1, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1, 2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1, 1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD21 reference */
		{
			1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0,
		},
		{	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
	},
	{ /* LD21 TV */
		{
			1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0,
		},
		{	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
	},
};
static int dio_ip_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			3, 3, 3, 2, 3, 2, 0, 2, 2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1, 2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2, 2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD20 TV */
		{
			3, 3, 3, 2, 3, 2, 0, 2, 2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1, 2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1, 2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2, 2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1, 2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD21 reference */
		{
			2, 2, 2, 2, 1, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1, 1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1,
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
	},
	{ /* LD21 TV */
		{
			2, 2, 2, 2, 1, 2, 2, 2, 2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1, 2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1, 1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1,
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		},
	},
};

/* umc */
static u32 umc_initctla[DRAM_FREQ_NR] = {0x71016D11};
static u32 umc_initctlb[DRAM_FREQ_NR] = {0x07E390AC};
static u32 umc_initctlc[DRAM_FREQ_NR] = {0x00FF00FF};
static u32 umc_drmmr0[DRAM_FREQ_NR] = {0x00000114};
static u32 umc_drmmr2[DRAM_FREQ_NR] = {0x000002a0};

static u32 umc_memconf0a[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000601, 0x00000801},	/* 1866 MHz */
};
static u32 umc_memconf0b[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000120, 0x00000130},	/* 1866 MHz */
};
static u32 umc_memconfch[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00033603, 0x00033803},	/* 1866 MHz */
};
static u32 umc_cmdctla[DRAM_FREQ_NR] = {0x060D0D20};
static u32 umc_cmdctlb[DRAM_FREQ_NR] = {0x2D211C08};
static u32 umc_cmdctlc[DRAM_FREQ_NR] = {0x00150C04};
static u32 umc_cmdctle[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x0049071D, 0x0078071D},	/* 1866 MHz */
};

static u32 umc_rdatactl_d0[DRAM_FREQ_NR] = {0x00000610};
static u32 umc_rdatactl_d1[DRAM_FREQ_NR] = {0x00000610};
static u32 umc_wdatactl_d0[DRAM_FREQ_NR] = {0x00000204};
static u32 umc_wdatactl_d1[DRAM_FREQ_NR] = {0x00000204};
static u32 umc_odtctl_d0[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_odtctl_d1[DRAM_FREQ_NR] = {0x02000002};
static u32 umc_dataset[DRAM_FREQ_NR] = {0x04000000};

static u32 umc_flowctla[DRAM_FREQ_NR] = {0x0081E01E};
static u32 umc_directbusctrla[DRAM_CH_NR] = {
	0x00000000, 0x00000001, 0x00000001
};

/* polling function for PHY Init Complete */
static void ddrphy_init_complete(void __iomem *dc_base)
{
	/* Wait for PHY Init Complete */
	while (!(readl(dc_base + UMC_DFISTCTLC) & BIT(0)))
		cpu_relax();
}

/* DDR PHY */
static void ddrphy_init(void __iomem *phy_base, void __iomem *dc_base,
			enum dram_freq freq, enum dram_board board, int ch)
{
	writel(0x0C001001, phy_base + PHY_UNIQUIFY_TSMC_IO_1);
	while (!(readl(phy_base + PHY_UNIQUIFY_TSMC_IO_1) & BIT(1)))
		cpu_relax();
	writel(0x0C001000, phy_base + PHY_UNIQUIFY_TSMC_IO_1);

	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_3);
	writel(0x00000000, phy_base + PHY_DLL_INCR_TRIM_1);
	writel(0x00000000, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000006, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x0000000c, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000012, phy_base + PHY_LANE_SEL);
	writel(0x00000005, phy_base + PHY_DLL_TRIM_1);
	writel(0x0000000a, phy_base + PHY_DLL_TRIM_3);
	writel(0x00000001, phy_base + PHY_SCL_WINDOW_TRIM);
	writel(0x00000000, phy_base + PHY_UNQ_ANALOG_DLL_1);
	writel(dio_phy_pad_ctrl[board][ch], phy_base + PHY_PAD_CTRL);
	writel(0x00000070, phy_base + PHY_VREF_TRAINING);
	writel(0x01000075, phy_base + PHY_SCL_CONFIG_1);
	writel(0x00000501, phy_base + PHY_SCL_CONFIG_2);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_3);
	writel(0x000261c0, phy_base + PHY_DYNAMIC_WRITE_BIT_LVL);
	writel(0x00000000, phy_base + PHY_SCL_CONFIG_4);
	writel(dio_scl_gate_timing[ch], phy_base + PHY_SCL_GATE_TIMING);
	writel(0x02a000a0, phy_base + PHY_WRLVL_DYN_ODT);
	writel(0x00840004, phy_base + PHY_WRLVL_ON_OFF);
	writel(0x0000020d, phy_base + PHY_DLL_ADRCTRL);
	writel(0x00000000, phy_base + PHY_LANE_SEL);
	writel(0x0000008d, phy_base + PHY_DLL_TRIM_CLK);
	writel(0xa800100d, phy_base + PHY_DLL_RECALIB);
	writel(0x00005076, phy_base + PHY_SCL_LATENCY);

	ddrphy_init_complete(dc_base);

	ddrphy_set_dll_adrctrl(dio_adrctrl_0[board][ch], 0, phy_base);
	ddrphy_set_dll_trim_clk(dio_dlltrimclk_0[board][ch], phy_base);
	ddrphy_set_dll_recalib(dio_dllrecalib_0[board][ch], 0x10, 0, 0xa,
			       phy_base);
}

static void ddrphy_shift_dq(u32 reg_mask, u32 reg_addr, int shift_val,
			    void __iomem *phy_base)
{
	u32 reg_val;
	int dq_val;

	reg_val = ddrphy_maskreadl(reg_mask, phy_base + reg_addr) & 0x7f;
	dq_val = reg_val & 0x3f;

	if ((reg_val & 0x40) == 0x00)
		dq_val = -1 * dq_val;

	/* value shift*/
	dq_val = dq_val + shift_val;

	if (dq_val >= 0)
		reg_val = 0x40 + (dq_val & 0x3f);
	else
		reg_val = ((-1 * dq_val) & 0x3f);

	ddrphy_maskwritel(reg_val, reg_mask, phy_base + reg_addr);
}

static void ddrphy_shift(void __iomem *phy_base, enum dram_board board, int ch)
{
	u32 dx, bit;

	/* set override = 1 */
	ddrphy_maskwritel(MSK_OVERRIDE, MSK_OVERRIDE,
			  phy_base + PHY_OP_DQ_DM_DQS_BITWISE_TRIM);
	ddrphy_maskwritel(MSK_OVERRIDE, MSK_OVERRIDE,
			  phy_base + PHY_IP_DQ_DQS_BITWISE_TRIM);

	for (dx = 0; dx < 4; dx++) {
		/* set byte to PHY_LANE_SEL.phy_lane_sel= dx * (PHY_BITLVL_DLY_WIDTH+1) */
		ddrphy_set_phy_lane_sel(dx * (PHY_BITLVL_DLY_WIDTH + 1),
					phy_base);

		for (bit = 0; bit < 8; bit++) {
			ddrphy_set_bit_sel(bit, phy_base);

			/* shift write reg value*/
			ddrphy_shift_dq(MSK_OP_DQ_DM_DQS_BITWISE_TRIM,
					PHY_OP_DQ_DM_DQS_BITWISE_TRIM,
					dio_op_dq_shift_val[board][ch][dx * 8 + bit],
					phy_base);
			/* shift read reg value */
			ddrphy_shift_dq(MSK_IP_DQ_DQS_BITWISE_TRIM,
					PHY_IP_DQ_DQS_BITWISE_TRIM,
					dio_ip_dq_shift_val[board][ch][dx * 8 + bit],
					phy_base);
		}

	}
	ddrphy_set_phy_lane_sel(0, phy_base);
	ddrphy_set_bit_sel(0, phy_base);
}

static int ddrphy_training(void __iomem *phy_base, enum dram_board board,
			   int ch)
{
	writel(0x0000000f, phy_base + PHY_WRLVL_AUTOINC_TRIM);
	writel(0x00010000, phy_base + PHY_DLL_TRIM_2);
	writel(0x50000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & BIT(28))
		cpu_relax();

	writel(0x00000000, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_0);
	writel(0xff00ff00, phy_base + PHY_SCL_DATA_1);
	writel(0xFBF8FFFF, phy_base + PHY_SCL_START_ADDR);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & BIT(28))
		cpu_relax();

	writel(0xFBF0FFFF, phy_base + PHY_SCL_START_ADDR);
	writel(0x30500000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & BIT(28))
		cpu_relax();

	writel(0x00000001, phy_base + PHY_DISABLE_GATING_FOR_SCL);
	writel(0x00000010, phy_base + PHY_SCL_MAIN_CLK_DELTA);
	writel(0x789b3de0, phy_base + PHY_SCL_DATA_0);
	writel(0xf10e4a56, phy_base + PHY_SCL_DATA_1);
	writel(0x11000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & BIT(28))
		cpu_relax();

	writel(0x34000000, phy_base + PHY_SCL_START);

	while (readl(phy_base + PHY_SCL_START) & BIT(28))
		cpu_relax();

	writel(0x00000003, phy_base + PHY_DISABLE_GATING_FOR_SCL);

	writel(0x000261c0, phy_base + PHY_DYNAMIC_WRITE_BIT_LVL);
	writel(0x00003270, phy_base + PHY_DYNAMIC_BIT_LVL);
	writel(0x011BD0C4, phy_base + PHY_DSCL_CNT);

	/* shift ip_dq, op_dq trim */
	ddrphy_shift(phy_base, board, ch);
	return 0;
}

static int umc_dc_init(void __iomem *dc_base, enum dram_freq freq,
		       unsigned long size, int ch)
{
	enum dram_size size_e;

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

	writel(umc_initctla[freq], dc_base + UMC_INITCTLA);
	writel(umc_initctlb[freq], dc_base + UMC_INITCTLB);
	writel(umc_initctlc[freq], dc_base + UMC_INITCTLC);

	writel(umc_drmmr0[freq], dc_base + UMC_DRMMR0);
	writel(0x00000004, dc_base + UMC_DRMMR1);
	writel(umc_drmmr2[freq], dc_base + UMC_DRMMR2);
	writel(0x00000000, dc_base + UMC_DRMMR3);

	writel(umc_memconf0a[freq][size_e], dc_base + UMC_MEMCONF0A);
	writel(umc_memconf0b[freq][size_e], dc_base + UMC_MEMCONF0B);
	writel(umc_memconfch[freq][size_e], dc_base + UMC_MEMCONFCH);
	writel(0x00000008, dc_base + UMC_MEMMAPSET);

	writel(umc_cmdctla[freq], dc_base + UMC_CMDCTLA);
	writel(umc_cmdctlb[freq], dc_base + UMC_CMDCTLB);
	writel(umc_cmdctlc[freq], dc_base + UMC_CMDCTLC);
	writel(umc_cmdctle[freq][size_e], dc_base + UMC_CMDCTLE);

	writel(umc_rdatactl_d0[freq], dc_base + UMC_RDATACTL_D0);
	writel(umc_rdatactl_d1[freq], dc_base + UMC_RDATACTL_D1);

	writel(umc_wdatactl_d0[freq], dc_base + UMC_WDATACTL_D0);
	writel(umc_wdatactl_d1[freq], dc_base + UMC_WDATACTL_D1);
	writel(umc_odtctl_d0[freq], dc_base + UMC_ODTCTL_D0);
	writel(umc_odtctl_d1[freq], dc_base + UMC_ODTCTL_D1);
	writel(umc_dataset[freq], dc_base + UMC_DATASET);

	writel(0x00400020, dc_base + UMC_DCCGCTL);
	writel(0x00000003, dc_base + UMC_ACSSETA);
	writel(0x00000103, dc_base + UMC_FLOWCTLG);
	writel(0x00010200, dc_base + UMC_ACSSETB);

	writel(umc_flowctla[freq], dc_base + UMC_FLOWCTLA);
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
		       enum dram_freq freq, enum dram_board board,
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

	ddrphy_init(phy_base, dc_base, freq, board, ch);

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
	enum dram_freq freq;
	enum dram_board board;
	int ch, ret;

	switch (bd->dram_freq) {
	case 1866:
		freq = DRAM_FREQ_1866M;
		break;
	default:
		pr_err("unsupported DRAM frequency %d MHz\n", bd->dram_freq);
		return -EINVAL;
	}

	switch (UNIPHIER_BD_BOARD_GET_TYPE(bd->flags)) {
	case UNIPHIER_BD_BOARD_LD20_REF:
		board = DRAM_BOARD_LD20_REF;
		break;
	case UNIPHIER_BD_BOARD_LD20_GLOBAL:
		board = DRAM_BOARD_LD20_GLOBAL;
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

		ret = umc_ch_init(umc_ch_base, phy_ch_base, freq, board,
				  size / (width / 16), ch);
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
